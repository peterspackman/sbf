cmake_minimum_required(VERSION 3.0)
project(sbf LANGUAGES CXX C Fortran VERSION "0.0.1")
set(SBF_VERSION_MAJOR "0")
set(SBF_VERSION_MINOR "0")
set(SBF_VERSION_MINOR_MINOR "1")

# necessary to put sbf.mod into build directory
set(CMAKE_Fortran_MODULE_DIRECTORY ${PROJECT_BINARY_DIR})

enable_testing()
add_subdirectory(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
