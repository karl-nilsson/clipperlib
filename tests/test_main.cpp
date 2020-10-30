#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <clipper/clipper.hpp>

using namespace ClipperLib;

TEST_CASE("testing empty paths") {
  Paths subject, clip, solution;
  Clipper clipper;

  clipper.Clear();

  clipper.AddPaths(subject, PolyType::Subject, true);
  clipper.AddPaths(clip, PolyType::Clip, true);

  bool result = clipper.Execute(ClipType::Intersection, solution, PolyFillType::EvenOdd, PolyFillType::EvenOdd);

  CHECK(result == true);
  CHECK(solution.size() == 0);


}
