test_srcs = [
    'basic.c',
    'read_file.c',
    'write_file.c'
]

foreach t : test_srcs
    name = t.split('.')[0]
    test_exe = executable(name, sources: t,
                          include_directories: inc)
    test(name, test_exe)
endforeach
