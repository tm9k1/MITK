/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPythonService_h
#define mitkPythonService_h

#include "mitkIPythonService.h"
#include <itkLightObject.h>
#include "mitkSurface.h"

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif

namespace mitk
{
  ///
  /// implementation of the IPythonService using ctkabstractpythonmanager
  /// \see IPythonService
  class PythonService: public itk::LightObject, public mitk::IPythonService
  {
  public:
      ///
      /// instantiate python manager here
      PythonService();
      ///
      /// empty implementation...
      ~PythonService() override;
      ///
      /// \see IPythonService::Execute()
      std::string Execute(const std::string &pythonCommand,
                          int commandType = Py_file_input) override;
      ///
      /// \see IPythonService::ExecuteScript()
      void ExecuteScript(const std::string &pathToPythonScript) override;
      ///
      /// \see IPythonService::PythonErrorOccured()
      bool PythonErrorOccured() const override;
      ///
      /// \see IPythonService::GetVariableStack()
      std::vector<PythonVariable> GetVariableStack() override;
      ///
      /// \see IPythonService::DoesVariableExist()
      bool DoesVariableExist(const std::string& name) override;
      ///
      /// \see IPythonService::GetVariable()
      std::string GetVariable(const std::string& name) override;
      ///
      /// \see IPythonService::AddPythonCommandObserver()
      void AddPythonCommandObserver( PythonCommandObserver* observer ) override;
      ///
      /// \see IPythonService::RemovePythonCommandObserver()
      void RemovePythonCommandObserver( PythonCommandObserver* observer ) override;
      ///
      /// \see IPythonService::NotifyObserver()
      void NotifyObserver( const std::string& command ) override;
      ///
      /// \see IPythonService::GetNumberOfObserver()
      int GetNumberOfObserver() override;
      ///
      /// \see IPythonService::IsItkPythonWrappingAvailable()
      bool IsSimpleItkPythonWrappingAvailable() override;
      ///
      /// \see IPythonService::CopyToPythonAsItkImage()
      bool CopyToPythonAsSimpleItkImage( mitk::Image::Pointer image, const std::string& varName ) override;
      ///
      /// \see IPythonService::CopyItkImageFromPython()
      mitk::Image::Pointer CopySimpleItkImageFromPython( const std::string& varName ) override;
      ///
      /// \see IPythonService::CopyMITKImageToPython()
      bool CopyMITKImageToPython(mitk::Image::Pointer &image, const std::string &varName) override;
      ///
      /// \see IPythonService::CopyMITKImageFromPython()
      mitk::Image::Pointer CopyMITKImageFromPython(const std::string &varName) override;
      ///
      /// \see IPythonService::CopyListOfMITKImagesFromPython()
      std::vector<mitk::Image::Pointer> CopyListOfMITKImagesFromPython(const std::string &listVarName) override;

      ///
      /// \see IPythonService::AddRelativeSearchDirs()
      void AddRelativeSearchDirs(std::vector< std::string > dirs) override;
      ///
      /// \see IPythonService::AddAbsoluteSearchDirs()
      void AddAbsoluteSearchDirs(std::vector< std::string > dirs) override;

  protected:

  private:
      std::vector<PythonCommandObserver*> m_Observer;
      PyThreadState *m_ThreadState;
      PyObject *m_GlobalDictionary;
      PyObject *m_LocalDictionary;
      bool m_ItkWrappingAvailable;
      bool m_ErrorOccured;
  };
}
#endif
