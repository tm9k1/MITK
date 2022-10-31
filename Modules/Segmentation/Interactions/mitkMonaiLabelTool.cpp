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
#include <map>
#include <nlohmann/json.hpp>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>
#include <vector>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MonaiLabelTool, "MonaiLabel");
}

mitk::MonaiLabelTool::MonaiLabelTool()
{
  InitializeRESTManager();
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
            MITK_ERROR << exception.what();
            return;
          }
        });

    while (!fetched); /* wait until fetched*/ // add timeout
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
  MITK_INFO << jsonString;
  MITK_INFO << "ashis inside mapper " << jsonObj["name"].get<std::string>(); // remove
  object->name = jsonObj["name"].get<std::string>();
  object->description = jsonObj["description"].get<std::string>();
  object->labels = jsonObj["labels"].get<std::vector<std::string>>();

  auto modelJsonMap = jsonObj["models"].get<std::map<std::string, nlohmann::json>>();
  for (const auto &[_name, _jsonObj] : modelJsonMap)
  {
    if (_jsonObj.is_discarded() || !_jsonObj.is_object())
    {
      MITK_ERROR << "Could not parse JSON object.";
    }
    ModelObject modelInfo;
    modelInfo.name = _name;
    try
    {
      auto labels = _jsonObj["labels"].get<std::unordered_map<std::string, int>>();
      modelInfo.labels = labels;
    }
    catch (const std::exception &)
    {
      auto labels = _jsonObj["labels"].get<std::vector<std::string>>();
      for (const auto &label : labels)
      {
        modelInfo.labels[label] = -1; // Hardcode -1 as label id
      }
    }
    modelInfo.type = _jsonObj["type"].get<std::string>();
    modelInfo.dimension = _jsonObj["dimension"].get<int>();
    modelInfo.description = _jsonObj["description"].get<std::string>();

    object->models.push_back(modelInfo);
  }
  return object;
}


std::vector<mitk::ModelObject> mitk::MonaiLabelTool::GetAutoSegmentationModels() 
{
  std::vector<mitk::ModelObject> autoModels;
  for (mitk::ModelObject& model: m_Parameters->models)
  {
    if (m_AUTO_SEG_TYPE_NAME.find(model.type) != m_AUTO_SEG_TYPE_NAME.end())
    {
      autoModels.push_back(model);
    }
  }
  return autoModels;
}

std::vector<mitk::ModelObject> mitk::MonaiLabelTool::GetInteractiveSegmentationModels()
{
  std::vector<mitk::ModelObject> interactiveModels;
  for (mitk::ModelObject &model : m_Parameters->models)
  {
    if (m_INTERACTIVE_SEG_TYPE_NAME.find(model.type) != m_INTERACTIVE_SEG_TYPE_NAME.end())
    {
      interactiveModels.push_back(model);
    }
  }
  return interactiveModels;
}

std::vector<mitk::ModelObject> mitk::MonaiLabelTool::GetScribbleSegmentationModels()
{
  std::vector<mitk::ModelObject> scribbleModels;
  for (mitk::ModelObject &model : m_Parameters->models)
  {
    if (m_SCRIBBLE_SEG_TYPE_NAME.find(model.type) != m_SCRIBBLE_SEG_TYPE_NAME.end())
    {
      scribbleModels.push_back(model);
    }
  }
  return scribbleModels;
}