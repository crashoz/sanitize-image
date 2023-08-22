find_package(PkgConfig REQUIRED)

pkg_check_modules(CHECK REQUIRED check)

if(NOT CHECK_FOUND)
	# TODO handle manual install
endif()