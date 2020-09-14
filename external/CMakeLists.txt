include(FetchContent)

# only include doctest if building tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  # doctest
  FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest
    GIT_TAG 2.4.0
  )
  # disable doctests' internal tests
  set(DOCTEST_WITH_TESTS OFF CACHE STRING "Omit doctest tests" FORCE)
  FetchContent_MakeAvailable(doctest)
endif()
