import os

from conan import ConanFile
from conan.tools.meson import Meson
from conan.tools.files import copy, save


class Pkg(ConanFile):
    name = "bpd"
    version = "0.2.4"
    license = "MIT"
    author = "Cooper Larson | cooper.larson1@gmail.com"
    url = ""
    description = ("A lightweight & performant, single-header bounded-priority-deque"
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
        copy(self, "*.hpp", self.source_folder, self.package_folder)

    def package_info(self):
        self.cpp_info.includedirs = ['include']
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []

    def package_id(self):
        self.info.clear()

    def merge_headers(self, filepath1, filepath2, output_filename):
        # Read files
        with open(filepath1, 'r') as f1, open(filepath2, 'r') as f2:
            content1 = f1.read()
            content2 = f2.read()

        # Create combined header content with include guards
        guard_name = output_filename.upper().replace('.', '_') + '_HPP'
        combined_content = f'#ifndef {guard_name}\n#define {guard_name}\n\n'
        combined_content += content1 + '\n' + content2
        combined_content += f'\n#endif // {guard_name}\n'

        # Write to build directory
        build_path = os.path.join(self.build_folder, output_filename)
        save(self, build_path, combined_content)
        self.output.info(f"Merged header file created at: {build_path}")

