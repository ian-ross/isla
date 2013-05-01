//----------------------------------------------------------------------
// FILE:   IslaCompute.hh
// DATE:   17-MAR-2013
// AUTHOR: Ian Ross
//
// Island bounding box calculation class for Isla island editor.
//----------------------------------------------------------------------

#ifndef _H_ISLACOMPUTE_
#define _H_ISLACOMPUTE_

#include <vector>
#include <set>
#include <map>
#include <wx/gdicmn.h>
#include "GridData.hh"
#include "IslaModel.hh"

class IslaCompute {
public:
  typedef wxRect Box;
  typedef int Score;
  typedef int BoxID;
  struct BoxInfo {
    BoxInfo() : score(-1) { }
    Box b;                      // Box.
    std::set<BoxID> cands;      // IDs of merge candidates.
    Score score;                // Cached score for this box.
  };
  typedef std::map<BoxID, BoxInfo> Seg;
  typedef std::vector<Box> Boxes;
  struct Merge {
    BoxID i, j;
    Score score;
    Merge(BoxID ii, BoxID ij, Score iscore) : i(ii), j(ij), score(iscore) { }
    bool operator<(Merge other) { return score < other.score; }
  };

  IslaCompute(const GridData<LMass> &glmin,
              const std::map<LMass, IslaModel::BBox> &lmbboxin,
              const GridData<int> &ismaskin) :
    glm(glmin), lmbbox(lmbboxin), ismask(ismaskin) { }

  // Calculate island segments for given landmass.
  void segment(LMass lm, int minsegs, Boxes &bs);
  void scoredSegmentation(LMass lm, int minsegs, Seg &segs);
  bool step(LMass lm, int minsegs, Seg &segs, BoxID segid);

  // Compute coincidence line segments between adjacent island
  // segments (used for distinguishing spatially adjacent segments
  // that belong to the same island and adjacent segments from
  // distinct islands).
  static void coincidence(const Boxes &bs,
                          IslaModel::CoincInfo &v, IslaModel::CoincInfo &h);

  // Remove redundant rows for polar "round the world" segments.
  void fixPolar(LMass lm, Boxes &bs);
  bool fullCircle(LMass lm, int y);

  // Determine bounding regions for a given landmass.
  Box boundBox(LMass lm);
  void boundRows(LMass lm, Seg &rs);
  void boundCols(LMass lm, Seg &cs);

  // Determine whether geometrical structures are admissible for a
  // given landmass.
  bool admissible(LMass lm, int x, int y) const;
  bool admissible(LMass lm, const Box &b) const;
  bool admissible(LMass lm, const Boxes &b) const;

  // Calculate heuristic score for segment list.
  Score score(LMass lm, Seg &ss,
              const Box *newb = 0, int exc1 = -1, int exc2 = -1) const;
  Score score(LMass lm, Seg &ss, const Box &newb, int exc1, int exc2) const {
    return score(lm, ss, &newb, exc1, exc2);
  }


  // Does a box overlap with any of a given set of boxes?
  static bool overlap(const Box &b, const Seg &ss, int exc1, int exc2);

  // Find runs of consecutive values in a vector of integers.
  static void runs(const std::vector<int> &vs,
                   std::vector< std::pair<int,int> > &vruns);

private:

  static void extractBoxes(const Seg &ss, Boxes &bs);

  const GridData<LMass> &glm;
  const std::map<LMass, IslaModel::BBox> &lmbbox;
  const GridData<int> &ismask;
  std::map<BoxID, std::set<BoxID> > known_bad;
  std::map<BoxID, std::map<BoxID, Score> > known_good;
};

#endif


