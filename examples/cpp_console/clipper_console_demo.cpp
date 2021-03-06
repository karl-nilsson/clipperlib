#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <clipper/clipper.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

using namespace ClipperLib;

//---------------------------------------------------------------------------
// SVGBuilder class
// a very simple class that creates an SVG image file
//---------------------------------------------------------------------------

class SVGBuilder {
  /**
   * @brief ColorToHtml Convert color value to HTML hex string
   * @param color Color value
   * @return string of hex color value
   */
  static std::string ColorToHtml(unsigned color) {
    // TODO: fmt
    // return fmt::format("#{:06X}", color)
    std::stringstream ss;
    ss << '#' << std::hex << std::setfill('0') << std::setw(6) << (color & 0xFFFFFF);
    return ss.str();
  }
  //------------------------------------------------------------------------------

  /**
   * @brief GetAlphaAsFrac Extract alpha value from color value
   * @param color Color value
   * @return alpha value as float
   */
  static float GetAlphaAsFrac(unsigned color) { return ((float)(color >> 24) / 255); }
  //------------------------------------------------------------------------------

  class StyleInfo {
  public:
    PolyFillType pft;
    unsigned     brushColor;
    unsigned     penColor;
    double       penWidth;
    bool         showCoords;

    StyleInfo() : pft(PolyFillType::NonZero) {
      brushColor = 0xFFFFFFCC;
      penColor   = 0xFF000000;
      penWidth   = 0.8;
      showCoords = false;
    }
  };

  class PolyInfo {
  public:
    Paths     paths;
    StyleInfo si;

    PolyInfo(Paths paths, const StyleInfo& style) {
      this->paths = paths;
      this->si    = style;
    }
  };

  typedef std::vector<PolyInfo> PolyInfoList;

private:
  PolyInfoList             polyInfos;
  static const std::string svg_xml_start[];
  static const std::string poly_end[];

public:
  StyleInfo style;

  void AddPaths(Paths& poly) {
    if(poly.empty())
      return;
    polyInfos.push_back(PolyInfo(poly, style));
  }

  bool SaveToFile(const std::string& filename, double scale = 1.0, int margin = 10) {
    // calculate the bounding rectangle
    bool empty = true;
    IntRect rec;
    rec.left   = std::numeric_limits<cInt>::max();
    rec.right  = std::numeric_limits<cInt>::min();
    rec.top    = std::numeric_limits<cInt>::max();
    rec.bottom = std::numeric_limits<cInt>::min();

    // find correct values for bounding rectangle
    for(const auto &p: polyInfos) {
      for(const auto &path: p.paths) {
        for(const auto &point: path) {
          if(empty) {
            empty = false;
          }

          if(point.X < rec.left) {
            rec.left = point.X;
          } else if(point.X > rec.right) {
            rec.right = point.X;
          }
          if(point.Y < rec.top) {
            rec.top = point.Y;
          } else if(point.Y > rec.bottom) {
            rec.bottom = point.Y;
          }
        }
      }
    }
    // short-circuit if all paths are empty
    if(empty) {
        return false;
    }

    // calculate scale and offset
    if(scale == 0)
      scale = 1.0;
    if(margin < 0)
      margin = 0;
    rec.left     = (cInt)((double)rec.left * scale);
    rec.top      = (cInt)((double)rec.top * scale);
    rec.right    = (cInt)((double)rec.right * scale);
    rec.bottom   = (cInt)((double)rec.bottom * scale);
    cInt offsetX = -rec.left + margin;
    cInt offsetY = -rec.top + margin;

    std::ofstream file;
    file.open(filename);
    if(!file.is_open())
      return false;
    file.setf(std::ios::fixed);
    file.precision(0);
    // clang-format off
    file << svg_xml_start[0] << ((rec.right - rec.left) + margin * 2) << "px"
         << svg_xml_start[1] << ((rec.bottom - rec.top) + margin * 2) << "px"
         << svg_xml_start[2] << ((rec.right - rec.left) + margin * 2) << " "
         << ((rec.bottom - rec.top) + margin * 2) << svg_xml_start[3];
    // clang-format on
    setlocale(LC_NUMERIC, "C");
    file.precision(2);

    // dump polygons to svg
    for(const auto &polyinfo: polyInfos) {
      file << " <path d=\"";
      for(const auto &path: polyinfo.paths) {
        // skip invalid polygons
        if(path.size() < 3)
          continue;

        // clang-format off
        file << " M " << ((double)path[0].X * scale + offsetX)
             << " "   << ((double)path[0].Y * scale + offsetY);
        // clang-format on

        for(const auto &point: path) {
          double   x  = (double)point.X * scale;
          double   y  = (double)point.Y * scale;
          file << " L " << (x + offsetX) << " " << (y + offsetY);
        }
        file << " z";
      }
      // clang-format off
      file << poly_end[0] << ColorToHtml(polyinfo.si.brushColor)
           << poly_end[1] << GetAlphaAsFrac(polyinfo.si.brushColor)
           << poly_end[2] << (polyinfo.si.pft == PolyFillType::EvenOdd ? "evenodd" : "nonzero")
           << poly_end[3] << ColorToHtml(polyinfo.si.penColor)
           << poly_end[4] << GetAlphaAsFrac(polyinfo.si.penColor)
           << poly_end[5] << polyinfo.si.penWidth
           << poly_end[6];
      // clang-format on

      if(polyinfo.si.showCoords) {
        file << "<g font-family=\"Verdana\" font-size=\"11\" fill=\"black\">\n\n";
        for(const auto &path: polyinfo.paths) {
          // skip invalid polygons
          if(path.size() < 3)
            continue;
          // print all points of polygon to file
          for(const auto &point: path) {
            file << "<text x=\"" << (int)(point.X * scale + offsetX)
                 << "\" y=\"" << (int)(point.Y * scale + offsetY) << "\">"
                 << point.X << "," << point.Y << "</text>\n\n";
          }
        }
        file << "</g>\n";
      }
    }
    file << "</svg>\n";
    file.close();
    setlocale(LC_NUMERIC, "");
    return true;
  }
};  // SVGBuilder
//------------------------------------------------------------------------------

// clang-format off
const std::string SVGBuilder::svg_xml_start[] = {
  "<?xml version=\"1.0\" standalone=\"no\"?>\n"
  "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n"
  "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n\n"
  "<svg width=\"",
  "\" height=\"",
  "\" viewBox=\"0 0 ",
  "\" version=\"1.0\" xmlns=\"http://www.w3.org/2000/svg\">\n\n"
};

const std::string SVGBuilder::poly_end[] = {
  "\"\n style=\"fill:",
  "; fill-opacity:",
  "; fill-rule:",
  "; stroke:",
  "; stroke-opacity:",
  "; stroke-width:",
  ";\"/>\n\n"
};
// clang-format on

//------------------------------------------------------------------------------
// Miscellaneous function ...
//------------------------------------------------------------------------------

bool SaveToFile(const std::string& filename, const Paths& ppg, double scale = 1.0, unsigned decimal_places = 0) {
  std::ofstream ofs(filename);
  if(!ofs)
    return false;

  if(decimal_places > 8)
    decimal_places = 8;
  ofs << std::setprecision(decimal_places) << std::fixed;

  Path pg;
  for(const auto &i: ppg) {
    for(const auto &j: i)
      ofs << j.X / scale << ", " << j.Y / scale << "," << std::endl;
    ofs << std::endl;
  }
  ofs.close();
  return true;
}
//------------------------------------------------------------------------------

/**
 * @brief LoadFromFile Load paths from file
 * file format assumes:
 * 1. path coordinates (x,y) are comma separated (+/- spaces) and
 * each coordinate is on a separate line
 * 2. each path is separated by one or more blank lines
 *
 * @param ppg Holds the processed data
 * @param filename
 * @param scale
 * @return true if file processed, false otherwise
 */
bool LoadFromFile(Paths& ppg, const std::string& filename, double scale) {

  ppg.clear();
  std::ifstream ifs(filename);
  if(!ifs)
    return false;
  std::string line;
  Path   pg;
  while(std::getline(ifs, line)) {
    std::stringstream ss(line);
    double X = 0.0, Y = 0.0;
    if(!(ss >> X)) {
      // ie blank lines => flag start of next polygon
      if(!pg.empty())
        ppg.push_back(pg);
      pg.clear();
      continue;
    }
    char c = ss.peek();
    while(c == ' ') {
      ss.read(&c, 1);
      c = ss.peek();
    }  // gobble spaces before comma
    if(c == ',') {
      ss.read(&c, 1);
      c = ss.peek();
    }  // gobble comma
    while(c == ' ') {
      ss.read(&c, 1);
      c = ss.peek();
    }  // gobble spaces after comma
    if(!(ss >> Y))
      break;  // oops!
    pg.push_back(IntPoint((cInt)(X * scale), (cInt)(Y * scale)));
  }
  if(!pg.empty())
    ppg.push_back(pg);
  ifs.close();
  return true;
}
//------------------------------------------------------------------------------

/**
 * @brief MakeRandomPoly Create random polygon
 * @param edgeCount Number of edges to generate
 * @param width Maximum X value
 * @param height Maximum Y value
 * @param poly Destination polygon
 */
void MakeRandomPoly(int edgeCount, int width, int height, Paths& poly) {
  poly.resize(1);
  poly[0].resize(edgeCount);
  for(auto& point: poly[0]) {
    point.X = rand() % width;
    point.Y = rand() % height;
  }
}
//------------------------------------------------------------------------------

/**
 * @brief ASCII_icompare Case-insensitive string comparison (ASCII only)
 * @param str1 First string
 * @param str2 Second string
 * @return
 */
bool ASCII_icompare(const char* str1, const char* str2) {
  // case insensitive compare for ASCII chars only
  while(*str1) {
    if(toupper(*str1) != toupper(*str2))
      return false;
    str1++;
    str2++;
  }
  return (!*str2);
}

//------------------------------------------------------------------------------
// Main entry point ...
//------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  if(argc > 1 && (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--benchmark") == 0)) {
    // do a benchmark test that creates a subject and a clip polygon both with
    // 100 vertices randomly placed in a 400 * 400 space. Then perform an
    // intersection operation based on even-odd filling. Repeat all this X times.
    int   loop_cnt = 1000;
    char* dummy;
    if(argc > 2)
      loop_cnt = strtol(argv[2], &dummy, 10);
    if(loop_cnt == 0)
      loop_cnt = 1000;
    std::cout << "\nPerforming " << loop_cnt << " random intersection operations ... ";
    srand((int)time(nullptr));
    int     error_cnt = 0;
    Paths   subject, clip, solution;
    Clipper clpr;

    time_t time_start = clock();
    for(int i = 0; i < loop_cnt; i++) {
      MakeRandomPoly(100, 400, 400, subject);
      MakeRandomPoly(100, 400, 400, clip);
      clpr.Clear();
      clpr.AddPaths(subject, PolyType::Subject, true);
      clpr.AddPaths(clip, PolyType::Clip, true);
      if(!clpr.Execute(ClipType::Intersection, solution, PolyFillType::EvenOdd, PolyFillType::EvenOdd))
        error_cnt++;
    }
    double time_elapsed = double(clock() - time_start) / CLOCKS_PER_SEC;

    std::cout << "\nFinished in " << time_elapsed << " secs with ";
    std::cout << error_cnt << " errors.\n\n";
    // let's save the very last result ...
    SaveToFile("Subject.txt", subject);
    SaveToFile("Clip.txt", clip);
    SaveToFile("Solution.txt", solution);

    // and see the final clipping op as an image too ...
    SVGBuilder svg;
    svg.style.penWidth   = 0.8;
    svg.style.pft        = PolyFillType::EvenOdd;
    svg.style.brushColor = 0x1200009C;
    svg.style.penColor   = 0xCCD3D3DA;
    svg.AddPaths(subject);
    svg.style.brushColor = 0x129C0000;
    svg.style.penColor   = 0xCCFFA07A;
    svg.AddPaths(clip);
    svg.style.brushColor = 0x6080ff9C;
    svg.style.penColor   = 0xFF003300;
    svg.style.pft        = PolyFillType::NonZero;
    svg.AddPaths(solution);
    svg.SaveToFile("solution.svg");
    return 0;
  }

  // print help message
  if(argc < 3) {
    std::cout << "\nUsage:\n"
         << "  clipper_console_demo S_FILE C_FILE CT [S_FILL C_FILL] [PRECISION] [SVG_SCALE]\n"
         << "or\n"
         << "  clipper_console_demo --benchmark [LOOP_COUNT]\n\n"
         << "Legend: [optional parameters in square braces]; {comments in curly braces}\n\n"
         << "Parameters:\n"
         << "  S_FILE & C_FILE are the subject and clip input files (see format below)\n"
         << "  CT: cliptype, either INTERSECTION or UNION or DIFFERENCE or XOR\n"
         << "  SUBJECT_FILL & CLIP_FILL: either EVENODD or NONZERO. Default: NONZERO\n"
         << "  PRECISION (in decimal places) for input data. Default = 0\n"
         << "  SVG_SCALE: scale of the output svg image. Default = 1.0\n"
         << "  LOOP_COUNT is the number of random clipping operations. Default = 1000\n\n"
         << "\nFile format for input and output files:\n"
         << "  X, Y[,] {first vertex of first path}\n"
         << "  X, Y[,] {next vertex of first path}\n"
         << "  {etc.}\n"
         << "  X, Y[,] {last vertex of first path}\n"
         << "  {blank line(s) between paths}\n"
         << "  X, Y[,] {first vertex of second path}\n"
         << "  X, Y[,] {next vertex of second path}\n"
         << "  {etc.}\n\n"
         << "Examples:\n"
         << "  clipper_console_demo \"subj.txt\" \"clip.txt\" INTERSECTION EVENODD EVENODD\n"
         << "  clipper_console_demo --benchmark 1000\n";
    return 1;
  }

  int   scale_log10 = 0;
  char* dummy;
  if(argc > 6)
    scale_log10 = strtol(argv[6], &dummy, 10);
  double scale = std::pow(double(10), scale_log10);

  double svg_scale = 1.0;
  if(argc > 7)
    svg_scale = strtod(argv[7], &dummy);
  svg_scale /= scale;

  Paths subject, clip;

  if(!LoadFromFile(subject, argv[1], scale)) {
    std::cerr << "\nCan't open the file " << argv[1] << " or the file format is invalid.\n";
    return 1;
  }
  if(!LoadFromFile(clip, argv[2], scale)) {
    std::cerr << "\nCan't open the file " << argv[2] << " or the file format is invalid.\n";
    return 1;
  }

  ClipType     clipType    = ClipType::Intersection;
  const std::string sClipType[] = {"INTERSECTION", "UNION", "DIFFERENCE", "XOR"};

  if(argc > 3) {
    if(ASCII_icompare(argv[3], "XOR"))
      clipType = ClipType::XOR;
    else if(ASCII_icompare(argv[3], "UNION"))
      clipType = ClipType::Union;
    else if(ASCII_icompare(argv[3], "DIFFERENCE"))
      clipType = ClipType::Difference;
    else
      clipType = ClipType::Intersection;
  }

  PolyFillType subj_pft = PolyFillType::NonZero, clip_pft = PolyFillType::NonZero;
  if(argc > 5) {
    if(ASCII_icompare(argv[4], "EVENODD"))
      subj_pft = PolyFillType::EvenOdd;
    if(ASCII_icompare(argv[5], "EVENODD"))
      clip_pft = PolyFillType::EvenOdd;
  }

  Clipper c;
  c.AddPaths(subject, PolyType::Subject, true);
  c.AddPaths(clip, PolyType::Clip, true);
  Paths solution;

  if(!c.Execute(clipType, solution, subj_pft, clip_pft)) {
    std::cout << (sClipType[static_cast<int>(clipType)] + " failed!\n\n");
    return 1;
  }

  std::cout << "\nFinished!\n\n";
  SaveToFile("solution.txt", solution, scale);

  // let's see the result too ...
  SVGBuilder svg;
  svg.style.penWidth = 0.8;
  svg.style.brushColor = 0x1200009C;
  svg.style.penColor   = 0xCCD3D3DA;
  svg.style.pft      = subj_pft;
  svg.AddPaths(subject);
  svg.style.brushColor = 0x129C0000;
  svg.style.penColor   = 0xCCFFA07A;
  svg.style.pft      = clip_pft;
  svg.AddPaths(clip);
  svg.style.brushColor = 0x6080ff9C;
  svg.style.penColor   = 0xFF003300;
  svg.style.pft      = PolyFillType::NonZero;
  svg.AddPaths(solution);
  svg.SaveToFile("solution.svg", svg_scale);

  // finally, show the svg image in the default viewing application
  system("solution.svg");
  return 0;
}
//---------------------------------------------------------------------------
