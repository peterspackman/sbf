test_srcs = [ 
    'basic.c', 
    'write_read_file.c',
    'basic.cpp',
    'write_read_file.cpp']

foreach t : test_srcs 
test_exe = executable(t, sources: t,
                         include_directories: inc)
    test(t, test_exe)
endforeach

basic_fortran = executable('basic_fortran', ['basic.F90', '../include/sbf.F90'], )
rw_fortran = executable('rw_fortran', ['write_read_file.F90', '../include/sbf.F90'])
test('basic_fortran', basic_fortran)
test('write_read_file_fortran', rw_fortran)
