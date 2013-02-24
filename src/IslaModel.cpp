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
  island_threshold(20),         // Reasonable default.
  landmass(gr, 0),            // All ocean.
  ismask(gr, 0)               // All ocean.
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
  recalcAll();
}


// Check for changes in grid or islands from the values generated from
// the originally loaded mask data.  These are used as indicators that
// there are changes that might need to be saved before certain
// actions.

bool IslaModel::hasGridChanges(void) const
{
  // Just check to see if any grid cells differ from their original
  // values.
  const vector<bool> &orig = orig_mask.data();
  const vector<bool> &curr = mask.data();
  return !equal(orig.begin(), orig.end(), curr.begin());
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
  cout << "Model recalculation triggered" << endl;
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
      st.push(Cell(r, (c + 1) % nc));
      st.push(Cell(r, (c - 1 + nc) % nc));
      if (r > 0) st.push(Cell(r - 1, c));
      if (r < nr - 1) st.push(Cell(r + 1, c));
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
}



