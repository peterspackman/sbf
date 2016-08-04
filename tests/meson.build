test_srcs = [ 
    'basic.c', 
    'write_read_file.c',
    'basic.cpp',
    'write_read_file.cpp' ]

foreach t : test_srcs 
test_exe = executable(t, sources: t,
                         include_directories: inc)
    test(t, test_exe)
endforeach
