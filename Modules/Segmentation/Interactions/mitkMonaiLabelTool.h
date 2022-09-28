/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKMONAILABELTOOL_H
#define MITKMONAILABELTOOL_H

#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>
#include <mitkIRESTManager.h>
#include <memory>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  // class Image;

  struct DataObject //rename --ashis
  {
    std::string URL;
    unsigned short port;
    std::string origin;
  };

  class MITKSEGMENTATION_EXPORT MonaiLabelTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(MonaiLabelTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void GetOverallInfo(std::string);
    std::unique_ptr<DataObject> m_Parameters; //contains all parameters from Server to serve the GUI


  protected:
    MonaiLabelTool();
    ~MonaiLabelTool() = default;

    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
   
  
  private:
    void InitializeRESTManager();
    std::unique_ptr<DataObject> DataMapper(web::json::value&);

    mitk::IRESTManager *m_RESTManager;

  }; // class
} // namespace
#endif
