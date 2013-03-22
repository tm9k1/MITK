/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#if !defined(QMITK_SurfaceUtilities_H__INCLUDED)
#define QMITK_SurfaceUtilities_H__INCLUDED

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkSurfaceUtilitiesControls.h"

/*!
  \brief TODO
  */
class QmitkSurfaceUtilities : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkSurfaceUtilities();
    ~QmitkSurfaceUtilities();

    virtual void CreateQtPartControl(QWidget *parent);

    void SetFocus();

  protected slots:


  protected:



    Ui::QmitkSurfaceUtilitiesControls m_Controls;

};
#endif // !defined(QMITK_ISOSURFACE_H__INCLUDED)
