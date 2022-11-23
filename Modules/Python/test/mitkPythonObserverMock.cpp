#include "mitkPythonObserverMock.h"

PythonObserverMock::PythonObserverMock()
  : m_Updated(false) 
{
}

void PythonObserverMock::CommandExecuted(const std::string &pythonCommand) 
{
  MITK_DEBUG << "received Command " << pythonCommand;
  m_Updated = true;
}
