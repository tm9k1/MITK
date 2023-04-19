/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGizmo.h"
#include "mitkGizmoInteractorCylinder.h"
#include "mitkGizmoInteractor.h"

// MITK includes
#include <mitkBaseRenderer.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkRenderingManager.h>
#include <mitkVtkInterpolationProperty.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCharArray.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyDataNormals.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTubeFilter.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkMath.h>
#include <vtkOBBTree.h>
#include <vtkQuad.h>

// ITK includes
#include <itkCommand.h>

// MicroServices
#include <usGetModuleContext.h>

namespace
{
  const char *PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY = "gizmo.originalObjectOpacity";
}

namespace mitk
{
  //! Private object, removing the Gizmo from data storage along with its manipulated object.
  class GizmoRemover
  {
  public:
    GizmoRemover() : m_Storage(nullptr), m_GizmoNode(nullptr), m_ManipulatedNode(nullptr), m_StorageObserverTag(0) {}
    //! Tell the object about the storage that contains both the nodes
    //! containing the gizmo and the manipulated object.
    //!
    //! The method sets up observation of
    //! - removal of the manipulated node from storage
    //! - destruction of storage itself
    void UpdateStorageObservation(mitk::DataStorage *storage,
                                  mitk::DataNode *gizmo_node,
                                  mitk::DataNode *manipulated_node)
    {
      if (m_Storage != nullptr)
      {
        m_Storage->RemoveNodeEvent.RemoveListener(
          mitk::MessageDelegate1<GizmoRemover, const mitk::DataNode *>(this, &GizmoRemover::OnDataNodeHasBeenRemoved));
        m_Storage->RemoveObserver(m_StorageObserverTag);
      }

      m_Storage = storage;
      m_GizmoNode = gizmo_node;
      m_ManipulatedNode = manipulated_node;

      if (m_Storage != nullptr)
      {
        m_Storage->RemoveNodeEvent.AddListener(
          mitk::MessageDelegate1<GizmoRemover, const mitk::DataNode *>(this, &GizmoRemover::OnDataNodeHasBeenRemoved));

        itk::SimpleMemberCommand<GizmoRemover>::Pointer command = itk::SimpleMemberCommand<GizmoRemover>::New();
        command->SetCallbackFunction(this, &mitk::GizmoRemover::OnDataStorageDeleted);
        m_StorageObserverTag = m_Storage->AddObserver(itk::ModifiedEvent(), command);
      }
    }

    //! Callback notified on destruction of DataStorage
    void OnDataStorageDeleted() { m_Storage = nullptr; }
    //! Callback notified on removal of _any_ object from data storage
    void OnDataNodeHasBeenRemoved(const mitk::DataNode *node)
    {
      if (node == m_ManipulatedNode)
      {
        // m_Storage is still alive because it is the emitter
        if (m_Storage->Exists(m_GizmoNode))
        {
          m_Storage->Remove(m_GizmoNode);
          // normally, gizmo will be deleted here (unless somebody
          // still holds a reference to it)
        }
      }
    }

    //! Clean up our observer registrations
    ~GizmoRemover()
    {
      if (m_Storage)
      {
        m_Storage->RemoveNodeEvent.RemoveListener(
          mitk::MessageDelegate1<GizmoRemover, const mitk::DataNode *>(this, &GizmoRemover::OnDataNodeHasBeenRemoved));
        m_Storage->RemoveObserver(m_StorageObserverTag);
      }
    }

  private:
    mitk::DataStorage *m_Storage;
    mitk::DataNode *m_GizmoNode;
    mitk::DataNode *m_ManipulatedNode;
    unsigned long m_StorageObserverTag;
  };

} // namespace MITK

bool mitk::Gizmo::HasGizmoAttached(DataNode *node, DataStorage *storage)
{
  auto typeCondition = TNodePredicateDataType<Gizmo>::New();
  auto gizmoChildren = storage->GetDerivations(node, typeCondition);
  return !gizmoChildren->empty();
}

bool mitk::Gizmo::RemoveGizmoFromNode(DataNode *node, DataStorage *storage)
{
  if (node == nullptr || storage == nullptr)
  {
    return false;
  }

  auto typeCondition = TNodePredicateDataType<Gizmo>::New();
  auto gizmoChildren = storage->GetDerivations(node, typeCondition);

  for (auto &gizmoChild : *gizmoChildren)
  {
    auto *gizmo = dynamic_cast<Gizmo *>(gizmoChild->GetData());
    if (gizmo)
    {
      storage->Remove(gizmoChild);
      gizmo->m_GizmoRemover->UpdateStorageObservation(nullptr, nullptr, nullptr);
    }
  }

  //--------------------------------------------------------------
  // Restore original opacity if we changed it
  //--------------------------------------------------------------
  float originalOpacity = 1.0;
  if (node->GetFloatProperty(PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY, originalOpacity))
  {
    node->SetOpacity(originalOpacity);
    node->GetPropertyList()->DeleteProperty(PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY);
  }

  return !gizmoChildren->empty();
}

mitk::DataNode::Pointer mitk::Gizmo::AddGizmoToNodeCylinder(DataNode *node, DataStorage *storage)
{
  assert(node);
  if (node->GetData() == nullptr || node->GetData()->GetGeometry() == nullptr)
  {
    return nullptr;
  }
  //--------------------------------------------------------------
  // Add visual gizmo that follows the node to be manipulated
  //--------------------------------------------------------------

  auto gizmo = Gizmo::New();
  auto gizmoNode = DataNode::New();
  gizmoNode->SetName("Gizmo");
  gizmoNode->SetData(gizmo);
  gizmo->FollowGeometryCylinder(node->GetData());

  //--------------------------------------------------------------
  // Add interaction to the gizmo
  //--------------------------------------------------------------

  mitk::GizmoInteractorCylinder::Pointer interactor = mitk::GizmoInteractorCylinder::New();
  interactor->LoadStateMachine("Gizmo3DStates.xml", us::GetModuleContext()->GetModule());
  interactor->SetEventConfig("Gizmo3DConfig.xml", us::ModuleRegistry::GetModule("MitkGizmo"));

  interactor->SetGizmoNode(gizmoNode);
  interactor->SetManipulatedObjectNode(node);

  //compute direction from surface
  mitk::Surface::Pointer cylinder = dynamic_cast<mitk::Surface*>(node->GetData());
  vtkSmartPointer<vtkPolyData> vtkPolyData = cylinder->GetVtkPolyData();
  mitk::Vector3D direction = ExtractOrientationFromSurface(vtkPolyData);
  Vector3D axisX, axisY, axisZ;
  ComputeOrientation(&direction, axisX, axisY, axisZ);
  interactor->SetInitialAxes(axisX, axisY, axisZ);

  //--------------------------------------------------------------
  // Note current opacity for later restore and lower it
  //--------------------------------------------------------------

  float currentNodeOpacity = 1.0;
  if (node->GetOpacity(currentNodeOpacity, nullptr))
  {
    if (currentNodeOpacity > 0.5f)
    {
      node->SetFloatProperty(PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY, currentNodeOpacity);
      node->SetOpacity(0.5f);
    }
  }

  if (storage)
  {
    storage->Add(gizmoNode, node);
    gizmo->m_GizmoRemover->UpdateStorageObservation(storage, gizmoNode, node);
  }

  return gizmoNode;
}

mitk::DataNode::Pointer mitk::Gizmo::AddGizmoToNode(DataNode *node, DataStorage *storage)
{
  assert(node);
  if (node->GetData() == nullptr || node->GetData()->GetGeometry() == nullptr)
  {
    return nullptr;
  }
  //--------------------------------------------------------------
  // Add visual gizmo that follows the node to be manipulated
  //--------------------------------------------------------------

  auto gizmo = Gizmo::New();
  auto gizmoNode = DataNode::New();
  gizmoNode->SetName("Gizmo");
  gizmoNode->SetData(gizmo);
  gizmo->FollowGeometry(node->GetData()->GetGeometry());

  //--------------------------------------------------------------
  // Add interaction to the gizmo
  //--------------------------------------------------------------

  mitk::GizmoInteractor::Pointer interactor = mitk::GizmoInteractor::New();
  interactor->LoadStateMachine("Gizmo3DStates.xml", us::GetModuleContext()->GetModule());
  interactor->SetEventConfig("Gizmo3DConfig.xml", us::ModuleRegistry::GetModule("MitkGizmo"));

  interactor->SetGizmoNode(gizmoNode);
  interactor->SetManipulatedObjectNode(node);

  //--------------------------------------------------------------
  // Note current opacity for later restore and lower it
  //--------------------------------------------------------------

  float currentNodeOpacity = 1.0;
  if (node->GetOpacity(currentNodeOpacity, nullptr))
  {
    if (currentNodeOpacity > 0.5f)
    {
      node->SetFloatProperty(PROPERTY_KEY_ORIGINAL_OBJECT_OPACITY, currentNodeOpacity);
      node->SetOpacity(0.5f);
    }
  }

  if (storage)
  {
    storage->Add(gizmoNode, node);
    gizmo->m_GizmoRemover->UpdateStorageObservation(storage, gizmoNode, node);
  }

  return gizmoNode;
}

mitk::Gizmo::Gizmo()
  : Surface(), m_AllowTranslation(true), m_AllowRotation(true), m_AllowScaling(true), m_GizmoRemover(new GizmoRemover())
{
  m_AllowRotationX = true;
  m_AllowRotationY = false;
  m_AllowRotationZ = true;
  m_AllowTranslation = true;
  m_AllowScaling = false;

  m_Center.Fill(0);

  m_AxisX.Fill(0);
  m_AxisX[0] = 1;
  m_AxisY.Fill(0);
  m_AxisY[1] = 1;
  m_AxisZ.Fill(0);
  m_AxisZ[2] = 1;

  m_Radius.Fill(1);

  UpdateRepresentation();
}

mitk::Gizmo::~Gizmo()
{
  if (m_FollowedGeometry.IsNotNull())
  {
    m_FollowedGeometry->RemoveObserver(m_FollowerTag);
  }
}


void mitk::Gizmo::ComputeOrientation(mitk::Vector3D* direction, Vector3D &axisX, Vector3D &axisY, Vector3D &axisZ)
{
    axisY = *direction;

    vtkSmartPointer<vtkMinimalStandardRandomSequence> rng =
        vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
    rng->SetSeed(8775070); // For testing.


    // The Z axis is an arbitrary vector cross X
    double arbitrary[3];
    for (auto i = 0; i < 3; ++i)
    {
      rng->Next();
      arbitrary[i] = rng->GetRangeValue(-10, 10);
    }
    vtkMath::Cross(&axisY[0], &arbitrary[0], &axisX[0]);


    // The Y axis is Z cross X
    vtkMath::Cross(&axisX[0], &axisY[0], &axisZ[0]);

    vtkMath::Normalize(&axisY[0]);
    vtkMath::Normalize(&axisX[0]);
    vtkMath::Normalize(&axisZ[0]);
}

void mitk::Gizmo::UpdateRepresentation()
{
  /* bounding box around the unscaled bounding object */
  ScalarType bounds[6] = {-m_Radius[0] * 1.2,
                          +m_Radius[0] * 1.2,
                          -m_Radius[1] * 1.2,
                          +m_Radius[1] * 1.2,
                          -m_Radius[2] * 1.2,
                          +m_Radius[2] * 1.2};
  GetGeometry()->SetBounds(bounds);
  GetTimeGeometry()->Update();

  SetVtkPolyData(BuildGizmo());
}

namespace
{
  void AssignScalarValueTo(vtkPolyData *polydata, char value)
  {
    vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();

    int numberOfPoints = polydata->GetNumberOfPoints();
    pointData->SetNumberOfComponents(1);
    pointData->SetNumberOfTuples(numberOfPoints);
    pointData->FillComponent(0, value);
    polydata->GetPointData()->SetScalars(pointData);
  }

  vtkSmartPointer<vtkPolyData> BuildAxis(const mitk::Point3D &center,
                                         const mitk::Vector3D &axis,
                                         double halflength,
                                         bool drawRing,
                                         char vertexValueAxis,
                                         char vertexValueRing,
                                         char vertexValueScale)
  {
    // Define all sizes relative to absolute size (thus that the gizmo will appear
    // in the same relative size for huge (size >> 1) and tiny (size << 1) objects).
    // This means that the gizmo will appear very different when a scene contains _both_
    // huge and tiny objects at the same time, but when the users zooms in on his
    // object of interest, the gizmo will always have the same relative size.
    const double shaftRadius = halflength * 0.02;
    const double arrowHeight = shaftRadius * 6;
    const int tubeSides = 15;

    // poly data appender to collect cones and tube that make up the axis
    vtkSmartPointer<vtkAppendPolyData> axisSource = vtkSmartPointer<vtkAppendPolyData>::New();

    // build two cones at the end of axis
    for (double sign = -1.0; sign < 3.0; sign += 2)
    {
      vtkSmartPointer<vtkConeSource> cone = vtkConeSource::New();
      // arrow tips at 110% of radius
      cone->SetCenter(center[0] + sign * axis[0] * (halflength * 1.1 + arrowHeight * 0.5),
                      center[1] + sign * axis[1] * (halflength * 1.1 + arrowHeight * 0.5),
                      center[2] + sign * axis[2] * (halflength * 1.1 + arrowHeight * 0.5));
      cone->SetDirection(sign * axis[0], sign * axis[1], sign * axis[2]);
      cone->SetRadius(shaftRadius * 3);
      cone->SetHeight(arrowHeight);
      cone->SetResolution(tubeSides);
      cone->CappingOn();
      cone->Update();
      AssignScalarValueTo(cone->GetOutput(), vertexValueScale);
      axisSource->AddInputData(cone->GetOutput());
    }

    // build the axis itself (as a tube around the line defining the axis)
    vtkSmartPointer<vtkPolyData> shaftSkeleton = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> shaftPoints = vtkSmartPointer<vtkPoints>::New();
    shaftPoints->InsertPoint(0, (center - axis * halflength * 1.1).GetDataPointer());
    shaftPoints->InsertPoint(1, (center + axis * halflength * 1.1).GetDataPointer());
    shaftSkeleton->SetPoints(shaftPoints);

    vtkSmartPointer<vtkCellArray> shaftLines = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType shaftLinePoints[] = {0, 1};
    shaftLines->InsertNextCell(2, shaftLinePoints);
    shaftSkeleton->SetLines(shaftLines);

    vtkSmartPointer<vtkTubeFilter> shaftSource = vtkSmartPointer<vtkTubeFilter>::New();
    shaftSource->SetInputData(shaftSkeleton);
    shaftSource->SetNumberOfSides(tubeSides);
    shaftSource->SetVaryRadiusToVaryRadiusOff();
    shaftSource->SetRadius(shaftRadius);
    shaftSource->Update();
    AssignScalarValueTo(shaftSource->GetOutput(), vertexValueAxis);

    axisSource->AddInputData(shaftSource->GetOutput());
    axisSource->Update();

    vtkSmartPointer<vtkTubeFilter> ringSource; // used after if block, so declare it here
    if (drawRing)
    {
      // build the ring orthogonal to the axis (as another tube)
      vtkSmartPointer<vtkPolyData> ringSkeleton = vtkSmartPointer<vtkPolyData>::New();
      vtkSmartPointer<vtkPoints> ringPoints = vtkSmartPointer<vtkPoints>::New();
      ringPoints->SetDataTypeToDouble(); // just some decision (see cast below)
      unsigned int numberOfRingPoints = 100;
      vtkSmartPointer<vtkCellArray> ringLines = vtkSmartPointer<vtkCellArray>::New();
      ringLines->InsertNextCell(numberOfRingPoints + 1);
      mitk::Vector3D ringPointer;
      for (unsigned int segment = 0; segment < numberOfRingPoints; ++segment)
      {
        ringPointer[0] = 0;
        ringPointer[1] = std::cos((double)(segment) / (double)numberOfRingPoints * 2.0 * vtkMath::Pi());
        ringPointer[2] = std::sin((double)(segment) / (double)numberOfRingPoints * 2.0 * vtkMath::Pi());

        ringPoints->InsertPoint(segment, (ringPointer * halflength).GetDataPointer());

        ringLines->InsertCellPoint(segment);
      }
      ringLines->InsertCellPoint(0);

      // transform ring points (copied from vtkConeSource)
      vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
      t->Translate(center.GetDataPointer());
      double vMag = vtkMath::Norm(axis.GetDataPointer());
      if (axis[0] < 0.0)
      {
        // flip x -> -x to avoid instability
        t->RotateWXYZ(180.0, (axis[0] - vMag) / 2.0, axis[1] / 2.0, axis[2] / 2.0);
        t->RotateWXYZ(180.0, 0, 1, 0);
      }
      else
      {
        t->RotateWXYZ(180.0, (axis[0] + vMag) / 2.0, axis[1] / 2.0, axis[2] / 2.0);
      }

      double thisPoint[3];
      for (unsigned int i = 0; i < numberOfRingPoints; ++i)
      {
        ringPoints->GetPoint(i, thisPoint);
        t->TransformPoint(thisPoint, thisPoint);
        ringPoints->SetPoint(i, thisPoint);
      }

      ringSkeleton->SetPoints(ringPoints);
      ringSkeleton->SetLines(ringLines);

      ringSource = vtkSmartPointer<vtkTubeFilter>::New();
      ringSource->SetInputData(ringSkeleton);
      ringSource->SetNumberOfSides(tubeSides);
      ringSource->SetVaryRadiusToVaryRadiusOff();
      ringSource->SetRadius(shaftRadius);
      ringSource->Update();
      AssignScalarValueTo(ringSource->GetOutput(), vertexValueRing);
    }

    // assemble axis and ring
    vtkSmartPointer<vtkAppendPolyData> appenderGlobal = vtkSmartPointer<vtkAppendPolyData>::New();
    appenderGlobal->AddInputData(axisSource->GetOutput());
    if (drawRing)
    {
      appenderGlobal->AddInputData(ringSource->GetOutput());
    }
    appenderGlobal->Update();

    // make everything shiny by adding normals
    vtkSmartPointer<vtkPolyDataNormals> normalsSource = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalsSource->SetInputConnection(appenderGlobal->GetOutputPort());
    normalsSource->ComputePointNormalsOn();
    normalsSource->ComputeCellNormalsOff();
    normalsSource->SplittingOn();
    normalsSource->Update();

    vtkSmartPointer<vtkPolyData> result = normalsSource->GetOutput();
    return result;
  }

} // unnamed namespace

double mitk::Gizmo::GetLongestRadius() const
{
  double longestAxis = std::max(m_Radius[0], m_Radius[1]);
  longestAxis = std::max(longestAxis, m_Radius[2]);
  return longestAxis;
}

vtkSmartPointer<vtkPolyData> mitk::Gizmo::BuildGizmo()
{
  double longestAxis = GetLongestRadius();

  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  MITK_INFO << "Scaling/Translation: " << m_AllowScaling << m_AllowTranslation;

  appender->AddInputData(BuildAxis(m_Center,
                                   m_AxisX,
                                   longestAxis,
                                   m_AllowRotationX,
                                   m_AllowTranslation ? MoveAlongAxisX : NoHandle,
                                   m_AllowRotationX ? RotateAroundAxisX : NoHandle,
                                   m_AllowScaling ? ScaleX : NoHandle));
  appender->AddInputData(BuildAxis(m_Center,
                                   m_AxisY,
                                   longestAxis,
                                   m_AllowRotationY,
                                   m_AllowTranslation ? MoveAlongAxisY : NoHandle,
                                   m_AllowRotationY ? RotateAroundAxisY : NoHandle,
                                   m_AllowScaling ? ScaleY : NoHandle));
  appender->AddInputData(BuildAxis(m_Center,
                                   m_AxisZ,
                                   longestAxis,
                                   m_AllowRotationZ,
                                   m_AllowTranslation ? MoveAlongAxisZ : NoHandle,
                                   m_AllowRotationZ ? RotateAroundAxisZ : NoHandle,
                                   m_AllowScaling ? ScaleZ : NoHandle));

  auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(m_Center[0], m_Center[1], m_Center[2]);
  sphereSource->SetRadius(longestAxis * 0.06);
  sphereSource->Update();
  AssignScalarValueTo(sphereSource->GetOutput(), MoveFreely);

  appender->AddInputData(sphereSource->GetOutput());

  appender->Update();
  return appender->GetOutput();
}

void mitk::Gizmo::FollowGeometry(BaseGeometry *geom)
{
  auto observer = itk::SimpleMemberCommand<Gizmo>::New();
  observer->SetCallbackFunction(this, &Gizmo::OnFollowedGeometryModified);

  if (m_FollowedGeometry.IsNotNull())
  {
    m_FollowedGeometry->RemoveObserver(m_FollowerTag);
  }

  m_FollowedGeometry = geom;
  m_FollowerTag = m_FollowedGeometry->AddObserver(itk::ModifiedEvent(), observer);

  // initial adjustment
  OnFollowedGeometryModified();
}

void mitk::Gizmo::FollowGeometryCylinder(BaseData *basedata)
{
  auto observer = itk::SimpleMemberCommand<Gizmo>::New();
  observer->SetCallbackFunction(this, &Gizmo::OnFollowedGeometryCylinderModified);

  if (m_FollowedGeometry.IsNotNull())
  {
    m_FollowedGeometry->RemoveObserver(m_FollowerTag);
  }

  m_FollowedGeometry = basedata->GetGeometry();

  m_FollowerTag = m_FollowedGeometry->AddObserver(itk::ModifiedEvent(), observer);


  // initial adjustment
  mitk::Surface::Pointer cylinder = dynamic_cast<mitk::Surface*>(basedata);
  vtkSmartPointer<vtkPolyData> vtkPolyData = cylinder->GetVtkPolyData();
  mitk::Vector3D direction = ExtractOrientationFromSurface(vtkPolyData);
  ComputeOrientation(&direction, m_AxisXorig, m_AxisYorig, m_AxisZorig);
  OnFollowedGeometryCylinderModified();
}


void mitk::Gizmo::OnFollowedGeometryCylinderModified()
{
  m_Center = m_FollowedGeometry->GetCenter();

  mitk::AffineTransform3D *affine = m_FollowedGeometry->GetIndexToWorldTransform();

  m_AxisX = affine->TransformVector(m_AxisXorig);
  m_AxisY = affine->TransformVector(m_AxisYorig);
  m_AxisZ = affine->TransformVector(m_AxisZorig);
  m_AxisX.Normalize();
  m_AxisY.Normalize();
  m_AxisZ.Normalize();

  for (int dim = 0; dim < 3; ++dim)
  {
    m_Radius[dim] = 0.5 * m_FollowedGeometry->GetExtentInMM(dim);
  }

  UpdateRepresentation();
}

void mitk::Gizmo::OnFollowedGeometryModified()
{
  m_Center = m_FollowedGeometry->GetCenter();

  m_AxisX = m_FollowedGeometry->GetAxisVector(0);
  m_AxisY = m_FollowedGeometry->GetAxisVector(1);
  m_AxisZ = m_FollowedGeometry->GetAxisVector(2);

  m_AxisX.Normalize();
  m_AxisY.Normalize();
  m_AxisZ.Normalize();

  for (int dim = 0; dim < 3; ++dim)
  {
    m_Radius[dim] = 0.5 * m_FollowedGeometry->GetExtentInMM(dim);
  }

  UpdateRepresentation();
}

double mitk::Gizmo::MakeAQuad(std::vector<std::array<double, 3>> points,
                 std::array<double, 3>& center)
{
  vtkNew<vtkQuad> aQuad;
  aQuad->GetPoints()->SetPoint(0, points[0].data());
  aQuad->GetPoints()->SetPoint(1, points[1].data());
  aQuad->GetPoints()->SetPoint(2, points[2].data());
  aQuad->GetPoints()->SetPoint(3, points[3].data());
  aQuad->GetPointIds()->SetId(0, 0);
  aQuad->GetPointIds()->SetId(1, 1);
  aQuad->GetPointIds()->SetId(2, 2);
  aQuad->GetPointIds()->SetId(3, 3);

  std::array<double, 3> pcenter;
  pcenter[0] = pcenter[1] = pcenter[2] = -12345.0;
  aQuad->GetParametricCenter(pcenter.data());
  std::vector<double> cweights(aQuad->GetNumberOfPoints());
  int pSubId = 0;
  aQuad->EvaluateLocation(pSubId, pcenter.data(), center.data(),
                          &(*cweights.begin()));

  return std::sqrt(aQuad->GetLength2()) / 2.0;
}

mitk::Vector3D mitk::Gizmo::ExtractOrientationFromSurface(vtkSmartPointer<vtkPolyData> polyData)
{


    //https://kitware.github.io/vtk-examples/site/Cxx/PolyData/OrientedBoundingCylinder/
    // Get bounds of polydata
    std::array<double, 6> bounds;
    polyData->GetBounds(bounds.data());

    // Create the tree
    vtkNew<vtkOBBTree> obbTree;
    obbTree->SetDataSet(polyData);
    obbTree->SetMaxLevel(1);
    obbTree->BuildLocator();

    // Get the PolyData for the OBB
    vtkNew<vtkPolyData> obbPolydata;
    obbTree->GenerateRepresentation(0, obbPolydata);

    // Get the points of the OBB
    vtkNew<vtkPoints> obbPoints;
    obbPoints->DeepCopy(obbPolydata->GetPoints());

    // Use a quad to find centers of OBB faces
    vtkNew<vtkQuad> aQuad;

    std::vector<std::array<double, 3>> facePoints(4);
    std::vector<std::array<double, 3>> centers(3);
    std::vector<std::array<double, 3>> endPoints(3);

    std::array<double, 3> center;
    std::array<double, 3> endPoint;
    std::array<double, 3> point0, point1, point2, point3, point4, point5, point6,
          point7;
    std::array<double, 3> radii;
    std::array<double, 3> lengths;

    // Transfer the points to std::array's
    obbPoints->GetPoint(0, point0.data());
    obbPoints->GetPoint(1, point1.data());
    obbPoints->GetPoint(2, point2.data());
    obbPoints->GetPoint(3, point3.data());
    obbPoints->GetPoint(4, point4.data());
    obbPoints->GetPoint(5, point5.data());
    obbPoints->GetPoint(6, point6.data());
    obbPoints->GetPoint(7, point7.data());

    // x face
    // ids[0] = 2; ids[1] = 3; ids[2] = 7; ids[3] = 6;
    facePoints[0] = point2;
    facePoints[1] = point3;
    facePoints[2] = point7;
    facePoints[3] = point6;
    radii[0] = MakeAQuad(facePoints, centers[0]);
    MakeAQuad(facePoints, centers[0]);
    // ids[0] = 0; ids[1] = 4; ids[2] = 5; ids[3] = 1;
    facePoints[0] = point0;
    facePoints[1] = point4;
    facePoints[2] = point5;
    facePoints[3] = point1;
    MakeAQuad(facePoints, endPoints[0]);
    lengths[0] = std::sqrt(vtkMath::Distance2BetweenPoints(centers[0].data(),
                           endPoints[0].data())) / 2.0;

    // y face
    // ids[0] = 0; ids[1] = 1; ids[2] = 2; ids[3] = 3;
    facePoints[0] = point0;
    facePoints[1] = point1;
    facePoints[2] = point2;
    facePoints[3] = point3;
    radii[1] = MakeAQuad(facePoints, centers[1]);
    // ids[0] = 4; ids[1] = 6; ids[2] = 7; ids[3] = 5;
    facePoints[0] = point4;
    facePoints[1] = point6;
    facePoints[2] = point7;
    facePoints[3] = point5;
    MakeAQuad(facePoints, endPoints[1]);
    lengths[1] = std::sqrt(vtkMath::Distance2BetweenPoints(centers[1].data(),
                                                           endPoints[1].data())) / 2.0;

    // z face
    // ids[0] = 0; ids[1] = 2; ids[2] = 6; ids[3] = 4;
    facePoints[0] = point0;
    facePoints[1] = point2;
    facePoints[2] = point6;
    facePoints[3] = point4;
    MakeAQuad(facePoints, centers[2]);
    radii[2] =
          std::sqrt(vtkMath::Distance2BetweenPoints(point0.data(), point2.data())) /
          2.0;
    // ids[0] = 1; ids[1] = 3; ids[2] = 7; ids[3] = 5;
    facePoints[0] = point1;
    facePoints[1] = point5;
    facePoints[2] = point7;
    facePoints[3] = point3;
    MakeAQuad(facePoints, endPoints[2]);
    lengths[2] = std::sqrt(vtkMath::Distance2BetweenPoints(centers[2].data(),
                                                           endPoints[2].data())) / 2.0;

    // Find long axis
    int longAxis = -1;
    double length = VTK_DOUBLE_MIN;
    for (auto i = 0; i < 3; ++i)
    {
        if (lengths[i] > length)
        {
          length = lengths[i];
          longAxis = i;
        }
    }
    //double radius = radii[longAxis];
    center = centers[longAxis];
    endPoint = endPoints[longAxis];

    //check assignment of head & axis point
    mitk::Point3D axis_point;
    mitk::Point3D head;
    if(endPoint[1]>center[1])
    {
        axis_point = static_cast<mitk::Point3D>(endPoint);
        head = static_cast<mitk::Point3D>(center);
    }
    else
    {
        head = static_cast<mitk::Point3D>(endPoint);
        axis_point = static_cast<mitk::Point3D>(center);
    }

    // The X axis is a vector from start to end
    mitk::Vector3D direction;
    vtkMath::Subtract(axis_point, head, direction);
    vtkMath::Norm(&direction[0]);


    return direction;

}

mitk::Gizmo::HandleType mitk::Gizmo::GetHandleFromPointDataValue(double value)
{
#define CheckHandleType(type)                                                                                          \
  if (static_cast<int>(value) == static_cast<int>(type))                                                               \
    return type;

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  CheckHandleType(ScaleX);
  CheckHandleType(ScaleY);
  CheckHandleType(ScaleZ);
  return NoHandle;
#undef CheckHandleType
}

mitk::Gizmo::HandleType mitk::Gizmo::GetHandleFromPointID(vtkIdType id)
{
  assert(GetVtkPolyData());
  assert(GetVtkPolyData()->GetPointData());
  assert(GetVtkPolyData()->GetPointData()->GetScalars());
  double dataValue = GetVtkPolyData()->GetPointData()->GetScalars()->GetTuple1(id);
  return GetHandleFromPointDataValue(dataValue);
}

std::string mitk::Gizmo::HandleTypeToString(HandleType type)
{
#define CheckHandleType(candidateType)                                                                                 \
  if (type == candidateType)                                                                                           \
    return std::string(#candidateType);

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  CheckHandleType(ScaleX);
  CheckHandleType(ScaleY);
  CheckHandleType(ScaleZ);
  CheckHandleType(NoHandle);
  return "InvalidHandleType";
#undef CheckHandleType
}
