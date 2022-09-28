/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkMonaiLabelTool.h"

// us
#include "mitkIOUtil.h"
#include "mitkRESTUtil.h"
#include <nlohmann/json.hpp>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MonaiLabelTool, "MonaiLabel");
}

mitk::MonaiLabelTool::MonaiLabelTool()
{
  MITK_INFO << "MonaiLabelTool constrc";
  InitializeRESTManager();
  GetOverallInfo("https://httpbin.org/get");
}

void mitk::MonaiLabelTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferMode(LabelTransferMode::AllLabels);
}

const char **mitk::MonaiLabelTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::MonaiLabelTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Otsu_48x48.png");
  return resource;
}

const char *mitk::MonaiLabelTool::GetName() const
{
  return "MonaiLabel";
}

void mitk::MonaiLabelTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                           const Image * /*oldSegAtTimeStep*/,
                                           LabelSetImage *previewImage,
                                           TimeStepType timeStep)
{
  std::string outputImagePath = "Z:/dataset/Task05_Prostate/labelsTr/prostate_00.nii.gz";
  try
  {
    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    previewImage->InitializeByLabeledImage(outputImage);
    previewImage->SetGeometry(inputAtTimeStep->GetGeometry());
  }
  catch (const mitk::Exception &e)
  {
    /*
    Can't throw mitk exception to the caller. Refer: T28691
    */
    MITK_ERROR << e.GetDescription();
    return;
  }
}

void mitk::MonaiLabelTool::InitializeRESTManager() // Don't call from constructor --ashis
{
  auto serviceRef = us::GetModuleContext()->GetServiceReference<mitk::IRESTManager>();
  if (serviceRef)
  {
    m_RESTManager = us::GetModuleContext()->GetService(serviceRef);
  }
}

void mitk::MonaiLabelTool::GetOverallInfo(std::string url)
{
  if (m_RESTManager != nullptr)
  {
    std::string jsonString;
    bool fetched = false;
    m_RESTManager->SendRequest(mitk::RESTUtil::convertToTString(url))
      .then(
        [=, &fetched](pplx::task<web::json::value> resultTask) /*It is important to use task-based continuation*/
        {
          try
          {
            // Get the result of the request
            // This will throw an exception if the ascendent task threw an exception (e.g. invalid URI)
            web::json::value result = resultTask.get();
            m_Parameters = DataMapper(result);
            fetched = true;
          }
          catch (const mitk::Exception &exception)
          {
            // Exceptions from ascendent tasks are catched here
            MITK_ERROR << exception.what();
            return;
          }
        });
    
    while (!fetched); /* wait until fetched*/
  }
  
}

std::unique_ptr<mitk::DataObject> mitk::MonaiLabelTool::DataMapper(web::json::value &result)
{
  auto object = std::make_unique<DataObject>();
  utility::string_t stringT = result.to_string();
  std::string jsonString(stringT.begin(), stringT.end());
  auto jsonObj = nlohmann::json::parse(jsonString);
  if (jsonObj.is_discarded() || !jsonObj.is_object())
  {
    MITK_ERROR << "Could not parse \"" << jsonString << "\" as JSON object!";
  }
  MITK_INFO << "ashis inside mapper " << jsonObj["origin"].get<std::string>(); //remove
  object->origin = jsonObj["origin"].get<std::string>();
  return object;
}