/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBlosc2IOMimeTypes.h>
#include <mitkIOMimeTypes.h>

#include <mitkFileSystem.h>

mitk::MitkBlosc2IOMimeTypes::MitkBlosc2MimeType::MitkBlosc2MimeType()
  : CustomMimeType(BLOSC2_MIMETYPE_NAME())
{
  this->AddExtension("b2frame");
  this->AddExtension("b2f");
  this->AddExtension("b2nd");
  this->AddExtension("chunk");

  this->SetCategory("Blosc2");
  this->SetComment("Blosc2");
}

bool mitk::MitkBlosc2IOMimeTypes::MitkBlosc2MimeType::AppliesTo(const std::string& path) const
{
  bool result = CustomMimeType::AppliesTo(path);

  if (!fs::exists(path)) // T18572
    return result;

  // TODO

  return false; // TODO
}

mitk::MitkBlosc2IOMimeTypes::MitkBlosc2MimeType* mitk::MitkBlosc2IOMimeTypes::MitkBlosc2MimeType::Clone() const
{
  return new MitkBlosc2MimeType(*this);
}

mitk::MitkBlosc2IOMimeTypes::MitkBlosc2MimeType mitk::MitkBlosc2IOMimeTypes::BLOSC2_MIMETYPE()
{
  return MitkBlosc2MimeType();
}

std::string mitk::MitkBlosc2IOMimeTypes::BLOSC2_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".image.blosc2";
}

std::vector<mitk::CustomMimeType*> mitk::MitkBlosc2IOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;
  mimeTypes.push_back(BLOSC2_MIMETYPE().Clone());
  return mimeTypes;
}
