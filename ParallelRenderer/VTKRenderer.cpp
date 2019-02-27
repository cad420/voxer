#include "Renderer.h"
#include "UserManager.h"
#include "config/CameraConfig.h"
#include "config/TransferFunctionConfig.h"
#include "util/Debugger.h"
#include <algorithm>
#include <vtk/vtkCamera.h>
#include <vtk/vtkColorTransferFunction.h>
#include <vtk/vtkImageData.h>
#include <vtk/vtkImageReader.h>
#include <vtk/vtkJPEGWriter.h>
#include <vtk/vtkMultiVolume.h>
#include <vtk/vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtk/vtkOpenGLRenderWindow.h>
#include <vtk/vtkOpenGLRenderer.h>
#include <vtk/vtkPiecewiseFunction.h>
#include <vtk/vtkPointData.h>
#include <vtk/vtkUnsignedCharArray.h>
#include <vtk/vtkVolume.h>
#include <vtk/vtkVolumeProperty.h>

using namespace std;
using namespace ospcommon;
namespace o = ospray::cpp;

extern DatasetManager datasets;
extern UserManager users;

static Debugger debug("renderer");

Image VTKRenderer::renderImage(
    const CameraConfig &cameraConfig, const vector<VolumeConfig> &volumeConfigs,
    const vector<SliceConfig> &sliceConfigs,
    const vector<IsosurfaceConfig> &isosurfaceConfigs,
    const vector<string> &volumesToRender, const vec2i &size) {
  auto start = chrono::steady_clock::now();

  vector<vtkNew<vtkVolume>> volumes;
  vector<string> volumeIds;
  for (auto &volumeConfig : volumeConfigs) {
    vtkNew<vtkVolume> volume;
    vtkNew<vtkVolumeProperty> volumeProperty;
    vtkNew<vtkPiecewiseFunction> opacities;
    vtkNew<vtkColorTransferFunction> colors;

    const auto &tfcnConfig = volumeConfig.tfcnConfig;
    if (volumeConfig.ranges.size() != 0) {
      for (auto range : volumeConfig.ranges) {
        for (auto i = range.start; i < range.end && i <= 255; i++) {
          opacities->AddPoint(i, tfcnConfig.opacities[i]);
        }
      }
    } else {
      for (int i = 0; i < tfcnConfig.opacities.size(); i++) {
        opacities->AddPoint(i, tfcnConfig.opacities[i]);
      }
    }

    for (int i = 0; i < tfcnConfig.colors.size(); i++) {
      auto &color = tfcnConfig.colors;
      colors->AddRGBPoint(i, color[0], color[1], color[2]);
    }

    volumeProperty->SetColor(colors);
    volumeProperty->SetScalarOpacity(opacities);
    volumeProperty->ShadeOn();
    volumeProperty->SetInterpolationTypeToLinear();

    volume->SetProperty(volumeProperty);
    volume->SetPosition(volumeConfig.translate[0], volumeConfig.translate[1],
                        volumeConfig.translate[2]);
    volume->SetScale(volumeConfig.scale[0], volumeConfig.scale[1], volumeConfig.scale[2]);

    volumes.push_back(volume);
    volumeIds.push_back(volumeConfig.id);
  }

  vtkNew<vtkOpenGLRenderer> renderer;

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetMultiSamples(0);
  renderWindow->OffScreenRenderingOn();

  vtkOpenGLRenderWindow *window =
      vtkOpenGLRenderWindow::SafeDownCast(renderWindow);
  window->DebugOn();
  window->SetUseOffScreenBuffers(true);
  window->OffScreenRenderingOn();

  vtkNew<vtkGPUVolumeRayCastMapper> mapper;
  vtkOpenGLGPUVolumeRayCastMapper *mappergl =
      vtkOpenGLGPUVolumeRayCastMapper::SafeDownCast(mapper);
  mappergl->SetUseJittering(1);

  vtkNew<vtkMultiVolume> volumes;

  for (int i = 0; i < volumesToRender.size(); i++) {
    auto &id = volumesToRender[i];
    auto pos = find(volumeIds.begin(), volumeIds.end(), id);
    auto volume = volumes[pos - volumeIds.begin()];
    auto &config = volumeConfigs[pos - volumeIds.begin()];
    auto &datasetConfig = config.datasetConfig;
    auto &dataset = datasets.get(datasetConfig.name);

    // mapper->SetInputData(i, ???);
    volumes->SetVolume(volume, i);
  }
  volumes->SetMapper(mappergl);

  renderer->AddVolume(volumes);
  renderer->SetBackground(255.0, 255.0, 255.0);
  auto &camera = renderer->GetActiveCamera();

  camera->SetPosition(cameraConfig.pos[0], cameraConfig.pos[1],
                      cameraConfig.pos[2]);
  camera->SetViewUp(cameraConfig.up[0], cameraConfig.up[1], cameraConfig.up[2]);
  camera->SetFocalPoint(cameraConfig.dir[0], cameraConfig.dir[1],
                        cameraConfig.dir[2]);
  renderer->ResetCameraClippingRange();
  // renderer->ResetCamera();

  window->SetSize(size[0], size[1]);
  window->Render();

  vtkNew<vtkImageData> image;
  image->SetDimensions(size[0], size[1], 1);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
  window->GetPixelData(0, 0, size[0] - 1, size[1] - 1, 0,
                       vtkArrayDownCast<vtkUnsignedCharArray>(
                           image->GetPointData()->GetScalars()),
                       0);

  vtkNew<vtkJPEGWriter> writer;
  writer->SetFileName("result.jpg");
  writer->SetInputData(image);
  writer->Write();

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  debug.log(to_string(delta.count()) + " ms");

  return image;
};