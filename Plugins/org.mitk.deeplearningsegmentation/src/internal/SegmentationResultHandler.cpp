/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include"SegmentationResultHandler.h"
#include <QMessageBox>

SegmentationResultHandler::SegmentationResultHandler() {
}

SegmentationResultHandler::~SegmentationResultHandler(){
}

void SegmentationResultHandler::SetResult(mitk::LabelSetImage::Pointer resultSegmentation,
                                          mitk::DeepLearningSegmentationTool *segTool)
{
  try
  {
    //create new data node with the segmentation output as data
    mitk::DataNode::Pointer outputNode = mitk::DataNode::New();
    outputNode->SetName(segTool->GetName());
    outputNode->SetData(resultSegmentation);
    //add data node to data storage and update GUI
    segTool->GetDataStorage()->Add(outputNode, segTool->GetReferenceData());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  catch (const mitk::Exception &e)
  {
    MITK_INFO << e.GetDescription();
  }
}

void SegmentationResultHandler::SetMultilabelResult(std::vector<mitk::LabelSetImage::Pointer> resultSegmentation,
                                                    mitk::DeepLearningSegmentationTool *segTool)
{
  try
  { int resultSegmentationSize= static_cast<int>(resultSegmentation.size());
    for (int i = 0; i<resultSegmentationSize; i++)
    {
      // create new data node with the segmentation output as data
      mitk::DataNode::Pointer outputNode = mitk::DataNode::New();
      std::string name = segTool->GetName()+std::to_string(i);
      outputNode->SetName(name);
      outputNode->SetData(resultSegmentation[i]);
      // add data node to data storage and update GUI
      segTool->GetDataStorage()->Add(outputNode, segTool->GetReferenceData());
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  catch (const mitk::Exception &e)
  {
    MITK_INFO << e.GetDescription();
  }
}

void SegmentationResultHandler::SegmentationProcessFailed()
{
    QMessageBox::warning(nullptr,
                         "Error in segmentation",
                         "There was an error in the segmentation process. No resulting segmentation can be loaded.");
}
