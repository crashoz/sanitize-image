if(${BUILD_SHARED_LIBS})
    set(BUILD_STATIC_LIBS OFF)
else()
    set(BUILD_STATIC_LIBS ON)
endif()

include(ExternalProject)
ExternalProject_Add(libspng
    GIT_REPOSITORY https://github.com/randy408/libspng.git
    GIT_TAG fb768002d4288590083a476af628e51c3f1d47cd # release v0.7.4
    GIT_SHALLOW ON
    INSTALL_COMMAND ""
    UPDATE_DISCONNECTED OFF
    CMAKE_ARGS
    -DSPNG_SHARED=${BUILD_SHARED_LIBS}
    -DSPNG_STATIC=${BUILD_STATIC_LIBS}
)

ExternalProject_Add(libjpeg-turbo
    GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo.git
    GIT_TAG 6c87537f60941f3c265c339fe60d1e31d2a42ccf # release v3.0.0
    GIT_SHALLOW ON
    INSTALL_COMMAND ""
    UPDATE_DISCONNECTED OFF
    CMAKE_ARGS
    -DENABLE_SHARED=${BUILD_SHARED_LIBS}
    -DENABLE_STATIC=${BUILD_STATIC_LIBS}
)

ExternalProject_Get_Property(libspng BINARY_DIR)
set(LIBSPNG_BINARY_DIR "${BINARY_DIR}")
ExternalProject_Get_Property(libspng SOURCE_DIR)
set(LIBSPNG_SOURCE_DIR "${SOURCE_DIR}")

ExternalProject_Get_Property(libjpeg-turbo BINARY_DIR)
set(LIBJPEG_BINARY_DIR "${BINARY_DIR}")
ExternalProject_Get_Property(libjpeg-turbo SOURCE_DIR)
set(LIBJPEG_SOURCE_DIR "${SOURCE_DIR}")

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${sanitize-image_SOURCE_DIR}/include/*.h")

add_library(sanitize-image SHARED sanitize-image.c magic.c decoders.c encoders.c randomizers.c resizers.c utils.c ${HEADER_LIST})

target_include_directories(sanitize-image PUBLIC "${LIBSPNG_SOURCE_DIR}/spng" "${LIBJPEG_BINARY_DIR}" "${LIBJPEG_SOURCE_DIR}" ../include)
target_link_directories(sanitize-image PUBLIC "${LIBSPNG_BINARY_DIR}" "${LIBJPEG_BINARY_DIR}")
target_link_libraries(sanitize-image PUBLIC jpeg turbojpeg spng)

add_executable(main main.c)
target_link_libraries(main PUBLIC sanitize-image)

export(TARGETS sanitize-image FILE SanitizeImage.cmake)
install(TARGETS sanitize-image DESTINATION lib)
install(FILES include/sanitize-image.h DESTINATION include)
