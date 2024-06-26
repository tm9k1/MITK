%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}
%module pyMITK

%naturalvar; // All non-primitive types will use const reference typemaps
%include <mitk_swig_std.i>
#define MITKCORE_EXPORT
%{
#include <mitkImage.h>
#include <mitkPixelType.h>
#include <mitkChannelDescriptor.h>
#include <mitkIOUtil.h>
using namespace mitk;
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

        %typemap(out) namespace ## :: ## class_name ## ::Pointer {
                std::cout << "namespace##::##class_name::Pointer" <<std::endl;
                namespace##::##class_name * ptr = $1;
                $result = SWIG_NewPointerObj(SWIG_as_voidptr(ptr), $descriptor(namespace::class_name *), 1);                
                if (ptr) {
                        ptr->Register();
                }
        }

        %extend namespace ## :: ## class_name {
                public:
                ~class_name() {std::cout << "namespace##::##class_name replace destructor" << std::endl;
                        self->UnRegister(); };
                void test_extend() {std::cout << "test extend" << std::endl;} //remove
        }

        %ignore namespace ## :: ## class_name ## :: ## ~class_name ## ;
        %ignore namespace ## :: ## class_name ## _Pointer;

%enddef

MITK_CLASS_SWIG_MACRO(mitk, Image)

%ignore mitk::Image::GetPixelType;
%ignore mitk::Image::GetChannelDescriptor;

%inline{

        mitk::Image::Pointer GetImage()
        {
                std::string filename = "/home/a178n/DKFZ/MITK_ws3/example_ct.nii.gz";
                auto mitkImage = mitk::IOUtil::Load<mitk::Image>(filename);
                return mitkImage;
        }

        mitk::Image* GetImagePtr()
        {
                std::string filename = "/home/a178n/DKFZ/MITK_ws3/example_ct.nii.gz";
                auto mitkImage = mitk::IOUtil::Load<mitk::Image>(filename);
                return mitkImage.GetPointer();
        }

}

%include <mitkImage.h>

%pythoncode %{
def GetName():
    return 'pyMITK'
%}


