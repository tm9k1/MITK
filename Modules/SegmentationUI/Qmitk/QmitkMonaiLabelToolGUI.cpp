#include "QmitkMonaiLabelToolGUI.h"


void QmitkOtsuTool3DGUI::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkOtsuTool3DGUI::InitializeUI(QBoxLayout* mainLayout)
{
  m_Controls.setupUi(this);
  mainLayout->addLayout(m_Controls.verticalLayout);
  
  Superclass::InitializeUI(mainLayout);
}

void QmitkOtsuTool3DGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}