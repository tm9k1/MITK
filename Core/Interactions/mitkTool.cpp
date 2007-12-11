/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkTool.h"
#include <itkObjectFactory.h>

mitk::Tool::Tool(const char* type)
:StateMachine(type)
{
}

mitk::Tool::~Tool()
{
}
    
const char* mitk::Tool::GetGroup() const
{
  return "default";
}

void mitk::Tool::SetToolManager(ToolManager* manager)
{
  m_ToolManager = manager;
}

void mitk::Tool::Activated()
{
}

void mitk::Tool::Deactivated()
{
  StateMachine::ResetStatemachineToStartState(); // forget about the past
}
    
itk::Object::Pointer mitk::Tool::GetGUI(const std::string& toolkitPrefix, const std::string& toolkitPostfix)
{
  itk::Object::Pointer object;

  std::string classname = this->GetNameOfClass();
  std::string guiClassname = toolkitPrefix + classname + toolkitPostfix;

  std::list<itk::LightObject::Pointer> allGUIs = itk::ObjectFactoryBase::CreateAllInstance(guiClassname.c_str());
  for( std::list<itk::LightObject::Pointer>::iterator iter = allGUIs.begin();
       iter != allGUIs.end();
       ++iter )
  {
    if (object.IsNull())
    {
      object = dynamic_cast<itk::Object*>( iter->GetPointer() );
    }
    else
    {
      std::cerr << "ERROR: There is more than one GUI for " << classname << " (several factories claim ability to produce a " << guiClassname << " ) " << std::endl;
      return NULL; // people should see and fix this error
    }
  }

  return object;
}

//--------------------------------------------------------------------------------
//----          ToolLogger
//--------------------------------------------------------------------------------

unsigned int mitk::Tool::ToolLogger::s_Verboseness = 0;
mitk::Tool::ToolLogger::NullStream mitk::Tool::ToolLogger::s_NullStream;

std::ostream& mitk::Tool::ToolLogger::Logger(unsigned int verboseness)
{
  if (verboseness <= s_Verboseness)
  {
    return std::cout;
  }
  else
  {
    return s_NullStream;
  }
}

unsigned int mitk::Tool::ToolLogger::GetVerboseness()
{
  return s_Verboseness;
}

void mitk::Tool::ToolLogger::SetVerboseness( unsigned int verboseness )
{
  s_Verboseness = verboseness;
}

