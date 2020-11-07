#include <doctest/doctest.h>

#include <clipper/clipper.hpp>

using namespace ClipperLib;

TEST_SUITE("Offset") {
  TEST_CASE("Empty ctor") {
    auto c = ClipperOffset{};

    c.AddPath({}, JoinType::Miter, EndType::ClosedLine);

    auto result = Paths{};
    c.Execute(result, 10.0);


  }


}
