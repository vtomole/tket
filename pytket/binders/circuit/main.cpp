// Copyright 2019-2021 Cambridge Quantum Computing
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <sstream>
#include "Utils/UnitID.hpp"

namespace py = pybind11;

namespace tket {

PYBIND11_MODULE(circuit, m) {
  py::enum_<UnitType>(
      m, "UnitType",
      "Enum for data types of units in circuits (e.g. Qubits vs Bits).")
      .value("qubit", UnitType::Qubit, "A single Qubit");
  py::class_<UnitID>(
      m, "UnitID", "A handle to a computational unit (e.g. qubit, bit)")
      .def("__eq__", &UnitID::operator==)
      .def("__lt__", &UnitID::operator<)
      .def("__hash__", [](const UnitID &) { return 0; })
      .def("__copy__", [](const UnitID &id) { return UnitID(id); })
      .def(
          "__deepcopy__", [](const UnitID &id, py::dict) { return UnitID(id); })
      .def(py::self == py::self);
  py::class_<Qubit, UnitID>(m, "Qubit", "A handle to a qubit")
      .def(
          py::init<unsigned>(),
          "Constructs an id for some index in the default qubit "
          "register\n\n:param index: The index in the register",
          py::arg("index"))
      .def(
          py::init<const std::string &>(),
          "Constructs a named id (i.e. corresponding to a singleton "
          "register)\n\n:param name: The readable name for the id",
          py::arg("name"))
      .def(
          py::init<const std::string &, unsigned>(),
          "Constructs an indexed id (i.e. corresponding to an element "
          "in a linear register)\n\n:param name: The readable name for "
          "the register\n:param index: The numerical index",
          py::arg("name"), py::arg("index"))
      .def(
          py::init<const std::string &, unsigned, unsigned>(),
          "Constructs a doubly-indexed id (i.e. corresponding to an "
          "element in a grid register)\n\n:param name: The readable "
          "name for the register\n:param row: The row index\n:param "
          "col: The column index",
          py::arg("name"), py::arg("row"), py::arg("col"))
      .def(
          py::init<const std::string &, std::vector<unsigned> &>(),
          "Constructs an id with an arbitrary-dimensional "
          "index\n\n:param name: The readable name for the "
          "register\n:param index: The index vector",
          py::arg("name"), py::arg("index"));
}

}  // namespace tket
