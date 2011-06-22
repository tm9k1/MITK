/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkFiberBundleInteractor.h"
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include <mitkOperationEvent.h>
//#include "mitkStatusBar.h"
#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <mitkInteractionConst.h>
#include <mitkAction.h>
#include <mitkProperties.h>
#include <vtkLinearTransform.h>
#include <mitkUndoController.h>
#include <mitkStateEvent.h>
#include <mitkState.h>
#include <mitkFiberBundle.h>
#include "mitkBaseRenderer.h"
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkInteractorObserver.h>

mitk::FiberBundleInteractor::FiberBundleInteractor(const char * type, DataNode* dataNode)
  : Interactor(type, dataNode), m_LastPosition(0)
{
  m_LastPoint.Fill(0);
}

mitk::FiberBundleInteractor::~FiberBundleInteractor()
{}

void mitk::FiberBundleInteractor::SelectFiber(int position)
{
  MITK_INFO << "mitk::FiberBundleInteractor::SelectFiber " << position;

  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if (pointSet == NULL)
    return;
  if (pointSet->GetSize()<=0)//if List is empty, then no select of a point can be done!
    return;

  mitk::Point3D noPoint;//dummyPoint... not needed anyway
  noPoint.Fill(0);
  mitk::PointOperation* doOp = new mitk::PointOperation(OpSELECTPOINT, noPoint, position);
  if (m_UndoEnabled)
  {
    mitk::PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT, noPoint, position);
    OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }
  pointSet->ExecuteOperation(doOp);
}

void mitk::FiberBundleInteractor::DeselectAllFibers()
{
  MITK_INFO << "mitk::FiberBundleInteractor::DeselectAllFibers ";

  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if (pointSet == NULL)
    return;

  mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet();
  mitk::PointSet::PointsContainer::Iterator it, end;
  end = itkPointSet->GetPoints()->End();

  for (it = itkPointSet->GetPoints()->Begin(); it != end; it++)
  {
    int position = it->Index();
    PointSet::PointDataType pointData = {0, false, PTUNDEFINED};
    itkPointSet->GetPointData(position, &pointData);
    if ( pointData.selected )//then declare an operation which unselects this point; UndoOperation as well!
    {
      mitk::Point3D noPoint;
      noPoint.Fill(0);

      mitk::PointOperation* doOp = new mitk::PointOperation(OpDESELECTPOINT, noPoint, position);
      if (m_UndoEnabled)
      {
        mitk::PointOperation* undoOp = new mitk::PointOperation(OpSELECTPOINT, noPoint, position);
        OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      pointSet->ExecuteOperation(doOp);
    }
  }
}

float mitk::FiberBundleInteractor::CanHandleEvent(StateEvent const* stateEvent) const
    //go through all points and check, if the given Point lies near a line
{
  MITK_INFO << "mitk::FiberBundleInteractor::CanHandleEvent ";

  float returnValue = 0;

  mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
  //checking if a keyevent can be handled:
  if (posEvent == NULL)
  {
    //check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0;
    }
  }

  //Mouse event handling:
  //on MouseMove do nothing! reimplement if needed differently
  if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  {
    return 0;
  }

  //if the event can be understood and if there is a transition waiting for that event
  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  {
    returnValue = 0.5;//it can be understood
  }

  //check on the right data-type
  mitk::FiberBundle* bundle = dynamic_cast<mitk::FiberBundle*>(m_DataNode->GetData());
  if (bundle == NULL)
    return 0;

  return 0.5;

}


bool mitk::FiberBundleInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  MITK_INFO << "mitk::FiberBundleInteractor::ExecuteAction ";

  bool ok = false;//for return type bool

  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::FiberBundle* bundle = dynamic_cast<mitk::FiberBundle*>(m_DataNode->GetData());
  if (bundle == NULL)
    return false;

  // Get Event and extract renderer
  const Event *event = stateEvent->GetEvent();
  BaseRenderer *renderer = NULL;
  vtkRenderWindow *renderWindow = NULL;
  vtkRenderWindowInteractor *renderWindowInteractor = NULL;
  vtkRenderer *currentVtkRenderer = NULL;
  vtkCamera *camera = NULL;

  if ( event != NULL )
  {
    renderer = event->GetSender();
    if ( renderer != NULL )
    {
      renderWindow = renderer->GetRenderWindow();
      if ( renderWindow != NULL )
      {
        renderWindowInteractor = renderWindow->GetInteractor();
        if ( renderWindowInteractor != NULL )
        {
          currentVtkRenderer = renderWindowInteractor
                               ->GetInteractorStyle()->GetCurrentRenderer();
          if ( currentVtkRenderer != NULL )
          {
            camera = currentVtkRenderer->GetActiveCamera();
          }
        }
      }
    }
  }

  // Check if we have a DisplayPositionEvent
  const DisplayPositionEvent *dpe =
      dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );
  if ( dpe != NULL )
  {
    m_CurrentPickedPoint = dpe->GetWorldPosition();
    m_CurrentPickedDisplayPoint = dpe->GetDisplayPosition();
  }

  MITK_INFO << "FiberBundleInteractor Got Action " << action->GetActionId();
  /*Each case must watch the type of the event!*/
  switch (action->GetActionId())
  {
  case AcCHECKHOVERING:
    {
      MITK_INFO << "FiberBundleInteractor AcCHECKOBJECT";

      // Re-enable VTK interactor (may have been disabled previously)
      if ( renderWindowInteractor != NULL )
      {
        renderWindowInteractor->Enable();
      }

      // Check if we have a DisplayPositionEvent
      const DisplayPositionEvent *dpe =
          dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );
      if ( dpe == NULL )
      {
        ok = true;
        break;
      }

      // Check if an object is present at the current mouse position
      DataNode *pickedNode = dpe->GetPickedObjectNode();
      StateEvent *newStateEvent;
      if ( pickedNode == m_DataNode )
      {
        // Yes: object will be selected
        MITK_INFO << "YES Hovering";
        newStateEvent = new StateEvent( EIDYES );
      }
      else
      {
        // No: back to start state
        MITK_INFO << "NO Hovering";
        newStateEvent = new StateEvent( EIDNO );
      }

      this->HandleEvent( newStateEvent );

      ok = true;
      break;
    }
    break;
  case AcSELECTPICKEDOBJECT:
    MITK_INFO << "FiberBundleInteractor AcSELECTPICKEDOBJECT";
    break;
  case AcDESELECTALL:
    MITK_INFO << "FiberBundleInteractor AcDESELECTALL";
    break;
  case AcREMOVE:
    MITK_INFO << "FiberBundleInteractor AcREMOVE";
    break;
  default:
    MITK_INFO << "FiberBundleInteractor NO ACTION";
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;

}


