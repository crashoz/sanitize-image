cmake_minimum_required(VERSION 3.7...3.27)

# Compatibility with older cmake
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(sanitize-image VERSION 1.0
    DESCRIPTION "Image sanitizer"
    LANGUAGES C)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_subdirectory(src)
add_subdirectory(tests)

enable_testing()
add_test(NAME test COMMAND test_all)