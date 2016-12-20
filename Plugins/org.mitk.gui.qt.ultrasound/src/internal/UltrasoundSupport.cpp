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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

//Mitk
#include <mitkDataNode.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIRenderingManager.h>
#include <mitkImageGenerator.h>
#include <mitkImageReadAccessor.h>
#include <mitkRenderingManager.h>
#include <mitkTextOverlay3D.h>
#include <mitkOverlay2DLayouter.h>
#include <mitkTextOverlay2D.h>
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "mitkScaleLegendOverlay.h"
#include <QApplication>
#include <QHBoxLayout>

// Qmitk
#include "UltrasoundSupport.h"

// Qt
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

// Ultrasound
#include "mitkUSDevice.h"
#include "QmitkUSAbstractCustomWidget.h"

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include "usServiceReference.h"
#include "internal/org_mitk_gui_qt_ultrasound_Activator.h"

const std::string UltrasoundSupport::VIEW_ID = "org.mitk.views.ultrasoundsupport";

void UltrasoundSupport::SetFocus()
{
}

void UltrasoundSupport::CreateQtPartControl(QWidget *parent)
{
  //initialize timers
  m_UpdateTimer = new QTimer(this);
  m_RenderingTimer2d = new QTimer(this);
  m_RenderingTimer3d = new QTimer(this);

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  //load persistence data before connecting slots (so no slots are called in this phase...)
  LoadUISettings();

  //connect signals and slots...
  connect(m_Controls.m_DeviceManagerWidget, SIGNAL(NewDeviceButtonClicked()), this, SLOT(OnClickedAddNewDevice())); // Change Widget Visibilities
  connect(m_Controls.m_DeviceManagerWidget, SIGNAL(NewDeviceButtonClicked()), this->m_Controls.m_NewVideoDeviceWidget, SLOT(CreateNewDevice())); // Init NewDeviceWidget
  connect(m_Controls.m_ActiveVideoDevices, SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(OnChangedActiveDevice()));
  connect(m_Controls.m_RunImageTimer, SIGNAL(clicked()), this, SLOT(OnChangedActiveDevice()));
  connect(m_Controls.m_ShowImageStream, SIGNAL(clicked()), this, SLOT(OnChangedActiveDevice()));
  connect(m_Controls.m_NewVideoDeviceWidget, SIGNAL(Finished()), this, SLOT(OnNewDeviceWidgetDone())); // After NewDeviceWidget finished editing
  connect(m_Controls.m_FrameRatePipeline, SIGNAL(valueChanged(int)), this, SLOT(OnChangedFramerateLimit()));
  connect(m_Controls.m_FrameRate2d, SIGNAL(valueChanged(int)), this, SLOT(OnChangedFramerateLimit()));
  connect(m_Controls.m_FrameRate3d, SIGNAL(valueChanged(int)), this, SLOT(OnChangedFramerateLimit()));
  connect(m_Controls.m_FreezeButton, SIGNAL(clicked()), this, SLOT(OnClickedFreezeButton()));
  connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateImage()));
  connect(m_RenderingTimer2d, SIGNAL(timeout()), this, SLOT(RenderImage2d()));
  connect(m_RenderingTimer3d, SIGNAL(timeout()), this, SLOT(RenderImage3d()));
  connect(m_Controls.m_Update2DView, SIGNAL(clicked()), this, SLOT(StartTimers()));
  connect(m_Controls.m_Update3DView, SIGNAL(clicked()), this, SLOT(StartTimers()));
  connect(m_Controls.m_DeviceManagerWidget, SIGNAL(EditDeviceButtonClicked(mitk::USDevice::Pointer)), this, SLOT(OnClickedEditDevice())); //Change Widget Visibilities
  connect(m_Controls.m_DeviceManagerWidget, SIGNAL(EditDeviceButtonClicked(mitk::USDevice::Pointer)), this->m_Controls.m_NewVideoDeviceWidget, SLOT(EditDevice(mitk::USDevice::Pointer)));

  // Initializations
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  std::string filter = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
    + "org.mitk.services.UltrasoundDevice)("
    + mitk::USDevice::GetPropertyKeys().US_PROPKEY_ISACTIVE + "=true))";
  m_Controls.m_ActiveVideoDevices->Initialize<mitk::USDevice>(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_LABEL, filter);
  m_Controls.m_ActiveVideoDevices->SetAutomaticallySelectFirstEntry(true);
  m_FrameCounterPipeline = 0;
  m_FrameCounter2d = 0;
  m_FrameCounter3d = 0;

  m_Controls.tabWidget->setTabEnabled(1, false);

  
  m_Layout_PA.setSpacing(2);
  m_Layout_PA.setMargin(0);

  m_LevelWindow_PA.SetDataStorage(m_PADataStorage);

  // Tell the RenderWindow which (part of) the datastorage to render
  m_PARenderWindow.GetRenderer()->SetDataStorage(m_PADataStorage);

  // Initialize the RenderWindow
  mitk::TimeGeometry::Pointer PAgeo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(PAgeo);

  // Use it as a 2D view
  m_PARenderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);


  m_Layout_PA.addWidget(&m_PARenderWindow,44);
  m_Layout_PA.addWidget(&m_LevelWindow_PA);
  m_ToplevelWidget_PA.setLayout(&m_Layout_PA);

  m_ToplevelWidget_PA.show();

  auto renderingManager = mitk::RenderingManager::GetInstance();
  renderingManager->AddRenderWindow(m_PARenderWindow.GetRenderWindow());


  m_Layout_US.setSpacing(2);
  m_Layout_US.setMargin(0);

  m_LevelWindow_US.SetDataStorage(m_USDataStorage);

  // Tell the RenderWindow which (part of) the datastorage to render
  m_USRenderWindow.GetRenderer()->SetDataStorage(m_USDataStorage);

  // Initialize the RenderWindow
  mitk::TimeGeometry::Pointer USgeo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(USgeo);

  // Use it as a 2D view
  m_USRenderWindow.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);


  m_Layout_US.addWidget(&m_USRenderWindow, 44);
  m_Layout_US.addWidget(&m_LevelWindow_US);
  m_ToplevelWidget_US.setLayout(&m_Layout_US);

  m_ToplevelWidget_US.show();

  renderingManager->AddRenderWindow(m_USRenderWindow.GetRenderWindow());
}

#include <mitkRenderingModeProperty.h>

void UltrasoundSupport::InitNewNode()
{
  m_Node.push_back(nullptr);
  auto& Node = m_Node.back();

  Node = mitk::DataNode::New();
  Node->SetName("No Data received yet ...");
  //create a dummy image (gray values 0..255) for correct initialization of level window, etc.
  mitk::Image::Pointer dummyImage = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 1, 1, 1, 1, 1, 255, 0);
  Node->SetData(dummyImage);
  m_OldGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(dummyImage->GetGeometry());
  UpdateColormaps();

  this->GetDataStorage()->Add(Node);
}

void UltrasoundSupport::DestroyLastNode()
{
  auto& Node = m_Node.back();
  this->GetDataStorage()->Remove(Node);
  Node->ReleaseData();
  m_Node.pop_back();
  UpdateColormaps();
}

void UltrasoundSupport::AddOverlays()
{

  //This creates a 2DLayouter that is only active for the recently fetched axialRenderer and positione
  //mitk::Overlay2DLayouter::Pointer topleftLayouter = mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(), m_PARenderer);

  //Now, the created Layouter is added to the OverlayManager and can be referred to by its identification string.
  //m_PAOverlayManager->AddLayouter(topleftLayouter.GetPointer());

  //Several other Layouters can be added to the overlayManager
  //mitk::Overlay2DLayouter::Pointer bottomLayouter = mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM(), m_PARenderer);
  //m_PAOverlayManager->AddLayouter(bottomLayouter.GetPointer());

  //Create a textOverlay2D
  //mitk::TextOverlay2D::Pointer textOverlay = mitk::TextOverlay2D::New();
  //
  //textOverlay->SetText("Test!"); //set UTF-8 encoded text to render
  //textOverlay->SetFontSize(40);
  //textOverlay->SetColor(1, 0, 0); //Set text color to red
  //textOverlay->SetOpacity(1);

  //The position of the Overlay can be set to a fixed coordinate on the display.
  /*mitk::Point2D pos;
  pos[0] = 10, pos[1] = 20;
  textOverlay->SetPosition2D(pos);*/

  //Add the overlay to the overlayManager. It is added to all registered renderers automaticly
  //m_PAOverlayManager->AddOverlay(textOverlay.GetPointer());

  mitk::ScaleLegendOverlay::Pointer scaleOverlay = mitk::ScaleLegendOverlay::New();
  scaleOverlay->SetLeftAxisVisibility(true);
  scaleOverlay->SetRightAxisVisibility(false);
  scaleOverlay->SetTopAxisVisibility(true);
  scaleOverlay->SetCornerOffsetFactor(0);
  m_PAOverlayManager->AddOverlay(scaleOverlay.GetPointer());
  m_USOverlayManager->AddOverlay(scaleOverlay.GetPointer());

  //Alternatively, a layouter can be used to manage the position of the overlay. If a layouter is set, the absolute position of the overlay is not used anymore
  //Because a Layouter is specified by the identification string AND the Renderer, both have to be passed to the call.
  //m_OverlayManager->SetLayouter(textOverlay.GetPointer(), mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT(), m_Renderer);
}

void UltrasoundSupport::RemoveOverlays()
{
  m_PAOverlayManager->RemoveAllOverlays();
}

void UltrasoundSupport::UpdateColormaps()
{
  //return; // TODO: review the colormap functionality

  // we update here both the colormaps of the nodes, as well as the
  // level window for the current dynamic range
  mitk::LevelWindow levelWindow;

  if (m_Node.size() > 1)
  {
    for (int index = 0; index < m_AmountOfOutputs - 1; ++index)
    {
      SetColormap(m_Node.at(index), mitk::LookupTable::LookupTableType::GRAYSCALE);
      m_Node.at(index)->GetLevelWindow(levelWindow);
      if (!m_Image->IsEmpty())
        levelWindow.SetAuto(m_Image, true, true);
      m_Node.at(index)->SetLevelWindow(levelWindow);
    }
    SetColormap(m_Node.back(), mitk::LookupTable::LookupTableType::GRAYSCALE);
    m_Node.back()->GetLevelWindow(levelWindow);
    levelWindow.SetWindowBounds(10, 150, true);
    m_Node.back()->SetLevelWindow(levelWindow);
  }
  else if (m_Node.size() == 1)
  {
    SetColormap(m_Node.back(), mitk::LookupTable::LookupTableType::GRAYSCALE);
    m_Node.back()->GetLevelWindow(levelWindow);
    if (!m_Image->IsEmpty())
      levelWindow.SetAuto(m_Image, true, true);
    m_Node.back()->SetLevelWindow(levelWindow);
  }
}
void UltrasoundSupport::SetColormap(mitk::DataNode::Pointer node, mitk::LookupTable::LookupTableType type)
{
  mitk::LookupTable::Pointer lookupTable = mitk::LookupTable::New();
  mitk::LookupTableProperty::Pointer lookupTableProperty = mitk::LookupTableProperty::New();
  lookupTable->SetType(type);
  lookupTableProperty->SetLookupTable(lookupTable);
  node->SetProperty("LookupTable", lookupTableProperty);

  mitk::RenderingModeProperty::Pointer renderingMode =
    dynamic_cast<mitk::RenderingModeProperty*>(node->GetProperty("Image Rendering.Mode"));
  renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
}

void UltrasoundSupport::OnClickedAddNewDevice()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(true);
  m_Controls.m_DeviceManagerWidget->setVisible(false);
  m_Controls.m_Headline->setText("Add New Video Device:");
  m_Controls.m_WidgetActiveDevices->setVisible(false);
}

void UltrasoundSupport::OnClickedEditDevice()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(true);
  m_Controls.m_DeviceManagerWidget->setVisible(false);
  m_Controls.m_WidgetActiveDevices->setVisible(false);
  m_Controls.m_Headline->setText("Edit Video Device:");
}

void UltrasoundSupport::UpdateAmountOfOutputs()
{
  // Update the amount of Nodes; there should be one Node for every slide that is set. Note that we must check whether the slices are set, 
  // just using the m_Image->dimension(3) will produce nulltpointers on slices of the image that were not set
  bool isSet = true;
  m_AmountOfOutputs = 0;
  while (isSet) {
    isSet = m_Image->IsSliceSet(m_AmountOfOutputs);
    if (isSet)
      ++m_AmountOfOutputs;
  }

  // correct the amount of Nodes to display data
  while (m_Node.size() < m_AmountOfOutputs) {
    InitNewNode();
  }
  while (m_Node.size() > m_AmountOfOutputs) {
    DestroyLastNode();
  }

  // correct the amount of image outputs that we feed the nodes with
  while (m_curOutput.size() < m_AmountOfOutputs) {
    m_curOutput.push_back(mitk::Image::New());

    // initialize the slice images as 2d images with the size of m_Images
    unsigned int* dimOld = m_Image->GetDimensions();
    unsigned int dim[2] = { dimOld[0], dimOld[1] };
    m_curOutput.back()->Initialize(m_Image->GetPixelType(), 2, dim);
  }
  while (m_curOutput.size() > m_AmountOfOutputs) {
    m_curOutput.pop_back();
  }

  auto iterPA = m_PADataStorage->GetAll();
  for (int i = 0; i < iterPA->Size(); ++i)
  {
    m_PADataStorage->Remove(iterPA->GetElement(i));
  }

  auto iterUS = m_USDataStorage->GetAll();
  for (int i = 0; i < iterUS->Size(); ++i)
  {
    m_USDataStorage->Remove(iterUS->GetElement(i));
  }

  m_PADataStorage->Add(m_Node.back());
  for (int i = 0; i < m_Node.size() - 1; ++i)
    m_USDataStorage->Add(m_Node.at(i));
}

void UltrasoundSupport::UpdateImage()
{
  if(m_Controls.m_ShowImageStream->isChecked())
  {
    if (m_PARenderer == nullptr || m_PAOverlayManager == nullptr || m_USRenderer == nullptr || m_USOverlayManager == nullptr)
    {
      //setup an overlay manager
      mitk::OverlayManager::Pointer OverlayManagerInstancePA = mitk::OverlayManager::New();
      m_PARenderer = mitk::BaseRenderer::GetInstance(m_PARenderWindow.GetVtkRenderWindow());
      m_PARenderer->SetOverlayManager(OverlayManagerInstancePA);
      m_PAOverlayManager = m_PARenderer->GetOverlayManager();

      mitk::OverlayManager::Pointer OverlayManagerInstanceUS = mitk::OverlayManager::New();
      m_USRenderer = mitk::BaseRenderer::GetInstance(m_USRenderWindow.GetVtkRenderWindow());
      m_USRenderer->SetOverlayManager(OverlayManagerInstanceUS);
      m_USOverlayManager = m_USRenderer->GetOverlayManager();

      AddOverlays();
    }

    m_Device->Modified();
    m_Device->Update();
    // Update device

    m_Image = m_Device->GetOutput();
    // get the Image data to display
    UpdateAmountOfOutputs();
    // create as many Nodes and Outputs as there are slices in m_Image

    if (m_AmountOfOutputs == 0)
      return;
    // if there is no image to be displayed, skip the rest of this method

    for (int index = 0; index < m_AmountOfOutputs; ++index)
    {
      if (m_curOutput.at(index)->GetDimension(0) != m_Image->GetDimension(0) ||
        m_curOutput.at(index)->GetDimension(1) != m_Image->GetDimension(1) ||
        m_curOutput.at(index)->GetDimension(2) != m_Image->GetDimension(2) ||
        m_curOutput.at(index)->GetPixelType() != m_Image->GetPixelType())
      {
        unsigned int* dimOld = m_Image->GetDimensions();
        unsigned int dim[2] = { dimOld[0], dimOld[1]};
        m_curOutput.at(index)->Initialize(m_Image->GetPixelType(), 2, dim);
        // if we switched image resolution or type the outputs must be reinitialized!
      } 

      if (!m_Image->IsEmpty())
      {
        mitk::ImageReadAccessor inputReadAccessor(m_Image, m_Image->GetSliceData(m_AmountOfOutputs-index-1,0,0,nullptr,mitk::Image::ReferenceMemory));
        // just reference the slices, to get a small performance gain
        m_curOutput.at(index)->SetSlice(inputReadAccessor.GetData());
        m_curOutput.at(index)->GetGeometry()->SetIndexToWorldTransform(m_Image->GetSlicedGeometry()->GetIndexToWorldTransform());
        // Update the image Output with seperate slices
      }

      if (m_curOutput.at(index)->IsEmpty())
      {
        m_Node.at(index)->SetName("No Data received yet ...");
        // create a noise image for correct initialization of level window, etc.
        mitk::Image::Pointer randomImage = mitk::ImageGenerator::GenerateRandomImage<float>(32, 32, 1, 1, 1, 1, 1, 255, 0);
        m_Node.at(index)->SetData(randomImage);
        m_curOutput.at(index)->SetGeometry(randomImage->GetGeometry());
      }
      else
      {
        char name[30];
        sprintf(name, "US Viewing Stream - Image %d", index);
        m_Node.at(index)->SetName(name);
        m_Node.at(index)->SetData(m_curOutput.at(index)); 
        // set the name of the Output
      }
    }

    // if the geometry changed: reinitialize the ultrasound image. we use the m_curOutput.at(0) to readjust the geometry
    if ((m_OldGeometry.IsNotNull()) &&
      (m_curOutput.at(0)->GetGeometry() != NULL) &&
      (!mitk::Equal(m_OldGeometry.GetPointer(), m_curOutput.at(0)->GetGeometry(), 0.0001, false))
      )
    {
      mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
      if ((renderWindow != NULL) && (m_curOutput.at(0)->GetTimeGeometry()->IsValid()) && (m_Controls.m_ShowImageStream->isChecked()))
      {
        renderWindow->GetRenderingManager()->InitializeViews(
          m_curOutput.at(0)->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
        renderWindow->GetRenderingManager()->RequestUpdateAll();
      }
      m_CurrentImageWidth = m_curOutput.at(0)->GetDimension(0);
      m_CurrentImageHeight = m_curOutput.at(0)->GetDimension(1);
      m_OldGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(m_curOutput.at(0)->GetGeometry());
    }
  }

  //Update frame counter
  m_FrameCounterPipeline++;
  if (m_FrameCounterPipeline >= 10)
  {
    // compute framerate of pipeline update
    int nMilliseconds = m_Clock.restart();
    int fps = 10000.0f / (nMilliseconds);
    m_FPSPipeline = fps;
    m_FrameCounterPipeline = 0;

    // display lowest framerate in UI
    int lowestFPS = m_FPSPipeline;
    if (m_Controls.m_Update2DView->isChecked() && (m_FPS2d < lowestFPS)) { lowestFPS = m_FPS2d; }
    if (m_Controls.m_Update3DView->isChecked() && (m_FPS3d < lowestFPS)) { lowestFPS = m_FPS3d; }
    m_Controls.m_FramerateLabel->setText("Current Framerate: " + QString::number(lowestFPS) + " FPS");
  }
}

void UltrasoundSupport::RenderImage2d()
{
  if (!m_Controls.m_Update2DView->isChecked())
    return;
  //mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
  //renderWindow->GetRenderingManager()->RequestUpdate(mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetRenderWindow());

  // TODO: figure out how to proceed with the standard display

  auto renderingManager = mitk::RenderingManager::GetInstance();
  renderingManager->RequestUpdate(m_PARenderWindow.GetRenderWindow());
  renderingManager->RequestUpdate(m_USRenderWindow.GetRenderWindow());

  //this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
  m_FrameCounter2d++;
  if (m_FrameCounter2d >= 10)
  {
    // compute framerate of 2d render window update
    int nMilliseconds = m_Clock2d.restart();
    int fps = 10000.0f / (nMilliseconds);
    m_FPS2d = fps;
    m_FrameCounter2d = 0;
  }
}

void UltrasoundSupport::RenderImage3d()
{
  if (!m_Controls.m_Update3DView->isChecked())
    return;

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  m_FrameCounter3d++;
  if (m_FrameCounter3d >= 10)
  {
    // compute framerate of 2d render window update
    int nMilliseconds = m_Clock3d.restart();
    int fps = 10000.0f / (nMilliseconds);
    m_FPS3d = fps;
    m_FrameCounter3d = 0;
  }
}

void UltrasoundSupport::OnChangedFramerateLimit()
{
  StopTimers();
  int intervalPipeline = (1000 / m_Controls.m_FrameRatePipeline->value());
  int interval2D = (1000 / m_Controls.m_FrameRate2d->value());
  int interval3D = (1000 / m_Controls.m_FrameRate3d->value());
  SetTimerIntervals(intervalPipeline, interval2D, interval3D);
  StartTimers();
}

void UltrasoundSupport::OnClickedFreezeButton()
{
  if (m_Device.IsNull())
  {
    MITK_WARN("UltrasoundSupport") << "Freeze button clicked though no device is selected.";
    return;
  }
  if (m_Device->GetIsFreezed())
  {
    m_Device->SetIsFreezed(false);
    m_Controls.m_FreezeButton->setText("Freeze");
  }
  else
  {
    m_Device->SetIsFreezed(true);
    m_Controls.m_FreezeButton->setText("Start Viewing Again");
  }
}

void UltrasoundSupport::OnChangedActiveDevice()
{
  //clean up, delete nodes and stop timer
  StopTimers();
  this->RemoveControlWidgets();
  for (auto& Node : m_Node)
  {
    this->GetDataStorage()->Remove(Node);
    Node->ReleaseData();
  }
  m_Node.clear();

  //get current device, abort if it is invalid
  m_Device = m_Controls.m_ActiveVideoDevices->GetSelectedService<mitk::USDevice>();
  if (m_Device.IsNull())
  {
    m_Controls.tabWidget->setTabEnabled(1, false);
    return;
  }

  //create the widgets for this device and enable the widget tab
  this->CreateControlWidgets();
  m_Controls.tabWidget->setTabEnabled(1, true);

  //start timer
  if (m_Controls.m_RunImageTimer->isChecked())
  {
    int intervalPipeline = (1000 / m_Controls.m_FrameRatePipeline->value());
    int interval2D = (1000 / m_Controls.m_FrameRate2d->value());
    int interval3D = (1000 / m_Controls.m_FrameRate3d->value());
    SetTimerIntervals(intervalPipeline, interval2D, interval3D);
    StartTimers();
    m_Controls.m_TimerWidget->setEnabled(true);
  }
  else
  {
    m_Controls.m_TimerWidget->setEnabled(false);
  }
}

void UltrasoundSupport::OnNewDeviceWidgetDone()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  m_Controls.m_DeviceManagerWidget->setVisible(true);
  m_Controls.m_Headline->setText("Ultrasound Devices:");
  m_Controls.m_WidgetActiveDevices->setVisible(true);
}

void UltrasoundSupport::CreateControlWidgets()
{
  m_ControlProbesWidget = new QmitkUSControlsProbesWidget(m_Device->GetControlInterfaceProbes(), m_Controls.m_ToolBoxControlWidgets);
  m_Controls.probesWidgetContainer->addWidget(m_ControlProbesWidget);

  // create b mode widget for current device
  m_ControlBModeWidget = new QmitkUSControlsBModeWidget(m_Device->GetControlInterfaceBMode(), m_Controls.m_ToolBoxControlWidgets);
  
  if (m_Device->GetControlInterfaceBMode())
  {
    m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlBModeWidget, "B Mode Controls");
    //m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count() - 1, false);
  }

  // create doppler widget for current device
  m_ControlDopplerWidget = new QmitkUSControlsDopplerWidget(m_Device->GetControlInterfaceDoppler(), m_Controls.m_ToolBoxControlWidgets);
  
  if (m_Device->GetControlInterfaceDoppler())
  {
    m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlDopplerWidget, "Doppler Controls");
    //m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count() - 1, false);
  }

  ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
  if (pluginContext)
  {
    std::string filter = "(ork.mitk.services.UltrasoundCustomWidget.deviceClass=" + m_Device->GetDeviceClass() + ")";

    QString interfaceName = QString::fromStdString(us_service_interface_iid<QmitkUSAbstractCustomWidget>());
    m_CustomWidgetServiceReference = pluginContext->getServiceReferences(interfaceName, QString::fromStdString(filter));

    if (m_CustomWidgetServiceReference.size() > 0)
    {
      m_ControlCustomWidget = pluginContext->getService<QmitkUSAbstractCustomWidget>
        (m_CustomWidgetServiceReference.at(0))->CloneForQt(m_Controls.tab2);
      m_ControlCustomWidget->SetDevice(m_Device);
      m_Controls.m_ToolBoxControlWidgets->addItem(m_ControlCustomWidget, "Custom Controls");
    }
    else
    {
      m_Controls.m_ToolBoxControlWidgets->addItem(new QWidget(m_Controls.m_ToolBoxControlWidgets), "Custom Controls");
      m_Controls.m_ToolBoxControlWidgets->setItemEnabled(m_Controls.m_ToolBoxControlWidgets->count() - 1, false);
    }
  }

  // select first enabled control widget
  for (int n = 0; n < m_Controls.m_ToolBoxControlWidgets->count(); ++n)
  {
    if (m_Controls.m_ToolBoxControlWidgets->isItemEnabled(n))
    {
      m_Controls.m_ToolBoxControlWidgets->setCurrentIndex(n);
      break;
    }
  }
}

void UltrasoundSupport::RemoveControlWidgets()
{
  if (!m_ControlProbesWidget) { return; } //widgets do not exist... nothing to do

  // remove all control widgets from the tool box widget
  while (m_Controls.m_ToolBoxControlWidgets->count() > 0)
  {
    m_Controls.m_ToolBoxControlWidgets->removeItem(0);
  }

  // remove probes widget (which is not part of the tool box widget)
  m_Controls.probesWidgetContainer->removeWidget(m_ControlProbesWidget);
  delete m_ControlProbesWidget;
  m_ControlProbesWidget = 0;

  delete m_ControlBModeWidget;
  m_ControlBModeWidget = 0;

  delete m_ControlDopplerWidget;
  m_ControlDopplerWidget = 0;

  // delete custom widget if it is present
  if (m_ControlCustomWidget)
  {
    ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
    delete m_ControlCustomWidget; m_ControlCustomWidget = 0;
    if (m_CustomWidgetServiceReference.size() > 0)
    {
      pluginContext->ungetService(m_CustomWidgetServiceReference.at(0));
    }
  }
}

void UltrasoundSupport::OnDeciveServiceEvent(const ctkServiceEvent event)
{
  if (m_Device.IsNull() || event.getType() != us::ServiceEvent::MODIFIED)
  {
    return;
  }

  ctkServiceReference service = event.getServiceReference();

  if (m_Device->GetManufacturer() != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_MANUFACTURER)).toString().toStdString()
    && m_Device->GetName() != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_NAME)).toString().toStdString())
  {
    return;
  }

  if (!m_Device->GetIsActive() && m_UpdateTimer->isActive())
  {
    StopTimers();
  }

  if (m_CurrentDynamicRange != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE)).toDouble())
  {
    m_CurrentDynamicRange = service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE)).toDouble();

    // update level window for the current dynamic range
    mitk::LevelWindow levelWindow;
    for (auto& Node : m_Node)
    {
      Node->GetLevelWindow(levelWindow);
      levelWindow.SetAuto(m_Image, true, true);
	  levelWindow.SetWindowBounds(55, 125,true);
      Node->SetLevelWindow(levelWindow);
    }
  }
}

UltrasoundSupport::UltrasoundSupport()
  : m_ControlCustomWidget(0), m_ControlBModeWidget(0),
  m_ControlProbesWidget(0), m_ImageAlreadySetToNode(false),
  m_CurrentImageWidth(0), m_CurrentImageHeight(0), m_AmountOfOutputs(0), 
  m_PARenderer(nullptr), m_PAOverlayManager(nullptr), m_USRenderer(nullptr), m_USOverlayManager(nullptr),
  m_PADataStorage(mitk::StandaloneDataStorage::New()), m_USDataStorage(mitk::StandaloneDataStorage::New())
{
  ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();

  if (pluginContext)
  {
    // to be notified about service event of an USDevice
    pluginContext->connectServiceListener(this, "OnDeciveServiceEvent",
      QString::fromStdString("(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::USDevice>() + ")"));
  }
}

UltrasoundSupport::~UltrasoundSupport()
{
  try
  {
    StopTimers();

    // Get all active devicesand deactivate them to prevent freeze
    std::vector<mitk::USDevice*> devices = this->m_Controls.m_ActiveVideoDevices->GetAllServices<mitk::USDevice>();
    for (int i = 0; i < devices.size(); i++)
    {
      mitk::USDevice::Pointer device = devices[i];
      if (device.IsNotNull() && device->GetIsActive())
      {
        device->Deactivate();
        device->Disconnect();
      }
    }

    StoreUISettings();
  }
  catch (std::exception &e)
  {
    MITK_ERROR << "Exception during call of destructor! Message: " << e.what();
  }
}

void UltrasoundSupport::StoreUISettings()
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(VIEW_ID));
  settings.setValue("DisplayImage", QVariant(m_Controls.m_ShowImageStream->isChecked()));
  settings.setValue("RunImageTimer", QVariant(m_Controls.m_RunImageTimer->isChecked()));
  settings.setValue("Update2DView", QVariant(m_Controls.m_Update2DView->isChecked()));
  settings.setValue("Update3DView", QVariant(m_Controls.m_Update3DView->isChecked()));
  settings.setValue("UpdateRatePipeline", QVariant(m_Controls.m_FrameRatePipeline->value()));
  settings.setValue("UpdateRate2d", QVariant(m_Controls.m_FrameRate2d->value()));
  settings.setValue("UpdateRate3d", QVariant(m_Controls.m_FrameRate3d->value()));
  settings.endGroup();
}

void UltrasoundSupport::LoadUISettings()
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(VIEW_ID));
  m_Controls.m_ShowImageStream->setChecked(settings.value("DisplayImage", true).toBool());
  m_Controls.m_RunImageTimer->setChecked(settings.value("RunImageTimer", true).toBool());
  m_Controls.m_Update2DView->setChecked(settings.value("Update2DView", true).toBool());
  m_Controls.m_Update3DView->setChecked(settings.value("Update3DView", true).toBool());
  m_Controls.m_FrameRatePipeline->setValue(settings.value("UpdateRatePipeline", 50).toInt());
  m_Controls.m_FrameRate2d->setValue(settings.value("UpdateRate2d", 20).toInt());
  m_Controls.m_FrameRate3d->setValue(settings.value("UpdateRate3d", 5).toInt());
  settings.endGroup();
}

void UltrasoundSupport::StartTimers()
{
  m_UpdateTimer->start();
  if (m_Controls.m_Update2DView->isChecked()) { m_RenderingTimer2d->start(); }
  if (m_Controls.m_Update3DView->isChecked()) { m_RenderingTimer3d->start(); }
}

void UltrasoundSupport::StopTimers()
{
  m_UpdateTimer->stop();
  m_RenderingTimer2d->stop();
  m_RenderingTimer3d->stop();
}

void UltrasoundSupport::SetTimerIntervals(int intervalPipeline, int interval2D, int interval3D)
{
  m_UpdateTimer->setInterval(intervalPipeline);
  m_RenderingTimer2d->setInterval(interval2D);
  m_RenderingTimer3d->setInterval(interval3D);
}
