/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBlosc2IO_h
#define mitkBlosc2IO_h

#include <mitkAbstractFileIO.h>

namespace mitk
{
  class Blosc2IO : public AbstractFileIO
  {
  public:
    Blosc2IO();

    ConfidenceLevel GetReaderConfidenceLevel() const override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

    using AbstractFileReader::Read;
    void Write() override;

  protected:
    std::vector<BaseData::Pointer> DoRead() override;

  private:
    Blosc2IO* IOClone() const override;
  };
}

#endif
