from conans import ConanFile, CMake, tools


class ItertoolsConan(ConanFile):
    name = "itertools"
    version = "0.1"

    exports_sources = "src/*"
    no_copy_source = True

    def requirements(self):
        self.requires("fmt/7.1.2")

    def package(self):
        self.copy("*.hpp")
