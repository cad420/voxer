#include <memory>
#include <pybind11/pybind11.h>
#include <voxer/RenderingContext.hpp>

using namespace std;
using namespace voxer;
namespace py = pybind11;

int add(int i, int j) { return i + j; }

PYBIND11_MODULE(pyvoxer, m) {
  py::class_<RenderingContext>(m, "Renderer")
      .def(py::init([]() {
        return make_unique<RenderingContext>(RenderingContext::Type::OSPRay);
      }))
      .def("render", &RenderingContext::render);

  m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------
        .. currentmodule:: pyvoxer
        .. autosummary::
           :toctree: _generate
           add
           subtract
    )pbdoc";

  m.def("add", &add, R"pbdoc(
        Add two numbers
        Some other explanation about the add function.
    )pbdoc");

  m.def(
      "subtract", [](int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers
        Some other explanation about the subtract function.
    )pbdoc");

#ifdef VERSION_INFO
  m.attr("__version__") = VERSION_INFO;
#else
  m.attr("__version__") = "dev";
#endif
}