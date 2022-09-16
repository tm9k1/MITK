#include "QmitkMonaiLabelToolGUI.h"


void QmitkMonaiLabelToolGUI::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);

  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkMonaiLabelToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  /*
  m_Controls.setupUi(this);
  mainLayout->addLayout(m_Controls.verticalLayout);
  
  Superclass::InitializeUI(mainLayout);*/
}

void QmitkMonaiLabelToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}