%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#define SWIG_FILE_WITH_INIT
%}
%module pyMITK
//%include "numpy.i"
%include <std_shared_ptr.i>

%naturalvar; // All non-primitive types will use const reference typemaps
%include <mitk_swig_std.i>
#define MITKCORE_EXPORT
%{
#include <mitkImage.h>
#include <shape.h>

// struct TypeDefinitions
// {
//    static const int ComponentTypeUInt8 = (int)mitk::MapPixelType<uint8_t, mitk::isPrimitiveType<uint8_t>::value>::IOComponentType;
//    static const int ComponentTypeInt8 = (int)mitk::MapPixelType<int8_t, mitk::isPrimitiveType<int8_t>::value>::IOComponentType;
//    static const int ComponentTypeUInt16 = (int)mitk::MapPixelType<uint16_t, mitk::isPrimitiveType<uint16_t>::value>::IOComponentType;
//    static const int ComponentTypeInt16 = (int)mitk::MapPixelType<int16_t, mitk::isPrimitiveType<int16_t>::value>::IOComponentType;
//    static const int ComponentTypeUInt32 = (int)mitk::MapPixelType<uint32_t, mitk::isPrimitiveType<uint32_t>::value>::IOComponentType;
//    static const int ComponentTypeInt32 = (int)mitk::MapPixelType<int32_t, mitk::isPrimitiveType<int32_t>::value>::IOComponentType;
//    static const int ComponentTypeFloat = (int)mitk::MapPixelType<float, mitk::isPrimitiveType<float>::value>::IOComponentType;
//    static const int ComponentTypeDouble = (int)mitk::MapPixelType<double, mitk::isPrimitiveType<double>::value>::IOComponentType;
// };

// NPY_TYPES MakePixelTypeFromTypeID(int componentTypeID)
// {
//    switch (componentTypeID)
//    {
//       case TypeDefinitions::ComponentTypeUInt8 :
//       return NPY_USHORT;
//       case TypeDefinitions::ComponentTypeInt8 :
//       return NPY_SHORT;
//       case TypeDefinitions::ComponentTypeUInt16 :
//       return NPY_USHORT;
//       case TypeDefinitions::ComponentTypeInt16 :
//       return NPY_SHORT;
//       case TypeDefinitions::ComponentTypeUInt32 :
//       return NPY_UINT;
//       case TypeDefinitions::ComponentTypeInt32 :
//       return NPY_INT;
//       case TypeDefinitions::ComponentTypeFloat :
//       return NPY_FLOAT;
//       case TypeDefinitions::ComponentTypeDouble :
//       return NPY_DOUBLE;
//     default:
//       return NPY_DOUBLE;
//    }
// }
%}
// %init %{
//     import_array();
// %}

// %typemap(out) std::vector<double> {
//     size_t dims = $1.size();
//     $result = PyArray_SimpleNewFromData(1, (npy_intp*) &(dims), NPY_DOUBLE, (void*)$1.data());
//     PyArray_ENABLEFLAGS((PyArrayObject*)$result, NPY_ARRAY_OWNDATA);
// }

// %typemap(out) mitk::Image::Pointer {
//     unsigned int dim = $1->GetDimension();
//     auto* writeAccess = new mitk::ImageWriteAccessor($1);
//     void* mitkBufferPtr = writeAccess->GetData();
//     std::cout << "pixel type: " << $1->GetPixelType().GetComponentTypeAsString()<< std::endl;
//     std::cout << "pixel component: " << $1->GetPixelType().GetComponentType()<< std::endl;
//     std::cout << "numpy type: "<< MakePixelTypeFromTypeID((int)$1->GetPixelType().GetComponentType())<< std::endl;;
//     std::vector<npy_intp> size;
//     for (unsigned int i = 0; i < dim; ++i)
//     {
//         size.push_back($1->GetDimension(i));
//     }
//     // if the image is a vector just treat is as another dimension
//     if ($1->GetPixelType().GetNumberOfComponents() > 1)
//     {
//         size.push_back( $1->GetPixelType().GetNumberOfComponents() );
//     }
//     std::reverse(size.begin(), size.end());
//     $result = PyArray_SimpleNewFromData(dim, size.data(), 
//                 MakePixelTypeFromTypeID((int)$1->GetPixelType().GetComponentType()),
//                 mitkBufferPtr);
//     PyArray_ENABLEFLAGS((PyArrayObject*)$result, NPY_ARRAY_OWNDATA);
//     //delete writeAccess; Don't delete write access!
// }

%shared_ptr(TestClass)
%shared_ptr(ImageWrapper)
%inline
{
    class TestClass
    { 
    public:
      std::string value; 
      TestClass(std::string v) : value(v) {}
      TestClass(const TestClass&) = delete;
      TestClass(TestClass&&) = delete;
      ~TestClass(){std::cout << "TestClass destructor" << std::endl;}
      void printVal(){std::cout << "Hi from C++ TestClass: " << value << std::endl;}
    };
    
    class ImageWrapper: mitk::Image
    {
    public:
     ImageWrapper():Image() {std::cout << "ImageWrapper constror " << std::endl;};
     ~ImageWrapper() {std::cout << "ImageWrapper destruct" << std::endl;}
    };

    TestClass* createPtr(std::string test)
    {
        TestClass* ptr = new TestClass(test); 
        std::cout << ptr << std::endl;
        return ptr;
    }

    std::shared_ptr<ImageWrapper> createSharedImage()
    {
        return std::make_shared<ImageWrapper>();
    }

    std::shared_ptr<TestClass> createSharedTestClass(std::string test)
    {
        return std::make_shared<TestClass>(test);
    }

    /*std::shared_ptr<mitk::Image> GetImageAsNumpy(std::string filename)
    {
        auto mitkImage = mitk::IOUtil::Load<mitk::Image>(filename);
        return std::shared_ptr<mitk::Image>(mitkImage.GetPointer());
    }*/
}
%ignore mitk::Shape::move;
%ignore mitk::Shape::~Shape;
%extend mitk::Square {
                public:
                ~Square(){std::cout << "Square replace destructor"<< std::endl;
                self->UnRegister();}
                void test_print(){std::cout << "test print"<< std::endl;}
}


// %typemap(out) mitk::Square::Pointer {
//         std::cout << "Square::Pointerr" <<std::endl;
//         // get the raw pointer from the smart pointer
//         mitk::Square * ptr = $1;
//         // always tell SWIG_NewPointerObj we're the owner
//         $result = SWIG_NewPointerObj(SWIG_as_voidptr(ptr), $descriptor(mitk::Square *), SWIG_POINTER_NEW | 0);
//         //$result = SWIG_NewPointerObj(SWIG_as_voidptr(ptr), SWIGTYPE_p_mitk__Square, SWIG_POINTER_NEW | 1);
//         // register the object, it it exists
//         if (ptr) {
//                 ptr->Register();
//         }
// }

%inline{
PyObject* GetSquare()
{
    auto result = mitk::Square::New(10);
    result->Register();
    PyObject* resultobj = SWIG_NewPointerObj(SWIG_as_voidptr(result), SWIGTYPE_p_mitk__Square, SWIG_POINTER_NEW | 1 );
    return resultobj;
}
}

%include <shape.h>

%pythoncode %{
def GetName():
    return 'pyMITK'
%}
