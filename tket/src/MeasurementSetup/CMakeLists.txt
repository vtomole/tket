# Copyright 2019-2021 Cambridge Quantum Computing
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(${TKET_MEASUREMENT})

add_library(${TKET_MEASUREMENT}
    MeasurementSetup.cpp
    MeasurementReduction.cpp)

target_include_directories(${TKET_MEASUREMENT} PRIVATE ${TKET_SRC_DIR})
target_link_libraries(${TKET_MEASUREMENT}
    PRIVATE
    ${TKET_UTILS}
    ${TKET_OPTYPE}
    ${TKET_GATE}
    ${TKET_CIRCUIT}
    ${TKET_CLIFFORD}
    ${TKET_DIAGONALISATION}
    ${TKET_CONVERTERS}
    ${CONAN_LIBS})
