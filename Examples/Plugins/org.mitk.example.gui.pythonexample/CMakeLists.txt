project(org_mitk_example_gui_pythonexample)

mitk_create_plugin(
  EXPORT_DIRECTIVE PYTHONEXAMPLE_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkPython
)
