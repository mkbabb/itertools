from conans import ConanFile, CMake, tools


class ItertoolsConan(ConanFile):
    name = "itertools"
    version = "0.1"

    exports_sources = "include/*"
    no_copy_source = True

    requires = ["fmt/7.1.2", "catch2/2.13.2"]

    def package(self):
        self.copy("*.hpp")
