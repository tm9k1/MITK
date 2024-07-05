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
#include <mitkImage.h>

#include <fstream>

#include <blosc2.h>

mitk::Blosc2IO::Blosc2IO()
  : AbstractFileIO(Image::GetStaticNameOfClass(), MitkBlosc2IOMimeTypes::BLOSC2_MIMETYPE(), "Blosc2")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::Blosc2IO::DoRead()
{
  auto* stream = this->GetInputStream();
  std::ifstream fileStream;

  if (nullptr == stream)
  {
    auto filename = this->GetInputLocation();

    if (filename.empty() || !fs::exists(filename))
      mitkThrow() << "Invalid or nonexistent file or directory name: \"" << filename << "\"!";

    fileStream.open(filename); // TODO: Can be directory

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for reading!";

    stream = &fileStream;
  }

  auto result = Image::New();

  // TODO

  return { result };
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
