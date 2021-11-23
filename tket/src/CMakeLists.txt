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

cmake_minimum_required(VERSION 3.16.0)
project(tket)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TKET_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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
set(TKET_SIMULATION tket-simulation)
set(TKET_DIAGONALISATION tket-diagonalisation)
set(TKET_PROGRAM tket-program)
set(TKET_CHARACTERISATION tket-characterisation)
set(TKET_CONVERTERS tket-converters)
set(TKET_ROUTING tket-routing)
set(TKET_MEASUREMENT tket-measurement)
set(TKET_TRANSFORM tket-transform)
set(TKET_AAS tket-aas)
set(TKET_PREDICATES tket-predicates)

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
add_subdirectory(Simulation)
add_subdirectory(Diagonalisation)
add_subdirectory(Program)
add_subdirectory(Characterisation)
add_subdirectory(Converters)
add_subdirectory(Routing)
add_subdirectory(MeasurementSetup)
add_subdirectory(Transformations)
add_subdirectory(ArchAwareSynth)
add_subdirectory(Predicates)

add_library(${TKET} INTERFACE)
target_link_libraries(${TKET}
    INTERFACE
    ${TKET_UTILS}
    ${TKET_ZX}
    ${TKET_CLIFFORD}
    ${TKET_OPS}
    ${TKET_GRAPHS}
    ${TKET_GATE}
    ${TKET_PAULIGRAPH}
    ${TKET_CIRCUIT}
    ${TKET_ARCHITECTURE}
    ${TKET_SIMULATION}
    ${TKET_DIAGONALISATION}
    ${TKET_PROGRAM}
    ${TKET_CHARACTERISATION}
    ${TKET_CHARACTERISATION}
    ${TKET_CONVERTERS}
    ${TKET_ROUTING}
    ${TKET_MEASUREMENT}
    ${TKET_TRANSFORM}
    ${TKET_AAS}
    ${TKET_PREDICATES}
    ${CONAN_LIBS})
if(WIN32)
    target_link_libraries(${TKET} INTERFACE bcrypt)
ENDIF()
