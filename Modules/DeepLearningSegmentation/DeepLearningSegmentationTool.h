/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef DeepLearningSegmentationTool_h
#define DeepLearningSegmentationTool_h

//#include <mitkAutoSegmentationTool.h>
//#include <MitkDeepLearningSegmentationExports.h>
//
//namespace us {
//class ModuleResource;
//}
//
//namespace mitk
//{
//  /**
//   * @class DeepLearningSegmentationTool
//   * @brief This is the base class for all Deep Learning Based Segmentations
//   */
//  class MITKDEEPLEARNINGSEGMENTATION_EXPORT DeepLearningSegmentationTool : public mitk::AutoSegmentationTool
//  {
//  public:
//    enum ImageType
//    {
//      SimpleITKImage,
//      MITKImage
//    };
//    /**
//     * @brief Getter for the icon of the module which is displayed in the Segmentation Plugin. 
//     * @return icon of the segmentation method
//     */
//    us::ModuleResource GetIconResource() const override;
//
//    //bool CanHandle(mitk::BaseData *referenceData) const override;
//
//     /**
//     * @brief Getter for the name of the module which is displayed in the Segmentation Plugin.
//     * @return name of the segmentation method
//     */
//    const char *GetName() const override;
//    const char **GetXPM() const override;
//
//     /**
//     * @brief Constructor
//     *
//     * @param pythonFolder the folder of the python code, should lie in "Modules/DeepLearningsegmentation"
//     * @param inputImageVarName the python variable name of the input image to segment
//     * @param pythonFileName the file name of the python script to execute. This is the entry point for the segmentation
//     * @param outputImageVarName the python variable name of the output image (segmentation)
//     */
//    DeepLearningSegmentationTool(std::string toolName,
//                                 std::string iconName,
//                                 std::string pythonFolder,
//                                 std::string inputImageVarName,
//                                 std::string pythonFileName,
//                                 std::string outputImageVarName,
//                                 ImageType imageType,
//                                 bool multilabel = false);
//    ~DeepLearningSegmentationTool() override;
//
//    void Activated() override;
//    void Deactivated() override;
//
//    /**
//     * @brief Executes the segmentation by running python code
//     *
//     * @throw mitk::Exception if something went wrong during a python call, python service is not found, or no input image is found
//     * @param networkPath the path to the trained network for segmentation
//     * @return the segmentation result as label set image
//     */
//    mitk::LabelSetImage::Pointer DoSegmentation(std::string networkPath);
//    /**
//     * @brief Executes the multilabel segmentation by running python code
//     *
//     * @throw mitk::Exception if something went wrong during a python call, python service is not found, or no input
//     * image is found
//     * @param networkPath the path to the trained network for segmentation
//     * @return the segmentation result as vector of label set image
//     */
//    std::vector<mitk::LabelSetImage::Pointer> DoMultilabelSegmentation(std::string networkPath);
//    /**
//     * @brief Get the input image for the semgentation which is currently selected in the Segmentation Plugin
//     *
//     * @throw mitk::Exception if the input is null
//     * @return input image for segmentation
//     */
//    mitk::Image::Pointer GetInputImage();
//    /**
//     * @brief Getter for m_SegmentaionRunning, to determine, if a segmentation is currently executed 
//     * @return m_SegmentaionRunning
//     */
//    bool IsSegmentationRunning();
//
//     /**
//     * @brief Getter for the data storage from the tool manager
//     * @return data storage
//     */
//    mitk::DataStorage* GetDataStorage();
//    /**
//     * @brief Getter for the reference data from the tool manager
//     * @return reference data
//     */
//    mitk::DataNode *GetReferenceData();
//
//    bool IsMultilabelSegmentation();
//
//  protected:
//    std::string m_PythonProjectPath;
//    std::string m_InputImageVarName;
//    std::string m_PythonFileName;
//    std::string m_OutputImageVarName;
//
//  private:
//    std::string m_IconName;
//    std::string m_ToolName;
//    bool m_SegmentationRunning;
//    ImageType m_ImageType;
//    bool m_MultilabelSegmentation;
//  };
//} // namespace mitk

#endif
