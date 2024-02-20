%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}
%module pyMITK

#define MITKCORE_EXPORT
%{

#include <mitkBaseData.h>
#include <shape.h>

using mitk::Operation;
using mitk::BaseProperty;
using mitk::BaseGeometry;
using mitk::TimeGeometry;
using mitk::PropertyList;
using mitk::Point3D;
using itk::DataObject;
%}

%include <mitkBaseData.h>
%include <shape.h>

