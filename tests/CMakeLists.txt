# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

include(ExternalProject)
ExternalProject_Add(libcheck
    GIT_REPOSITORY https://github.com/libcheck/check.git
    GIT_TAG 11970a7e112dfe243a2e68773f014687df2900e8 # release v0.15.2
    GIT_SHALLOW ON
    INSTALL_COMMAND ""
    UPDATE_DISCONNECTED OFF
)

ExternalProject_Get_Property(libcheck BINARY_DIR)
set(LIBCHECK_BINARY_DIR "${BINARY_DIR}")
ExternalProject_Get_Property(libcheck SOURCE_DIR)
set(LIBCHECK_SOURCE_DIR "${SOURCE_DIR}")

add_executable(test_all test_sanitize-image.c)
add_dependencies(test_all libcheck)

target_include_directories(test_all PUBLIC "${LIBCHECK_SOURCE_DIR}/src")
target_link_directories(test_all PUBLIC "${LIBCHECK_BINARY_DIR}/src")
target_link_libraries(test_all sanitize-image check)
