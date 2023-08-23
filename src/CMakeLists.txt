file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${sanitize-image_SOURCE_DIR}/include/*.h")

add_library(sanitize-image STATIC sanitize-image.c decoders.c encoders.c randomizers.c ${HEADER_LIST})
target_include_directories(sanitize-image PUBLIC ../include)
target_link_libraries(sanitize-image PRIVATE spng turbojpeg)

add_executable(main main.c)
target_link_libraries(main PUBLIC sanitize-image)