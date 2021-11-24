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
#
from conans import ConanFile, CMake, tools
import os


class SymengineConan(ConanFile):
    name = "symengine"
    version = "0.8.1.1"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    requires = "boost/1.77.0"

    def source(self):
        git = tools.Git()
        git.clone(
            "https://github.com/symengine/symengine",
            branch="2eb109a0e554b62683662cc5559fccf2ea0c0348",
            shallow=True,
        )

    def build(self):
        cmake = CMake(self)
        tools.replace_in_file(
            os.path.join(self.source_folder, "CMakeLists.txt"),
            "project(symengine)",
            """project(symengine)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()""",
        )
        cmake.configure(
            source_folder=self.source_folder,
            defs={
                "BUILD_TESTS": "OFF",
                "BUILD_BENCHMARKS": "OFF",
                "INTEGER_CLASS": "boostmp",
                "MSVC_USE_MT": "OFF",
            },
        )

        cmake.build()
        cmake.install()
        cmake.patch_config_paths()

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*symengine.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["symengine"]
