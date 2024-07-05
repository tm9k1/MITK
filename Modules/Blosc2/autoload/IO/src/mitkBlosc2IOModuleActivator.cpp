/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBlosc2IOModuleActivator.h"

#include <mitkBlosc2IOMimeTypes.h>
#include "mitkBlosc2IO.h"

#include <usModuleContext.h>

US_EXPORT_MODULE_ACTIVATOR(mitk::Blosc2IOModuleActivator)

void mitk::Blosc2IOModuleActivator::Load(us::ModuleContext* context)
{
  auto mimeTypes = MitkBlosc2IOMimeTypes::Get();

  us::ServiceProperties props;
  props[us::ServiceConstants::SERVICE_RANKING()] = 10;

  for (auto mimeType : mimeTypes)
    context->RegisterService(mimeType, props);

  m_FileIOs.push_back(std::make_shared<Blosc2IO>());
}

void mitk::Blosc2IOModuleActivator::Unload(us::ModuleContext*)
{
}
