#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <voxer/Rendering/VolumeRenderer.hpp>

using namespace std;
using namespace voxer;
namespace py = pybind11;

PYBIND11_MODULE(pyvoxer, m) {
  py::class_<StructuredGrid, std::shared_ptr<StructuredGrid>> dataset(
      m, "Dataset");
  dataset.def(py::init())
      .def("get_slice", &StructuredGrid::get_slice)
      .def("get_histogram", &StructuredGrid::get_histogram,
           py::return_value_policy::take_ownership)
      .def_static("Load", &StructuredGrid::Load);

  py::enum_<StructuredGrid::Axis>(dataset, "Axis")
      .value("X", StructuredGrid::Axis::X)
      .value("Y", StructuredGrid::Axis::Y)
      .value("Z", StructuredGrid::Axis::Z)
      .export_values();

  py::class_<TransferFunction, std::shared_ptr<TransferFunction>>(
      m, "TransferFunction")
      .def(py::init())
      .def("add_point", &TransferFunction::add_point);

  py::class_<Volume, std::shared_ptr<Volume>>(m, "Volume")
      .def(py::init())
      .def("set_dataset",
           [](Volume &volume, std::shared_ptr<StructuredGrid> dataset) {
             volume.dataset = move(dataset);
           })
      .def("set_transfer_function",
           [](Volume &volume, std::shared_ptr<TransferFunction> tfcn) {
             volume.tfcn = move(tfcn);
           })
      .def_readwrite("spacing", &Volume::spacing)
      .def_readwrite("range", &Volume::range);

  py::class_<Isosurface, std::shared_ptr<Isosurface>>(m, "Isosurface")
      .def(py::init())
      .def_readwrite("value", &Isosurface::value)
      .def_readwrite("dataset", &Isosurface::dataset)
      .def_readwrite("color", &Isosurface::color);

  py::class_<Camera>(m, "Camera")
      .def(py::init())
      .def_readwrite("width", &Camera::width)
      .def_readwrite("height", &Camera::height)
      .def_readwrite("pos", &Camera::pos)
      .def_readwrite("up", &Camera::up)
      .def_readwrite("target", &Camera::target)
      .def_readwrite("enable_ao", &Camera::enable_ao);

  py::class_<Image> image(m, "Image");
  py::enum_<Image::Format>(image, "Format")
      .value("RAW", Image::Format::RAW)
      .value("JPEG", Image::Format::JPEG);
  py::enum_<Image::Quality>(image, "Quality")
      .value("HIGH", Image::Quality::HIGH)
      .value("MEDIUM", Image::Quality::MEDIUM)
      .value("LOW", Image::Quality::LOW);

  image.def(py::init())
      .def_readwrite("width", &Image::width)
      .def_readwrite("height", &Image::height)
      .def_readwrite("channels", &Image::channels)
      .def_readwrite("format", &Image::format)
      .def_readwrite("data", &Image::data)
      .def_static(
          "encode",
          py::overload_cast<const uint8_t *, uint32_t, uint32_t, uint8_t,
                            Image::Format, Image::Quality, bool>(
              &Image::encode))
      .def_static(
          "encode",
          py::overload_cast<const Image &, Image::Format, Image::Quality>(
              &Image::encode));

  py::class_<VolumeRenderer> volume_renderer(m, "VolumeRenderer");
  volume_renderer.def(py::init<const char *>())
      .def("set_camera", &VolumeRenderer::set_camera)
      .def("add_volume", &VolumeRenderer::add_volume)
      .def("add_isosurface", &VolumeRenderer::add_isosurface)
      .def("clear_scene", &VolumeRenderer::clear_scene)
      .def("render", &VolumeRenderer::render)
      .def("get_colors", &VolumeRenderer::get_colors,
           py::return_value_policy::reference);

  //  py::class_<SliceRenderer>(m, "SliceRenderer")
  //      .def(py::init())
  //      .def("set_dataset", &SliceRenderer::set_dataset)
  //      .def("add_mark", py::overload_cast<Dataset *, const string &>(
  //                           &SliceRenderer::add_mark))
  //      .def("render", &SliceRenderer::render);

  m.doc() = R"pbdoc(
        python bindings for voxer
        -----------------------
        .. currentmodule:: pyvoxer
        .. autosummary::
           :toctree: _generate
    )pbdoc";

#ifdef VERSION_INFO
  m.attr("__version__") = VERSION_INFO;
#else
  m.attr("__version__") = "dev";
#endif
}