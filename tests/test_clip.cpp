#include <doctest/doctest.h>

#include <clipper/clipper.hpp>

using namespace ClipperLib;

TEST_SUITE("Offset") {
  TEST_CASE("Empty ctor") {
    auto c = Clipper{};

    c.AddPath(Path{}, PolyType::Subject, true);

    auto result = Paths{};

    c.Execute(ClipType::Difference, result);


  }


}
