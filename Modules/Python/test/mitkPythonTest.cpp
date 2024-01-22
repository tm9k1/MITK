/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkCommon.h>
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <mitkIPythonService.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkStandardFileLocations.h>
#include <mitkPythonObserverMock.h>
#include <mitkIOUtil.h>




class mitkPythonTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonTestSuite);
  
  MITK_TEST(TestEvaluateOperationWithResult);
  MITK_TEST(TestExecuteStatement);
  MITK_TEST(TestSettingVariable);
  MITK_TEST(TestSettingVariableAndUseIt);
  MITK_TEST(TestRunningScript);
  MITK_TEST(TestRunningScriptCallOtherScript);
  MITK_TEST(TestRunningScriptCallOtherScriptInSubfolder);
  MITK_TEST(TestGetVariableStack);
  MITK_TEST(TestGetVariable);
  MITK_TEST(TestDoesVariableExist_True);
  MITK_TEST(TestDoesVariableExist_False);
  MITK_TEST(TestAddObserver);
  MITK_TEST(TestRemoveObserver);
  MITK_TEST(TestNotifyObserver);
  MITK_TEST(TestCopyImageSimpleITK);
  MITK_TEST(TestCopyImageMITK);
  CPPUNIT_TEST_SUITE_END();

public:

    void setUp() 
    { 
      mitk::IPythonService::ForceLoadModule(); 
    }

  void TestEvaluateOperationWithResult()
  {
    std::string result = "";
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService = dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));
      result = m_PythonService->Execute("5+5", mitk::IPythonService::EVAL_COMMAND);
      try
      {
        std::string result = m_PythonService->Execute("5+5", mitk::IPythonService::EVAL_COMMAND);
        CPPUNIT_ASSERT_MESSAGE("Testing if running python code 5+5 results in 10", result == "10");
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in Python Execution");
      }
    }
    else
    {
      CPPUNIT_FAIL("No Service Reference found");
    }
  }

    void TestExecuteStatement()
  {
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService =
        dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

      try
      {
        std::string result = m_PythonService->Execute("print('Hello')");
        //std::string result = "None";
        CPPUNIT_ASSERT_MESSAGE("Testing if executing a statement works", result == "None");
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in Python Execution");
      }
    }
    else
    {
      CPPUNIT_FAIL("No Service Reference found");
    }
  }

  void TestSettingVariable() 
  {
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService =
        dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

      try
      {
        std::string result = m_PythonService->Execute("number = 5");
        CPPUNIT_ASSERT_MESSAGE("Testing if initializing a variable works", result == "None");
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in Python Execution");
      }
    }
    else
    {
      CPPUNIT_FAIL("No Service Reference found");
    }
  }

  void TestSettingVariableAndUseIt() 
  {
    std::string result = "";
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService =
        dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

      try
      {
        result = m_PythonService->Execute("number = 5");
        result = m_PythonService->Execute("number+5", mitk::IPythonService::EVAL_COMMAND);
        CPPUNIT_ASSERT_MESSAGE("Testing if initializing a variable and using it works", result == "10");
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in Python Execution");
      }
    }
    else
    {
      CPPUNIT_FAIL("No Service Reference found");
    }
  }

  void TestRunningScript() 
  {
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService =
        dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

      try
      {
        std::string pythonFileName = "hello.py";
        std::string fileName = mitk::StandardFileLocations::GetInstance()->FindFile(
          pythonFileName.c_str(), "Modules/Python/test/hello_world_project");
        m_PythonService->ExecuteScript(fileName);
      }
      catch(const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in Python Execution for Script");
        return;
      }
    }
    else
    {
      CPPUNIT_FAIL("No Service Reference found");
    }
  }

    void TestRunningScriptCallOtherScript()
  {
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService =
        dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

      try
      {
        try
        {
          std::string path = "Modules/Python/test/hello_world_project";
          std::vector<std::string> pathVector;
          pathVector.push_back(path);
          m_PythonService->AddRelativeSearchDirs(pathVector);
        }
        catch (const mitk::Exception &e)
        {
          MITK_ERROR << e.GetDescription();
          CPPUNIT_FAIL("Error in setting the search directory");
          return;
        }

        std::string pythonFileName = "call_hello.py";
        std::string fileName = mitk::StandardFileLocations::GetInstance()->FindFile(
          pythonFileName.c_str(), "Modules/Python/test/hello_world_project");
        m_PythonService->ExecuteScript(fileName);
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in Python Execution for Script");
        return;
      }
    }
    else
    {
      CPPUNIT_FAIL("No Service Reference found");
    }
  }

    void TestRunningScriptCallOtherScriptInSubfolder() 
    {
      us::ModuleContext *context = us::GetModuleContext();
      std::string filter = "(Name=PythonService)";
      auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

      if (!m_PythonServiceRefs.empty())
      {
        mitk::IPythonService *m_PythonService =
          dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

        try
        {
          try
          {
            std::string path = "Modules/Python/test/hello_world_project";
            std::vector<std::string> pathVector;
            pathVector.push_back(path);
            m_PythonService->AddRelativeSearchDirs(pathVector);
          }
          catch (const mitk::Exception &e)
          {
            MITK_ERROR << e.GetDescription();
            CPPUNIT_FAIL("Error in setting the search directory");
            return;
          }

          std::string pythonFileName = "call_hello_in_subfolder.py";
          std::string fileName = mitk::StandardFileLocations::GetInstance()->FindFile(
            pythonFileName.c_str(), "Modules/Python/test/hello_world_project");
          m_PythonService->ExecuteScript(fileName);
        }
        catch (const mitk::Exception &e)
        {
          MITK_ERROR << e.GetDescription();
          CPPUNIT_FAIL("Error in Python Execution for Script");
          return;
        }
      }
      else
      {
        CPPUNIT_FAIL("No Service Reference found");
      }
   }

     void TestGetVariableStack()
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));
       try
       {
         auto list = m_PythonService->GetVariableStack();
         if (!(list.size() > 0))
         {
           CPPUNIT_FAIL("Failed to get variable stack");
         }
         //for (mitk::PythonVariable var : list)
         //{
         //  MITK_INFO << var.m_Name << ", " << var.m_Value << ", " << var.m_Type;
         //}
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in getting variable stack");
         return;
       }       
     }
     else
     {
       CPPUNIT_FAIL("No Service Reference found");
     }
   }

   void TestGetVariable()
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

       try
       {
         m_PythonService->Execute("variable_to_get ='Get this variable'");
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in setting the variable");
         return;
       }   
       try
       {
         std::string variableToGet = m_PythonService->GetVariable("variable_to_get");
         CPPUNIT_ASSERT_MESSAGE("Testing if getting a variable as string representation works",
                                variableToGet == "'Get this variable'");
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in getting a variable as string representation");
         return;
       }   
     }
     else
     {
       CPPUNIT_FAIL("No Service Reference found");
     }
   }

   void TestDoesVariableExist_True()
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);
     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

       try
       {
         m_PythonService->Execute("existingVariable ='This variable exists'");
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in setting the variable");
         return;
       }   
       try
       {
         bool variableExists = m_PythonService->DoesVariableExist("existingVariable");
         CPPUNIT_ASSERT_MESSAGE("Testing if an existing variable is recognized", variableExists == true);
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in checking if a variable exists");
         return;
       }   
     }
     else
     {
       CPPUNIT_FAIL("No Service Reference found");
     }
   }

   void TestDoesVariableExist_False()
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

       try
       {
         bool variableExists = m_PythonService->DoesVariableExist("nonExistingVariable");
         CPPUNIT_ASSERT_MESSAGE("Testing if an not existing variable is not recognized", variableExists == false);
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in checking if a variable exists");
         return;
       }   
     }
     else
     {
       CPPUNIT_FAIL("No Service Reference found");
     }
   }

   void TestAddObserver() 
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

       int numberBeforeAdding = m_PythonService->GetNumberOfObserver();
       auto observer = new PythonObserverMock;
       m_PythonService->AddPythonCommandObserver(observer);
       int numberAfterAdding = m_PythonService->GetNumberOfObserver();
       CPPUNIT_ASSERT_MESSAGE("Testing if a new command observer can be added", numberAfterAdding == numberBeforeAdding+1);
     }
   }

   void TestRemoveObserver() 
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

       int numberBeforeAdding = m_PythonService->GetNumberOfObserver();
       auto observer = new PythonObserverMock;
       m_PythonService->AddPythonCommandObserver(observer);
       int numberAfterAdding = m_PythonService->GetNumberOfObserver();
       CPPUNIT_ASSERT_MESSAGE("Testing if a new command observer can be added",
                              numberAfterAdding == numberBeforeAdding + 1);

       m_PythonService->RemovePythonCommandObserver(observer);
       int numberAfterRemoving = m_PythonService->GetNumberOfObserver();
       CPPUNIT_ASSERT_MESSAGE("Testing if a command observer can be removed",
                              numberAfterRemoving == numberBeforeAdding);
     }
   }

   void TestNotifyObserver() 
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));

       auto observer = new PythonObserverMock;
       m_PythonService->AddPythonCommandObserver(observer);
       std::string command = "number = 5";
       try
       {
         m_PythonService->Execute(command);
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in Python Execution");
       }
       CPPUNIT_ASSERT_MESSAGE("Testing if a command observer is notified",
                              observer->m_Updated == true);
     }
   }


   void TestCopyImageSimpleITK()
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));
       mitk::Image::Pointer image_in = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
       mitk::Image::Pointer image_out;
       try
       {
         m_PythonService->CopyToPythonAsSimpleItkImage(image_in, "image");
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in copying Image to Python");
       }
       try
       {
         image_out = m_PythonService->CopySimpleItkImageFromPython("image");
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in copying Image to Python");
       }
       CPPUNIT_ASSERT_MESSAGE("copy an image to python and back should result in equal image",
                              mitk::Equal(*image_in, *image_out, mitk::eps, true));
     }
   }

   void TestCopyImageMITK() 
   {
     us::ModuleContext *context = us::GetModuleContext();
     std::string filter = "(Name=PythonService)";
     auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);

     if (!m_PythonServiceRefs.empty())
     {
       mitk::IPythonService *m_PythonService =
         dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));
       mitk::Image::Pointer image_in = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
       mitk::Image::Pointer image_out;
       try
       {
         m_PythonService->CopyMITKImageToPython(image_in, "mitkimage");
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in copying Image to Python");
       }
       try
       {
         image_out = m_PythonService->CopyMITKImageFromPython(/*image_in,*/"mitkimage");
       }
       catch (const mitk::Exception &e)
       {
         MITK_ERROR << e.GetDescription();
         CPPUNIT_FAIL("Error in copying Image to Python");
       }
       if (image_out ==nullptr)
       {
         MITK_INFO << "ups";
       }
       CPPUNIT_ASSERT_MESSAGE("copy an image to python and back should result in equal image",
                              mitk::Equal(*image_in, *image_out, mitk::eps, true));
     }
   }
};

MITK_TEST_SUITE_REGISTRATION(mitkPython)
