/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include"PythonWorker.h"
#include<mitkIPythonService.h>
#include <usModule.h>
#include <usServiceTracker.h>
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>
#include<mitkIPythonService.h>
#include <usModuleInitialization.h>
#include <mitkStandardFileLocations.h>

void PythonWorker::DoWork() 
{
  MITK_INFO << "Here";
  auto *context = us::GetModuleContext();
  std::string filter = "(Name=PythonService)";
  mitk::IPythonService::ForceLoadModule();
  auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

  if (!m_PythonServiceRefs.empty())
  {
    mitk::IPythonService *m_PythonService =
      dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

    try
    {
      std::string pythonFileName = "hello.py";
      std::string fileName = mitk::StandardFileLocations::GetInstance()->FindFile(
        pythonFileName.c_str(), "Examples/Plugins/org.mitk.example.gui.pythonexample/resources");
      m_PythonService->ExecuteScript(fileName);
      emit Finished();
    }
    catch (const mitk::Exception &e)
    {
      MITK_ERROR << e.GetDescription();
      emit Failed();
    }
  }
  else
  {
    MITK_ERROR << "No Service reference found";
  }
}
