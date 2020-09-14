# list of all test names
set(TEST_NAMES
  test_main.cpp
)


add_executable(unit_test ${TEST_NAMES})

target_compile_features(unit_test PRIVATE cxx_std_17)

target_link_libraries(unit_test
  PRIVATE
    doctest::doctest
    polyclipping::polyclipping
)

add_test(NAME UnitTests COMMAND unit_test)
