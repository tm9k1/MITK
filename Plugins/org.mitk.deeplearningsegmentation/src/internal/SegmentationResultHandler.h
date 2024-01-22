/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef SegmentationResultHandler_h
#define SegmentationResultHandler_h

#include <QObject>
#include <DeepLearningSegmentationTool.h>

/**
* @class SegmentationResultHandler
* @brief Class which sets the result in the UI after a deep learning method is finished
*/
class SegmentationResultHandler : public QObject
{
  Q_OBJECT
public:
  SegmentationResultHandler();
  ~SegmentationResultHandler();
public slots:
  /**
   * @brief display the result of the segmentation if the segmentation process was successful
   *
   * @param resultSegmentation the resulting segmentation from the segmentation process to display
   * @param segTool the Segmentation Tool for running the segmentation
   */
  void SetResult(mitk::LabelSetImage::Pointer resultSegmentation, mitk::DeepLearningSegmentationTool *segTool);
  /**
   * @brief display the result of the segmentation if the segmentation process was successful
   *
   * @param resultSegmentation the resulting segmentation from the segmentation process to display
   * @param segTool the Segmentation Tool for running the segmentation
   */
  void SetMultilabelResult(std::vector<mitk::LabelSetImage::Pointer> resultSegmentation, mitk::DeepLearningSegmentationTool *segTool);
  /**
   * @brief display a warning for the user if segmentation process failed
   */
  void SegmentationProcessFailed();

signals:

};

#endif
