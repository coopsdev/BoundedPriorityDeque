from conan import ConanFile
from conan.tools.meson import Meson
from conan.tools.files import copy


class Pkg(ConanFile):
    name = "bpd"
    version = "0.2.0"
    license = "MIT"
    author = "Cooper Larson | cooper.larson1@gmail.com"
    url = ""
    description = ("A lightweight, performant, header-only bounded-priority-deque"
                   " that supports: { min, max, custom-comparator }.")
    topics = ("c++", "data structures", "algorithms", "performance")
    settings = "os", "compiler", "arch", "build_type"
    requires = [
        "gtest/1.14.0"
    ]
    generators = "PkgConfigDeps", "MesonToolchain"
    exports_sources = "meson.build", "include/*", "test/*"
    implements = ["auto_header_only"]

    def layout(self):
        self.folders.source = '.'
        self.folders.build = 'build/meson'
        self.folders.generators = 'build/generators'
        self.folders.package = 'build/package'

    def build(self):
        meson = Meson(self)
        meson.configure()
        meson.build()

    def test(self):
        meson = Meson(self)
        meson.test()

    def package(self):
        copy(self, "*.h", self.source_folder, self.package_folder)

    def package_info(self):
        self.cpp_info.includedirs = ['include']
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []

    def package_id(self):
        self.info.clear()

