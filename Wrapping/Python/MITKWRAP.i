%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#define SWIG_FILE_WITH_INIT
%}
%module pyMITK
%include "numpy.i"

%naturalvar; // All non-primitive types will use const reference typemaps
%include <mitk_swig_std.i>
#define MITKCORE_EXPORT
%{
//#include <mitkBaseData.h>
#include <mitkIOUtil.h>
#include <mitkImageWriteAccessor.h>
#include <mitkImage.h>
#include <shape.h>
#include <mitkPixelType.h>
// using mitk::DataStorage;
// using mitk::IFileReader;
// using mitk::IFileWriter;
// using mitk::Operation;
// using mitk::BaseProperty;
// using mitk::BaseGeometry;
// using mitk::TimeGeometry;
// using mitk::PropertyList;
// using mitk::Point3D;
// using itk::DataObject;

struct TypeDefinitions
{
   static const int ComponentTypeUInt8 = (int)mitk::MapPixelType<uint8_t, mitk::isPrimitiveType<uint8_t>::value>::IOComponentType;
   static const int ComponentTypeInt8 = (int)mitk::MapPixelType<int8_t, mitk::isPrimitiveType<int8_t>::value>::IOComponentType;
   static const int ComponentTypeUInt16 = (int)mitk::MapPixelType<uint16_t, mitk::isPrimitiveType<uint16_t>::value>::IOComponentType;
   static const int ComponentTypeInt16 = (int)mitk::MapPixelType<int16_t, mitk::isPrimitiveType<int16_t>::value>::IOComponentType;
   static const int ComponentTypeUInt32 = (int)mitk::MapPixelType<uint32_t, mitk::isPrimitiveType<uint32_t>::value>::IOComponentType;
   static const int ComponentTypeInt32 = (int)mitk::MapPixelType<int32_t, mitk::isPrimitiveType<int32_t>::value>::IOComponentType;
   static const int ComponentTypeFloat = (int)mitk::MapPixelType<float, mitk::isPrimitiveType<float>::value>::IOComponentType;
   static const int ComponentTypeDouble = (int)mitk::MapPixelType<double, mitk::isPrimitiveType<double>::value>::IOComponentType;
};

NPY_TYPES MakePixelTypeFromTypeID(int componentTypeID)
{
   switch (componentTypeID)
   {
      case TypeDefinitions::ComponentTypeUInt8 :
      return NPY_USHORT;
      case TypeDefinitions::ComponentTypeInt8 :
      return NPY_SHORT;
      case TypeDefinitions::ComponentTypeUInt16 :
      return NPY_USHORT;
      case TypeDefinitions::ComponentTypeInt16 :
      return NPY_SHORT;
      case TypeDefinitions::ComponentTypeUInt32 :
      return NPY_UINT;
      case TypeDefinitions::ComponentTypeInt32 :
      return NPY_INT;
      case TypeDefinitions::ComponentTypeFloat :
      return NPY_FLOAT;
      case TypeDefinitions::ComponentTypeDouble :
      return NPY_DOUBLE;
    default:
      return NPY_DOUBLE;
   }
}

%}
%init %{
    import_array();
%}

%typemap(out) std::vector<double> {
    size_t dims = $1.size();
    $result = PyArray_SimpleNewFromData(1, (npy_intp*) &(dims), NPY_DOUBLE, (void*)$1.data());
    PyArray_ENABLEFLAGS((PyArrayObject*)$result, NPY_ARRAY_OWNDATA);
}

%typemap(out) mitk::Image::Pointer {
    unsigned int dim = $1->GetDimension();
    std::cout << "dim: " << dim << std::endl;
    auto* writeAccess = new mitk::ImageWriteAccessor($1);
    void* mitkBufferPtr = writeAccess->GetData();
    std::cout << "pixel type: " << $1->GetPixelType().GetComponentTypeAsString()<< std::endl;
    std::cout << "pixel component: " << $1->GetPixelType().GetComponentType()<< std::endl;
    std::cout << "numpy type: "<< MakePixelTypeFromTypeID((int)$1->GetPixelType().GetComponentType())<< std::endl;;
    std::vector<unsigned int> size;
    for (int i = 0; i < dim; ++i)
    {
        size.push_back($1->GetDimension(i));
    }
    // if the image is a vector just treat is as another dimension
    if ($1->GetPixelType().GetNumberOfComponents() > 1)
    {
        size.push_back( $1->GetPixelType().GetNumberOfComponents() );
    }
    std::reverse(size.begin(), size.end());
    npy_intp shape[size.size()];//= {112, 122};
    for(int i = 0; i < size.size(); ++i ) 
        shape[i] = size.data()[i];
    
    $result = PyArray_SimpleNewFromData(dim, shape, 
                MakePixelTypeFromTypeID((int)$1->GetPixelType().GetComponentType()),
                mitkBufferPtr);
    //PyArray_ENABLEFLAGS((PyArrayObject*)$result, NPY_ARRAY_OWNDATA);
    //delete writeAccess; Don't delete write access!
}

%inline
{
    mitk::Image::Pointer GetImageAsNumpy(std::string filename)
    {
        auto mitkImage = mitk::IOUtil::Load<mitk::Image>(filename);
        return mitkImage;
    }
}

//%include <mitkBaseData.h>
//%include <mitkIOUtil.h>
%include <shape.h>

%pythoncode %{
def GetName():
    return 'pyMITK'
%}