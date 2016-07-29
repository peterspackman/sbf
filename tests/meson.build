test_srcs = [
    'basic.c',
    'write_read_file.c'
]

foreach t : test_srcs
    name = t.split('.')[0]
    test_exe = executable(name, sources: t,
                          include_directories: inc)
    test(name, test_exe)
endforeach
