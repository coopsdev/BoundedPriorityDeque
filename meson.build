project('bounded_priority_deque', 'cpp',
        version : '0.1.0',
        default_options : ['warning_level=3', 'cpp_std=c++23', 'optimization=3', 'buildtype=release'])

source_root = meson.source_root()

gtest_dep = dependency('gtest', required : true, main : true)

# Check if the build type is debug
if get_option('buildtype') == 'debug'
    add_project_arguments('-DENABLE_DEBUG', language : 'cpp')
endif

test('testHeap', executable('testHeap', files(source_root + '/test/heap_tests.cpp'), dependencies : gtest_dep))

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_MESON_LIBRARY']

shared_library('bounded_heap', 'main.cpp',
               install : true,
               cpp_args : lib_args
)
