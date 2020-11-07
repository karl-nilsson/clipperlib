#include <doctest/doctest.h>


#include <clipper/clipper.hpp>

using namespace ClipperLib;


TEST_SUITE("cInt") {
  TEST_CASE("cInt values") {
    auto i = cInt{};

    CHECK_EQ(i, 0);
  }




}
