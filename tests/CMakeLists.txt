set(TEST_SRC basic.c write_read_file.c basic.cpp write_read_file.cpp)


foreach(SRC ${TEST_SRC})
    string(REPLACE "." "-" EXE ${SRC})
    add_executable(${EXE} ${SRC})
    target_include_directories(${EXE} PUBLIC ${PROJECT_SOURCE_DIR}/include)
    set_property(TARGET ${EXE} PROPERTY CXX_STANDARD 11)
    add_test(NAME ${EXE} COMMAND ${EXE})
endforeach()
