#include <vtk/vtkArray.h>
#include "MultivariateRenderer.h"

int tmp(int argc, char *argv[]) {
  vtkNew<vtkOpenGLRenderer> ren1;

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(ren1);
  renderWindow->SetMultiSamples(0);
  renderWindow->OffScreenRenderingOn();

  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(renderWindow);
  renWin->DebugOn();
  renWin->SetUseOffScreenBuffers(true);
  renWin->OffScreenRenderingOn();

  // Create the reader for the data
  vtkNew<vtkImageReader2> reader;
  reader->SetFileName("/run/media/ukabuer/B6A8919FA8915F25/Bucky-uchar-32/Bucky-uchar-32.raw");
  reader->SetFileDimensionality(3);
  reader->SetDataSpacing(1, 1, 1);
  reader->SetDataOrigin(0.0, 0.0, 0.0);
  reader->SetDataScalarType(VTK_UNSIGNED_CHAR);
  reader->SetDataExtent(0, 31, 0, 31, 0, 31);
  reader->SetDataByteOrderToLittleEndian();
  reader->UpdateWholeExtent();

  vtkNew<vtkImageReader2> reader2;
  reader2->SetFileName("/run/media/ukabuer/B6A8919FA8915F25/Bucky-uchar-32/Bucky-uchar-32.raw");
  reader2->SetFileDimensionality(3);
  reader2->SetDataSpacing(1, 1, 1);
  reader2->SetDataOrigin(0.0, 0.0, 0.0);
  reader2->SetDataScalarType(VTK_UNSIGNED_CHAR);
  reader2->SetDataExtent(0, 31, 0, 31, 0, 31);
  reader2->SetDataByteOrderToLittleEndian();
  reader2->UpdateWholeExtent();

  vtkNew<vtkPiecewiseFunction> opacityTransferFunction;
  opacityTransferFunction->AddPoint(60, 0.0);
  opacityTransferFunction->AddPoint(255, 0.2);

  // Create transfer mapping scalar value to color
  vtkNew<vtkColorTransferFunction> colorTransferFunction;
  colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(64.0, 0.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(128.0, 0.0, 0.0, 1.0);
  colorTransferFunction->AddRGBPoint(192.0, 0.0, 0.0, 1.0);
  colorTransferFunction->AddRGBPoint(255.0, 0.0, 0.0, 0.2);

  vtkNew<vtkColorTransferFunction> colorTransferFunction2;
  colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(64.0, 0.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(128.0, 1.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(192.0, 1.0, 0.0, 0.0);
  colorTransferFunction->AddRGBPoint(255.0, 0.2, 0.0, 0.0);

  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->SetColor(colorTransferFunction);
  volumeProperty->SetScalarOpacity(opacityTransferFunction);
  volumeProperty->ShadeOn();
  volumeProperty->SetInterpolationTypeToLinear();

  vtkNew<vtkVolume> vol;
  vol->SetProperty(volumeProperty);

  vtkNew<vtkVolumeProperty> volumeProperty2;
  volumeProperty2->SetColor(colorTransferFunction2);
  volumeProperty2->SetScalarOpacity(opacityTransferFunction);
  volumeProperty2->ShadeOn();
  volumeProperty2->SetInterpolationTypeToLinear();

  vtkNew<vtkVolume> vol2;
  vol2->SetProperty(volumeProperty2);
  vol2->SetPosition(0, 32, 0);

  vtkNew<vtkGPUVolumeRayCastMapper> mapper;
  vtkOpenGLGPUVolumeRayCastMapper* mappergl = vtkOpenGLGPUVolumeRayCastMapper::SafeDownCast(mapper);
  mappergl->SetUseJittering(1);

  vtkNew<vtkMultiVolume> volumes;
  volumes->SetMapper(mappergl);

  // auto *volData1 = vtkArray::CreateArray();
  // mapper->SetInputData(0, );
  mapper->SetInputConnection(0, reader->GetOutputPort());
  volumes->SetVolume(vol, 0);

  mapper->SetInputConnection(3, reader2->GetOutputPort());
  volumes->SetVolume(vol2, 3);

  ren1->AddVolume(volumes);
  ren1->SetBackground(255.0, 255.0, 255.0);
  ren1->GetActiveCamera()->Azimuth(45);
  ren1->GetActiveCamera()->Elevation(30);
  ren1->ResetCameraClippingRange();
  ren1->ResetCamera();

  renWin->SetSize(600, 600);
  renWin->Render();

  int *size = renWin->GetSize();
  vtkNew<vtkImageData> image;
  image->SetDimensions(size[0], size[1], 1);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
  renWin->GetPixelData(
    0, 0, size[0] - 1, size[1] - 1, 0,
    vtkArrayDownCast<vtkUnsignedCharArray>(image->GetPointData()->GetScalars()), 0
  );

  vtkNew<vtkJPEGWriter> writer;
  writer->SetFileName("result.jpg");
  writer->SetInputData(image);
  writer->Write();

  return EXIT_SUCCESS;
}
