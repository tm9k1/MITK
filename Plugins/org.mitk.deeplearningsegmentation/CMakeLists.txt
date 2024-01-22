project(org_mitk_deeplearningsegmentation)

mitk_create_plugin(
  EXPORT_DIRECTIVE DEEPLEARNINGSEGMENTATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkDeepLearningSegmentation
)
