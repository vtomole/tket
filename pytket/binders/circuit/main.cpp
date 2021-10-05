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

#include "Utils/Constants.hpp"
#include "Utils/Symbols.hpp"
#include "binder_utils.hpp"
#include "typecast.hpp"

namespace py = pybind11;

namespace tket {

void init_unitid(py::module &m);

PYBIND11_MODULE(circuit, m) {
  init_unitid(m);
  py::enum_<BasisOrder>(
      m, "BasisOrder",
      "Enum for readout basis and ordering.\n"
      "Readouts are viewed in increasing lexicographic order (ILO) of "
      "the bit's UnitID. This is our default convention for column "
      "indexing for ALL readout forms (shots, counts, statevector, and "
      "unitaries). e.g. :math:`\\lvert abc \\rangle` corresponds to the "
      "readout: ('c', 0) --> :math:`a`, ('c', 1) --> :math:`b`, ('d', 0) "
      "--> :math:`c`\n"
      "For statevector and unitaries, the string abc is interpreted as "
      "an index in a big-endian (BE) fashion. e.g. the statevector "
      ":math:`(a_{00}, a_{01}, a_{10}, a_{11})`\n"
      "Some backends (Qiskit, ProjectQ, etc.) use a DLO-BE (decreasing "
      "lexicographic order, big-endian) convention. This is the same as "
      "ILO-LE (little-endian) for statevectors and unitaries, but gives "
      "shot tables/readouts in a counter-intuitive manner.\n"
      "Every backend and matrix-based box has a BasisOrder option which "
      "can toggle between ILO-BE (ilo) and DLO-BE (dlo).")
      .value(
          "ilo", BasisOrder::ilo,
          "Increasing Lexicographic Order of UnitID, big-endian")
      .value(
          "dlo", BasisOrder::dlo,
          "Decreasing Lexicographic Order of UnitID, big-endian");
}

}  // namespace tket
