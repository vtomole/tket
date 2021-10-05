#include <pybind11/pybind11.h>
#include "Utils/UnitID.hpp"

namespace py = pybind11;

namespace tket {

PYBIND11_MODULE(circuit, m) {
  py::class_<UnitID>(m, "UnitID", "A handle to a computational unit (e.g. qubit, bit)");
  py::class_<Qubit, UnitID>(m, "Qubit", "A handle to a qubit");
}

}  // namespace tket
