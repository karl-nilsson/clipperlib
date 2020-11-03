include(FetchContent)

# by default, prevent external dependencies from building
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL YES)

# include fmt
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 7.1.1
)
set(FMT_INSTALL OFF)
set(BUILD_SHARED_LIBS OFF)
set(FMT_HEADER_ONLY ON)
FetchContent_MakeAvailable(fmt)

if(IS_DIRECTORY "${fmt_SOURCE_DIR}")
    set_property(DIRECTORY ${fmt_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)
endif()

