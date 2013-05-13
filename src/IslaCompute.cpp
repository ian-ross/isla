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

void IslaCompute::segment(LMass lm, int minsegs, Boxes &bs)
{
  Seg byrows, bycols, bycells;
  boundRows(lm, byrows);
  boundCols(lm, bycols);
  bool dorows = true, docols = true;
  if (byrows.size() > 5 * bycols.size()) dorows = false;
  if (bycols.size() > 5 * byrows.size()) docols = false;
  if (dorows) scoredSegmentation(lm, minsegs, byrows);
  if (docols) scoredSegmentation(lm, minsegs, bycols);
  Seg *extr = 0;
  unsigned int minsize = glm.grid()->nlon() * glm.grid()->nlat();
  if (dorows && (extr == 0 || byrows.size() < minsize)) {
    extr = &byrows;
    minsize = byrows.size();
  }
  if (docols && (extr == 0 || bycols.size() < minsize)) {
    extr = &bycols;
    minsize = bycols.size();
  }
  if (extr == 0) throw runtime_error("Segmentation failed!");
  extractBoxes(*extr, bs);
  fixPolar(lm, bs);
}

void IslaCompute::scoredSegmentation(LMass lm, int minsegs, Seg &segs)
{
  if (segs.size() == 1) return;
  BoxID segid = segs.size();
  known_bad.clear();
  known_good.clear();
  while (step(lm, minsegs, segs, segid)) ++segid;
}

bool IslaCompute::step(LMass lm, unsigned int minsegs, Seg &segs, BoxID segid)
{
  // Determine acceptable merges from candidates for each box.
  if (segs.size() <= minsegs) return false;
  vector<Merge> ok;
  for (Seg::iterator it = segs.begin(); it != segs.end(); ++it) {
    BoxID aid = it->first;
    set<BoxID> &bads = known_bad[aid];
    map<BoxID, Score> &goods = known_good[aid];
    BoxInfo &a = it->second;
    for (set<BoxID>::iterator jt = a.cands.begin(); jt != a.cands.end(); ++jt) {
      BoxID bid = *jt;
      if (bads.find(bid) != bads.end()) continue;
      map<BoxID, Score>::iterator git = goods.find(bid);
      if (git != goods.end())
        ok.push_back(Merge(aid, bid, git->second));
      else {
        Box newb = a.b;
        newb.Union(segs[bid].b);
        if (!overlap(newb, segs, aid, bid) && admissible(lm, newb)) {
          Score sc = score(lm, segs, newb, aid, bid);
          goods[bid] = sc;
          ok.push_back(Merge(aid, bid, sc));
        } else bads.insert(bid);
      }
    }
  }
  if (ok.size() == 0) return false;

  // Find best merge.
  vector<Merge>::const_iterator it = min_element(ok.begin(), ok.end());
  BoxID aid = it->i, bid = it->j;
  BoxInfo a = segs[aid], b = segs[bid], x;

  // Merged box.
  x.b = a.b;
  x.b.Union(b.b);

  // Calculate merge candidates for new box.
  set<BoxID> tmp;
  set_union(a.cands.begin(), a.cands.end(), b.cands.begin(), b.cands.end(),
            insert_iterator<set<BoxID> >(tmp, tmp.begin()));
  set<BoxID> ab;  ab.insert(it->i);  ab.insert(it->j);
  set_difference(tmp.begin(), tmp.end(), ab.begin(), ab.end(),
                 insert_iterator<set<BoxID> >(x.cands, x.cands.begin()));

  // Remove old boxes from merge candidates for other segments.
  for (set<BoxID>::iterator fit = x.cands.begin();
       fit != x.cands.end(); ++fit) {
    set<BoxID> tmp;
    BoxInfo &fix = segs[*fit];
    set_difference(fix.cands.begin(), fix.cands.end(), ab.begin(), ab.end(),
                   insert_iterator<set<BoxID> >(tmp, tmp.begin()));
    tmp.insert(segid);
    fix.cands = tmp;
  }

  // Clear "known good" and "known bad" cache items for boxes being
  // merged.
  known_bad.erase(aid);   known_bad.erase(bid);
  known_good.erase(aid);  known_good.erase(bid);

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
      if (((r1 - l2 + 360) % 360 == 0 || (l1 - r2 + 360) % 360 == 0) &&
          ((b1 >= b2 && b1 <= t2) || (t1 >= b2 && t1 <= t2) ||
           (b2 >= b1 && b2 <= t1) || (t2 >= b1 && t2 <= t1))) {
        pair<int, int> bnds = make_pair(max(b1,b2), min(t1,t2));
        v.insert(make_pair((r1 - l2 + 360) % 360 == 0 ? r1 : l1, bnds));
      }
      if ((t1 == b2 || b1 == t2) &&
          ((l1 >= l2 && l1 <= r2) || (r1 >= l2 && r1 <= r2) ||
           (l2 >= l1 && l2 <= r1) || (r2 >= l1 && r2 <= r1))) {
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


// Optimise polar islands.

void IslaCompute::fixPolar(LMass lm, Boxes &bs)
{
  int nx = glm.nlon(), ny = glm.nlat();
  for (Boxes::iterator it = bs.begin(); it != bs.end(); ++it) {
    if (it->width == nx && (it->y == 0 || it->y + it->height == ny)) {
      while(it->height > 1 && fullCircle(lm, it->y + 1)) {
        ++it->y;
        --it->height;
      }
    }
  }
}

bool IslaCompute::fullCircle(LMass lm, int y)
{
  bool ret = true;
  int nx = glm.nlon();
  for (int x = 0; x < nx; ++x)
    if (glm(y, x) != lm) { ret = false; break; }
  return ret;
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
  IslaModel::BBox bbox = lmbbox.find(lm)->second;
  rs.clear();
  int segid = 0;
  set<int> last, cur;
  for (int ib = 0; ib < (bbox.both ? 2 : 1); ++ib) {
    wxRect &box = ib == 0 ? bbox.b1 : bbox.b2;
    for (int y = 0; y < box.height; ++y) {
      vector<int> xs;
      for (int x = 0; x < box.width; ++x)
        if (glm(box.y + y, box.x + x) == lm)
          xs.push_back(box.x + x);
      vector< pair<int,int> > xruns;
      runs(xs, xruns);
      cur.clear();
      for (unsigned int i = 0; i < xruns.size(); ++i) {
        BoxInfo info;
        info.b = Box(xruns[i].first, box.y + y,
                     xruns[i].second - xruns[i].first + 1, 1);
        if (i > 0) info.cands.insert(segid - 1);
        if (i < xruns.size() - 1) info.cands.insert(segid + 1);
        cur.insert(segid);
        rs[segid++] = info;
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
}

void IslaCompute::boundCols(LMass lm, Seg &cs)
{
  IslaModel::BBox bbox = lmbbox.find(lm)->second;
  cs.clear();
  int segid = 0;
  set<int> last, cur;
  for (int ib = 0; ib < (bbox.both ? 2 : 1); ++ib) {
    wxRect &box = ib == 0 ? bbox.b1 : bbox.b2;
    for (int x = 0; x < box.width; ++x) {
      vector<int> ys;
      for (int y = 0; y < box.height; ++y)
        if (glm(box.y + y, box.x + x) == lm)
          ys.push_back(box.y + y);
      vector< pair<int,int> > yruns;
      runs(ys, yruns);
      cur.clear();
      for (unsigned int i = 0; i < yruns.size(); ++i) {
        BoxInfo info;
        info.b = Box(box.x + x, yruns[i].first,
                     1, yruns[i].second - yruns[i].first + 1);
        if (i > 0) info.cands.insert(segid - 1);
        if (i < yruns.size() - 1) info.cands.insert(segid + 1);
        cur.insert(segid);
        cs[segid++] = info;
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
  for (unsigned int i = 0; i < bs.size(); ++i)
    if (!admissible(lm, bs[i])) { ret = false;  break; }
  return ret;
}


// Calculate heuristic score for segment list.  Low scores are better.
// Inadmissible sets of boxes should never be passed to this function.

int IslaCompute::score(LMass lm, Seg &ss,
                       const Box *newb, int exc1, int exc2) const
{
  int ret = 0;
  for (Seg::iterator it = ss.begin(); it != ss.end(); ++it) {
    if ((exc1 > 0 && it->first == exc1) ||
        (exc2 > 0 && it->first == exc2)) continue;
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
  if (newb)
    for (int x = 0; x < newb->width; ++x)
      for (int y = 0; y < newb->height; ++y)
        if (glm(newb->y + y, newb->x + x) != lm) ++ret;
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
  for (unsigned int i = 1; i < vs.size(); ++i) {
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
