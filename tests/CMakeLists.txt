set(TEST_SRC basic.c write_read_file.c basic.cpp write_read_file.cpp)


foreach(SRC ${TEST_SRC})
    string(REPLACE "." "-" EXE ${SRC})
    add_executable(${EXE} ${SRC})
    target_include_directories(${EXE} PUBLIC ${PROJECT_SOURCE_DIR}/include)
    set_property(TARGET ${EXE} PROPERTY C_STANDARD 11)
    set_property(TARGET ${EXE} PROPERTY CXX_STANDARD 11)
    add_test(NAME ${EXE} COMMAND ${EXE})
endforeach()

add_executable(basic_fortran basic.F90)
target_link_libraries(basic_fortran sbf_fortran)
add_test(NAME basic_fortran COMMAND basic_fortran)

add_executable(write_read_file_fortran write_read_file.F90)
target_link_libraries(write_read_file_fortran sbf_fortran)
add_test(NAME write_read_file_fortran COMMAND write_read_file_fortran)
