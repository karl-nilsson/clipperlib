#include <doctest/doctest.h>

#include <clipper/clipper.hpp>

using namespace ClipperLib;

TEST_SUITE("Free Functions") {

  TEST_CASE("Orientation") {
    // simple 1x1 square
    Path path{IntPoint{},
        IntPoint{1,0},
        IntPoint{1,1},
        IntPoint(0,1)};

    SUBCASE("") {
      CHECK_EQ(Orientation(path), true);
    }
  }

  void check_square(int value) {
    Path path{IntPoint{},
              IntPoint{value, 0},
              IntPoint{value, value},
              IntPoint(0, value)};

    REQUIRE(Area(path) == doctest::Approx(value * value));
  }

  void check_triangle(int value) {
    Path path{IntPoint{},
              IntPoint{value, 0},
              IntPoint(0, value)};

    REQUIRE(Area(path) == doctest::Approx(value * value / 2.0));
  }

  TEST_CASE("Area") {
    SUBCASE("Squares") {
      // simple squares, ixi
      for(int i = 0; i < 4; ++i) {
        CAPTURE(i);
        check_square(i);

      }
    }
    SUBCASE("Triangles") {
      // simple triangles
      for(int i = 0; i < 4; ++i) {
        CAPTURE(i);
        check_triangle(i);
      }
    }
  }

  TEST_CASE("PointInPolygon") {

    SUBCASE("Basic") {
      Path path{IntPoint{},
                IntPoint{4,0},
                IntPoint{4,4},
                IntPoint{0,4}};

      // outside
      CHECK_EQ(PointInPolygon(IntPoint{-2,-2}, path), 0);
      // inside
      CHECK_EQ(PointInPolygon(IntPoint{2,2}, path), 1);
      // boundary
      CHECK_EQ(PointInPolygon(IntPoint{0,0}, path), -1);
    }

    SUBCASE("Self-Intersecting Polygon") {
      // figure 8
      Path path{IntPoint{},
                IntPoint{4, 4},
                IntPoint{0, 4},
                IntPoint{4, 0}};

      // outside
      CHECK_EQ(PointInPolygon(IntPoint{1, 2}, path), 0);
      CHECK_EQ(PointInPolygon(IntPoint{3, 2}, path), 0);
      // inside
      CHECK_EQ(PointInPolygon(IntPoint{2, 1}, path), 1);
      CHECK_EQ(PointInPolygon(IntPoint{2, 3}, path), 1);
      // boundary
      CHECK_EQ(PointInPolygon(IntPoint{}, path), -1);
      CHECK_EQ(PointInPolygon(IntPoint{2, 2}, path), -1);
      CHECK_EQ(PointInPolygon(IntPoint{0, 4}, path), -1);
      CHECK_EQ(PointInPolygon(IntPoint{4,0}, path), -1);
      CHECK_EQ(PointInPolygon(IntPoint{4, 4}, path), -1);
    }
  }

  TEST_CASE("SimplifyPolygon") {
    // SUBCASE("") { SimplifyPolygon(); }
  }


  TEST_CASE("SimplifyPolygons") {
    SUBCASE("") {
      // SimplifyPolygons();
    }
  }

  TEST_CASE("CleanPolygon") {
    /* SUBCASE() {
      CleanPolygon();
    }
    */
  }


  TEST_CASE("MinkowskiSum") {
  }

  TEST_CASE("MinkowskiDiff") {

  }

  TEST_CASE("PolyTreeToPaths") {

  }

  TEST_CASE("ClosedPathsFromPolyTree") {

  }

  TEST_CASE("OpenPathsFromPolyTree") {

  }


}
