/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnUnetTool.h"

#include "mitkIOUtil.h"
#include "mitkProcessExecutor.h"
#include <itksys/SystemTools.hxx>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnUNetTool, "nnUNet tool");
}

mitk::MonaiLabelTool::MonaiLabelTool()
{
  this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
}

void mitk::MonaiLabelTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferMode(LabelTransferMode::AllLabels);
}


us::ModuleResource mitk::MonaiLabelTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI_48x48.png");
  return resource;
}

const char **mitk::MonaiLabelTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::MonaiLabelTool::GetName() const
{
  return "MONAI";
}

mitk::DataStorage *mitk::MonaiLabelTool::GetDataStorage()
{
  return this->GetToolManager()->GetDataStorage();
}

mitk::DataNode *mitk::MonaiLabelTool::GetRefNode()
{
  return this->GetToolManager()->GetReferenceData(0);
}



void mitk::nnUNetTool::DoUpdatePreview(const Image* inputAtTimeStep, const Image* /*oldSegAtTimeStep*/, LabelSetImage* previewImage, TimeStepType /*timeStep*/)
{
  std::string inDir, outDir, inputImagePath, outputImagePath, scriptPath;

  ProcessExecutor::Pointer spExec = ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onPythonProcessEvent);
  spExec->AddObserver(ExternalProcessOutputEvent(), spCommand);
  ProcessExecutor::ArgumentListType args;

  inDir = IOUtil::CreateTemporaryDirectory("nnunet-in-XXXXXX", this->GetMitkTempDir());
  std::ofstream tmpStream;
  inputImagePath = IOUtil::CreateTemporaryFile(tmpStream, m_TEMPLATE_FILENAME, inDir + IOUtil::GetDirectorySeparator());
  tmpStream.close();
  std::size_t found = inputImagePath.find_last_of(IOUtil::GetDirectorySeparator());
  std::string fileName = inputImagePath.substr(found + 1);
  std::string token = fileName.substr(0, fileName.find("_"));

  try
  {
    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    previewImage->InitializeByLabeledImage(outputImage);
    previewImage->SetGeometry(inputAtTimeStep->GetGeometry());
    m_InputBuffer = inputAtTimeStep;
    m_OutputBuffer = mitk::LabelSetImage::New();
    m_OutputBuffer->InitializeByLabeledImage(outputImage);
    m_OutputBuffer->SetGeometry(inputAtTimeStep->GetGeometry());
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
