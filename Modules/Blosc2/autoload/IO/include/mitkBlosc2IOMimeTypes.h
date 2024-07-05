/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBlosc2IOMimeTypes_h
#define mitkBlosc2IOMimeTypes_h

#include <mitkCustomMimeType.h>
#include <MitkBlosc2IOExports.h>

namespace mitk
{
  namespace MitkBlosc2IOMimeTypes
  {
    class MITKBLOSC2IO_EXPORT MitkBlosc2MimeType : public CustomMimeType
    {
    public:
      MitkBlosc2MimeType();

      bool AppliesTo(const std::string& path) const override;
      MitkBlosc2MimeType* Clone() const override;
    };

    MITKBLOSC2IO_EXPORT MitkBlosc2MimeType BLOSC2_MIMETYPE();
    MITKBLOSC2IO_EXPORT std::string BLOSC2_MIMETYPE_NAME();
    MITKBLOSC2IO_EXPORT std::vector<CustomMimeType*> Get();
  }
}

#endif
