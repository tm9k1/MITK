/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnUnetTool_h_Included
#define mitknnUnetTool_h_Included

#include "mitkSegWithPreviewTool.h"
#include "mitkCommon.h"
#include "mitkToolManager.h"
#include <MitkSegmentationExports.h>
#include <mitkStandardFileLocations.h>
#include <numeric>
#include <utility>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
   * @brief nnUNet parameter request object holding all model parameters for input.
   * Also holds output temporary directory path.
   */
  struct ModelParams
  {
    std::string task;
    std::vector<std::string> folds;
    std::string model;
    std::string trainer;
    std::string planId;
    std::string outputDir;
    std::string inputName;
    std::string timeStamp;

    size_t generateHash() const
    {
      std::string toHash;
      std::string foldsConcatenated = std::accumulate(folds.begin(), folds.end(), std::string(""));
      toHash += this->task;
      toHash += this->model;
      toHash += this->inputName;
      toHash += foldsConcatenated;
      toHash += this->timeStamp;
      size_t hashVal = std::hash<std::string>{}(toHash);
      return hashVal;
    }
  };

  /**
    \brief nnUNet segmentation tool.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT MonaiLabelTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(nnUNetTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;

    
    /**
     *  @brief Returns the DataStorage from the ToolManager
     */
    mitk::DataStorage *GetDataStorage();

    mitk::DataNode *GetRefNode();

  protected:
    /**
     * @brief Construct a new nnUNet Tool object and temp directory.
     *
     */
    MonaiLabelTool();

    /**
     * @brief Destroy the nnUNet Tool object and deletes the temp directory.
     *
     */
    ~MonaiLabelTool() = default;

    /**
     * @brief Overriden method from the tool manager to execute the segmentation
     * Implementation:
     * 1. Saves the inputAtTimeStep in a temporary directory.
     * 2. Copies other modalities, renames and saves in the temporary directory, if required.
     * 3. Sets RESULTS_FOLDER and CUDA_VISIBLE_DEVICES variables in the environment.
     * 3. Iterates through the parameter queue (m_ParamQ) and executes "nnUNet_predict" command with the parameters
     * 4. Expects an output image to be saved in the temporary directory by the python proces. Loads it as
     *    LabelSetImage and sets to previewImage.
     *
     * @param inputAtTimeStep
     * @param oldSegAtTimeStep
     * @param previewImage
     * @param timeStep
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;

  private:
    bool m_Predict;
    LabelSetImage::Pointer m_OutputBuffer;
    const std::string m_TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
  };
} // namespace mitk
#endif
