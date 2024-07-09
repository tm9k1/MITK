/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBlosc2IO.h"
#include <mitkBlosc2IOMimeTypes.h>

#include <mitkFileSystem.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

#include <fstream>
#include <thread>

#include <b2nd.h>

namespace Blosc2
{
  // RAII
  struct LibraryEnvironment
  {
    LibraryEnvironment()
    {
      blosc2_init();
    }

    ~LibraryEnvironment()
    {
      blosc2_destroy();
    }
  };

  void ThrowOnError(int rc)
  {
    if (rc < BLOSC2_ERROR_SUCCESS)
      mitkThrow() << print_error(rc);
  }

  void SuggestNumberOfThreads(unsigned int numThreads)
  {
    // Note: Can be overridden by the BLOSC_NTHREADS environment variable.
    blosc2_set_nthreads(static_cast<int16_t>(std::min(numThreads, std::max(1U, std::thread::hardware_concurrency()))));
  }
}

namespace
{
  mitk::PixelType CreatePixelType(std::string typestr)
  {
    // typestr: https://numpy.org/doc/stable/reference/arrays.interface.html#python-side

    if (typestr.size() < 3)
      mitkThrow() << "Unexpected type string: \"" << typestr << "\"!";

    if (typestr[0] == '>')
      mitkThrow() << "Big-endian data types are not supported as pixel types in MITK!";

    char type = typestr[1];
    int size;

    try
    {
      size = std::stoi(typestr.substr(2));
    }
    catch (const std::exception&)
    {
      mitkThrow() << "Unexpected type string (expected an integer at third position): \"" << typestr << "\"!";
    }

    switch (type)
    {
      case 'i': // integer
        switch (size)
        {
          case 1:
            return mitk::MakeScalarPixelType<char>();
          case 2:
            return mitk::MakeScalarPixelType<short>();
          case 4:
            return mitk::MakeScalarPixelType<int>();
          default:
            mitkThrow() << "Unsupported signed integer size: " << size << '!';
        }

      case 'u': // unsigned integer
        switch (size)
        {
          case 1:
            return mitk::MakeScalarPixelType<unsigned char>();
          case 2:
            return mitk::MakeScalarPixelType<unsigned short>();
          case 4:
            return mitk::MakeScalarPixelType<unsigned int>();
          default:
            mitkThrow() << "Unsupported unsigned integer size: " << size << '!';
        }

      case 'f': // floating point
        switch (size)
        {
          case 4:
            return mitk::MakeScalarPixelType<float>();
          case 8:
            return mitk::MakeScalarPixelType<double>();
          default:
            mitkThrow() << "Unsuppoted floating point size: " << size << '!';
        }

      default:
        mitkThrow() << "Unsupported or unknown type character code: \"" << type << "\"!";
    }
  }

  char* CreateDType(const mitk::PixelType& pixelType)
  {
    if (pixelType.GetNumberOfComponents() != 1)
      mitkThrow() << "Multi-component pixel types are not supported!";

    switch (pixelType.GetComponentType())
    {
      case itk::IOComponentEnum::CHAR:
        return "|i1";
      case itk::IOComponentEnum::SHORT:
        return "<i2";
      case itk::IOComponentEnum::INT:
        return "<i4";

      case itk::IOComponentEnum::UCHAR:
        return "|u1";
      case itk::IOComponentEnum::USHORT:
        return "<u2";
      case itk::IOComponentEnum::UINT:
        return "<u4";

      case itk::IOComponentEnum::FLOAT:
        return "<f4";
      case itk::IOComponentEnum::DOUBLE:
        return "<f8";

      default:
        mitkThrow() << "Unsupported pixel type: \"" << pixelType.GetComponentTypeAsString() << "\"!";
    }
  }

  std::vector<unsigned int> GetDimensionsFromShape(const int64_t* shape, int8_t ndim)
  {
    std::vector<unsigned int> dimensions;
    dimensions.reserve(ndim);

    for (int i = ndim - 1; i >= 0; --i)
      dimensions.push_back(static_cast<unsigned int>(shape[i]));

    return dimensions;
  }

  int64_t CalculatePixelBufferSize(const mitk::PixelType& pixelType, const std::vector<unsigned int>& dimensions)
  {
    int64_t size = pixelType.GetSize();

    for (auto dimension : dimensions)
      size *= dimension;

    return size;
  }
}

mitk::Blosc2IO::Blosc2IO()
  : AbstractFileIO(Image::GetStaticNameOfClass(), MitkBlosc2IOMimeTypes::BLOSC2_MIMETYPE(), "Blosc2 NDim")
{
  this->RegisterService();
}

mitk::IFileIO::ConfidenceLevel mitk::Blosc2IO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
    return Unsupported;

  Blosc2::LibraryEnvironment blosc2;

  auto schunk = blosc2_schunk_open(this->GetLocalFileName().c_str());

  if (schunk == nullptr)
    return Unsupported;

  bool hasBlosc2NDimMetalayer = blosc2_meta_exists(schunk, "b2nd") != BLOSC2_ERROR_NOT_FOUND;

  blosc2_schunk_free(schunk);

  return hasBlosc2NDimMetalayer
    ? Supported
    : Unsupported;
}

mitk::IFileIO::ConfidenceLevel mitk::Blosc2IO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
    return Unsupported;

  auto image = dynamic_cast<const Image*>(this->GetInput());

  if (image == nullptr)
    return Unsupported;

  if (std::string(image->GetNameOfClass()) != "Image")
    return Unsupported;

  if (!image->IsInitialized())
    return Unsupported;

  if (image->GetPixelType().GetNumberOfComponents() != 1)
    return Unsupported;

  return Supported;
}

std::vector<mitk::BaseData::Pointer> mitk::Blosc2IO::DoRead()
{
  Blosc2::LibraryEnvironment blosc2;
  Blosc2::SuggestNumberOfThreads(4);

  b2nd_array_t* array = nullptr;
  Blosc2::ThrowOnError(b2nd_open(this->GetLocalFileName().c_str(), &array));

  try
  {
    Blosc2::ThrowOnError(b2nd_print_meta(array));

    if (array->dtype_format != DTYPE_NUMPY_FORMAT)
      mitkThrow() << "Unknown data type format (expected DTYPE_NUMPY_FORMAT [0]): " << array->dtype_format << "!";

    auto pixelType = CreatePixelType(array->dtype);
    auto dimensions = GetDimensionsFromShape(array->shape, array->ndim);
    auto bufferSize = CalculatePixelBufferSize(pixelType, dimensions);

    auto image = Image::New();
    image->Initialize(pixelType, dimensions.size(), dimensions.data());

    {
      ImageWriteAccessor writeAccessor(image);
      Blosc2::ThrowOnError(b2nd_to_cbuffer(array, writeAccessor.GetData(), bufferSize));
    }

    b2nd_free(array);
    return { image };
  }
  catch (Exception& e)
  {
    b2nd_free(array);
    mitkReThrow(e);
  }
}

void mitk::Blosc2IO::Write()
{
  auto image = dynamic_cast<const Image*>(this->GetInput());
  auto ndim = static_cast<int8_t>(image->GetDimension());

  std::vector<int64_t> shape;
  shape.reserve(ndim);

  std::vector<int32_t> chunkshape;
  chunkshape.reserve(ndim);

  std::vector<int32_t> blockshape;
  blockshape.reserve(ndim);

  for (int i = ndim - 1; i >= 0; --i)
  {
    auto dim = static_cast<int32_t>(image->GetDimension(i));
    shape.push_back(dim);

    chunkshape.push_back(std::min(dim, 64));
    blockshape.push_back(std::min(dim, 16));
  }

  if (image->GetTimeGeometry()->CountTimeSteps() > 1)
  {
    chunkshape[0] = 1;
    blockshape[0] = 1;
  }

  auto typesize = static_cast<int32_t>(image->GetPixelType().GetSize());

  int64_t nelem = 1;

  for (auto dim : shape)
    nelem *= dim;

  int64_t size = nelem * typesize;

  blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
  cparams.compcode = BLOSC_ZSTD;
  cparams.typesize = typesize;
  cparams.nthreads = static_cast<int16_t>(std::max(1U, std::thread::hardware_concurrency()));

  blosc2_storage b2_storage = BLOSC2_STORAGE_DEFAULTS;
  b2_storage.contiguous = true;
  b2_storage.cparams = &cparams;

  Blosc2::LibraryEnvironment blosc2;
  Blosc2::SuggestNumberOfThreads(cparams.nthreads);

  auto ctx = b2nd_create_ctx(
    &b2_storage,
    ndim,
    shape.data(),
    chunkshape.data(),
    blockshape.data(),
    CreateDType(image->GetPixelType()),
    DTYPE_NUMPY_FORMAT,
    nullptr,
    0);

  b2nd_array_t* array = nullptr;

  {
    ImageReadAccessor readAccessor(image);
    b2nd_from_cbuffer(ctx, &array, readAccessor.GetData(), size);
  }

  b2nd_print_meta(array);
  b2nd_save(array, const_cast<char*>(this->GetOutputLocation().c_str()));

  b2nd_free(array);
  b2nd_free_ctx(ctx);
}

mitk::Blosc2IO* mitk::Blosc2IO::IOClone() const
{
  return new Blosc2IO(*this);
}
