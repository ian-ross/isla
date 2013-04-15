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
  struct BoxInfo {
    Box b;                      // Box.
    std::set<int> cands;        // IDs of merge candidates.
    int score;                  // Cached score for this box.
  };
  typedef std::map<int, BoxInfo> Seg;
  typedef std::vector<Box> Boxes;
  struct Merge {
    int i, j, score;
    Merge(int ii, int ij, int iscore) : i(ii), j(ij), score(iscore) { }
    bool operator<(Merge other) { return score < other.score; }
  };

  IslaCompute(const GridData<LMass> &glmin,
              const std::map<LMass, Box> &lmbboxin,
              const GridData<int> &ismaskin) :
    glm(glmin), lmbbox(lmbboxin), ismask(ismaskin) { }

  // Calculate island segments for given landmass.
  void segment(LMass lm, Boxes &bs);
  void scoredSegmentation(LMass lm, Seg &segs);
  bool step(LMass lm, Seg &segs, int segid);

  // Compute coincidence line segments between adjacent island
  // segments (used for distinguishing spatially adjacent segments
  // that belong to the same island and adjacent segments from
  // distinct islands).
  static void coincidence(const Boxes &bs,
                          IslaModel::CoincInfo &v, IslaModel::CoincInfo &h);

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
  int score(LMass lm, Seg &ss, const Box &newb, int exc1, int exc2) const;


  // Does a box overlap with any of a given set of boxes?
  static bool overlap(const Box &b, const Seg &ss, int exc1, int exc2);

  // Find runs of consecutive values in a vector of integers.
  static void runs(const std::vector<int> &vs,
                   std::vector< std::pair<int,int> > &vruns);

private:

  static void extractBoxes(const Seg &ss, Boxes &bs);

  const GridData<LMass> &glm;
  const std::map<LMass, wxRect> &lmbbox;
  const GridData<int> &ismask;
};

#endif


