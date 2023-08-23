# Look for libturbo-jpeg include dir and libraries

find_package(PkgConfig REQUIRED)
pkg_check_modules(TURBOJPEG turbojpeg)

# TODO Fix this var (does not read from cache ?)
set(TURBOJPEG_INSTALL_DIR "/opt/libjpeg-turbo" CACHE PATH "TurboJpeg install directory")

if(NOT TURBOJPEG_FOUND)
    if(TURBOJPEG_INSTALL_DIR)
        message(STATUS "Using override ${TURBOJPEG_INSTALL_DIR} to find TurboJpeg")
        set(TURBOJPEG_INCLUDE_DIR "${TURBOJPEG_INSTALL_DIR}/include")
        set(TURBOJPEG_INCLUDE_DIRS "${TURBOJPEG_INCLUDE_DIR}")

        find_library(JPEG_LIBRARY NAMES jpeg PATHS "${TURBOJPEG_INSTALL_DIR}/lib64")
        find_library(TURBOJPEG_LIBRARY NAMES turbojpeg PATHS "${TURBOJPEG_INSTALL_DIR}/lib64")
        set(TURBOJPEG_LIBRARIES "${JPEG_LIBRARY}" "${TURBOJPEG_LIBRARY}")

    else(TURBOJPEG_INSTALL_DIR)
        find_path(TURBOJPEG_INCLUDE_DIR jpeglib.h)
        find_library(TURBOJPEG_LIBRARIES NAMES jpeg turbojpeg)
    endif(TURBOJPEG_INSTALL_DIR)

    if(TURBOJPEG_INCLUDE_DIR AND TURBOJPEG_LIBRARIES)
        set(TURBOJPEG_FOUND 1)

        if(NOT TurboJpeg_FIND_QUIETLY)
            message(STATUS "Found TURBOJPEG: ${TURBOJPEG_LIBRARIES}")
        endif(NOT TurboJpeg_FIND_QUIETLY)
    else(TURBOJPEG_INCLUDE_DIR AND TURBOJPEG_LIBRARIES)
        if(TurboJpeg_FIND_REQUIRED)
            message(FATAL_ERROR "Could NOT find TURBOJPEG")
        else(TurboJpeg_FIND_REQUIRED)
            if(NOT TurboJpeg_FIND_QUIETLY)
                message(STATUS "Could NOT find TURBOJPEG")
            endif(NOT TurboJpeg_FIND_QUIETLY)
        endif(TurboJpeg_FIND_REQUIRED)
    endif(TURBOJPEG_INCLUDE_DIR AND TURBOJPEG_LIBRARIES)
endif(NOT TURBOJPEG_FOUND)