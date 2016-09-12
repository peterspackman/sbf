cmake_minimum_required(VERSION 3.0)
project(sbf LANGUAGES CXX C Fortran VERSION "0.0.1")
set(SBF_VERSION_MAJOR "0")
set(SBF_VERSION_MINOR "0")
set(SBF_VERSION_MINOR_MINOR "1")

enable_testing()
add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
