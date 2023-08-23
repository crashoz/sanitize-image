file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${sanitize-image_SOURCE_DIR}/include/*.h")

find_package(TurboJpeg REQUIRED)

add_library(sanitize-image STATIC sanitize-image.c decoders.c encoders.c randomizers.c ${HEADER_LIST})
target_include_directories(sanitize-image PUBLIC ${TURBOJPEG_INCLUDE_DIRS} ../include)

message("${TURBOJPEG_LIBRARIES}")
target_link_libraries(sanitize-image PRIVATE ${TURBOJPEG_LIBRARIES} spng)

add_executable(main main.c)
target_link_libraries(main PUBLIC sanitize-image)