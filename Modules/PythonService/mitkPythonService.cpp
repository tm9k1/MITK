/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPythonService.h"
#include <mitkIOUtil.h>

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif

#include "PythonPath.h"
#include <vtkPolyData.h>
#include <mitkRenderingManager.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <itksys/SystemTools.hxx>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <mitkExceptionMacro.h>

#ifndef WIN32
#include <dlfcn.h>
#endif

#include <swigpyrun.h>
#include <boost/algorithm/string.hpp>

typedef itksys::SystemTools ist;

mitk::PythonService::PythonService()
  : m_ItkWrappingAvailable(true),
    m_ErrorOccured(false)
{
  // for Linux, libpython needs to be opened before initializing the Python Interpreter to enable imports
  #ifndef WIN32
    std::vector<std::string> pythonIncludeVector;
    // get libpython file to open (taken from "Python3_INCLUDE_DIR" variable from CMake to dynamically do it for different python versions)
    boost::split(pythonIncludeVector, PYTHON_INCLUDE, boost::is_any_of( "/" ) );
    std::string libPython = "lib"+pythonIncludeVector[pythonIncludeVector.size()-1]+".so";
    dlopen(libPython.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  #endif
  // Initialize Python interpreter and ensure global interpreter lock in order to execute python code safely
  if (!Py_IsInitialized())
  {
    Py_Initialize();
  }

  PyGILState_Ensure();
  std::string programPath = mitk::IOUtil::GetProgramPath();
  std::replace(programPath.begin(), programPath.end(), '\\', '/');
  programPath.append("/");
  MITK_INFO << programPath;
  std::string pythonCommand;

  // execute a string which imports packages that are needed and sets paths to directories
  pythonCommand.append("import SimpleITK as sitk\n");
  pythonCommand.append("import SimpleITK._SimpleITK as _SimpleITK\n");
  pythonCommand.append("import numpy\n");

  pythonCommand.append("import site, sys\n");
  pythonCommand.append("import os\n");
  pythonCommand.append("sys.path.append('')\n");
  pythonCommand.append("sys.path.append('" + programPath + "')\n");
  pythonCommand.append("sys.path.append('" + std::string(BIN_DIR) + "')\n");
  pythonCommand.append("sys.path.append('" +std::string(EXTERNAL_DIST_PACKAGES) + "')\n");
  pythonCommand.append("\nsite.addsitedir('"+std::string(EXTERNAL_SITE_PACKAGES)+"')\n");
  // in python 3.8 onwards, the path system variable is not longer used to find dlls
  // that's why the dlls that are needed for swig wrapping need to be searched manually
  std::string searchForDll = "if sys.version_info[1] > 7:\n"
                             "  os.add_dll_directory('"+std::string(EP_DLL_DIR)+"')\n";
  pythonCommand.append(searchForDll);
  if (PyRun_SimpleString(pythonCommand.c_str()) == -1)
  {
    MITK_ERROR << "Something went wrong in setting the path in Python";
  }
  PyObject *main = PyImport_AddModule("__main__");
  m_GlobalDictionary = PyModule_GetDict(main);
  m_LocalDictionary = m_GlobalDictionary;
  m_ThreadState = PyEval_SaveThread();
}

mitk::PythonService::~PythonService()
{
}

void mitk::PythonService::AddRelativeSearchDirs(std::vector< std::string > dirs)
{
  for (const auto& dir : dirs)
  {
    try
    {
      std::string path = std::string(MITK_ROOT) + dir;
      // sys.path.append enables to call scripts which are located in the given path
      std::string pythonCommand = "import sys\nsys.path.append('" + path + "')\n";
      this->Execute(pythonCommand.c_str());
    }
    catch (const mitk::Exception&)
    {
      mitkThrow() << "An error occured setting the relative project path";
    }
  }
}

void mitk::PythonService::AddAbsoluteSearchDirs(std::vector< std::string > dirs)
{
  for (const auto& dir : dirs)
  {
    try
    {
      // sys.path.append enables to call scripts which are located in the given path
      std::string pythonCommand = "import sys\nsys.path.append('" + dir + "')\n";
      this->Execute(pythonCommand.c_str());
    }
    catch (const mitk::Exception&)
    {
      mitkThrow() << "An error occured setting the absolute project path";
    }
  }
}

std::string mitk::PythonService::Execute(const std::string &stdpythonCommand, int commandType)
{
  if (!Py_IsInitialized())
  {
    Py_Initialize();
  }
  std::string result = "";

  PyGILState_Ensure();
  try
  {
    // command type is start symbol for python interpreter
    switch (commandType)
    {
      case IPythonService::SINGLE_LINE_COMMAND:
        commandType = Py_single_input;
        break;
      case IPythonService::MULTI_LINE_COMMAND:
        commandType = Py_file_input;
        break;
      case IPythonService::EVAL_COMMAND:
        commandType = Py_eval_input;
        break;
      default:
        commandType = Py_file_input;
    }
    // executes python code given as string
    // if result is NULL, an error occured
    PyObject* executionResult = PyRun_String(stdpythonCommand.c_str(), commandType, m_GlobalDictionary, m_LocalDictionary);
    // notifies registered observers that command has been executed
    this->NotifyObserver(stdpythonCommand);

    // if no error occured, the result is represented as string which is returned
    if (executionResult)
    {
      PyObject *objectsRepresentation = PyObject_Repr(executionResult);
      const char *resultChar = PyUnicode_AsUTF8(objectsRepresentation);
      result = std::string(resultChar);
      m_ThreadState = PyEval_SaveThread();
      m_ErrorOccured = false;
    }
    else
    {
      m_ErrorOccured = true;
      mitkThrow() << "An error occured while running the Python code";
    }
  }
  catch (const mitk::Exception& ) 
  {
    PyErr_Print();
    m_ThreadState = PyEval_SaveThread();
    throw;
  }
  return result;
}


void mitk::PythonService::ExecuteScript(const std::string &pythonScript)
{
  // read given file as string
  std::ifstream t(pythonScript.c_str());
  std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  t.close();
  // pass the file as string to the Execute() function
  try
  {
    this->Execute(str.c_str(), MULTI_LINE_COMMAND);
  }
  catch (const mitk::Exception&)
  {
    throw;
  }
}

std::vector<mitk::PythonVariable> mitk::PythonService::GetVariableStack()
{
  // variables are returned as a list of type mitk::PythonVariable
  std::vector<mitk::PythonVariable> list;
  PyGILState_Ensure();
  try
  {
    // vaiables are taken from the main module
    // get dictionary where these variables are stored
    PyObject *dict = PyImport_GetModuleDict();
    PyObject *object = PyDict_GetItemString(dict, "__main__");
    if (!object)
    {
      mitkThrow() << "An error occured getting the Dictionary";
    }
    PyObject *dirMain = PyObject_Dir(object);
    PyObject *tempObject = nullptr;

    if (dirMain)
    {
      std::string name, attrValue, attrType;

      //iterate over python list of variables to get desired representation and store in returned variable list
      for (int i = 0; i < PyList_Size(dirMain); i++)
      {
        // get variable at current index
        tempObject = PyList_GetItem(dirMain, i);
        if (!tempObject)
        {
          mitkThrow() << "An error occured getting an item from the dictionary";
        }
        // get variable name as string
        PyObject *objectsRepresentation = PyObject_Repr(tempObject);
        const char *objectChar = PyUnicode_AsUTF8(objectsRepresentation);
        std::string name = std::string(objectChar);
        name = name.substr(1, name.size() - 2);
        // get variable type as string
        tempObject = PyObject_GetAttrString(object, name.c_str());
        if (!tempObject)
        {
          mitkThrow() << "Could not get the attribute to determine type";
        }
        attrType = tempObject->ob_type->tp_name;

        // get variable value as string
        PyObject *valueStringRepresentation = PyObject_Repr(tempObject);
        const char *valueChar = PyUnicode_AsUTF8(valueStringRepresentation);
        std::string attrValue = std::string(valueChar);

        // build variable type to store
        mitk::PythonVariable var;
        var.m_Name = name;
        var.m_Value = attrValue;
        var.m_Type = attrType;
        list.push_back(var);
      }
    }
    m_ThreadState = PyEval_SaveThread();
  }
  catch (const mitk::Exception&)
  {
    m_ThreadState = PyEval_SaveThread();
    throw;
  }
  return list;
}

std::string mitk::PythonService::GetVariable(const std::string& name)
{
  // get all variables from python context
  std::vector<mitk::PythonVariable> allVars;
  try
  {
    allVars = this->GetVariableStack();
  }
  catch (const mitk::Exception&)
  {
    mitkThrow() << "Error getting the variable stack";
  }
  // search for variable with given name
  for (const auto& var: allVars)
  {
    if (var.m_Name == name)
    {
      return var.m_Value;
    }
  }

  return "";
}

bool mitk::PythonService::DoesVariableExist(const std::string& name)
{
  bool varExists = false;
  // get all variables from python context
  std::vector<mitk::PythonVariable> allVars;
  try
  {
    allVars = this->GetVariableStack();
  }
  catch (const mitk::Exception&)
  {
    mitkThrow() << "Error getting the variable stack";
  }
  // check if variable with given name exists in context
  for (const auto& var: allVars)
  {
    if (var.m_Name == name)
    {
      varExists = true;
      break;
    }
  }

  return varExists;
}

void mitk::PythonService::AddPythonCommandObserver(mitk::PythonCommandObserver *observer)
{
  //only add observer if it isn't already in list of observers
  if (!(std::find(m_Observer.begin(), m_Observer.end(), observer) != m_Observer.end()))
  {
    m_Observer.push_back(observer);
  }
}

void mitk::PythonService::RemovePythonCommandObserver(mitk::PythonCommandObserver *observer)
{
  // iterate over all registered observers and remove the passed observer
  for (std::vector<mitk::PythonCommandObserver *>::iterator iter = m_Observer.begin(); iter != m_Observer.end(); ++iter)
  {
    if (*iter == observer)
    {
      m_Observer.erase(iter);
      break;
    }
  }
}

void mitk::PythonService::NotifyObserver(const std::string &command)
{
  // call CommandExecuted() from observer interface in order to inform observers that command has been executed
  for (const auto& observer: this->m_Observer)
  {
    observer->CommandExecuted(command);
  }

}


int mitk::PythonService::GetNumberOfObserver() 
{
  return m_Observer.size();
}

bool mitk::PythonService::IsSimpleItkPythonWrappingAvailable()
{
  return m_ItkWrappingAvailable;
}

bool mitk::PythonService::CopyToPythonAsSimpleItkImage(mitk::Image::Pointer image, const std::string &stdvarName)
{
  // this string is a python command which consists of two functions
  // setup() is called in order to create an image variable on python side. At this point this is a MITK image
  // convert_to_sitk() converts the MITK image from the previous step into a SimpleITK image. Spacing and Origin need to be set manually
  // as these informations get lost when converting the image to an numpy array and back (only contains pixel information)
  std::string transferToPython = "import pyMITK\n"
                                 "import SimpleITK as sitk\n"
                                 "mitk_image = None\n"
                                 "geometry = None\n"
                                 +stdvarName+" = None\n"
                                 "\n"
                                 "def setup(image_from_cxx):\n"
                                 "    print ('setup called with', image_from_cxx)\n"
                                 "    global mitk_image\n"
                                 "    mitk_image = image_from_cxx\n"
                                 "\n"
                                 "def convert_to_sitk(spacing, origin):\n"
                                 "    np = pyMITK.GetArrayViewFromImage(mitk_image)\n"
                                 "    global "+stdvarName+"\n"
                                 "    "+stdvarName+" = sitk.GetImageFromArray(np)\n"
                                 "    npSpacingArray = numpy.array(spacing , dtype=float)\n"
                                 "    "+stdvarName+".SetSpacing(npSpacingArray)\n"
                                 "    npOriginArray = numpy.array(origin , dtype=float)\n"
                                 "    " +stdvarName +".SetOrigin(npOriginArray)\n";
  // execute code to make the implemented functions available in the Python context (function is not executed, only definition loaded in Python)
  this->Execute(transferToPython);

  mitk::Image::Pointer *img = &image;
  PyGILState_Ensure();
  //necessary for transfer array from C++ to Python
  _import_array();
  PyObject *main = PyImport_ImportModule("__main__");
  if (main == NULL)
  {
    mitkThrow() << "Something went wrong getting the main module";
  }

  // create new pyobject from given image using SWIG
  swig_type_info *pTypeInfo = nullptr;
  pTypeInfo = SWIG_TypeQuery("_p_itk__SmartPointerT_mitk__Image_t");
  int owned = 0;
  PyObject *pInstance = SWIG_NewPointerObj(reinterpret_cast<void *>(img), pTypeInfo, owned);
  if (pInstance == NULL)
  {
    mitkThrow() << "Something went wrong creating the Python instance of the image";
  }
  // get image spacing and convert it to Python array
  int numberOfImageDimension = image->GetDimension();
  auto spacing = image->GetGeometry()->GetSpacing();
  auto *spacingptr = &spacing;
  
  npy_intp dims[] = {numberOfImageDimension};
  PyObject *spacingArray = PyArray_SimpleNewFromData(1, dims, NPY_DOUBLE, (void *)spacingptr);

  // get image origin and convert it to Python array
  auto origin = image->GetGeometry()->GetOrigin();
  auto *originptr = &origin;
  PyObject *originArray = PyArray_SimpleNewFromData(1, dims, NPY_DOUBLE, (void *)originptr);

  // transfer image to python context by calling setup() function which was defined in Python string above
  PyObject *setup = PyObject_GetAttrString(main, "setup");
  PyObject *result = PyObject_CallFunctionObjArgs(setup, pInstance, NULL);
  if (result == NULL)
  {
    mitkThrow() << "Something went wrong setting the MITK image in Python";
  }
  // convert MITK image in Python context to SimpleITK image and tranfer correct origin and spacing calling convert_to_sitk() from above
  PyObject *convert = PyObject_GetAttrString(main, "convert_to_sitk");
  result = PyObject_CallFunctionObjArgs(convert,spacingArray, originArray, NULL);
  if (result == NULL)
  {
    mitkThrow() << "Something went wrong converting the MITK image to a SimpleITK image";
  }
  m_ThreadState = PyEval_SaveThread();
  return true;
}

mitk::Image::Pointer mitk::PythonService::CopySimpleItkImageFromPython(const std::string &stdvarName)
{
  mitk::Image::Pointer mitkImage;
  // Python code to convert SimpleITK image to MITK image
  std::string convertToMITKImage = "import SimpleITK as sitk\n"
                                   "import pyMITK\n"
                                   "numpy_array = sitk.GetArrayViewFromImage("+stdvarName+")\n"
                                   "mitk_image = pyMITK.GetImageFromArray(numpy_array)\n"
                                   "spacing = "+stdvarName+".GetSpacing()\n"
                                   "origin = "+stdvarName +".GetOrigin()\n";
  // after executing this, the desired mitk image is available in the Python context with the variable name mitk_image
  this->Execute(convertToMITKImage);

  PyGILState_Ensure();
  // get dictionary with variables from main context
  PyObject *main = PyImport_AddModule("__main__");
  PyObject *globals = PyModule_GetDict(main);
  // get mitk_image from Python context
  PyObject *pyImage = PyDict_GetItemString(globals, "mitk_image");
  if (pyImage==NULL)
  {
    mitkThrow() << "Could not get image from Python";
  }

  // res is status variable to check if result when getting the image from Python context is OK
  int res = 0;
  void *voidImage;
  swig_type_info *pTypeInfo = nullptr;
  pTypeInfo = SWIG_TypeQuery("_p_itk__SmartPointerT_mitk__Image_t");
  // get image from Python context as C++ void pointer
  res = SWIG_ConvertPtr(pyImage, &voidImage, pTypeInfo, 0);
  if (!SWIG_IsOK(res))
  {
    mitkThrow() << "Could not cast image to C++ type";
  }
  // cast C++ void pointer to mitk::Image::Pointer
  mitkImage = *(reinterpret_cast<mitk::Image::Pointer *>(voidImage));

  // as spacing and origin again got lost when transferring between SimpleITK and MITK (see CopyToPythonAsSimpleItkImage()) we need to set them manually
  // get spacing from Python context
  PyObject *spacing = PyDict_GetItemString(globals, "spacing");
  mitk::Vector3D spacingMITK;
  // convert Python Array to C++ vector
  for (int i = 0; i < PyTuple_GET_SIZE(spacing); i++)
  {
    PyObject *spacingPy = PyTuple_GetItem(spacing, i);
    double elem = PyFloat_AsDouble(spacingPy);
    spacingMITK[i] = elem;
  }

  // get origin from Python context
  PyObject *origin = PyDict_GetItemString(globals, "origin");
  mitk::Point3D originMITK;
  // convert Python Array to Point3D
  for (int i = 0; i < PyTuple_GET_SIZE(origin); i++)
  {
    PyObject *originPy = PyTuple_GetItem(origin, i);
    double elem = PyFloat_AsDouble(originPy);
    originMITK[i] = elem;
  }

  m_ThreadState = PyEval_SaveThread();
  // set origin and spacing correctly
  mitkImage->GetGeometry()->SetSpacing(spacingMITK);
  mitkImage->GetGeometry()->SetOrigin(originMITK);
  return mitkImage;
}

bool mitk::PythonService::CopyMITKImageToPython(mitk::Image::Pointer &image, const std::string &stdvarName)
{
  // this string is a python command which consists one functions
  // setup() is called in order to create an image variable on python side. This is the MITK image which is passed
  std::string transferToPython = "import pyMITK\n"
                                 + stdvarName +" = None\n"
                                 "\n"
                                 "spacing = None\n"
                                 "origin = None\n"
                                 "def setup(image_from_cxx):\n"
                                 "    print ('setup called with', image_from_cxx)\n"
                                 "    global "+stdvarName+"\n"
                                 "    "+stdvarName+" = image_from_cxx\n"
                                 "\n"
                                 "def set_origin_and_spacing(s,o):\n"
                                 "    global spacing\n"
                                 "    global origin\n"                                
                                 "    spacing = numpy.array(s , dtype=float)\n"
                                 "    origin = numpy.array(o , dtype=float)\n";

  // execute code to make the implemented functions available in the Python context (function is not executed, only
  // definition loaded in Python)
  this->Execute(transferToPython);
  mitk::Image::Pointer *img = &image;
  // load main context to have access to defined function from above
  PyGILState_Ensure();
  _import_array();
  PyObject *main = PyImport_ImportModule("__main__");
  if (main == NULL)
  {
    mitkThrow() << "Something went wrong getting the main module";
  }

  // create new pyobject from given image using SWIG
  swig_type_info *pTypeInfo = nullptr;
  pTypeInfo = SWIG_TypeQuery("_p_itk__SmartPointerT_mitk__Image_t");
  int owned = 0;
  PyObject *pInstance = SWIG_NewPointerObj(reinterpret_cast<void *>(img), pTypeInfo, owned);
  if (pInstance == NULL)
  {
    mitkThrow() << "Something went wrong creating the Python instance of the image";
  }

  // call setup() function in Python with created PyObject of image
  PyObject *setup = PyObject_GetAttrString(main, "setup");
  PyObject *result = PyObject_CallFunctionObjArgs(setup, pInstance, NULL);
  if (result == NULL)
  {
    mitkThrow() << "Something went wrong setting the MITK image in Python";
  }
  // get image spacing and convert it to Python array
  int numberOfImageDimension = image->GetDimension();
  auto spacing = image->GetGeometry()->GetSpacing();
  auto *spacingptr = &spacing;
  
  npy_intp dims[] = {numberOfImageDimension};
  PyObject *spacingArray = PyArray_SimpleNewFromData(1, dims, NPY_DOUBLE, (void *)spacingptr);

  // get image origin and convert it to Python array
  auto origin = image->GetGeometry()->GetOrigin();
  auto *originptr = &origin;
  PyObject *originArray = PyArray_SimpleNewFromData(1, dims, NPY_DOUBLE, (void *)originptr);

  // convert correct origin and spacing calling set_origin_and_spacing from above
  PyObject *convert = PyObject_GetAttrString(main, "set_origin_and_spacing");
  result = PyObject_CallFunctionObjArgs(convert,spacingArray, originArray, NULL);
  if (result == NULL)
  {
    mitkThrow() << "Something went wrong setting the origin and spacing for the image";
  }
  m_ThreadState = PyEval_SaveThread();
  return true;
}

mitk::Image::Pointer GetImageFromPyObject(PyObject *pyImage) 
{
  // helper function to get a MITK image from a PyObject
  mitk::Image::Pointer mitkImage;
  // res is status variable to check if result when getting the image from Python context is OK
  int res = 0;
  void *voidImage;
  swig_type_info *pTypeInfo = nullptr;
  pTypeInfo = SWIG_TypeQuery("_p_itk__SmartPointerT_mitk__Image_t");
  // get image from Python context as C++ void pointer
  res = SWIG_ConvertPtr(pyImage, &voidImage, pTypeInfo, 0);
  if (!SWIG_IsOK(res))
  {
    mitkThrow() << "Could not cast image to C++ type";
  }
  // cast C++ void pointer to mitk::Image::Pointer
  mitkImage = *(reinterpret_cast<mitk::Image::Pointer *>(voidImage));

  return mitkImage;
}

mitk::Image::Pointer mitk::PythonService::CopyMITKImageFromPython(const std::string &stdvarName)
{
  mitk::Image::Pointer mitkImage;

  PyGILState_Ensure();
  // get image from Python context as PyObject
  PyObject *main = PyImport_AddModule("__main__");
  PyObject *globals = PyModule_GetDict(main);
  PyObject *pyImage = PyDict_GetItemString(globals, stdvarName.c_str());
  if (pyImage==NULL)
  {
    mitkThrow() << "Could not get image from Python";
  }
  // convert PyObject to mitk::Image
  mitkImage = GetImageFromPyObject(pyImage);

  m_ThreadState = PyEval_SaveThread();
  return mitkImage;
}

std::vector<mitk::Image::Pointer> mitk::PythonService::CopyListOfMITKImagesFromPython(const std::string &listVarName) 
{
  std::vector<mitk::Image::Pointer> mitkImages;

  PyGILState_Ensure();
  // get the list of the variable name as python list
  PyObject *main = PyImport_AddModule("__main__");
  PyObject *globals = PyModule_GetDict(main);
  PyObject *pyImageList = PyDict_GetItemString(globals, listVarName.c_str());
  if (pyImageList == NULL)
  {
    mitkThrow() << "Could not get image list from Python";
  }
  // iterate over python list of images and convert them to C++ mitk::Image
  for (int i = 0; i < PyList_GET_SIZE(pyImageList);i++)
  {
    PyObject *pyImage = PyList_GetItem(pyImageList, i);
    mitk::Image::Pointer img = GetImageFromPyObject(pyImage);
    mitkImages.push_back(img);
  }

  m_ThreadState = PyEval_SaveThread();
  return mitkImages;
}

bool mitk::PythonService::PythonErrorOccured() const
{
  return m_ErrorOccured;
}

