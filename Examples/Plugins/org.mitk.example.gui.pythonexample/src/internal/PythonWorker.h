/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef PythonWorker_h
#define PythonWorker_h

#include <QObject>

class PythonWorker : public QObject
{
  Q_OBJECT
public slots:
  void DoWork();

signals:
  void Finished();
  void Failed();
};

#endif