from conans import ConanFile, CMake, tools
from parktoma.vscconan import update_cpp_tools


class ItertoolsConan(ConanFile):
    name = "itertools"
    version = "0.1"
    generators = "cmake"

    exports_sources = "../src*", "../tests*"
    no_copy_source = True

    def requirements(self):
        self.requires("random_v/0.1")
        self.requires("fmt/6.0.0@bincrafters/stable")

    def configure(self):
        return super().configure()

    def imports(self):
        update_cpp_tools(
            self, c_cpp_properties_path="../.vscode/c_cpp_properties.json")

    def build(self):
        cmake = CMake(self, build_type="Debug")
        cmake.configure(source_folder="tests")
        cmake.build()

    def package(self):
        self.copy("*.hpp", dst="include/random_v", src="src")
