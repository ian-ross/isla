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

const int HADCM3_NLAT = 37, HADCM3_NLON = 48;
const double HADCM3_LAT0 = 90.0, HADCM3_LON0 = 0.0;
const double HADCM3_DLAT = -5.0, HADCM3_DLON = 7.5;


// Create a default model: HadCM3 grid, no land.

IslaModel::IslaModel() :
  gr(new Grid(HADCM3_NLAT, HADCM3_LAT0, HADCM3_DLAT,
              HADCM3_NLON, HADCM3_LON0, HADCM3_DLON)),
  orig_mask(gr, false),
  mask(orig_mask),              // Unchanged from "original".
  grid_changes(0),
  island_threshold(20),         // Reasonable default.
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

void IslaModel::LoadMask(std::string file, std::string var)
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

void IslaModel::SaveMask(std::string file)
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


// Get and set current island size threshold value.  Changing this
// value may trigger recalculations.

void IslaModel::setIslandThreshold(int thresh)
{
  if (thresh != island_threshold) {
    island_threshold = thresh;
    recalcAll();
  }
}


// Recalculate everything: land masses, ISMASK, islands.

void IslaModel::recalcAll(void)
{
  landmass = GridData<int>(gr, 0);
  calcLandMasses();
  ismask = GridData<int>(gr, 0);
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
  lmsizes.resize(region, 0);
  for (int r = 0; r < gr->nlat(); ++r)
    for (int c = 0; c < gr->nlon(); ++c)
      if (landmass(r, c) >= 0) ++lmsizes[landmass(r, c)];
  set<int> island_regions;
  for (int i = 1; i < region; ++i)
    if (lmsizes[i] <= island_threshold) island_regions.insert(i);
  is_island = false;
  for (int r = 0; r < gr->nlat(); ++r)
    for (int c = 0; c < gr->nlon(); ++c)
      is_island(r, c) =
        island_regions.find(landmass(r, c)) != island_regions.end();
}



