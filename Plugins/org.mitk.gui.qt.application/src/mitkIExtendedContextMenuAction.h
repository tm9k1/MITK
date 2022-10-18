/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIEXTENDEDCONTEXTMENUACTION_H
#define MITKIEXTENDEDCONTEXTMENUACTION_H

#include <mitkIContextMenuAction.h>

namespace mitk
{
  /**
  * An extension of the IContextMenuAction for the extension point mechanism.
  */
  struct IExtendedContextMenuAction : public IContextMenuAction
  {
    virtual void SetSmoothed(bool smoothed) = 0;
    virtual void SetDecimated(bool decimated) = 0;
  };
}

Q_DECLARE_INTERFACE(mitk::IExtendedContextMenuAction, "org.mitk.datamanager.IExtendedContextMenuAction")

#endif // MITKIEXTENDEDCONTEXTMENUACTION_H
