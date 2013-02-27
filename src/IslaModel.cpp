//----------------------------------------------------------------------
// FILE:   IslaModel.cpp
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Model class for Isla island editor.
//----------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <stack>
using namespace std;

#include "ncFile.h"
using namespace netCDF;

#include "IslaModel.hh"
#include "IslaPreferences.hh"

GridPtr IslaModel::makeGrid(GridType g)
{
  Grid *newgr;
  switch (g) {
  case HadCM3L: {
    const int HADCM3L_NLAT = 37, HADCM3L_NLON = 48;
    const double HADCM3L_LAT0 = 90.0, HADCM3L_LON0 = 0.0;
    const double HADCM3L_DLAT = -5.0, HADCM3L_DLON = 7.5;
    newgr = new Grid(HADCM3L_NLAT, HADCM3L_LAT0, HADCM3L_DLAT,
                     HADCM3L_NLON, HADCM3L_LON0, HADCM3L_DLON);
    break;
  }
  case HadCM3: {
    const int HADCM3_NLAT = 73, HADCM3_NLON = 96;
    const double HADCM3_LAT0 = 89.5, HADCM3_LON0 = 0.0;
    const double HADCM3_DLAT = -2.5, HADCM3_DLON = 3.75;
    newgr = new Grid(HADCM3_NLAT, HADCM3_LAT0, HADCM3_DLAT,
                     HADCM3_NLON, HADCM3_LON0, HADCM3_DLON);
    break;
  }
  case HadGEM2: {
    const int HADGEM2_NLAT = 145, HADGEM2_NLON = 192;
    const double HADGEM2_LAT0 = -90.0, HADGEM2_LON0 = 0.0;
    const double HADGEM2_DLAT = 1.25, HADGEM2_DLON = 1.875;
    newgr = new Grid(HADGEM2_NLAT, HADGEM2_LAT0, HADGEM2_DLAT,
                     HADGEM2_NLON, HADGEM2_LON0, HADGEM2_DLON);
    break;
  }
  }
  return GridPtr(newgr);
}

// Create a default model: HadCM3 grid, no land.

IslaModel::IslaModel() :
  gr(makeGrid(IslaPreferences::get()->getGrid())),
  orig_mask(gr, false),
  mask(orig_mask),              // Unchanged from "original".
  grid_changes(0),
  landmass(gr, 0),              // All ocean.
  is_island(gr, false),         // All ocean.
  ismask(gr, 0)                 // All ocean.
{ }


// Reset to original empty mask.

void IslaModel::reset(void)
{
  *this = IslaModel();
}


// Load a new mask from a NetCDF file.

void IslaModel::loadMask(std::string file, std::string var)
{
  NcFile nc(file, NcFile::read);
  GridPtr newgr = GridPtr(new Grid(nc));
  GridData<bool> new_mask(newgr, nc, var);
  maskfile = file;
  maskvar = var;
  gr = newgr;
  orig_mask = new_mask;
  mask = orig_mask;
  is_island = GridData<bool>(newgr, false);
  recalcAll();
}


// Load a new mask from a NetCDF file.

void IslaModel::saveMask(std::string file)
{
  // Set up NetCDF file.
  NcFile nc(file, NcFile::replace);
  NcDim latdim = nc.addDim("lat", gr->nlat());
  NcVar latvar = nc.addVar("lat", NcType::nc_DOUBLE, latdim);
  latvar.putAtt("long_name", "latitude");
  latvar.putAtt("units", "degrees_north");
  NcDim londim = nc.addDim("lon", gr->nlon());
  NcVar lonvar = nc.addVar("lon", NcType::nc_DOUBLE, londim);
  lonvar.putAtt("long_name", "longitude");
  lonvar.putAtt("units", "degrees_east");
  vector<NcDim> maskdims(2);
  maskdims[0] = latdim;
  maskdims[1] = londim;
  NcVar maskvar = nc.addVar("mask", NcType::nc_INT, maskdims);

  // Write data.
  latvar.putVar(gr->lats().data());
  lonvar.putVar(gr->lons().data());
  GridData<int> intmask(gr, 0);
  mask.process(intmask, GridData<bool>::Convert<int>());
  maskvar.putVar(intmask.data().data());

  // Record that we've saved the grid.
  orig_mask = mask;
  grid_changes = 0;
}


// Recalculate everything: land masses, ISMASK, islands.

void IslaModel::recalcAll(void)
{
  landmass = GridData<int>(gr, 0);
  calcLandMasses();
  ismask = GridData<int>(gr, 0);
  calcIsMask();
  isles.clear();
  calcIslands();
}


// Set island state of landmass around a given cell.

void IslaModel::setIsIsland(int cr, int cc, bool val)
{
  if (!mask(cr, cc)) return;
  int lm = landmass(cr, cc);
  for (int r = 0; r < gr->nlat(); ++r)
    for (int c = 0; c < gr->nlon(); ++c)
      if (landmass(r, c) == lm) is_island(r, c) = val;
}


// Index land masses using flood fill.

template<typename T>
static void floodFill(GridData<T> &res, int r0, int c0, T val, T empty,
                      const GridData<bool> &mask)
{
  typedef pair<int,int> Cell;
  stack<Cell> st;
  int nc = res.grid()->nlon(), nr = res.grid()->nlat();
  st.push(Cell(r0, c0));
  while (!st.empty()) {
    Cell chk = st.top();
    st.pop();
    int r = chk.first, c =chk.second;
    if (mask(r, c) && res(r, c) == empty) {
      res(r, c) = val;
      int cp1 = (c + 1) % nc, cm1 = (c - 1 + nc) % nc;
      st.push(Cell(r, cp1));
      st.push(Cell(r, cm1));
      if (r > 0) {
        st.push(Cell(r - 1, c));
        st.push(Cell(r - 1, cp1));
        st.push(Cell(r - 1, cm1));
      }
      if (r < nr - 1) {
        st.push(Cell(r + 1, c));
        st.push(Cell(r + 1, cp1));
        st.push(Cell(r + 1, cm1));
      }
    }
  }
}

void IslaModel::calcLandMasses(void)
{
  landmass = -1;
  int region = 1;
  for (int r = 0; r < gr->nlat(); ++r)
    for (int c = 0; c < gr->nlon(); ++c) {
      if (landmass(r, c) >= 0) continue;
      if (!mask(r, c)) { landmass(r, c) = 0; continue; }
      floodFill(landmass, r, c, region++, -1, mask);
    }
  lmsizes.clear();
  lmsizes.resize(region, 0.0);
  for (int r = 0; r < gr->nlat(); ++r)
    for (int c = 0; c < gr->nlon(); ++c)
      if (landmass(r, c) >= 0) lmsizes[landmass(r, c)] += gr->cellArea(r, c);
  set<int> island_regions;
  double island_threshold = IslaPreferences::get()->getIslandThreshold();
  for (int i = 1; i < region; ++i)
    if (lmsizes[i] <= island_threshold) island_regions.insert(i);
  is_island = false;
  for (int r = 0; r < gr->nlat(); ++r)
    for (int c = 0; c < gr->nlon(); ++c)
      is_island(r, c) =
        island_regions.find(landmass(r, c)) != island_regions.end();
}


// Calculate ISMASK field for island boundary calculations.

void IslaModel::calcIsMask(void)
{
  int nlon = gr->nlon(), nlat = gr->nlat();
  for (int r = 0; r < nlat; ++r)
    for (int c = 0; c < nlon; ++c) {
      int v = mask(r, c);
      v += (r == 0) || mask(r - 1, c);
      v += mask(r, (c - 1 + nlon) % nlon);
      v += (r == 0) || mask(r - 1, (c - 1 + nlon) % nlon);
      switch (v) {
      case 4:  ismask(r, c) = 2; break;
      case 0:  ismask(r, c) = 0; break;
      default: ismask(r, c) = 1; break;
      }
    }
}


// Recalculate island information.

void IslaModel::calcIslands(void)
{
  // For each island landmass...
  for (int lm = 1; lm < lmsizes.size(); ++lm) {
    int startr, startc;
    bool found = false;
    for (startr = 0; !found && startr < gr->nlat(); ++startr)
      for (startc = 0; !found && startc < gr->nlon(); ++startc)
        if (landmass(startr, startc) == lm) found = true;
    if (!is_island(startr, startc)) continue;

    // Determine the extent of the landmass.
    int minr = startr, maxr = startr, minc = startc, maxc = startc;
    for (int r = 0; r < gr->nlat(); ++r)
      for (int c = 0; c < gr->nlon(); ++c)
        if (landmass(r, c) == lm) {
          maxr = max(r, maxr);  maxc = max(c, maxc);
          minr = min(r, minr);  minc = min(c, minc);
        }

    // Make a bounding box (potentially broken -- no account of
    // longitude wraparound).
    IslandInfo is;
    char tmp[15];
    sprintf(tmp, "Landmass %d", lm);
    is.name = tmp;
    is.bbox = Rect(minc, minr, maxc - minc + 1, maxr - minr + 1);
    is.segments.push_back(is.bbox);
    isles[lm] = is;

    cout << "Island: " << is.name
         << "   l=" << is.bbox.l << " b=" << is.bbox.b
         << " w=" << is.bbox.w << " h=" << is.bbox.h << endl;
  }
}
