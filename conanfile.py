from conan import ConanFile
from conan.tools.meson import Meson


class TSPConan(ConanFile):
    name = "bounded_priority_deque"
    version = "v0.1.0"
    license = "MIT"
    author = "Cooper Larson | cooper.larson1@gmail.com"
    url = ""
    description = ("A performant and minimal, header only bounded priority deque"
                   " that supports { min, max, custom-comparator }.")
    topics = ("c++", "data structures", "algorithms", "performance")
    settings = "os", "compiler", "arch", "build_type"
    requires = [
        "gtest/1.14.0"
    ]
    generators = "PkgConfigDeps", "MesonToolchain"
    exports_sources = "meson.build", "main.cpp", "test/*"

    def layout(self):
        self.folders.source = '.'
        self.folders.build = 'build/meson'  # primary build dir with executables and .o files
        self.folders.generators = 'build/generators'  # conan package locator/builder files
        self.folders.package = 'build/package'  # package output directory

    def build(self):
        meson = Meson(self)
        meson.configure()
        meson.build()

    def test(self):
        meson = Meson(self)
        meson.test()

    def package(self):
        meson = Meson(self)
        meson.install()
