mitk_create_module(Blosc2IO
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS PRIVATE Blosc2 msgpack-cxx
  AUTOLOAD_WITH MitkCore
)
