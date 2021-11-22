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

set(TKET_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(tket-compiler)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_file(CONANBUILDINFO_FILE conanbuildinfo.cmake HINTS ${CMAKE_BINARY_DIR})
include(${CONANBUILDINFO_FILE})
conan_basic_setup()

set(TKET tket)

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

set(TKET_UTILS tket-utils)
set(TKET_ZX tket-zx)
set(TKET_OPTYPE tket-optype)
set(TKET_CLIFFORD tket-clifford)
set(TKET_OPS tket-ops)
set(TKET_GRAPHS tket-graphs)
set(TKET_GATE tket-gate)
set(TKET_PAULIGRAPH tket-pauligraph)
set(TKET_CIRCUIT tket-circuit)
set(TKET_ARCHITECTURE tket-architecture)

add_subdirectory(Utils)
add_subdirectory(ZX)
add_subdirectory(OpType)
add_subdirectory(Clifford)
add_subdirectory(Ops)
add_subdirectory(Graphs)
add_subdirectory(Gate)
add_subdirectory(PauliGraph)
add_subdirectory(Circuit)
add_subdirectory(Architecture)

set(TKET_AAS_DIR ${TKET_SRC_DIR}/ArchAwareSynth)
set(TKET_DIAGONALISATION_DIR ${TKET_SRC_DIR}/Diagonalisation)
set(TKET_SIMULATION_DIR ${TKET_SRC_DIR}/Simulation)
set(TKET_ROUTING_DIR ${TKET_SRC_DIR}/Routing)
set(TKET_TRANSFORM_DIR ${TKET_SRC_DIR}/Transformations)
set(TKET_CHARACTERISATION_DIR ${TKET_SRC_DIR}/Characterisation)
set(TKET_PREDS_DIR ${TKET_SRC_DIR}/Predicates)
set(TKET_CONVERTERS_DIR ${TKET_SRC_DIR}/Converters)
set(TKET_PROGRAM_DIR ${TKET_SRC_DIR}/Program)
set(TKET_MEASUREMENT_DIR ${TKET_SRC_DIR}/MeasurementSetup)

set(TKET_SOURCES

    # Simulation
    ${TKET_SIMULATION_DIR}/BitOperations.cpp
    ${TKET_SIMULATION_DIR}/CircuitSimulator.cpp
    ${TKET_SIMULATION_DIR}/DecomposeCircuit.cpp
    ${TKET_SIMULATION_DIR}/GateNode.cpp
    ${TKET_SIMULATION_DIR}/GateNodesBuffer.cpp
    ${TKET_SIMULATION_DIR}/PauliExpBoxUnitaryCalculator.cpp

    # Diagonalisation
    ${TKET_DIAGONALISATION_DIR}/DiagUtils.cpp
    ${TKET_DIAGONALISATION_DIR}/Diagonalisation.cpp
    ${TKET_DIAGONALISATION_DIR}/PauliPartition.cpp

    # Transformations
    ${TKET_TRANSFORM_DIR}/Combinator.cpp
    ${TKET_TRANSFORM_DIR}/Rebase.cpp
    ${TKET_TRANSFORM_DIR}/BasicOptimisation.cpp
    ${TKET_TRANSFORM_DIR}/PauliOptimisation.cpp
    ${TKET_TRANSFORM_DIR}/CliffordOptimisation.cpp
    ${TKET_TRANSFORM_DIR}/CliffordReductionPass.cpp
    ${TKET_TRANSFORM_DIR}/OptimisationPass.cpp
    ${TKET_TRANSFORM_DIR}/PhaseOptimisation.cpp
    ${TKET_TRANSFORM_DIR}/ControlledGates.cpp
    ${TKET_TRANSFORM_DIR}/Decomposition.cpp
    ${TKET_TRANSFORM_DIR}/Replacement.cpp
    ${TKET_TRANSFORM_DIR}/MeasurePass.cpp
    ${TKET_TRANSFORM_DIR}/ContextualReduction.cpp
    ${TKET_TRANSFORM_DIR}/ThreeQubitSquash.cpp

    # Routing
    ${TKET_ROUTING_DIR}/Qubit_Placement.cpp
    ${TKET_ROUTING_DIR}/Swap_Analysis.cpp
    ${TKET_ROUTING_DIR}/Board_Analysis.cpp
    ${TKET_ROUTING_DIR}/Routing.cpp
    ${TKET_ROUTING_DIR}/Slice_Manipulation.cpp
    ${TKET_ROUTING_DIR}/subgraph_mapping.cpp
    ${TKET_ROUTING_DIR}/Placement.cpp
    ${TKET_ROUTING_DIR}/Verification.cpp

    # Architecture Aware Synthesis
    ${TKET_AAS_DIR}/Path.cpp
    ${TKET_AAS_DIR}/SteinerTree.cpp
    ${TKET_AAS_DIR}/SteinerForest.cpp

    # Predicates
    ${TKET_PREDS_DIR}/Predicates.cpp
    ${TKET_PREDS_DIR}/CompilationUnit.cpp
    ${TKET_PREDS_DIR}/CompilerPass.cpp
    ${TKET_PREDS_DIR}/PassGenerators.cpp
    ${TKET_PREDS_DIR}/PassLibrary.cpp

    # Converters
    ${TKET_CONVERTERS_DIR}/CliffTableauConverters.cpp
    ${TKET_CONVERTERS_DIR}/PauliGadget.cpp
    ${TKET_CONVERTERS_DIR}/PauliGraphConverters.cpp
    ${TKET_CONVERTERS_DIR}/Gauss.cpp
    ${TKET_CONVERTERS_DIR}/PhasePoly.cpp

    # Program
    ${TKET_PROGRAM_DIR}/Program_accessors.cpp
    ${TKET_PROGRAM_DIR}/Program_analysis.cpp
    ${TKET_PROGRAM_DIR}/Program_iteration.cpp
    ${TKET_PROGRAM_DIR}/Program_manipulation.cpp
    ${TKET_PROGRAM_DIR}/Program_units.cpp

    # MeasurementSetup
    ${TKET_MEASUREMENT_DIR}/MeasurementSetup.cpp
    ${TKET_MEASUREMENT_DIR}/MeasurementReduction.cpp

    # Characterisation
    ${TKET_CHARACTERISATION_DIR}/Cycles.cpp
    ${TKET_CHARACTERISATION_DIR}/FrameRandomisation.cpp
    ${TKET_CHARACTERISATION_DIR}/DeviceCharacterisation.cpp
)

add_library(${TKET} SHARED ${TKET_SOURCES})
# ----- Location of header files ----------------------------------------------
target_include_directories(${TKET} PRIVATE ${TKET_SRC_DIR})
target_link_libraries(${TKET} PRIVATE ${TKET_UTILS} ${TKET_ZX} ${TKET_CLIFFORD} ${TKET_OPS} ${TKET_GRAPHS} ${TKET_GATE} ${TKET_PAULIGRAPH} ${TKET_CIRCUIT} ${TKET_ARCHITECTURE} ${CONAN_LIBS})
if(WIN32)
    target_link_libraries(${TKET} PRIVATE bcrypt)
ENDIF()
