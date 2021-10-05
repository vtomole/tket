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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_minimum_required(VERSION 3.16.0)

set(BUBBLE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(tket-compiler)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_file(CONANBUILDINFO_FILE conanbuildinfo.cmake HINTS ${CMAKE_BINARY_DIR})
include(${CONANBUILDINFO_FILE})
conan_basic_setup()

set(TKET "tket")

IF (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /EHsc")
ELSE()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wunreachable-code -Wunused")
ENDIF()
if(CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winconsistent-missing-override -Wloop-analysis")
endif()

set(PROFILE_COVERAGE no CACHE BOOL "Build library with profiling for test coverage")
IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    IF (PROFILE_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
        # Bug in gcc 10: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95353
        IF (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-stringop-overflow")
        ENDIF()
    ENDIF()
ENDIF()

IF (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS yes)
ELSEIF(APPLE)
    # set correct install_name
    set(CMAKE_INSTALL_NAME_DIR "@loader_path")
    set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
ENDIF()

set(BUBBLE_ARCHITECTURE_DIR ${BUBBLE_SRC_DIR}/Architecture)
set(BUBBLE_AAS_DIR ${BUBBLE_SRC_DIR}/ArchAwareSynth)
set(BUBBLE_CIRCUIT_DIR ${BUBBLE_SRC_DIR}/Circuit)
set(BUBBLE_CLIFFORD_DIR ${BUBBLE_SRC_DIR}/Clifford)
set(BUBBLE_DIAGONALISATION_DIR ${BUBBLE_SRC_DIR}/Diagonalisation)
set(BUBBLE_GRAPH_DIR ${BUBBLE_SRC_DIR}/Graphs)
set(BUBBLE_UTILS_DIR ${BUBBLE_SRC_DIR}/Utils)
set(BUBBLE_OPTYPE_DIR ${BUBBLE_SRC_DIR}/OpType)
set(BUBBLE_OPS_DIR ${BUBBLE_SRC_DIR}/Ops)
set(BUBBLE_GATE_DIR ${BUBBLE_SRC_DIR}/Gate)
set(BUBBLE_SIMULATION_DIR ${BUBBLE_SRC_DIR}/Simulation)
set(BUBBLE_ROUTING_DIR ${BUBBLE_SRC_DIR}/Routing)
set(BUBBLE_TRANSFORM_DIR ${BUBBLE_SRC_DIR}/Transformations)
set(BUBBLE_CHARACTERISATION_DIR ${BUBBLE_SRC_DIR}/Characterisation)
set(BUBBLE_PREDS_DIR ${BUBBLE_SRC_DIR}/Predicates)
set(BUBBLE_PAULIGRAPH_DIR ${BUBBLE_SRC_DIR}/PauliGraph)
set(BUBBLE_CONVERTERS_DIR ${BUBBLE_SRC_DIR}/Converters)
set(BUBBLE_PROGRAM_DIR ${BUBBLE_SRC_DIR}/Program)
set(BUBBLE_MEASUREMENT_DIR ${BUBBLE_SRC_DIR}/MeasurementSetup)

set(TKET_SOURCES

    # OpType
    ${BUBBLE_OPTYPE_DIR}/OpDesc.cpp
    ${BUBBLE_OPTYPE_DIR}/OpTypeInfo.cpp
    ${BUBBLE_OPTYPE_DIR}/OpTypeFunctions.cpp

    # Ops
    ${BUBBLE_OPS_DIR}/Conditional.cpp
    ${BUBBLE_OPS_DIR}/ConjugatePauliFunctions.cpp
    ${BUBBLE_OPS_DIR}/FlowOp.cpp
    ${BUBBLE_OPS_DIR}/MetaOp.cpp
    ${BUBBLE_OPS_DIR}/Op.cpp
    ${BUBBLE_OPS_DIR}/ClassicalOps.cpp

    # Gate
    ${BUBBLE_GATE_DIR}/Gate.cpp
    ${BUBBLE_GATE_DIR}/GatePtr.cpp
    ${BUBBLE_GATE_DIR}/OpPtrFunctions.cpp
    ${BUBBLE_GATE_DIR}/Rotation.cpp
    ${BUBBLE_GATE_DIR}/SymTable.cpp
    ${BUBBLE_GATE_DIR}/GateUnitaryMatrix.cpp
    ${BUBBLE_GATE_DIR}/GateUnitaryMatrixComposites.cpp
    ${BUBBLE_GATE_DIR}/GateUnitaryMatrixError.cpp
    ${BUBBLE_GATE_DIR}/GateUnitaryMatrixFixedMatrices.cpp
    ${BUBBLE_GATE_DIR}/GateUnitaryMatrixPrimitives.cpp
    ${BUBBLE_GATE_DIR}/GateUnitaryMatrixUtils.cpp
    ${BUBBLE_GATE_DIR}/GateUnitaryMatrixVariableQubits.cpp
    ${BUBBLE_GATE_DIR}/GateUnitarySparseMatrix.cpp

    # Clifford
    ${BUBBLE_CLIFFORD_DIR}/CliffTableau.cpp

    # Utils
    ${BUBBLE_UTILS_DIR}/TketLog.cpp
    ${BUBBLE_UTILS_DIR}/UnitID.cpp
    ${BUBBLE_UTILS_DIR}/HelperFunctions.cpp
    ${BUBBLE_UTILS_DIR}/MatrixAnalysis.cpp
    ${BUBBLE_UTILS_DIR}/PauliStrings.cpp
    ${BUBBLE_UTILS_DIR}/CosSinDecomposition.cpp
    ${BUBBLE_UTILS_DIR}/Expression.cpp

    # PauliGraph
    ${BUBBLE_PAULIGRAPH_DIR}/PauliGraph.cpp
)

add_library(${TKET} SHARED ${TKET_SOURCES})
# ----- Location of header files ----------------------------------------------
target_include_directories(${TKET} PRIVATE ${BUBBLE_SRC_DIR})
target_link_libraries(${TKET} ${CONAN_LIBS})
if(WIN32)
    target_link_libraries(${TKET} bcrypt)
ENDIF()
