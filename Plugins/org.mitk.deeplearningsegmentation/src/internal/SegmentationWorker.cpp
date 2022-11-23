/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include"SegmentationWorker.h"

SegmentationWorker::SegmentationWorker() 
{
}
void SegmentationWorker::DoWork(mitk::DeepLearningSegmentationTool* segTool,
                                SegmentationResultHandler *resultSetter,
                                QString networkPath)
{
  //connect signals/slots with the result setter which sets the result in the main thread afterwards
  connect(this, &SegmentationWorker::Finished, resultSetter, &SegmentationResultHandler::SetResult);
  connect(this, &SegmentationWorker::FinishedMultilabel, resultSetter, &SegmentationResultHandler::SetMultilabelResult);
  connect(this, &SegmentationWorker::Failed, resultSetter, &SegmentationResultHandler::SegmentationProcessFailed);

  try
  {
    bool multilabel = segTool->IsMultilabelSegmentation();
      //execute segmentation with segmentation tool
    if (!multilabel)
    {
      mitk::LabelSetImage::Pointer result = segTool->DoSegmentation(networkPath.toStdString());
      MITK_INFO << "Back in Worker";
      emit Finished(result, segTool);
    }
    else
    {
      std::vector<mitk::LabelSetImage::Pointer> result = segTool->DoMultilabelSegmentation(networkPath.toStdString());
      MITK_INFO << "Back in Worker";
      emit FinishedMultilabel(result, segTool);
    }
    //disconnect from result setter. Otherwise, the result is set twice after second execution,
    //three times after third execution,...
    disconnect(this, &SegmentationWorker::Finished, resultSetter, &SegmentationResultHandler::SetResult);
    disconnect(this, &SegmentationWorker::FinishedMultilabel, resultSetter, &SegmentationResultHandler::SetMultilabelResult);
    disconnect(this, &SegmentationWorker::Failed, resultSetter, &SegmentationResultHandler::SegmentationProcessFailed);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    emit Failed();
    // disconnect from result setter. Otherwise, the result is set twice after second execution, 
    // three times after third execution,...
    disconnect(this, &SegmentationWorker::Finished, resultSetter, &SegmentationResultHandler::SetResult);
    disconnect(this, &SegmentationWorker::FinishedMultilabel, resultSetter, &SegmentationResultHandler::SetMultilabelResult);
    disconnect(this, &SegmentationWorker::Failed, resultSetter, &SegmentationResultHandler::SegmentationProcessFailed);
  }
}

void SegmentationWorker::WaitForSegmentationToFinish(mitk::DeepLearningSegmentationTool *segTool) 
{
  while (segTool->IsSegmentationRunning())
  {
      //Wait
  }
  emit PreviousSegmentationFinished();
}
