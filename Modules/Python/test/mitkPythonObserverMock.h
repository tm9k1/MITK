/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPythonObserverMock_h
#define mitkPythonObserverMock_h
#include<mitkIPythonService.h>

class PythonObserverMock : public mitk::PythonCommandObserver
{
public:
  PythonObserverMock();
  void CommandExecuted(const std::string &pythonCommand) override;
  bool m_Updated;
};

#endif