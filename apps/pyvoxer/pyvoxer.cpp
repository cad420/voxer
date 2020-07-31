#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <voxer/SliceRenderer.hpp>
#include <voxer/VolumeRenderer.hpp>
#include <voxer/utils.hpp>

using namespace std;
using namespace voxer;
namespace py = pybind11;

PYBIND11_MODULE(pyvoxer, m) {
  py::class_<Dataset> dataset(m, "Dataset");
  dataset.def(py::init())
      .def("get_slice", &Dataset::get_slice)
      .def_static("Load", &Dataset::Load);

  py::enum_<Dataset::Axis>(dataset, "Axis")
      .value("X", Dataset::Axis::X)
      .value("Y", Dataset::Axis::Y)
      .value("Z", Dataset::Axis::Z)
      .export_values();

  py::class_<DatasetStore>(m, "DatasetStore")
      .def(py::init())
      .def("load", &DatasetStore::load)
      .def("load_from_file", &DatasetStore::load_from_file)
      .def("load_from_json", &DatasetStore::load_from_json)
      .def("load_one", &DatasetStore::load_one)
      .def("add_from_json", &DatasetStore::add_from_json)
      .def("print", &DatasetStore::print)
      .def(
          "get",
          py::overload_cast<const std::string &, const std::string &, uint32_t>(
              &DatasetStore::get, py::const_),
          py::return_value_policy::reference);

  py::class_<Scene>(m, "Scene")
      .def(py::init())
      .def_readwrite("datasets", &Scene::datasets)
      .def_readwrite("tfcns", &Scene::tfcns)
      .def_readwrite("volumes", &Scene::volumes)
      .def_readwrite("isosurfaces", &Scene::isosurfaces)
      .def_readwrite("camera", &Scene::camera);

  py::class_<SceneDataset>(m, "SceneDataset")
      .def(py::init())
      .def_readwrite("name", &SceneDataset::name)
      .def_readwrite("variable", &SceneDataset::variable)
      .def_readwrite("timestep", &SceneDataset::timestep);

  py::class_<ControlPoint>(m, "ControlPoint")
      .def(py::init())
      .def_readwrite("x", &ControlPoint::x)
      .def_readwrite("y", &ControlPoint::y)
      .def_readwrite("hex_color", &ControlPoint::hex_color)
      .def_property(
          "color", [](ControlPoint &point) { return point.hex_color; },
          [](ControlPoint &point, const string &value) {
            point.hex_color = value;
            point.color = hex_color_to_float(value);
          });

  py::bind_vector<vector<ControlPoint>>(m, "TransferFunction");

  py::class_<Volume>(m, "Volume")
      .def(py::init())
      .def_readwrite("dataset", &Volume::dataset_idx)
      .def_readwrite("tfcn", &Volume::tfcn_idx)
      .def_readwrite("spacing", &Volume::spacing)
      .def_readwrite("range", &Volume::range)
      .def_readwrite("render", &Volume::render);

  py::class_<Isosurface>(m, "Isosurface")
      .def(py::init())
      .def_readwrite("value", &Isosurface::value)
      .def_readwrite("dataset", &Isosurface::dataset_idx)
      .def_readwrite("color", &Isosurface::color)
      .def_readwrite("render", &Isosurface::render);

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
  py::enum_<VolumeRenderer::Type>(volume_renderer, "Type")
      .value("OSPRay", VolumeRenderer::Type::OSPRay)
      .value("OpenGL", VolumeRenderer::Type::OpenGL)
      .export_values();
  volume_renderer.def(py::init<VolumeRenderer::Type>())
      .def("render", &VolumeRenderer::render)
      .def("get_colors", &VolumeRenderer::get_colors,
           py::return_value_policy::reference);

  py::class_<SliceRenderer>(m, "SliceRenderer")
      .def(py::init())
      .def("set_dataset", &SliceRenderer::set_dataset)
      .def("add_mark", py::overload_cast<Dataset *, const string &>(
                           &SliceRenderer::add_mark))
      .def("render", &SliceRenderer::render);

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