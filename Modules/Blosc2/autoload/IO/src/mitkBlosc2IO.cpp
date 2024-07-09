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
  : AbstractFileIO(Image::GetStaticNameOfClass(), MitkBlosc2IOMimeTypes::BLOSC2_MIMETYPE(), "Blosc2")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::Blosc2IO::DoRead()
{
  fs::path filename = this->GetInputLocation();

  if (filename.empty())
    mitkThrow() << "Filename of Blosc2 NDim file not set!";

  if (!fs::exists(filename))
    mitkThrow() << "File " << filename << "\" does not exist!";

  Blosc2::LibraryEnvironment blosc2;
  Blosc2::SuggestNumberOfThreads(4);

  b2nd_array_t* array = nullptr;
  Blosc2::ThrowOnError(b2nd_open(this->GetInputLocation().c_str(), &array));

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

    Blosc2::ThrowOnError(b2nd_free(array));

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
  auto input = dynamic_cast<const Image*>(this->GetInput());

  if (input == nullptr)
    mitkThrow() << "Invalid input for writing!";

  auto* stream = this->GetOutputStream();
  std::ofstream fileStream;

  if (stream == nullptr)
  {
    auto filename = this->GetOutputLocation();

    if (filename.empty())
      mitkThrow() << "Neither an output stream nor an output file or directory name was specified!";

    fileStream.open(filename); // TODO: Can be directory

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for writing!";

    stream = &fileStream;
  }

  if (!stream->good())
    mitkThrow() << "Stream for writing is not good!";

  // TODO
}

mitk::Blosc2IO* mitk::Blosc2IO::IOClone() const
{
  return new Blosc2IO(*this);
}
