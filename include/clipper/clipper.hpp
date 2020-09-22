/*******************************************************************************
 *                                                                              *
 * Author    :  Angus Johnson                                                   *
 * Version   :  6.4.2                                                           *
 * Date      :  27 February 2017                                                *
 * Website   :  http://www.angusj.com                                           *
 * Copyright :  Angus Johnson 2010-2017                                         *
 *                                                                              *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *                                                                              *
 * Attributions:                                                                *
 * The code in this library is an extension of Bala Vatti's clipping algorithm: *
 * "A generic solution to polygon clipping"                                     *
 * Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
 * http://portal.acm.org/citation.cfm?id=129906                                 *
 *                                                                              *
 * Computer graphics and geometric modeling: implementation and algorithms      *
 * By Max K. Agoston                                                            *
 * Springer; 1 edition (January 4, 2005)                                        *
 * http://books.google.com/books?q=vatti+clipping+agoston                       *
 *                                                                              *
 * See also:                                                                    *
 * "Polygon Offsetting by Computing Winding Numbers"                            *
 * Paper no. DETC2005-85513 pp. 565-575                                         *
 * ASME 2005 International Design Engineering Technical Conferences             *
 * and Computers and Information in Engineering Conference (IDETC/CIE2005)      *
 * September 24-28, 2005 , Long Beach, California, USA                          *
 * http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf              *
 *                                                                              *
 *******************************************************************************/

/**
 * @file clipper.hpp
 * @brief Polygon Clipper library
 * @author Angus Johnson
 * @version 6.4.2
 * @date 27 February 2017
 * @copyright Boost Software License v1.0
 *
 *
 */

#pragma once

// TODO: move to separate header
#define CLIPPER_VERSION "6.4.2"

// use_int32: When enabled 32bit ints are used instead of 64bit ints. This
// improve performance but coordinate values are limited to the range +/- 46340
//#define use_int32

// use_xyz: adds a Z member to IntPoint. Adds a minor cost to performance.
//#define use_xyz

// use_lines: Enables line clipping. Adds a very minor cost to performance.
#define use_lines

#include <cstdlib>
#include <cstring>
#include <functional>
#include <list>
#include <ostream>
#include <queue>
#include <set>
#include <stdexcept>
#include <vector>

namespace ClipperLib {

/**
 * @brief ClipType enum, boolean operations
 */
enum class ClipType {
  Intersection,  //! Intersection
  Union,         //! Union
  Difference,    //! Difference
  XOR            //! Xor (exclusive or)
};

/**
 * @brief The PolyType enum
 */
enum class PolyType {
  Subject,  //! TODO: document
  Clip      //! TODO: document
};

/**
 * @brief The PolyFillType enum
 *
 *  @details By far the most widely used winding rules for polygon filling are
 * EvenOdd & NonZero (GDI, GDI+, XLib, OpenGL, Cairo, AGG, Quartz, SVG, Gr32)
 * Others rules include Positive, Negative and ABS_GTR_EQ_TWO (only in OpenGL)
 * see http://glprogramming.com/red/chapter11.html
 */
enum class PolyFillType {
  EvenOdd,   //!
  NonZero,   //!
  Positive,  //!
  Negative   //!
};

#ifdef use_int32
typedef int       cInt;
static cInt const loRange = 0x7FFF;
static cInt const hiRange = 0x7FFF;
#else
//! @typedef
typedef signed long long cInt;
static cInt const        loRange = 0x3FFFFFFF;
static cInt const        hiRange = 0x3FFFFFFFFFFFFFFFLL;
//! @typedef used by the Int128 class
typedef signed long long long64;
//! @typedef
typedef unsigned long long ulong64;

#endif

/**
 * @brief IntPoint struct, a point with integer values
 */
struct IntPoint {
  cInt X;  //! X value
  cInt Y;  //! Y value
#ifdef use_xyz
  cInt Z;  //! Z value

  /**
   * @brief IntPoint constructor
   * @param x
   * @param y
   * @param z
   */
  IntPoint(cInt x = 0, cInt y = 0, cInt z = 0) : X(x), Y(y), Z(z){};
#else

  /**
   * @brief IntPoint constructor
   * @param X value
   * @param Y value
   */
  IntPoint(cInt x = 0, cInt y = 0) : X(x), Y(y){}
#endif

  /**
   * @brief operator ==
   * @param first IntPoint
   * @param second IntPoint
   * @return Whether both IntPoints are the same
   */
  friend inline bool operator==(const IntPoint& a, const IntPoint& b) noexcept { return a.X == b.X && a.Y == b.Y; }

  /**
   * @brief operator !=
   * @param first IntPoint
   * @param second IntPoint
   * @return Whether the IntPoints are different
   */
  friend inline bool operator!=(const IntPoint& a, const IntPoint& b) noexcept { return a.X != b.X || a.Y != b.Y; }
};
//------------------------------------------------------------------------------

//! @typedef std::vector of IntPoints
typedef std::vector<IntPoint> Path;
//! @typedef std::vector of Paths
typedef std::vector<Path> Paths;

/**
 * @brief Append IntPoint to Path
 * @param Path
 * @param Point
 * @return amended Path
 */
inline Path& operator<<(Path& poly, const IntPoint& p) {
  poly.push_back(p);
  return poly;
}

/**
 * @brief Append Path to (vector of) Paths
 * @param Paths target vector
 * @param Path
 * @return amended Paths
 */
inline Paths& operator<<(Paths& polys, const Path& p) {
  polys.push_back(p);
  return polys;
}

std::ostream& operator<<(std::ostream& s, const IntPoint& p);
std::ostream& operator<<(std::ostream& s, const Path& p);
std::ostream& operator<<(std::ostream& s, const Paths& p);

/**
 * @brief The DoublePoint struct
 *
 * 2 Dimensional point using double values
 */
struct DoublePoint {
  double X;  //! X value
  double Y;  //! Y value

  /**
   * @brief DoublePoint constructor
   * @param X value
   * @param Y value
   */
  DoublePoint(double x = 0, double y = 0) : X(x), Y(y) {}

  /**
   * @brief DoublePoint constructor
   * @param point to copy
   */
  DoublePoint(const IntPoint& ip) : X((double)ip.X), Y((double)ip.Y) {}
};
//------------------------------------------------------------------------------

#ifdef use_xyz
//! @typedef
typedef void (*ZFillCallback)(IntPoint& e1bot, IntPoint& e1top, IntPoint& e2bot, IntPoint& e2top, IntPoint& pt);
#endif

/**
 * @brief JoinType enum, methods to join the corner of paths
 *
 * TODO: include graphics: http://www.angusj.com/delphi/clipper/documentation/Images/jointypes.png
 * http://www.angusj.com/delphi/clipper/documentation/Docs/Units/ClipperLib/Types/JoinType.htm
 */
enum class JoinType {
  Square,  //! Square the ends
  Round,   //! Round the end
  Miter    //! Miter
};

/**
 * @brief The EndType enum
 *
 * TODO: include graphics
 * http://www.angusj.com/delphi/clipper/documentation/Docs/Units/ClipperLib/Types/EndType.htm
 */
enum class EndType {
  ClosedPolygon,  //!
  ClosedLine,     //!
  OpenButt,       //!
  OpenSquare,     //!
  OpenRound       //!
};

// forward declaration
class PolyNode;

//! @typedef vector of PolyNode
typedef std::vector<PolyNode*> PolyNodes;

/**
 * @brief The PolyNode class
 *
 * @see PolyTree
 */
class PolyNode {
public:
  PolyNode();
  virtual ~PolyNode(){}
  Path      Contour;
  PolyNodes Childs;
  PolyNode* Parent;
  /**
   * @brief GetNext Get the next child or sibling
   * @return First child, if exists. Otherwise, next sibling
   */
  PolyNode* GetNext() const;

  /**
   * @brief IsHole
   * @return
   */
  bool IsHole() const;
  bool IsOpen() const;
  int  ChildCount() const;

private:
  // PolyNode& operator =(PolyNode& other);
  unsigned Index;       //! node index in Parent.Childs
  bool     m_IsOpen;    //!
  JoinType m_jointype;  //!
  EndType  m_endtype;
  /**
   * @brief GetNextSiblingUp
   * @return
   */
  PolyNode* GetNextSiblingUp() const;
  void      AddChild(PolyNode& child);
  friend class Clipper;  // to access Index
  friend class ClipperOffset;
};

/**
 * @brief The PolyTree class
 *
 * @see PolyNode
 */
class PolyTree : public PolyNode {
public:
  ~PolyTree() { Clear(); }

  /**
   * @brief GetFirst Get the first child in the tre
   * @return First child in tree, nullptr if no children
   */
  PolyNode* GetFirst() const;

  /**
   * @brief Clear
   */
  void      Clear();

  /**
   * @brief Total Get the total number of child nodes in the tree
   * @return Number of childen
   */
  int       Total() const;

private:
  // PolyTree& operator =(PolyTree& other);
  PolyNodes AllNodes;   //! list of nodes
  friend class Clipper; // to access AllNodes
};

/**
 * @brief Orientation
 * @param poly Target polygon
 * @return Whether the area of the specified polygon is positive
 */
bool Orientation(const Path& poly);

/**
 * @brief Area Determine the area of a polygon
 * @param poly Target polygon
 * @return Area of polygon
 */
double Area(const Path& poly);

/**
 * @brief Determine whether a point lies inside a polygon
 *
 * See "The Point in Polygon Problem for Arbitrary Polygons" by Hormann & Agathos
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.88.5498&rep=rep1&type=pdf
 *
 * @param Point
 * @param Polygon
 * @return 0 if outside, 1 if inside, and -1 if on boundary
 *
 */
int PointInPolygon(const IntPoint& pt, const Path& path);

/**
 * @brief SimplifyPolygon
 * @param in_poly
 * @param out_polys
 * @param fillType
 */
void SimplifyPolygon(const Path& in_poly, Paths& out_polys, PolyFillType fillType = PolyFillType::EvenOdd);

/**
 * @brief SimplifyPolygons
 * @param in_polys
 * @param out_polys
 * @param fillType
 */
void SimplifyPolygons(const Paths& in_polys, Paths& out_polys, PolyFillType fillType = PolyFillType::EvenOdd);

/**
 * @brief SimplifyPolygons
 * @param polys
 * @param fillType
 */
void SimplifyPolygons(Paths& polys, PolyFillType fillType = PolyFillType::EvenOdd);

/**
 * @brief CleanPolygon
 * @param in_poly
 * @param out_poly
 * @param distance
 */
void CleanPolygon(const Path& in_poly, Path& out_poly, double distance = 1.415);
void CleanPolygon(Path& poly, double distance = 1.415);
void CleanPolygons(const Paths& in_polys, Paths& out_polys, double distance = 1.415);
void CleanPolygons(Paths& polys, double distance = 1.415);

void MinkowskiSum(const Path& pattern, const Path& path, Paths& solution, bool pathIsClosed);
void MinkowskiSum(const Path& pattern, const Paths& paths, Paths& solution, bool pathIsClosed);
void MinkowskiDiff(const Path& poly1, const Path& poly2, Paths& solution);

void PolyTreeToPaths(const PolyTree& polytree, Paths& paths);
void ClosedPathsFromPolyTree(const PolyTree& polytree, Paths& paths);
void OpenPathsFromPolyTree(PolyTree& polytree, Paths& paths);

void ReversePath(Path& p);
void ReversePaths(Paths& p);

/**
 * @brief Rectangle, defined by 4 values
 */
struct IntRect {
  cInt left;    //! leftmost X value
  cInt top;     //! topmost Y value
  cInt right;   //! rightmost X value
  cInt bottom;  //! bottom Y value
};

/**
 * @brief The EdgeSide enum
 * private
 */
enum class EdgeSide {
  Left  = 1,  //!
  Right = 2   //!
};

// forward declarations (for stuff used internally) ...
struct TEdge;
struct IntersectNode;
struct LocalMinimum;
struct OutPt;
struct OutRec;
struct Join;

//! @typedef Vector of paths
typedef std::vector<OutRec*> PolyOutList;
//! @typedef Vector of edges
typedef std::vector<TEdge*> EdgeList;
//! @typedef vector of joins
typedef std::vector<Join*> JoinList;
//! @typedef vector of lists of intersections
typedef std::vector<IntersectNode*> IntersectList;

//------------------------------------------------------------------------------

/**
 * @brief The ClipperBase class
 *
 * @details ClipperBase is the ancestor to the Clipper class. It should not be
 * instantiated directly. This class simply abstracts the conversion of sets of
 * polygon coordinates into edge objects that are stored in a LocalMinima list.
 */
class ClipperBase {
public:
  ClipperBase();
  virtual ~ClipperBase();

  /**
   * @brief AddPath Add a path to a polygon
   * @param pg Path to add to polygon
   * @param PolyTyp Polygon type
   * @param Closed whether path is closed
   * @return Whether operation was successful
   */
  virtual bool AddPath(const Path& pg, PolyType PolyTyp, bool Closed);

  /**
   * @brief AddPaths Add a list of paths to a polygon
   * @param ppg Path to add
   * @param PolyTyp Polygon type
   * @param Closed Whether path is closed
   * @return Whether operation was successful
   */
  bool AddPaths(const Paths& ppg, PolyType PolyTyp, bool Closed);

  /**
   * @brief Clear Remove all edges, reset flags
   */
  virtual void Clear();

  /**
   * @brief GetBounds Calculate an axis-aligned bounding box
   * @return Bounding box
   */
  IntRect GetBounds();

  /**
   * @brief PreserveCollinear
   * @return
   */
  bool PreserveCollinear() noexcept { return m_PreserveCollinear; }

  /**
   * @brief PreserveCollinear
   * @param value
   */
  void PreserveCollinear(bool value) noexcept { m_PreserveCollinear = value; }

protected:
  void   DisposeLocalMinimaList();
  TEdge* AddBoundsToLML(TEdge* e, bool IsClosed);

  /**
   * @brief Reset Reset class
   */
  virtual void Reset();
  TEdge*       ProcessBound(TEdge* E, bool IsClockwise);
  void         InsertScanbeam(const cInt Y);
  bool         PopScanbeam(cInt& Y);
  bool         LocalMinimaPending();
  bool         PopLocalMinima(cInt Y, const LocalMinimum*& locMin);
  OutRec*      CreateOutRec();
  void         DisposeAllOutRecs();
  void         DisposeOutRec(PolyOutList::size_type index);
  void         SwapPositionsInAEL(TEdge* edge1, TEdge* edge2);
  void         DeleteFromAEL(TEdge* e);
  void         UpdateEdgeIntoAEL(TEdge*& e);

  typedef std::vector<LocalMinimum> MinimaList;
  MinimaList::iterator              m_CurrentLM;
  MinimaList                        m_MinimaList;

  bool        m_UseFullRange;
  EdgeList    m_edges;
  bool        m_PreserveCollinear;
  bool        m_HasOpenPaths;
  PolyOutList m_PolyOuts;
  TEdge*      m_ActiveEdges;

  typedef std::priority_queue<cInt> ScanbeamList;
  ScanbeamList                      m_Scanbeam;
};
//------------------------------------------------------------------------------

/**
 * @brief The Clipper class
 */
class Clipper : public virtual ClipperBase {
public:
  Clipper(bool ReverseOutput = false, bool StrictSimple = false, bool PreserveCollinear = false);
  bool Execute(ClipType clipType, Paths& solution, PolyFillType fillType = PolyFillType::EvenOdd);
  bool Execute(ClipType clipType, Paths& solution, PolyFillType subjFillType, PolyFillType clipFillType);
  bool Execute(ClipType clipType, PolyTree& polytree, PolyFillType fillType = PolyFillType::EvenOdd);
  bool Execute(ClipType clipType, PolyTree& polytree, PolyFillType subjFillType, PolyFillType clipFillType);
  bool ReverseSolution() { return m_ReverseOutput; }
  void ReverseSolution(bool value) { m_ReverseOutput = value; }
  bool StrictlySimple() { return m_StrictSimple; }
  void StrictlySimple(bool value) { m_StrictSimple = value; }
#ifdef use_xyz
  /**
   * @brief ZFillFunction
   *
   * set the callback function for z value filling on intersections (otherwise Z is 0)
   *
   * @param zFillFunc
   */
  void ZFillFunction(ZFillCallback zFillFunc);
#endif
protected:
  virtual bool ExecuteInternal();

private:
  JoinList                m_Joins;
  JoinList                m_GhostJoins;
  IntersectList           m_IntersectList;
  ClipType                m_ClipType;
  typedef std::list<cInt> MaximaList;
  MaximaList              m_Maxima;
  TEdge*                  m_SortedEdges;
  bool                    m_ExecuteLocked;
  PolyFillType            m_ClipFillType;
  PolyFillType            m_SubjFillType;
  bool                    m_ReverseOutput;
  bool                    m_UsingPolyTree;
  bool                    m_StrictSimple;
#ifdef use_xyz
  ZFillCallback m_ZFill;  // custom callback
#endif
  void    SetWindingCount(TEdge& edge);
  bool    IsEvenOddFillType(const TEdge& edge) const;
  bool    IsEvenOddAltFillType(const TEdge& edge) const;
  void    InsertLocalMinimaIntoAEL(const cInt botY);
  /**
   * @brief InsertEdgeIntoAEL Insert edge into Active Edge List
   * @param edge
   * @param startEdge
   */
  void    InsertEdgeIntoAEL(TEdge* edge, TEdge* startEdge);
  void    AddEdgeToSEL(TEdge* edge);
  bool    PopEdgeFromSEL(TEdge*& edge);
  /**
   * @brief CopyAELToSEL Copy Active Edge List to ...
   */
  void    CopyAELToSEL();
  void    DeleteFromSEL(TEdge* e);
  void    SwapPositionsInSEL(TEdge* edge1, TEdge* edge2);
  bool    IsContributing(const TEdge& edge) const;
  bool    IsTopHorz(const cInt XPos);
  void    DoMaxima(TEdge* e);
  void    ProcessHorizontals();
  void    ProcessHorizontal(TEdge* horzEdge);
  void    AddLocalMaxPoly(TEdge* e1, TEdge* e2, const IntPoint& pt);
  OutPt*  AddLocalMinPoly(TEdge* e1, TEdge* e2, const IntPoint& pt);
  OutRec* GetOutRec(int idx);
  void    AppendPolygon(TEdge* e1, TEdge* e2);
  void    IntersectEdges(TEdge* e1, TEdge* e2, IntPoint& pt);
  OutPt*  AddOutPt(TEdge* e, const IntPoint& pt);
  OutPt*  GetLastOutPt(TEdge* e);
  bool    ProcessIntersections(const cInt topY);
  void    BuildIntersectList(const cInt topY);
  void    ProcessIntersectList();
  void    ProcessEdgesAtTopOfScanbeam(const cInt topY);
  void    BuildResult(Paths& polys);
  void    BuildResult2(PolyTree& polytree);
  void    SetHoleState(TEdge* e, OutRec* outrec);
  void    DisposeIntersectNodes();
  bool    FixupIntersectionOrder();
  void    FixupOutPolygon(OutRec& outrec);
  void    FixupOutPolyline(OutRec& outrec);
  bool    IsHole(TEdge* e);
  bool    FindOwnerFromSplitRecs(OutRec& outRec, OutRec*& currOrfl);
  void    FixHoleLinkage(OutRec& outrec);
  void    AddJoin(OutPt* op1, OutPt* op2, const IntPoint &offPt);
  void    ClearJoins();
  void    ClearGhostJoins();
  void    AddGhostJoin(OutPt* op, const IntPoint& offPt);
  bool    JoinPoints(Join* j, OutRec* outRec1, OutRec* outRec2);
  void    JoinCommonEdges();
  void    DoSimplePolygons();
  void    FixupFirstLefts1(OutRec* OldOutRec, OutRec* NewOutRec);
  void    FixupFirstLefts2(OutRec* InnerOutRec, OutRec* OuterOutRec);
  void    FixupFirstLefts3(OutRec* OldOutRec, OutRec* NewOutRec);
#ifdef use_xyz
  void SetZ(IntPoint& pt, TEdge& e1, TEdge& e2);
#endif
};
//------------------------------------------------------------------------------

/**
 * @brief The ClipperOffset class
 *
 * Create polygon offsets
 *
 */
class ClipperOffset {
public:
  /**
   * @brief ClipperOffset
   * @param miterLimit
   * @param roundPrecision
   */
  ClipperOffset(double miterLimit = 2.0, double roundPrecision = 0.25);
  ~ClipperOffset();
  void   AddPath(const Path& path, JoinType joinType, EndType endType);
  void   AddPaths(const Paths& paths, JoinType joinType, EndType endType);
  void   Execute(Paths& solution, double delta);
  void   Execute(PolyTree& solution, double delta);
  void   Clear();
  double MiterLimit;
  double ArcTolerance;

private:
  Paths                    m_destPolys;
  Path                     m_srcPoly;
  Path                     m_destPoly;
  std::vector<DoublePoint> m_normals;
  double                   m_delta, m_sinA, m_sin, m_cos;
  double                   m_miterLim, m_StepsPerRad;
  IntPoint                 m_lowest;
  PolyNode                 m_polyNodes;

  void FixOrientations();
  void DoOffset(double delta);
  void OffsetPoint(int j, int& k, JoinType jointype);
  void DoSquare(int j, int k);
  void DoMiter(int j, int k, double r);
  void DoRound(int j, int k);
};
//------------------------------------------------------------------------------

/**
 * @brief The clipperException class
 */
class clipperException : public std::exception {
public:
  clipperException(const char* description) : m_descr(description) {}
  virtual ~clipperException() throw() {}
  virtual const char* what() const throw() { return m_descr.c_str(); }

private:
  std::string m_descr;
};
//------------------------------------------------------------------------------

}  // namespace ClipperLib
