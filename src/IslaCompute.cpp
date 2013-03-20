//----------------------------------------------------------------------
// FILE:   IslaCompute.cpp
// DATE:   17-MAR-2013
// AUTHOR: Ian Ross
//
// Island bounding box calculation class for Isla island editor.
//----------------------------------------------------------------------

#include <algorithm>
#include "IslaCompute.hh"

using namespace std;


// Calculate island segments for given landmass.

void IslaCompute::segment(LMass lm, Boxes &bs)
{
  Boxes brows, byrows, bcols, bycols;
  boundRows(lm, brows);
  scoredSegmentation(lm, brows, byrows);
  boundCols(lm, bcols);
  scoredSegmentation(lm, bcols, bycols);
  bs = byrows.size() <= bycols.size() ? byrows : bycols;
  cout << "IslaCompute::segment: lm=" << lm << endl;
  for (int i = 0; i < bs.size(); ++i)
    cout << "  x:" << bs[i].x << " y:" << bs[i].y
         << "  w:" << bs[i].width << " h:" << bs[i].height << endl;
}

void IslaCompute::scoredSegmentation(LMass lm, const Boxes &init, Boxes &segs)
{
  if (init.size() == 1)
    segs = init;
  else {
    Boxes before = init, after;
    while (step(lm, before, after)) before = after;
    segs = before;
  }
}

bool IslaCompute::step(LMass lm, const Boxes &before, Boxes &after)
{
  if (before.size() == 1) return false;
  vector<Merge> ok;
  Boxes testbs;
  for (int i = 0; i < before.size(); ++i)
    for (int j = i + 1; j < before.size(); ++j) {
      testbs = before;
      testbs.erase(testbs.begin() + j);
      testbs.erase(testbs.begin() + i);
      Box newb = before[i].Union(before[j]);
      if (!overlap(newb, testbs)) {
        testbs.push_back(newb);
        if (admissible(lm, testbs))
          ok.push_back(Merge(i, j, score(lm, testbs)));
      }
    }
  if (ok.size() == 0) return false;
  vector<Merge>::const_iterator it = min_element(ok.begin(), ok.end());
  after = before;
  after.erase(after.begin() + it->j);
  after.erase(after.begin() + it->i);
  after.push_back(before[it->i].Union(before[it->j]));
  return true;
}



// Determine bounding regions for a given landmass.

IslaCompute::Box IslaCompute::boundBox(LMass lm)
{
  bool found = false;
  int nx = glm.grid()->nlon(), ny = glm.grid()->nlat();
  int minx = nx-1, maxx = 0, miny = ny-1, maxy = 0;
  for (int x = 0; x < nx; ++x)
    for (int y = 0; y < ny; ++y)
      if (glm(y, x) == lm) {
        found = true;
        minx = min(minx, x);  maxx = max(maxx, x);
        miny = min(miny, y);  maxy = max(maxy, y);
      }
  if (found)
    return Box(minx, miny, maxx-minx+1, maxy-miny+1);
  else
    throw runtime_error("Invalid land mass in IslaCompute::boundBox");
}

void IslaCompute::boundRows(LMass lm, Boxes &rs)
{
  Box bbox = boundBox(lm);
  rs.clear();
  for (int y = 0; y < bbox.height; ++y) {
    vector<int> xs;
    for (int x = 0; x < bbox.width; ++x)
      if (glm(bbox.y + y, bbox.x + x) == lm)
        xs.push_back(bbox.x + x);
    vector< pair<int,int> > xruns;
    runs(xs, xruns);
    for (int i = 0; i < xruns.size(); ++i)
      rs.push_back(Box(xruns[i].first, bbox.y + y,
                       xruns[i].second - xruns[i].first + 1, 1));
  }
}

void IslaCompute::boundCols(LMass lm, Boxes &cs)
{
  Box bbox = boundBox(lm);
  cs.clear();
  for (int x = 0; x < bbox.width; ++x) {
    vector<int> ys;
    for (int y = 0; y < bbox.height; ++y)
      if (glm(bbox.y + y, bbox.x + x) == lm)
        ys.push_back(bbox.y + y);
    vector< pair<int,int> > yruns;
    runs(ys, yruns);
    for (int i = 0; i < yruns.size(); ++i)
      cs.push_back(Box(bbox.x + x, yruns[i].first,
                       1, yruns[i].second - yruns[i].first + 1));
  }
}


// Determine whether points, boxes and sets of boxes are admissible
// for a given landmass.

bool IslaCompute::admissible(LMass lm, int x, int y) const
{
  return glm(y, x) == lm || glm(y, x) == 0 || ismask(y, x) == 0;
}

bool IslaCompute::admissible(LMass lm, const Box &b) const
{
  bool ret = true;
  for (int x = 0; x < b.width; ++x)
    for (int y = 0; y < b.height; ++y)
      if (!admissible(lm, b.x + x, b.y + y)) { ret = false;  break; }
  return ret;
}

bool IslaCompute::admissible(LMass lm, const Boxes &bs) const
{
  bool ret = true;
  for (int i = 0; i < bs.size(); ++i)
    if (!admissible(lm, bs[i])) { ret = false;  break; }
  return ret;
}


// Calculate heuristic score for segment list.  Low scores are better,
// so inadmissible sets of boxes are assigned a score larger than that
// for any possible set of boxes.

int IslaCompute::score(LMass lm, const Boxes &bs) const
{
  if (!admissible(lm, bs)) return glm.grid()->nlat() * glm.grid()->nlon() + 1;
  int ret = 0;
  for (Boxes::const_iterator it = bs.begin(); it != bs.end(); ++it) {
    Box b = *it;
    for (int x = 0; x < b.width; ++x)
      for (int y = 0; y < b.height; ++y)
        if (glm(b.y + y, b.x + x) != lm) ++ret;
  }
  return ret;
}


// Does a box overlap with any of a given set of boxes?

bool IslaCompute::overlap(const Box &b, const Boxes &bs)
{
  bool ret = false;
  for (int i = 0; i < bs.size(); ++i)
    if (bs[i].Intersects(b)) { ret = true;  break; }
  return ret;
}


// Find runs of consecutive values in a vector of integers.

void IslaCompute::runs(const vector<int> &vs, vector< pair<int,int> > &vruns)
{
  vruns.clear();
  if (vs.size() == 0) return;
  int start = vs[0], cur = vs[0];
  for (int i = 1; i < vs.size(); ++i) {
    if (vs[i] == cur + 1)
      cur = vs[i];
    else {
      vruns.push_back(make_pair(start, cur));
      start = cur = vs[i];
    }
  }
  vruns.push_back(make_pair(start, cur));
}
