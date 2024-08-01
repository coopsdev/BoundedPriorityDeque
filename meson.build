project('bpd', 'cpp',
        version : '0.2.0',
        default_options : ['warning_level=3', 'cpp_std=c++23', 'optimization=3', 'buildtype=release'])

source_root = meson.source_root()

gtest_dep = dependency('gtest', required : true, main : true)

if get_option('buildtype') == 'debug'
    add_project_arguments('-DENABLE_DEBUG', language : 'cpp')

    test('testHeap', executable('testHeap', files(source_root + '/test/heap_tests.cpp'), dependencies : gtest_dep))
endif
