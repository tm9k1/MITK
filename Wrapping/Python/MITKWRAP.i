%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#define SWIG_FILE_WITH_INIT
%}
%module pyMITK

%naturalvar; // All non-primitive types will use const reference typemaps
%include <mitk_swig_std.i>
%include "numpy.i"

#define MITKCORE_EXPORT
#define ITKCommon_EXPORT
%{
#include <mitkImage.h>
#include <mitkPixelType.h>
#include <mitkChannelDescriptor.h>
#include <mitkIOUtil.h>
#include <mitkImageReadAccessor.h>
using namespace mitk;

using itk::LightObject;
using itk::SmartPointer;

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

// ignore reference count management
%ignore Delete;
%ignore SetReferenceCount;
%ignore Register;
%ignore UnRegister;

%define MITK_CLASS_SWIG_MACRO(namespace, class_name)
        
    %typemap(out) namespace ## :: ## class_name ## *, namespace ## :: ## class_name & {
        std::cout << "class_name *, class_name &" <<std::endl;
        $result = SWIG_NewPointerObj(SWIG_as_voidptr($1), $1_descriptor, 1);
        /*if ($1) {
                $1->Register();
        }*/
    }

    %typemap(out) namespace ## :: ## class_name ## ::Pointer 
    {
        std::cout << "namespace##::##class_name::Pointer" <<std::endl;
        namespace##::##class_name * ptr = $1.GetPointer();
        $result = SWIG_NewPointerObj(SWIG_as_voidptr(ptr), $descriptor(namespace::class_name *), 1);                
        if (ptr) {
            ptr->Register();
        }
    }

    %extend namespace ## :: ## class_name
    {
        public:
        ~class_name()   
        {   std::cout << "namespace##::##class_name replace destructor" << std::endl;
            self->UnRegister(); 
        };
    }


    %ignore namespace ## :: ## class_name ## :: ## ~class_name ## ;
    %ignore namespace ## :: ## class_name ## _Pointer;

%enddef

MITK_CLASS_SWIG_MACRO(mitk, Image)

%ignore mitk::Image::GetPixelType;
%ignore mitk::Image::GetChannelDescriptor;

%inline{

    mitk::Image::Pointer GetImage() //for test, delete later
    {
        std::string filename = "/home/a178n/DKFZ/MITK_ws3/example_ct.nii.gz";
        auto mitkImage = mitk::IOUtil::Load<mitk::Image>(filename);
        return mitkImage;
    }

    mitk::Image* GetImagePtr() //for test, delete later
    {
        std::string filename = "/home/a178n/DKFZ/MITK_ws3/example_ct.nii.gz";
        auto mitkImage = mitk::IOUtil::Load<mitk::Image>(filename);
        return mitkImage.GetPointer();
    }

}

%extend mitk::IOUtil 
{
    public:
    static void imsave(const mitk::Image* image, const std::string& filePath)
    {
        mitk::IOUtil::Save(image, filePath);
    }
}

%extend mitk::Image
{
    public:
    PyObject* GetAsNumpy() 
    {
        unsigned int dim = self->GetDimension();
        auto* readAccess = new mitk::ImageReadAccessor(self);
        const void* mitkBufferPtr = readAccess->GetData();
        std::cout << "n dims: "<< dim << std::endl;
        std::cout << "pixel type: " << self->GetPixelType().GetComponentTypeAsString()<< std::endl;
        std::cout << "pixel component: " << self->GetPixelType().GetComponentType()<< std::endl;
        std::cout << "numpy type: "<< MakePixelTypeFromTypeID((int)self->GetPixelType().GetComponentType())<< std::endl;;
        std::vector<npy_intp> size;
        for (unsigned int i = 0; i < dim; ++i)
        {
            size.push_back(self->GetDimension(i));
        }
        if (self->GetPixelType().GetNumberOfComponents() > 1) // if the image is a vector just treat is as another dimension
        {    
            size.push_back(self->GetPixelType().GetNumberOfComponents() );
        }
        std::reverse(size.begin(), size.end());
        PyObject* result = PyArray_SimpleNewFromData(dim, size.data(), 
                MakePixelTypeFromTypeID((int)self->GetPixelType().GetComponentType()),
                const_cast<void*>(mitkBufferPtr));
        //PyArray_ENABLEFLAGS((PyArrayObject*)result, NPY_ARRAY_OWNDATA);
        delete readAccess;
        return result;
    }
}

%ignore mitk::IOUtil::Save;

%include <itkMacro.h>
%include <mitkCommon.h>
%include <mitkImage.h>
%include <mitkIOUtil.h>
%template(imread) mitk::IOUtil::Load<mitk::Image>;

%pythoncode %{
    def GetName():
        return 'pyMITK'
%}

