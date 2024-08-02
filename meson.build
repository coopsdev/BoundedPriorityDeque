project('bpd', 'cpp',
        version : '0.2.1',
        default_options : ['warning_level=3', 'cpp_std=c++23', 'optimization=3', 'buildtype=release'])

source_root = meson.source_root()

gtest_dep = dependency('gtest', required : true, main : true)

if get_option('buildtype') == 'debug'
    add_project_arguments('-DENABLE_DEBUG', language : 'cpp')

    test('testDeque', executable('testDeque', files(source_root + '/test/deque_tests.cpp'), dependencies : gtest_dep))
endif
