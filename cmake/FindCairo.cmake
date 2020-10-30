#------------------------------------------------------------------------------
# Usage: find_package(Cairo [REQUIRED])
#
# Sets variables:
#     Cairo_FOUND
#     Cairo_INCLUDE_DIRS
#     Cairo_LIBRARIES
#     Cairo_LINK_LIBRARIES
#     Cairo_RUNTIME_LIBRARIES
#------------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)


# WARNING: win32 untested
if(WIN32)
  # Search for Cairo
  find_path(Cairo_INCLUDE_DIRS cairo/cairo.h)
  find_library(Cairo_LIBRARIES NAMES cairo)

  find_program(Cairo_RUNTIME_LIBRARIES NAMES cairo.dll)

  find_package_handle_standard_args(Cairo DEFAULT_MSG
    Cairo_INCLUDE_DIRS
    Cairo_LIBRARIES
    Cairo_RUNTIME_LIBRARIES
  )

  set(Cairo_LINK_LIBRARIES ${CAIRO_LIBRARIES})


else()

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(Cairo REQUIRED cairo)

  # On MacOS, should be installed via Macports
  # On Ubuntu, install with: apt-get install libcairo2-dev

  find_package_handle_standard_args(Cairo DEFAULT_MSG
    Cairo_INCLUDE_DIRS
    Cairo_LIBRARIES
    Cairo_LINK_LIBRARIES
  )
endif()
