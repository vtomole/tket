from conans import ConanFile, CMake, tools
import os

required_conan_version = ">=1.33.0"


class SymengineConan(ConanFile):
    name = "symengine"
    version = "0.8.2"
    description = "A fast symbolic manipulation library, written in C++"
    license = "MIT"
    topics = ("symbolic", "algebra")
    homepage = "https://symengine.org/"
    url = "https://github.com/conan-io/conan-center-index"
    exports_sources = ["CMakeLists.txt", "patches/**"]
    generators = "cmake"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "integer_class": ["boostmp", "gmp"],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "integer_class": "gmp",
    }
    short_paths = True

    _cmake = None

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def requirements(self):
        if self.options.integer_class == "boostmp":
            self.requires("boost/1.77.0")
        else:
            self.requires("gmp/6.2.1")

    def source(self):
        git = tools.Git(folder=self._source_subfolder)
        git.clone("https://github.com/cqc-alec/symengine.git", "fiasco")

    def _configure_cmake(self):
        if self._cmake is None:
            self._cmake = CMake(self)
            self._cmake.definitions["BUILD_TESTS"] = False
            self._cmake.definitions["BUILD_BENCHMARKS"] = False
            self._cmake.definitions["INTEGER_CLASS"] = self.options.integer_class
            self._cmake.definitions["MSVC_USE_MT"] = False
            self._cmake.definitions["Boost_NO_BOOST_CMAKE"] = True
            self._cmake.configure(
                source_folder=self._source_subfolder, build_folder=self._build_subfolder
            )
        return self._cmake

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy("LICENSE", dst="licenses", src=self._source_subfolder)
        cmake = self._configure_cmake()
        cmake.install()
        # [CMAKE-MODULES-CONFIG-FILES (KB-H016)]
        tools.remove_files_by_mask(self.package_folder, "*.cmake")
        # [DEFAULT PACKAGE LAYOUT (KB-H013)]
        tools.rmdir(os.path.join(self.package_folder, "CMake"))

    def package_info(self):
        self.cpp_info.libs = ["symengine"]
        if any("teuchos" in v for v in tools.collect_libs(self)):
            self.cpp_info.libs.append("teuchos")
        self.cpp_info.names["cmake_find_package"] = "symengine"
        # FIXME: symengine exports a non-namespaced `symengine` target.
        self.cpp_info.names["cmake_find_package_multi"] = "symengine"
