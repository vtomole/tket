#include <pybind11/pybind11.h>
#include "Utils/UnitID.hpp"

PYBIND11_MODULE(circuit, m) {
  pybind11::class_<UnitID>(m, "UnitID", "A handle to a UnitID");
}

