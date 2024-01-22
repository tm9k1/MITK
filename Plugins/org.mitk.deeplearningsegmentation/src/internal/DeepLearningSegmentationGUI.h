/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef DeepLearningSegmentationGUI_h
#define DeepLearningSegmentationGUI_h

#include <org_mitk_deeplearningsegmentation_Export.h>
#include "ui_DeepLearningSegmentationGUI.h"
#include <QmitkToolGUI.h>
#include "DeepLearningSegmentationTool.h"
#include <QThread>
#include"SegmentationWorker.h"

namespace Ui {
class DeepLearningSegmentationGUI;
}

/**
 * @class DeepLearningSegmentationGUI
 * @brief This is the base class for all GUIs of Deep Learning Based Segmentations
 */
class DEEPLEARNINGSEGMENTATION_EXPORT DeepLearningSegmentationGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  /**
   * @brief Constructor. Mainly for setting UI up and connect some signals and slots
   * icon resource.
   */
  DeepLearningSegmentationGUI();
  ~DeepLearningSegmentationGUI() override;

  /**
   * @brief Adapt the gui (e.g. deactivate buttons) if a segmentation is running.
   */
  void SegmentationRunning();

signals: 
    
   /**
   * @brief signal for starting the segmentation which is caught by a worker thread.
   *
   * @param tool the Segmentation Tool for running the segmentation
   * @param guiSetter the result handler which displays the result in the UI after the segmentation
   * @param networkPath the path to the trained network which is needed for the segmentation
   */
  void Operate(mitk::DeepLearningSegmentationTool* tool, SegmentationResultHandler* guiSetter, QString networkPath);
  /**
   * @brief if a segmentation is executed when the tool is started, emit a signal for waiting in the worker thread.
   *
   * @param tool the Segmentation Tool to check in worker, if the segmentation is still running.
   */
  void Wait(mitk::DeepLearningSegmentationTool *tool);

  protected slots:
    /** 
    * @brief set up the UI depending on whether a segmentation is running
    */
    virtual void SetUpUI();
    /**
     * @brief start the segmentation by emitting a operate signal which is caught by a worker thread.
     *        Called when the "Run Segmentation" button is pressed.
     */
    virtual void OnDoSegmentation();
    /**
     * @brief set m_TrainedNetwork after a File Dialog is appearing to select a trained network.
     *        Called when the "Load trained network" button is pressed.
     */
    void DoLoadTrainedNet();
    /**
     * @brief Set the segmentation method m_SegTool.
     *        This method has to be overwritten by every individual segmentation method to set the correct segmentation tool.
     */
    virtual void OnNewToolAssociated(mitk::Tool *) = 0;
    /**
     * @brief set up the UI if a segmentation is finished
     */
    void DoSegmentationProcessFinished();

  protected:
    mitk::DeepLearningSegmentationTool* m_SegTool;
    QScopedPointer<Ui::DeepLearningSegmentationGUI> m_Ui;
    QString m_TrainedNet;

  private:

    QThread *m_SegmentationThread;
    SegmentationWorker *m_Worker;
    
};

#endif // DeepLearningSegmentationGUI_h
