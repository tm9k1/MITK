#ifndef MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F
#define MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F

#include "mitkCommon.h"
#include "mitkInteractor.h"
#include "mitkVector.h"

namespace mitk {
class DisplayPositionEvent;

//##Documentation
//## @brief Interactor for Affine transformations translate, rotate and scale
//## @ingroup Interaction
//##
//## An object of this class can translate, rotate and scale the data objects
//## by modifying its geometry.
class AffineInteractor : public Interactor
{
public:
  mitkClassMacro(AffineInteractor,Interactor);

  //##Documentation
  //## @brief Constructor
  //##
  //## @params dataTreeNode is the node, this Interactor is connected to
  //## type is the type of StateMachine like declared in the XML-Configure-File
  //##
  AffineInteractor(const char * type, DataTreeNode* dataTreeNode);

  //##Documentation
  //## @brief Destructor
  ~AffineInteractor(){};

protected:

  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  bool CheckSelected(const mitk::Point3D& worldPoint);
  bool ConvertDisplayEventToWorldPosition(mitk::DisplayPositionEvent const* displayEvent, mitk::Point3D& worldPoint);

  mitk::Point3D m_LastMousePosition;
};

} // namespace mitk

#endif /* MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F */
