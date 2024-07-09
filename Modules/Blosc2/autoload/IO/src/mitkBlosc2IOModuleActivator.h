/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>

#include <memory>

namespace mitk
{
  class AbstractFileIO;

  class Blosc2IOModuleActivator : public us::ModuleActivator
  {
  public:
    Blosc2IOModuleActivator();
    ~Blosc2IOModuleActivator() override = default;

    Blosc2IOModuleActivator(const Blosc2IOModuleActivator&) = delete;
    Blosc2IOModuleActivator& operator=(const Blosc2IOModuleActivator&) = delete;

    void Load(us::ModuleContext* context) override;
    void Unload(us::ModuleContext* context) override;

  private:
    std::unique_ptr<AbstractFileIO> m_FileIO;
  };
}
