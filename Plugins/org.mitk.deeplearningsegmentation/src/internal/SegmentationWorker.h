/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef SegmentationWorker_h
#define SegmentationWorker_h

#include <QObject>
#include<DeepLearningSegmentationTool.h>
#include"SegmentationResultHandler.h"

Q_DECLARE_METATYPE(mitk::DeepLearningSegmentationTool*)
Q_DECLARE_METATYPE(mitk::LabelSetImage::Pointer)
Q_DECLARE_METATYPE(std::vector<mitk::LabelSetImage::Pointer>)

/**
 * @class SegmentationWorker
 * @brief Class to execute some functions (mainly segmentation) from the Segmentation Plugin in a seperate thread
 */
class SegmentationWorker : public QObject
{
  Q_OBJECT
public:
  SegmentationWorker();
public slots:
  /**
   * @brief execute segmentation with the correct segmentation tool
   *
   * @param segTool the Segmentation Tool for running the segmentation
   * @param resultSetter the SegmentationResultHandler which sets the result in the GUI after the segmentation
   * @param networkPath the path to the trained network which is needed for the segmentation
   */
  void DoWork(mitk::DeepLearningSegmentationTool* segTool,
              SegmentationResultHandler *resultSetter,
              QString networkPath);
  /**
   * @brief if a segmentation is executed when the tool is started, 
   *        wait for the segmentation to finish and emit a signal (PreviouesSegmentationFinished) afterwards.
   *
   * @param tool the Segmentation Tool to check, if the segmentation is still running.
   */
  void WaitForSegmentationToFinish(mitk::DeepLearningSegmentationTool *segTool);

signals:
  /**
   * @brief the signal emitted when a segmentation process finished successful
   *
   * @param result the resulting segmentation
   * @param segTool the Segmentation Tool for running the segmentation
   */
  void Finished(mitk::LabelSetImage::Pointer result, mitk::DeepLearningSegmentationTool* segTool);
  /**
   * @brief the signal emitted when a multilabel segmentation process finished successful
   *
   * @param result the resulting segmentation
   * @param segTool the Segmentation Tool for running the segmentation
   */
  void FinishedMultilabel(std::vector<mitk::LabelSetImage::Pointer> result, mitk::DeepLearningSegmentationTool *segTool);
  /**
   * @brief the signal emitted when a segmentation process failed
   */
  void Failed();
  /**
   * @brief the signal emitted when a segmentation, which ran when the tool was started, finished
   */
  void PreviousSegmentationFinished();
};

#endif
