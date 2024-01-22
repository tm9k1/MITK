/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include"org_mitk_example_gui_pythonexample_Activator.h"
// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "PythonExample.h"

// Qt
#include <QMessageBox>
#include <QtConcurrent>


#include <usModule.h>
#include <usServiceTracker.h>
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>
#include<mitkIPythonService.h>
#include <usModuleInitialization.h>
#include <mitkStandardFileLocations.h>


US_INITIALIZE_MODULE


const std::string PythonExample::VIEW_ID = "org.mitk.views.pythonexample";

PythonExample::~PythonExample() 
{
  if (m_PythonThread->isRunning())
  {
    m_PythonThread->quit();
  }
}

void PythonExample::SetFocus()
{
  m_Controls.buttonRunPythonCode->setFocus();
}

void PythonExample::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonRunPythonCode, &QPushButton::clicked, this, &PythonExample::OnRunPythonCode);
  m_Controls.labelCodeRunning->setVisible(false);
  m_PythonThread = new QThread;
  m_Worker = new PythonWorker;
  m_Worker->moveToThread(m_PythonThread);
  connect(m_PythonThread, &QThread::finished, m_Worker, &QObject::deleteLater);
  connect(this, &PythonExample::Operate, m_Worker, &PythonWorker::DoWork);
  connect(m_Worker, &PythonWorker::Finished, this, &PythonExample::ExecutionFinished);
  connect(m_Worker, &PythonWorker::Failed, this, &PythonExample::ExecutionFailed);
}

void PythonExample::OnRunPythonCode()
{
  MITK_INFO << "[Start] Run Python Code";
  m_Controls.labelCodeRunning->setVisible(true);
  m_Controls.buttonRunPythonCode->setEnabled(false);
  m_PythonThread->start();
  emit Operate();
}

void PythonExample::ExecutionFinished() 
{
  QMessageBox::information(nullptr, "Finished python execution", "Execution of python code finished successfully.");
  m_Controls.labelCodeRunning->setVisible(false);
  m_Controls.buttonRunPythonCode->setEnabled(true);
}

void PythonExample::ExecutionFailed() 
{
  QMessageBox::warning(nullptr, "Error in running python", "There was an error running the Python Code.");
  m_Controls.labelCodeRunning->setVisible(false);
  m_Controls.buttonRunPythonCode->setEnabled(true);
}
