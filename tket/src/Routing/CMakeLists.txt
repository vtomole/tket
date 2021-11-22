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

project(${TKET_ROUTING})

add_library(${TKET_ROUTING}
    Qubit_Placement.cpp
    Swap_Analysis.cpp
    Board_Analysis.cpp
    Routing.cpp
    Slice_Manipulation.cpp
    subgraph_mapping.cpp
    Placement.cpp
    Verification.cpp)

add_dependencies(${TKET_ROUTING}
    ${TKET_UTILS}
    ${TKET_GRAPHS}
    ${TKET_CIRCUIT}
    ${TKET_ARCHITECTURE}
    ${TKET_CHARACTERISATION})

target_include_directories(${TKET_ROUTING} PRIVATE ${TKET_SRC_DIR})
