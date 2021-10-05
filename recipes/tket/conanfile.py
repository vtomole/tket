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

from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import os
import shutil


class TketConan(ConanFile):
    name = "tket"
    version = "1.0.1"
    license = "CQC Proprietary"
    author = "Alec Edgington <alec.edgington@cambridgequantum.com>"
    url = "https://github.com/CQCL-DEV/tket"
    description = "Quantum SDK"
    topics = ("quantum", "computation", "compiler")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True], "profile_coverage": [True, False]}
    default_options = {"shared": True, "profile_coverage": False}
    generators = "cmake"
    # Putting "patches" in both "exports_sources" and "exports" means that this works
    # in either the CI workflow (`conan create`) or the development workflow
    # (`conan build`). Maybe there is a better way?
    exports_sources = ["../../bubble/src/*", "!*/build/*", "patches/*"]
    exports = ["patches/*"]

    _cmake = None

    def _configure_cmake(self):
        if self._cmake is None:
            self._cmake = CMake(self)
            self._cmake.definitions["PROFILE_COVERAGE"] = self.options.profile_coverage
            self._cmake.configure()
        return self._cmake

    def validate(self):
        if self.options.profile_coverage and self.settings.compiler != "gcc":
            raise ConanInvalidConfiguration(
                "`profile_coverage` option only available with gcc"
            )

    def configure(self):
        pass

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy("*.hpp", dst="include")
        self.copy("*.dll", dst="lib", keep_path=False)
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["tket"]
