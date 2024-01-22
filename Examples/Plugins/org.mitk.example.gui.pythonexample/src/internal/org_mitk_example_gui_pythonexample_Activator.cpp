/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "org_mitk_example_gui_pythonexample_Activator.h"
#include "PythonExample.h"

namespace mitk
{
  ctkPluginContext *org_mitk_example_gui_pythonexample_Activator::m_Context = nullptr;
  void org_mitk_example_gui_pythonexample_Activator::start(ctkPluginContext *context)
  {
    m_Context = context;
    BERRY_REGISTER_EXTENSION_CLASS(PythonExample, context)
  }

  void org_mitk_example_gui_pythonexample_Activator::stop(ctkPluginContext *context) { Q_UNUSED(context) }

  ctkPluginContext *org_mitk_example_gui_pythonexample_Activator::GetContext() { return m_Context; }
}
