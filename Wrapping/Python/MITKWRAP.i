%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#define SWIG_FILE_WITH_INIT
%}
%module pyMITK
%include <mitk_swig_std.i>

%naturalvar; // All non-primitive types will use const reference typemaps
%include "numpy.i"

%constant int ComponentTypeUInt8 = TypeDefinitions::ComponentTypeUInt8;
%constant int ComponentTypeInt8 = TypeDefinitions::ComponentTypeInt8;
%constant int ComponentTypeUInt16 = TypeDefinitions::ComponentTypeUInt16;
%constant int ComponentTypeInt16 = TypeDefinitions::ComponentTypeInt16;
%constant int ComponentTypeUInt32 = TypeDefinitions::ComponentTypeUInt32;
%constant int ComponentTypeInt32 = TypeDefinitions::ComponentTypeInt32;
%constant int ComponentTypeFloat = TypeDefinitions::ComponentTypeFloat;
%constant int ComponentTypeDouble = TypeDefinitions::ComponentTypeDouble;

#define MITKCORE_EXPORT
#define ITKCommon_EXPORT
%{
#include <itkSmartPointer.h>
#include <mitkIOUtil.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

using namespace mitk;

using itk::DataObject;
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

mitk::PixelType MakePixelTypeFromTypeID(int componentTypeID, int numberOfComponents)
{
   switch (componentTypeID)
   {
      case TypeDefinitions::ComponentTypeUInt8 :
      return mitk::MakePixelType<uint8_t, uint8_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt8 :
      return mitk::MakePixelType<int8_t, int8_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeUInt16 :
      return mitk::MakePixelType<uint16_t, uint16_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt16 :
      return mitk::MakePixelType<int16_t, int16_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeUInt32 :
      return mitk::MakePixelType<uint32_t, uint32_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt32 :
      return mitk::MakePixelType<int32_t, int32_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeFloat :
      return mitk::MakePixelType<float, float>(numberOfComponents);
      case TypeDefinitions::ComponentTypeDouble :
      return mitk::MakePixelType<double, double>(numberOfComponents);
    default:
      return mitk::MakePixelType<double, double>(numberOfComponents);
   }
}

    /** An internal function that performs a deep copy of the image buffer
     * into a python byte array. The byte array can later be converted
     * into a numpy array with the frombuffer method.
     */
    static PyObject*
    mitk_SetImageFromArray( PyObject *SWIGUNUSEDPARM(self), PyObject *args )
    {
        PyObject * pyImage = NULL;

        const void *buffer;
        Py_ssize_t buffer_len;
        Py_buffer  pyBuffer;
        memset(&pyBuffer, 0, sizeof(Py_buffer));

        mitk::Image * mitkImage = NULL;
        void * mitkBufferPtr = NULL;
        size_t pixelSize = 1;

        unsigned int dimension = 0;
        std::vector< unsigned int > size;
        size_t len = 1;

        unsigned int accuValue = 1;

        // We wish to support both the new PEP3118 buffer interface and the
        // older. So we first try to parse the arguments with the new buffer
        // protocol, then the old.
        if (!PyArg_ParseTuple(args, "s*O", &pyBuffer, &pyImage ))
        {
            PyErr_Clear();
        #ifdef PY_SSIZE_T_CLEAN
            typedef Py_ssize_t bufSizeType;
        #else
            typedef int bufSizeType;
        #endif
            bufSizeType _len;
            // This function takes 2 arguments from python, the first is an
            // python object which support the old "ReadBuffer" interface
            if( !PyArg_ParseTuple( args, "s#O", &buffer, &_len, &pyImage ) )
            {
                return NULL;
            }
            buffer_len = _len;
        }
        else
        {
            if ( PyBuffer_IsContiguous( &pyBuffer, 'C' ) != 1 )
            {
                PyBuffer_Release( &pyBuffer );
                PyErr_SetString( PyExc_TypeError, "A C Contiguous buffer object is required." );
                return NULL;
            }
            buffer_len = pyBuffer.len;
            buffer = pyBuffer.buf;
        }
        {
            void * voidImage;
            int res = 0;
            res = SWIG_ConvertPtr(pyImage, &voidImage, SWIGTYPE_p_mitk__Image, 0);
            if (!SWIG_IsOK(res))
            {
                SWIG_exception_fail(SWIG_ArgError(res), "Pointer conversion failed.");
            }
            mitkImage = reinterpret_cast< mitk::Image * >(voidImage);
        }
        try
        {
            mitk::ImageWriteAccessor writeAccess(mitkImage, mitkImage->GetVolumeData(0));
            mitkBufferPtr = writeAccess.GetData();
            pixelSize= mitkImage->GetPixelType().GetBitsPerComponent() / 8;
            std::cout << "pixelSize "<< pixelSize << std::endl;
        }
        catch( const std::exception &e )
        {
            std::string msg = "Exception thrown in MITK new Image: ";
            msg += e.what();
            PyErr_SetString( PyExc_RuntimeError, msg.c_str() );
            goto fail;
        }
        dimension = mitkImage->GetDimension();
        for (unsigned int i = 0; i < dimension; ++i)
        {
            size.push_back(mitkImage->GetDimension(i));
        }
        // if the image is a vector just treat is as another dimension
        if (mitkImage->GetPixelType().GetNumberOfComponents() > 1)
        {
            size.push_back(mitkImage->GetPixelType().GetNumberOfComponents());
        }

        len = std::accumulate(size.begin(), size.end(), accuValue, std::multiplies<unsigned int>());
        len *= pixelSize;
        std::cout << "len "<< len << std::endl;
        std::cout << "buffer_len "<< buffer_len << std::endl;
        if ( buffer_len != len )
        {
            PyErr_SetString( PyExc_RuntimeError, "Size mismatch of image and Buffer." );
            goto fail;
        }

        memcpy((void *)mitkBufferPtr, buffer, len );
        PyBuffer_Release( &pyBuffer );
        Py_RETURN_NONE;

        fail:
            PyBuffer_Release( &pyBuffer );
            return NULL;
    }

%}
%init %{
    import_array();
%}

%native(_SetImageFromArray) PyObject *mitk_SetImageFromArray( PyObject *(self), PyObject *args );

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

    %typemap(out) namespace ## :: ## class_name {
        std::cout << "class_name" <<std::endl;
        /*$result = SWIG_NewPointerObj(SWIG_as_voidptr($1), $1_descriptor, 1);
        if ($1) {
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

%inline{

    mitk::Image::Pointer MakeImage(mitk::PixelType pixelType, std::vector<unsigned int> shape)
    {
        mitk::Image::Pointer image = mitk::Image::New();
        image->Initialize(pixelType, shape.size(), shape.data());
        return image;
    }

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

    std::vector<mitk::Image::Pointer> GetImageVector() //for test, delete later
    {
        return {GetImage()};
    }
}

%extend mitk::Image
{
    public:
    PyObject* GetAsNumpy() 
    {
        unsigned int dim = self->GetDimension();
        mitk::ImageReadAccessor readAccess(self);
        const void* mitkBufferPtr = readAccess.GetData();
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
        return result;
    }

    %pythoncode %{
        def SayHi(self):
            print('Hi!')
    %}
}

mitk::PixelType MakePixelTypeFromTypeID(int componentTypeID, int numberOfComponents);

%ignore mitk::BaseData;
%ignore mitk::SlicedData;
%ignore mitk::Image::GetPixelType;
%ignore mitk::Image::GetChannelDescriptor;
%ignore mitk::IOUtil::Save;
%rename(imsave) mitk::IOUtil::Save(const mitk::BaseData*, const std::string&, bool);

%include <itkMacro.h>
%include <itkSmartPointer.h>
%include <mitkCommon.h>
%include <mitkPixelType.h>
%include <mitkBaseData.h>
%include <mitkSlicedData.h>
%include <mitkImage.h>
%include <mitkIOUtil.h>

//%template(VectorImageP) std::vector<mitk::Image::Pointer>;
%template(imread) mitk::IOUtil::Load<mitk::Image>;

%pythoncode %{

    HAVE_NUMPY = True
    try:
        import numpy
    except ImportError:
        HAVE_NUMPY = False


    def GetName():
        return 'pyMITK'


    def _get_mitk_pixelid(numpy_array_type):
        """Returns a MITK PixelID given a numpy array."""
        if not HAVE_NUMPY:
            raise ImportError('Numpy not available.')

        # Mapping from numpy array types to mitk pixel types.
        _np_mitk = {
                numpy.character:ComponentTypeUInt8,
                numpy.uint8:ComponentTypeUInt8,
                numpy.uint16:ComponentTypeUInt16,
                numpy.uint32:ComponentTypeUInt32,
                numpy.int8:ComponentTypeInt8,
                numpy.int16:ComponentTypeInt16,
                numpy.int32:ComponentTypeInt32,
                numpy.float32:ComponentTypeFloat,
                numpy.float64:ComponentTypeDouble
                }
        try:
            return _np_mitk[numpy_array_type.dtype]
        except KeyError:
            for key in _np_mitk:
                if numpy.issubdtype(numpy_array_type.dtype, key):
                    return _np_mitk[key]
            raise TypeError('dtype: {0} is not supported.'.format(numpy_array_type.dtype))


    def GetImageFromArray(arr: numpy.ndarray, isVector=False, isShared = False):
        """
        Get a MITK Image from a numpy array. If isVector is True, then a 3D array will be treated as a 2D vector image,
        otherwise it will be treated as a 3D image. If isShared is True, then the MITK Image and numpy array would point
        to same underlying buffer.
        """

        if not HAVE_NUMPY:
            raise ImportError('Numpy not available.')

        assert arr.ndim in ( 2, 3, 4 ), \
        "Only arrays of 2, 3 or 4 dimensions are supported."

        id = _get_mitk_pixelid(arr)        
        if (arr.ndim == 3 and isVector ) or (arr.ndim == 4):
            pixelType=MakePixelTypeFromTypeID(id, arr.shape[-1])
            newShape=VectorUInt32(arr.shape[-2::-1])
            img = MakeImage(pixelType, newShape)
        elif arr.ndim in ( 2, 3 ):
            pixelType=MakePixelTypeFromTypeID(id, 1)
            newShape=VectorUInt32(arr.shape[::-1])
            img = MakeImage(pixelType, newShape)

        _SetImageFromArray(arr.tobytes(), img)
        if isShared:
            arr.data = img.GetAsNumpy().data
        return img

%}
