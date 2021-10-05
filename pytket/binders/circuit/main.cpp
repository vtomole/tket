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

#include <sstream>

namespace py = pybind11;

#include "Utils/UnitID.hpp"

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "UnitRegister.hpp"
#include "unit_downcast.hpp"

namespace tket {

template <typename T>
void declare_register(py::module &m, const std::string &typestr) {
  py::class_<UnitRegister<T>>(
      m, typestr.c_str(), "Linear register of UnitID types.")
      .def(
          py::init<const std::string &, std::size_t>(),
          ("Construct a new " + typestr + "." +
           "\n\n:param name: Name of the register." +
           "\n:param size: Size of register.")
              .c_str(),
          py::arg("name"), py::arg("size"))
      .def("__getitem__", &UnitRegister<T>::operator[])
      .def("__lt__", &UnitRegister<T>::operator<)
      .def("__eq__", &UnitRegister<T>::operator==)
      .def("__contains__", &UnitRegister<T>::contains)
      .def("__len__", &UnitRegister<T>::size)
      .def("__str__", &UnitRegister<T>::name)
      .def(
          "__repr__",
          [typestr](const UnitRegister<T> &reg) {
            return typestr + "(\"" + reg.name() + "\", " +
                   std::to_string(reg.size()) + ")";
          })
      .def_property(
          "name", &UnitRegister<T>::name, &UnitRegister<T>::set_name,
          "Name of register.")
      .def_property(
          "size", &UnitRegister<T>::size, &UnitRegister<T>::set_size,
          "Size of register.")
      .def(
          "__hash__",
          [](const UnitRegister<T> &reg) {
            return py::hash(py::make_tuple(reg.name(), reg.size()));
          })
      .def(
          "__copy__",
          [](const UnitRegister<T> &reg) { return UnitRegister<T>(reg); })
      .def("__deepcopy__", [](const UnitRegister<T> &reg, py::dict) {
        return UnitRegister<T>(reg);
      });
}
void init_unitid(py::module &m) {
  m.attr("_TEMP_REG_SIZE") = 32;
  m.attr("_TEMP_BIT_NAME") = "tk_SCRATCH_BIT";
  m.attr("_TEMP_BIT_REG_BASE") = "tk_SCRATCH_BITREG";

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
      .def(py::self == py::self)
      .def_property_readonly(
          "reg_name", &UnitID::reg_name, "Readable name of register")
      .def_property_readonly(
          "index", &UnitID::index,
          "Index vector describing position in the register. The "
          "length of this vector is the dimension of the register")
      .def_property_readonly(
          "type", &UnitID::type,
          "Type of unit, either ``UnitType.qubit`` or "
          "``UnitType.bit``");

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
          py::arg("name"), py::arg("index"))
      .def(py::pickle(
          [](const Qubit &q) {
            return py::make_tuple(q.reg_name(), q.index());
          },
          [](const py::tuple &t) {
            if (t.size() != 2)
              throw std::runtime_error(
                  "Invalid state: tuple size: " + std::to_string(t.size()));
            return Qubit(
                t[0].cast<std::string>(), t[1].cast<std::vector<unsigned>>());
          }));

  declare_register<Qubit>(m, "QubitRegister");
}

PYBIND11_MODULE(circuit, m) {
  init_unitid(m);
}

}  // namespace tket
