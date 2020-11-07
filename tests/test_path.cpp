#include <doctest/doctest.h>

#include <clipper/clipper.hpp>

using namespace ClipperLib;

TEST_SUITE("Path") {
  TEST_CASE("Empty ctor") {
    auto p = Path{};

    CHECK_EQ(p.size(), 0);

    SUBCASE("Append") {

      for (int i = 0; i < 10; ++i) {
        auto point = IntPoint{i, i * i};
        p << point;
        CHECK_EQ(p.size(), i + 1);
        CHECK_EQ(p[i], point);
      }
    }
  }

  TEST_CASE("Reverse") {
    auto path = Path{};
    auto path_reversed = Path{};

    for(int i = 0; i < 10; ++i) {
        auto point = IntPoint{i, i*i};
        path << point;
        path_reversed.insert(path_reversed.begin(), point);
    }

    ReversePath(path);

    REQUIRE_EQ(path, path_reversed);
  }
}
