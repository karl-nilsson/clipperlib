cmake_minimum_required(VERSION 3.15)

# detect if this is a subproject
if(NOT DEFINED PROJECT_NAME)
  set(IS_SUBPROJECT OFF)
endif()

# refuse to build in-tree
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR "Bulding in-source is not allowed!")
endif()

# project basics
project(clipper
  VERSION 6.4.2
  DESCRIPTION "Polygon clipping library"
  HOMEPAGE_URL "https://github.com/karl-nilsson/clipperlib"
  LANGUAGES CXX
)

# extra scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# disable tests if this is a submodule
if(NOT IS_SUBPROJECT)
  # automatically adds BUILD_TESTING switch, default ON
  include(CTest)
endif()

# documentation switch
option(CLIPPER_BUILD_DOC "Build library documentation" OFF)
# examples switch
option(CLIPPER_BUILD_EXAMPLES "Build example programs" ON)
# install switch
option(CLIPPER_ENABLE_INSTALL "Generate install target" ON)

# global settings
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(${PROJECT_NAME})
# create clipper::clipper alias
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# require C++17 support
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_sources(${PROJECT_NAME}
  PRIVATE
    src/clipper.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# add external dependencies
# add_subdirectory(external)

# tests switch
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

# documentation switch
if(CLIPPER_BUILD_DOC)
  add_subdirectory(doc)
endif()

if(CLIPPER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# install target
if(CLIPPER_ENABLE_INSTALL)

  include(GNUInstallDirs)

  include(CMakePackageConfigHelpers)

  set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

  install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT ${PROJECT_NAME}_Runtime
      NAMELINK_COMPONENT ${PROJECT_NAME}_Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT ${PROJECT_NAME}_Development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT ${PROJECT_NAME}_Runtime
  )

  # create polyclippingConfigVersion.cmake

  write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  # create polyclippingConfig.cmake
  configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
  )

  # export targets to a script
  install(
    EXPORT ${PROJECT_NAME}_Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${INSTALL_CONFIGDIR}
  )

  # install scripts
  install(
    FILES
      "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
      "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
      ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
  )

  # install headers
  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()
