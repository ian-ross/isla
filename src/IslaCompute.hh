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
#include <wx/gdicmn.h>
#include "GridData.hh"
#include "IslaModel.hh"

class IslaCompute {
public:
  typedef wxRect Box;
  typedef std::vector<Box> Boxes;
  struct Merge {
    int i, j, score;
    Merge(int ii, int ij, int iscore) : i(ii), j(ij), score(iscore) { }
    bool operator<(Merge other) { return score < other.score; }
  };

  IslaCompute(const GridData<LMass> &glmin,
              const GridData<int> &ismaskin) :
    glm(glmin), ismask(ismaskin) { }

  // Calculate island segments for given landmass.
  void segment(LMass lm, Boxes &bs);
  void scoredSegmentation(LMass lm, const Boxes &init, Boxes &segs);
  bool step(LMass lm, const Boxes &before, Boxes &after);

  // Determine bounding regions for a given landmass.
  Box boundBox(LMass lm);
  void boundRows(LMass lm, Boxes &rs);
  void boundCols(LMass lm, Boxes &cs);

  // Determine whether geometrical structures are admissible for a
  // given landmass.
  bool admissible(LMass lm, int x, int y) const;
  bool admissible(LMass lm, const Box &b) const;
  bool admissible(LMass lm, const Boxes &b) const;

  // Calculate heuristic score for segment list.
  int score(LMass lm, const Boxes &bs) const;


  // Does a box overlap with any of a given set of boxes?
  static bool overlap(const Box &b, const Boxes &bs);

  // Find runs of consecutive values in a vector of integers.
  static void runs(const std::vector<int> &vs,
                   std::vector< std::pair<int,int> > &vruns);

private:
  const GridData<LMass> &glm;
  const GridData<int> &ismask;
};

#endif


