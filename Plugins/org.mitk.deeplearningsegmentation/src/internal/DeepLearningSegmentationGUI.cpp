/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "DeepLearningSegmentationGUI.h"
#include <QFileDialog>
#include <mitkIOUtil.h>
#include"SegmentationResultHandler.h"

DeepLearningSegmentationGUI::DeepLearningSegmentationGUI()
  : m_Ui(new Ui::DeepLearningSegmentationGUI)
{
  //register Meta types which is necessary for the qt signals/slots with those classes as parameter
  qRegisterMetaType<mitk::DeepLearningSegmentationTool*>();
  qRegisterMetaType<mitk::LabelSetImage::Pointer>();
  qRegisterMetaType<std::vector<mitk::LabelSetImage::Pointer>>();

  qRegisterMetaType<QVector<int>>();

  //set up the ui
  m_Ui->setupUi(this);

  //connect the UI elements in signals/slots
  connect(m_Ui->buttonPerformImageProcessing, &QPushButton::clicked, this, &DeepLearningSegmentationGUI::OnDoSegmentation);
  connect(m_Ui->buttonLoadTrainedNetwork, &QPushButton::clicked, this, &DeepLearningSegmentationGUI::DoLoadTrainedNet);
  m_Ui->buttonPerformImageProcessing->setEnabled(false);

  //create a new worker thread to execute some functions in a seperate thread
  m_SegmentationThread = new QThread;
  m_Worker = new SegmentationWorker;
  m_Worker->moveToThread(m_SegmentationThread);

  // Signal/Slot connects between worker thread and GUI 
  connect(this, &DeepLearningSegmentationGUI::Operate, m_Worker, &SegmentationWorker::DoWork);
  connect(this, &DeepLearningSegmentationGUI::Wait, m_Worker, &SegmentationWorker::WaitForSegmentationToFinish);
  connect(m_Worker, &SegmentationWorker::Finished, this, &DeepLearningSegmentationGUI::DoSegmentationProcessFinished);
  connect(m_Worker, &SegmentationWorker::FinishedMultilabel, this, &DeepLearningSegmentationGUI::DoSegmentationProcessFinished);
  connect(m_Worker, &SegmentationWorker::Failed, this, &DeepLearningSegmentationGUI::DoSegmentationProcessFinished);
  connect(m_Worker, &SegmentationWorker::PreviousSegmentationFinished, this, &DeepLearningSegmentationGUI::DoSegmentationProcessFinished);

  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(SetUpUI()));
}

DeepLearningSegmentationGUI::~DeepLearningSegmentationGUI() {
}

void DeepLearningSegmentationGUI::SetUpUI() 
{
  // Disable buttons if a segmentation is already running
  if (m_SegTool->IsSegmentationRunning())
  {
    this->SegmentationRunning();
    m_SegmentationThread->start();
    emit Wait(m_SegTool);
  }
}

void DeepLearningSegmentationGUI::DoLoadTrainedNet()
{
  //Open a file dialog to select a trained network
  QString tempPath = QString::fromStdString(mitk::IOUtil::GetTempPath());
  QString pretrainedNetResourcesPath =
    QFileDialog::getOpenFileName(nullptr, tr("Open File"), tempPath, tr("Images (*.pth.tar)"));

  //set the trained network
  m_TrainedNet = pretrainedNetResourcesPath;

  //enable segmentation 
  if (m_TrainedNet != "")
  {
    m_Ui->labelWarning->setVisible(false);
    m_Ui->buttonPerformImageProcessing->setEnabled(true);
  }
}

void DeepLearningSegmentationGUI::OnDoSegmentation()
{
    MITK_INFO << "[Start] Segmentation";
    //adapt gui to show that segmentation is running
    this->SegmentationRunning();

    SegmentationResultHandler *resultSetter = new SegmentationResultHandler;
    if (!m_SegmentationThread->isRunning())
    {
      m_SegmentationThread->start();
    }
    //start segmentation in worker thread
    emit Operate(m_SegTool, resultSetter, m_TrainedNet);
}

void DeepLearningSegmentationGUI::SegmentationRunning() 
{
  m_Ui->labelWarning->setText("Segmentation running. This might take a while.");
  m_Ui->labelWarning->setVisible(true);
  m_Ui->buttonLoadTrainedNetwork->setEnabled(false);
  m_Ui->buttonPerformImageProcessing->setEnabled(false);
}

void DeepLearningSegmentationGUI::DoSegmentationProcessFinished() 
{
  m_Ui->buttonLoadTrainedNetwork->setEnabled(true);
  if (m_TrainedNet == "")
  {
    m_Ui->labelWarning->setText("Please load a network!");
    m_Ui->labelWarning->setVisible(true);
    m_Ui->buttonPerformImageProcessing->setEnabled(false);
  }
  else
  {
    m_Ui->labelWarning->setVisible(false);
    m_Ui->buttonPerformImageProcessing->setEnabled(true);
  }
}

