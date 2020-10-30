include(FetchContent)

# include fmt
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 7.1.1
)
FetchContent_MakeAvailable(fmt)

# prevent fmt building library, use header-only
if(IS_DIRECTORY "${fmt_SOURCE_DIR}")
    set_property(DIRECTORY ${fmt_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)
endif()
