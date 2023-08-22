include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

add_executable(test_all test_sanitize-image.c)
target_link_libraries(test_all sanitize-image ${CHECK_LIBRARIES})

# check_pic rt m subunit