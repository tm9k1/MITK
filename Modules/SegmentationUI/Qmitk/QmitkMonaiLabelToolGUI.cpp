#include "QmitkMonaiLabelToolGUI.h"
#include "mitkMonaiLabelTool.h"

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include "usServiceReference.h"


MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkMonaiLabelToolGUI, "")

QmitkMonaiLabelToolGUI::QmitkMonaiLabelToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { 
    return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false;
  };
}

void QmitkMonaiLabelToolGUI::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkMonaiLabelToolGUI::InitializeUI(QBoxLayout* mainLayout)
{ 
  m_Controls.setupUi(this);
  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));

  Superclass::InitializeUI(mainLayout);
}

void QmitkMonaiLabelToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}

void QmitkMonaiLabelToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr != tool)
  {
    QString url = m_Controls.url->text();
    MITK_INFO << "tool found" << url.toStdString();

    tool->GetOverallInfo("https://httpbin.org/get");
    std::string response = tool->m_Parameters->origin;
    m_Controls.url->setText(QString::fromStdString(response));
  }
   //tool->UpdatePreview();
}
