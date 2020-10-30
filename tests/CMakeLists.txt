include(FetchContent)

# doctest
FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/onqtam/doctest
  GIT_TAG 2.4.0
)
# disable doctests' internal tests
set(DOCTEST_WITH_TESTS OFF CACHE STRING "Omit doctest tests" FORCE)
FetchContent_MakeAvailable(doctest)


# list of all test names
set(TEST_NAMES
  test_main.cpp
)


add_executable(unit_test ${TEST_NAMES})

target_compile_features(unit_test PRIVATE cxx_std_17)

target_link_libraries(unit_test
  PRIVATE
    doctest::doctest
    clipper::clipper
)

add_test(NAME UnitTests COMMAND unit_test)
