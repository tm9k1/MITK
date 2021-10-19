#include "QmitknnUNetToolGUI.h"
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <algorithm>
#include <ctkCollapsibleGroupBox.h>
#include <QmitknnUNetEnsembleLayout.h>
#include <utility>

void QmitknnUNetToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
  m_Controls.previewButton->setEnabled(false);
}

void QmitknnUNetToolGUI::ClearAllComboBoxes()
{
  m_Controls.modelBox->clear();
  m_Controls.taskBox->clear();
  m_Controls.foldBox->clear();
  m_Controls.trainerBox->clear();
}

template <typename T>
T QmitknnUNetToolGUI::FetchFoldersFromDir(const QString &path)
{
  T folders;
  for (QDirIterator it(path, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
  {
    it.next();
    if (!it.fileName().startsWith('.'))
    {
      folders.push_back(it.fileName());
    }
  }
  return folders;
}

void QmitknnUNetToolGUI::OnDirectoryChanged(const QString &resultsFolder)
{
  m_Controls.previewButton->setEnabled(false);
  this->ClearAllComboBoxes();
  m_ModelDirectory = resultsFolder + QDir::separator() + "nnUNet";
  auto models = FetchFoldersFromDir<QStringList>(m_ModelDirectory);
  QStringList validlist; // valid list of models supported by nnUNet
  validlist << "2d"
            << "3d_lowres"
            << "3d_fullres"
            << "3d_cascade_fullres"
            << "ensembles";
  std::for_each(models.begin(),
                models.end(),
                [this, validlist](QString model)
                {
                  if (validlist.contains(model, Qt::CaseInsensitive))
                    m_Controls.modelBox->addItem(model);
                });
}


void QmitknnUNetToolGUI::ShowEnsembleLayout(bool visible) 
{
  if (m_EnsembleParams.empty())
  {
    ctkCollapsibleGroupBox *groupBoxModel1 = new ctkCollapsibleGroupBox(this);
    auto lay1 = std::make_unique<QmitknnUNetTaskParamsUITemplate>(groupBoxModel1);
    m_EnsembleParams.push_back(std::move(lay1));
    groupBoxModel1->setObjectName(QString::fromUtf8("model_1_Box"));
    groupBoxModel1->setTitle(QString::fromUtf8("Model 1"));
    groupBoxModel1->setMinimumSize(QSize(0, 0));
    groupBoxModel1->setCollapsedHeight(5);
    groupBoxModel1->setCollapsed(false);
    groupBoxModel1->setFlat(true);
    groupBoxModel1->setAlignment(Qt::AlignRight);
    m_Controls.advancedSettingsLayout->addWidget(groupBoxModel1, 2, 0, 1, 2);

    ctkCollapsibleGroupBox *groupBoxModel2 = new ctkCollapsibleGroupBox(this);
    auto lay2 = std::make_unique<QmitknnUNetTaskParamsUITemplate>(groupBoxModel2);
    m_EnsembleParams.push_back(std::move(lay2));
    groupBoxModel2->setObjectName(QString::fromUtf8("model_2_Box"));
    groupBoxModel2->setTitle(QString::fromUtf8("Model 2"));
    groupBoxModel2->setMinimumSize(QSize(0, 0));
    groupBoxModel2->setCollapsedHeight(5);
    groupBoxModel2->setCollapsed(false);
    groupBoxModel2->setFlat(true);
    groupBoxModel2->setAlignment(Qt::AlignLeft);
    m_Controls.advancedSettingsLayout->addWidget(groupBoxModel2, 2, 2, 1, 2);
  }
  for (std::unique_ptr<QmitknnUNetTaskParamsUITemplate>& layout: m_EnsembleParams)
  {
    layout->parent->setVisible(visible);
  }
}

void QmitknnUNetToolGUI::OnModelChanged(const QString &text)
{
  QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + text));
  m_Controls.taskBox->clear();
  auto tasks = FetchFoldersFromDir<QStringList>(updatedPath);
  std::for_each(tasks.begin(), tasks.end(), [this](QString task) { m_Controls.taskBox->addItem(task); });
}

void QmitknnUNetToolGUI::OnTaskChanged(const QString &text)
{
  if (text.isEmpty())
  {
    return;
  }
  QString updatedPath = QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Controls.modelBox->currentText() +
                                        QDir::separator() + text);
  MITK_INFO << "updated path ashis " << updatedPath.toStdString();
  m_Controls.trainerBox->clear();
  m_Controls.plannerBox->clear();
  auto trainerPlanners = FetchFoldersFromDir<QStringList>(updatedPath);
  if (m_Controls.modelBox->currentText() == "ensembles")
  {
    m_Controls.trainerBox->setVisible(false);
    m_Controls.trainerLabel->setVisible(false);
    m_Controls.plannerBox->setVisible(false);
    m_Controls.plannerLabel->setVisible(false);
    m_Controls.foldBox->setVisible(false);
    m_Controls.foldLabel->setVisible(false);
    ShowEnsembleLayout(true);
    QString splitterString = "--";
    QStringList models, trainers, planners;
    foreach (QString trainerPlanner, trainerPlanners)
    {
      QStringList trainerSplitParts = trainerPlanner.split(splitterString, QString::SplitBehavior::SkipEmptyParts);
      foreach (QString modelSet, trainerSplitParts)
      {
        modelSet.remove("ensemble_", Qt::CaseInsensitive);
        QStringList splitParts = modelSet.split("__", QString::SplitBehavior::SkipEmptyParts);
        QString modelName = splitParts.first();
        QString trainer = splitParts.at(1);
        QString planId = splitParts.at(2);
        models << modelName;
        trainers << trainer;
        planners << planId;
      }
    }
    trainers.removeDuplicates();
    planners.removeDuplicates();
    models.removeDuplicates();
    for (std::unique_ptr<QmitknnUNetTaskParamsUITemplate> &layout : m_EnsembleParams)
    {
      layout->modelBox->clear();
      layout->trainerBox->clear();
      layout->plannerBox->clear();
      std::for_each(models.begin(), models.end(), [&layout](QString model) { layout->modelBox->addItem(model); });
      std::for_each(
        trainers.begin(), trainers.end(), [&layout](QString trainer) { layout->trainerBox->addItem(trainer); });
      std::for_each(
        planners.begin(), planners.end(), [&layout](QString planner) { layout->plannerBox->addItem(planner); });
    }
  }
  else
  {
    m_Controls.trainerBox->setVisible(true);
    m_Controls.trainerLabel->setVisible(true);
    m_Controls.plannerBox->setVisible(true);
    m_Controls.plannerLabel->setVisible(true);
    m_Controls.foldBox->setVisible(true);
    m_Controls.foldLabel->setVisible(true);
    ShowEnsembleLayout(false);
    QStringList trainers, planners;
    foreach (QString trainerPlanner, trainerPlanners)
    {
      trainers << trainerPlanner.split("__", QString::SplitBehavior::SkipEmptyParts).first();
      planners << trainerPlanner.split("__", QString::SplitBehavior::SkipEmptyParts).last();
    }
    trainers.removeDuplicates();
    planners.removeDuplicates();
    std::for_each(trainers.begin(), trainers.end(), [this](QString trainer) { m_Controls.trainerBox->addItem(trainer); });
    std::for_each(planners.begin(), planners.end(), [this](QString planner) { m_Controls.plannerBox->addItem(planner); });
  }
}

void QmitknnUNetToolGUI::OnTrainerChanged(const QString &trainerSelected)
{
  m_Controls.foldBox->clear();
  if (m_Controls.modelBox->currentText() != "ensembles")
  {
    QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Controls.modelBox->currentText() +
                                        QDir::separator() + m_Controls.taskBox->currentText() + QDir::separator() +
                                        m_Controls.trainerBox->currentText() + "__" + trainerSelected));
    auto folds = FetchFoldersFromDir<QStringList>(updatedPath);
    std::for_each(folds.begin(),
                  folds.end(),
                  [this](QString fold)
                  {
                    if (fold.startsWith("fold_", Qt::CaseInsensitive)) // imposed by nnUNet
                      m_Controls.foldBox->addItem(fold);
                  });
    if (m_Controls.foldBox->count() != 0)
    {
      m_Controls.previewButton->setEnabled(true);
    }
  }
  else
  {
    m_Controls.previewButton->setEnabled(true);
  }
}

void QmitknnUNetToolGUI::OnPythonPathChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select"))
  {
    QString path =
      QFileDialog::getExistingDirectory(m_Controls.pythonEnvComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      m_Controls.pythonEnvComboBox->insertItem(0, path);
      m_Controls.pythonEnvComboBox->setCurrentIndex(0);
    }
  }
  else if (!IsNNUNetInstalled(pyEnv))
  {
    std::string warning =
      "WARNING: nnUNet is not detected on the Python environment you selected. Please select another "
      "environment or create one. For more info refer https://github.com/MIC-DKFZ/nnUNet";
    ShowErrorMessage(warning);
  }
}

void QmitknnUNetToolGUI::OnCheckBoxChanged(int state)
{
  bool visibility = false;
  if (state == Qt::Checked)
  {
    visibility = true;
  }
  ctkCheckBox *box = qobject_cast<ctkCheckBox *>(sender());
  if (box != nullptr)
  {
    if (box->objectName() == QString("nopipBox"))
    {
      m_Controls.codedirectoryBox->setVisible(visibility);
      m_Controls.nnUnetdirLabel->setVisible(visibility);
    }
    else if (box->objectName() == QString("multiModalBox"))
    {
      m_Controls.multiModalSpinLabel->setVisible(visibility);
      m_Controls.multiModalSpinBox->setVisible(visibility);
      m_Controls.posSpinBoxLabel->setVisible(visibility);
      m_Controls.posSpinBox->setVisible(visibility);
      if (visibility)
      {
        QmitkDataStorageComboBox *defaultImage = new QmitkDataStorageComboBox(this, true);
        defaultImage->setObjectName(QString("multiModal_" + QString::number(0)));
        defaultImage->SetPredicate(this->m_MultiModalPredicate);
        defaultImage->setDisabled(true);
        mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
        if (tool != nullptr)
        {
          defaultImage->SetDataStorage(tool->GetDataStorage());
          defaultImage->SetSelectedNode(tool->GetRefNode());
        }
        m_Controls.advancedSettingsLayout->addWidget(defaultImage, this->m_UI_ROWS + m_Modalities.size() + 1, 1, 1, 3);
        m_Modalities.push_back(defaultImage);
        m_Controls.posSpinBox->setMaximum(this->m_Modalities.size() - 1);
        m_UI_ROWS++;
      }
      else
      {
        OnModalitiesNumberChanged(0);
        m_Controls.multiModalSpinBox->setValue(0);
        m_Controls.posSpinBox->setMaximum(0);
        delete this->m_Modalities[0];
        m_Modalities.pop_back();
      }
    }
  }
}

void QmitknnUNetToolGUI::OnModalitiesNumberChanged(int num)
{
  while (num > static_cast<int>(this->m_Modalities.size()-1))
  {
    QmitkDataStorageComboBox *multiModalBox = new QmitkDataStorageComboBox(this, true);
    mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
    multiModalBox->SetDataStorage(tool->GetDataStorage());
    multiModalBox->SetPredicate(this->m_MultiModalPredicate);
    multiModalBox->setObjectName(QString("multiModal_" + QString::number(m_Modalities.size() + 1)));
    m_Controls.advancedSettingsLayout->addWidget(multiModalBox, this->m_UI_ROWS + m_Modalities.size() + 1, 1, 1, 3);
    m_Modalities.push_back(multiModalBox);
  }
  while (num < static_cast<int>(this->m_Modalities.size()-1) && !m_Modalities.empty())
  {
    QmitkDataStorageComboBox *child = m_Modalities.back();
    if (child->objectName() == "multiModal_0")
    {
      std::iter_swap(this->m_Modalities.end() - 2, this->m_Modalities.end()-1);
      child = m_Modalities.back();
    }
    delete child; // delete the layout item
    m_Modalities.pop_back();
  }
  m_Controls.posSpinBox->setMaximum(this->m_Modalities.size()-1);
  m_Controls.advancedSettingsLayout->update();
}


void QmitknnUNetToolGUI::OnModalPositionChanged(int posIdx)
{
  if (posIdx < static_cast<int>(this->m_Modalities.size()))
  {
    int currPos = 0;
    bool stopCheck = false;
    // for-loop clears all widgets from the QGridLayout and also, finds the position of loaded-image widget.
    for (QmitkDataStorageComboBox *multiModalBox : this->m_Modalities) 
    {
      m_Controls.advancedSettingsLayout->removeWidget(multiModalBox);
      multiModalBox->setParent(nullptr);
      if (multiModalBox->objectName() != "multiModal_0" && !stopCheck)
      {
        currPos++;
      }
      else
      {
        stopCheck = true;
      }
    }
    // moving the loaded-image widget to the required position
    std::iter_swap(this->m_Modalities.begin() + currPos, this->m_Modalities.begin() + posIdx); 
    // re-adding all widgets in the order
    for (int i = 0; i < static_cast<int>(this->m_Modalities.size()); ++i)
    {
      QmitkDataStorageComboBox *multiModalBox = this->m_Modalities[i];
      m_Controls.advancedSettingsLayout->addWidget(multiModalBox, this->m_UI_ROWS + i + 1, 1, 1, 3);
    }
    m_Controls.advancedSettingsLayout->update();
  }
}

void QmitknnUNetToolGUI::AutoParsePythonPaths()
{
  QString homeDir = QDir::homePath();
  std::vector<QString> searchDirs;
#ifdef _WIN32
  searchDirs.push_back(QString("C:") + QDir::separator() + QString("ProgramData") + QDir::separator() +
                       QString("anaconda3"));
#else
  // Add search locations for possible standard python paths here
  searchDirs.push_back(homeDir + QDir::separator() + "environments");
  searchDirs.push_back(homeDir + QDir::separator() + "anaconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "anaconda3");
#endif
  for (QString searchDir : searchDirs)
  {
    if (searchDir.endsWith("anaconda3", Qt::CaseInsensitive))
    {
      if (QDir(searchDir).exists())
      {
        m_Controls.pythonEnvComboBox->insertItem(0, "(base): " + searchDir);
        searchDir.append((QDir::separator() + QString("envs")));
      }
    }
    for (QDirIterator subIt(searchDir, QDir::AllDirs, QDirIterator::NoIteratorFlags); subIt.hasNext();)
    {
      subIt.next();
      QString envName = subIt.fileName();
      if (!envName.startsWith('.')) // Filter out irrelevent hidden folders, if any.
      {
        m_Controls.pythonEnvComboBox->insertItem(0, "(" + envName + "): " + subIt.filePath());
      }
    }
  }
  m_Controls.pythonEnvComboBox->setCurrentIndex(-1);
}

std::vector<std::string> QmitknnUNetToolGUI::FetchSelectedFoldsFromUI()
{
  std::vector<std::string> folds;
  if (!(m_Controls.foldBox->allChecked() || m_Controls.foldBox->noneChecked()))
  {
    QModelIndexList foldList = m_Controls.foldBox->checkedIndexes();
    foreach (QModelIndex index, foldList)
    {
      QString foldQString =
        m_Controls.foldBox->itemText(index.row()).split("_", QString::SplitBehavior::SkipEmptyParts).last();
      folds.push_back(foldQString.toStdString());
    }
  }
  return folds;
}

mitk::ModelParams QmitknnUNetToolGUI::MapToRequest(const QString &modelName,
                                                   const QString &taskName,
                                                   const QString &trainer,
                                                   const QString &planId,
                                                   const std::vector<std::string> &folds)
{
  mitk::ModelParams requestObject;
  requestObject.model = modelName.toStdString();
  requestObject.trainer = trainer.toStdString();
  requestObject.planId = planId.toStdString();
  requestObject.task = taskName.toStdString();
  requestObject.folds = folds;
  return requestObject;
}

void QmitknnUNetToolGUI::SegmentationProcessFailed()
{
  m_Controls.statusLabel->setText(
    "<b>STATUS: </b><i>Error in the segmentation process. No resulting segmentation can be loaded.</i>");
  this->setCursor(Qt::ArrowCursor);
  std::stringstream stream;
  stream << "Error in the segmentation process. No resulting segmentation can be loaded.";
  QMessageBox *messageBox = new QMessageBox(QMessageBox::Critical, nullptr, stream.str().c_str());
  messageBox->exec();
  delete messageBox;
  MITK_ERROR << stream.str();
}

void QmitknnUNetToolGUI::SegmentationResultHandler(mitk::nnUNetTool *tool)
{
  MITK_INFO << "Finished slot";
  tool->RenderOutputBuffer();
  this->SetLabelSetPreview(tool->GetMLPreview());
  tool->IsTimePointChangeAwareOn();
  m_Controls.statusLabel->setText("<b>STATUS: </b><i>Segmentation task finished successfully.</i>");
}
