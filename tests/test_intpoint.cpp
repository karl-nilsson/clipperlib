#include <doctest/doctest.h>

#include <clipper/clipper.hpp>

using namespace ClipperLib;

TEST_SUITE("IntPoint") {

  TEST_CASE("Empty Constructor") {
    const auto i = IntPoint{};

    CHECK_EQ(i.X, 0);
    CHECK_EQ(i.Y, 0);

#ifdef use_xyz
    CHECK_EQ(i.Z, 0);
#endif
  }

  TEST_CASE("Default Constructor") {
    const auto z = 10000;
#ifndef use_xyz
    const auto i = IntPoint{z, z};

    CHECK_EQ(i.X, z);
    CHECK_EQ(i.Y, z);

#else
    const auto i = IntPoint{10000, 10000, 10000};

    CHECK_EQ(i.X, z);
    CHECK_EQ(i.Y, z);
    CHECK_EQ(i.Z, z)

    CHECK_EQ(i.Z, 0);
#endif
  }

  TEST_CASE("Equality & Inequality") {

    CHECK_EQ(IntPoint{}, IntPoint{});
#ifndef use_xyz
    CHECK_EQ(IntPoint{100, 100}, IntPoint{100, 100});
    CHECK_NE(IntPoint{}, IntPoint{1, 1});
#else
    CHECK_EQ(IntPoint{100, 100, 100}, IntPoint{100, 100, 100});
    CHECK_NE(IntPoint{}, IntPoint{1, 1, 1});
#endif
  }
}
