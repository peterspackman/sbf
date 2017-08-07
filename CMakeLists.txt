cmake_minimum_required(VERSION 3.1)
project(sbf LANGUAGES CXX C VERSION "0.0.1")
set(SBF_VERSION_MAJOR "0")
set(SBF_VERSION_MINOR "0")
set(SBF_VERSION_MINOR_MINOR "1")
set(WITH_SBF_TESTS NO CACHE BOOL "Build the sbf tests")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT SBF_DISABLE_FORTRAN)
  enable_language(Fortran)
endif()

# necessary to put sbf.mod into build directory
set(CMAKE_Fortran_MODULE_DIRECTORY ${PROJECT_BINARY_DIR})
if(NOT ${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    set(SBF_MODULE_DIRECTORY ${CMAKE_Fortran_MODULE_DIRECTORY} PARENT_SCOPE)
    set(SBF_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
else()
    #TODO set ctest configuration type to include tests
    # can do manually with ctest -C WITH_SBF_TESTS 
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/include)
if(WITH_SBF_TESTS)
    enable_testing()
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
