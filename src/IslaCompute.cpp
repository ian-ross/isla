//----------------------------------------------------------------------
// FILE:   IslaCompute.cpp
// DATE:   17-MAR-2013
// AUTHOR: Ian Ross
//
// Island bounding box calculation class for Isla island editor.
//----------------------------------------------------------------------

#include <algorithm>
#include <map>
#include "IslaCompute.hh"

using namespace std;


// Calculate island segments for given landmass.

void IslaCompute::segment(LMass lm, Boxes &bs)
{
  Seg byrows, bycols;
  boundRows(lm, byrows);
  boundCols(lm, bycols);
  bool dorows = true, docols = true;
  if (byrows.size() > 5 * bycols.size()) dorows = false;
  if (bycols.size() > 5 * byrows.size()) docols = false;
  if (dorows) scoredSegmentation(lm, byrows);
  if (docols) scoredSegmentation(lm, bycols);
  if (dorows && docols)
    extractBoxes(byrows.size() <= bycols.size() ? byrows : bycols, bs);
  else
    extractBoxes(dorows ? byrows : bycols, bs);
}

void IslaCompute::scoredSegmentation(LMass lm, Seg &segs)
{
  if (segs.size() == 1) return;
  int segid = segs.size();
  while (step(lm, segs, segid)) ++segid;
}

bool IslaCompute::step(LMass lm, Seg &segs, int segid)
{
  // Determine acceptable merges from candidates for each box.
  if (segs.size() == 1) return false;
  vector<Merge> ok;
  for (Seg::iterator it = segs.begin(); it != segs.end(); ++it) {
    BoxInfo &a = it->second;
    for (set<int>::iterator jt = a.cands.begin(); jt != a.cands.end(); ++jt) {
      Box newb = a.b;
      newb.Union(segs[*jt].b);
      if (!overlap(newb, segs, it->first, *jt) && admissible(lm, newb))
        ok.push_back(Merge(it->first, *jt,
                           score(lm, segs, newb, it->first, *jt)));
    }
  }
  if (ok.size() == 0) return false;

  // Find best merge.
  vector<Merge>::const_iterator it = min_element(ok.begin(), ok.end());
  BoxInfo a = segs[it->i], b = segs[it->j], x;

  // Merged box.
  x.b = a.b;
  x.b.Union(b.b);

  // Calculate merge candidates for new box.
  set<int> tmp;
  set_union(a.cands.begin(), a.cands.end(), b.cands.begin(), b.cands.end(),
            insert_iterator<set<int> >(tmp, tmp.begin()));
  set<int> ab;  ab.insert(it->i);  ab.insert(it->j);
  set_difference(tmp.begin(), tmp.end(), ab.begin(), ab.end(),
                 insert_iterator<set<int> >(x.cands, x.cands.begin()));

  // Remove old boxes from merge candidates for other segments.
  for (set<int>::iterator fit = x.cands.begin(); fit != x.cands.end(); ++fit) {
    set<int> tmp;
    BoxInfo &fix = segs[*fit];
    set_difference(fix.cands.begin(), fix.cands.end(), ab.begin(), ab.end(),
                   insert_iterator<set<int> >(tmp, tmp.begin()));
    tmp.insert(segid);
    fix.cands = tmp;
  }

  // Replace boxes with merged box.
  segs.erase(it->i);
  segs.erase(it->j);
  segs[segid] = x;
  return true;
}


// Compute coincidence line segments between adjacent island segments
// (used for distinguishing spatially adjacent segments that belong to
// the same island and adjacent segments from distinct islands).

void IslaCompute::coincidence
(const Boxes &bs, IslaModel::CoincInfo &v, IslaModel::CoincInfo &h)
{
  v.clear();  h.clear();
  for (Boxes::const_iterator i1 = bs.begin(); i1 != bs.end(); ++i1) {
    int l1 = i1->x, b1 = i1->y, r1 = l1 + i1->width, t1 = b1 + i1->height;
    for (Boxes::const_iterator i2 = i1 + 1; i2 != bs.end(); ++i2) {
      int l2 = i2->x, b2 = i2->y, r2 = l2 + i2->width, t2 = b2 + i2->height;
      if ((r1 == l2 || l1 == r2) &&
          (b1 >= b2 && b1 <= t2 || t1 >= b2 && t1 <= t2 ||
           b2 >= b1 && b2 <= t1 || t2 >= b1 && t2 <= t1)) {
        pair<int, int> bnds = make_pair(max(b1,b2), min(t1,t2));
        v.insert(make_pair(r1 == l2 ? r1 : l1, bnds));
      }
      if ((t1 == b2 || b1 == t2) &&
          (l1 >= l2 && l1 <= r2 || r1 >= l2 && r1 <= r2 ||
           l2 >= l1 && l2 <= r1 || r2 >= l1 && r2 <= r1)) {
        pair<int, int> bnds = make_pair(max(l1,l2), min(r1,r2));
        h.insert(make_pair(t1 == b2 ? t1 : b1, bnds));
      }
    }
  }

// #ifdef ISLA_DEBUG
//   if (v.size() > 0 || h.size() > 0) {
//     for (int i = 0; i < bs.size(); ++i)
//       cout << "  x:" << bs[i].x << " y:" << bs[i].y
//            << "  w:" << bs[i].width << " h:" << bs[i].height << endl;
//     if (v.size() > 0) {
//       cout << "vhatch:" << endl;
//       for (multimap<int, pair<int,int> >::const_iterator it = v.begin();
//            it != v.end(); ++it)
//         cout << " col=" << it->first << "  "
//              << it->second.first << "-" << it->second.second << endl;
//     }
//     if (h.size() > 0) {
//       cout << "hhatch:" << endl;
//       for (multimap<int, pair<int,int> >::const_iterator it = h.begin();
//            it != h.end(); ++it)
//         cout << " row=" << it->first << "  "
//              << it->second.first << "-" << it->second.second << endl;
//     }
//   }
// #endif
}


// Determine bounding regions for a given landmass.

IslaCompute::Box IslaCompute::boundBox(LMass lm)
{
  bool found = false;
  int nx = glm.nlon(), ny = glm.nlat();
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

void IslaCompute::boundRows(LMass lm, Seg &rs)
{
  Box bbox = lmbbox.find(lm)->second;
  rs.clear();
  int segid = 0;
  set<int> last, cur;
  for (int y = 0; y < bbox.height; ++y) {
    vector<int> xs;
    for (int x = 0; x < bbox.width; ++x)
      if (glm(bbox.y + y, bbox.x + x) == lm)
        xs.push_back(bbox.x + x);
    vector< pair<int,int> > xruns;
    runs(xs, xruns);
    cur.clear();
    for (int i = 0; i < xruns.size(); ++i) {
      BoxInfo box;
      box.b = Box(xruns[i].first, bbox.y + y,
                  xruns[i].second - xruns[i].first + 1, 1);
      box.score = -1;
      if (i > 0) box.cands.insert(segid - 1);
      if (i < xruns.size() - 1) box.cands.insert(segid + 1);
      cur.insert(segid);
      rs[segid++] = box;
    }
    for (set<int>::iterator it = cur.begin(); it != cur.end(); ++it) {
      for (set<int>::iterator jt = last.begin(); jt != last.end(); ++jt) {
        rs[*it].cands.insert(*jt);
        rs[*jt].cands.insert(*it);
      }
    }
    last = cur;
  }
}

void IslaCompute::boundCols(LMass lm, Seg &cs)
{
  Box bbox = boundBox(lm);
  cs.clear();
  int segid = 0;
  set<int> last, cur;
  for (int x = 0; x < bbox.width; ++x) {
    vector<int> ys;
    for (int y = 0; y < bbox.height; ++y)
      if (glm(bbox.y + y, bbox.x + x) == lm)
        ys.push_back(bbox.y + y);
    vector< pair<int,int> > yruns;
    runs(ys, yruns);
    cur.clear();
    for (int i = 0; i < yruns.size(); ++i) {
      BoxInfo box;
      box.b = Box(bbox.x + x, yruns[i].first,
                  1, yruns[i].second - yruns[i].first + 1);
      box.score = -1;
      if (i > 0) box.cands.insert(segid - 1);
      if (i < yruns.size() - 1) box.cands.insert(segid + 1);
      cur.insert(segid);
      cs[segid++] = box;
    }
    for (set<int>::iterator it = cur.begin(); it != cur.end(); ++it) {
      for (set<int>::iterator jt = last.begin(); jt != last.end(); ++jt) {
        cs[*it].cands.insert(*jt);
        cs[*jt].cands.insert(*it);
      }
    }
    last = cur;
  }
}


// Determine whether points, boxes and sets of boxes are admissible
// for a given landmass.

bool IslaCompute::admissible(LMass lm, int x, int y) const
{
  return glm(y, x) == lm || ismask(y, x) == 0;
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


// Calculate heuristic score for segment list.  Low scores are better.
// Inadmissible sets of boxes should never be passed to this function.

int IslaCompute::score(LMass lm, Seg &ss,
                       const Box &newb, int exc1, int exc2) const
{
  int ret = 0;
  for (Seg::iterator it = ss.begin(); it != ss.end(); ++it) {
    if (it->first == exc1 || it->first == exc2) continue;
    if (it->second.score >= 0)
      ret += it->second.score;
    else {
      Box b = it->second.b;
      int tmp = 0;
      for (int x = 0; x < b.width; ++x)
        for (int y = 0; y < b.height; ++y)
          if (glm(b.y + y, b.x + x) != lm) ++tmp;
      it->second.score = tmp;
      ret += tmp;
    }
  }
  for (int x = 0; x < newb.width; ++x)
    for (int y = 0; y < newb.height; ++y)
      if (glm(newb.y + y, newb.x + x) != lm) ++ret;
  return ret;
}


// Does a box overlap with any of a given set of boxes?

bool IslaCompute::overlap(const Box &b, const Seg &ss, int exc1, int exc2)
{
  bool ret = false;
  for (Seg::const_iterator it = ss.begin(); it != ss.end(); ++it) {
    if (it->first == exc1 || it->first == exc2) continue;
    if (it->second.b.Intersects(b)) { ret = true;  break; }
  }
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


// Extract segment boxes from segment information map.

void IslaCompute::extractBoxes(const Seg &ss, Boxes &bs)
{
  bs.clear();
  for (Seg::const_iterator it = ss.begin(); it != ss.end(); ++it)
    bs.push_back(it->second.b);
}
