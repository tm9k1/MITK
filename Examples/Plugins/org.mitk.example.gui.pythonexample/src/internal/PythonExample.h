/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef PythonExample_h
#define PythonExample_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "PythonWorker.h"

#include "ui_PythonExampleControls.h"
#include <QThread>
#include <QFuture>
#include<QFutureWatcher>

/**
  \brief PythonExample

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class PythonExample : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;
  ~PythonExample();

signals:
  void Operate();

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  //virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
  //                                const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void OnRunPythonCode();

  void ExecutionFinished();

  void ExecutionFailed();

  QThread* m_PythonThread;
  PythonWorker *m_Worker;

  Ui::PythonExampleControls m_Controls;
};

#endif // PythonExample_h
