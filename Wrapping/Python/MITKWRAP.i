%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}
%module pyMITK

#define MITKCORE_EXPORT

%{
#include <shape.h>
%}
%include <shape.h>
