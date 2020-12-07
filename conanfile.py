from conans import ConanFile, CMake, tools


class ItertoolsConan(ConanFile):
    name = "itertools"
    version = "0.1"
    generators = "cmake"

    exports_sources = "../src*", "../tests*"
    no_copy_source = True

    def requirements(self):
        self.requires("random_v/0.1")
        self.requires("fmt/7.1.2")

    def configure(self):
        return super().configure()

    def build(self):
        # cmake = CMake(self, build_type="Debug")
        # cmake.configure(source_folder="tests")
        # cmake.build()
        pass

    def package(self):
        self.copy("*.hpp", dst="include/itertools", src="src")
