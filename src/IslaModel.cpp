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
#include <limits>
using namespace std;

#include <wx/ffile.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#include "ncFile.h"
using namespace netCDF;

#include "IslaModel.hh"
#include "IslaCompute.hh"
#include "IslaPreferences.hh"

const double HadGEM2_lats[] = {
  -90, -89, -88, -87, -86, -85, -84, -83, -82, -81, -80, -79, -78, -77,
  -76, -75, -74, -73, -72, -71, -70, -69, -68, -67, -66, -65, -64, -63,
  -62, -61, -60, -59, -58, -57, -56, -55, -54, -53, -52, -51, -50, -49,
  -48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, -36, -35,
  -34, -33, -32, -31.00039, -30.00231, -29.0073, -28.01687, -27.03253,
  -26.05573, -25.08791, -24.13045, -23.1847, -22.25195, -21.33341,
  -20.43026, -19.54357, -18.67437, -17.82357, -16.99202, -16.18048,
  -15.38961, -14.61997, -13.87203, -13.14615, -12.44259, -11.76151,
  -11.10297, -10.4669, -9.853161, -9.261481, -8.691499, -8.142748,
  -7.614665, -7.106587, -6.617763, -6.147347, -5.694411, -5.257942,
  -4.836853, -4.429985, -4.03611, -3.653943, -3.282142, -2.919319,
  -2.564043, -2.214844, -1.870231, -1.528686, -1.188679, -0.8490555,
  -0.5094325, -0.1698095, 0.1698095, 0.5094325, 0.8490555, 1.188679,
  1.528686, 1.870231, 2.214844, 2.564043, 2.919319, 3.282142, 3.653943,
  4.03611, 4.429985, 4.836853, 5.257942, 5.694411, 6.147347, 6.617763,
  7.106587, 7.614665, 8.142748, 8.691499, 9.261481, 9.853161, 10.4669,
  11.10297, 11.76151, 12.44259, 13.14615, 13.87203, 14.61997, 15.38961,
  16.18048, 16.99202, 17.82357, 18.67437, 19.54357, 20.43026, 21.33341,
  22.25195, 23.1847, 24.13045, 25.08791, 26.05573, 27.03253, 28.01687,
  29.0073, 30.00231, 31.00039, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
  43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
  61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90
};

GridPtr IslaModel::makeGrid(GridType g)
{
  Grid *newgr;
  switch (g) {
  case HadCM3L: {
    const int HADCM3L_NLAT = 73, HADCM3L_NLON = 96;
    const double HADCM3L_LAT0 = -90.0, HADCM3L_LON0 = 0.0;
    const double HADCM3L_DLAT = 2.5, HADCM3L_DLON = 3.75;
    newgr = new Grid(HADCM3L_NLAT, HADCM3L_LAT0, HADCM3L_DLAT,
                     HADCM3L_NLON, HADCM3L_LON0, HADCM3L_DLON);
    break;
  }
  case HadCM3: {
    const int HADCM3_NLAT = 144, HADCM3_NLON = 288;
    const double HADCM3_LAT0 = 89.375, HADCM3_LON0 = 0.0;
    const double HADCM3_DLAT = -1.25, HADCM3_DLON = 1.25;
    newgr = new Grid(HADCM3_NLAT, HADCM3_LAT0, HADCM3_DLAT,
                     HADCM3_NLON, HADCM3_LON0, HADCM3_DLON);
    break;
  }
  case HadGEM2: {
    vector<double> lats(HadGEM2_lats,
                        HadGEM2_lats + sizeof(HadGEM2_lats) / sizeof(double));
    const int HADGEM2_NLON = 360;
    const double HADGEM2_LON0 = 0.0;
    const double HADGEM2_DLON = 1.0;
    newgr = new Grid(lats, HADGEM2_NLON, HADGEM2_LON0, HADGEM2_DLON);
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
  nlandmass(0),
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
  landmass = GridData<LMass>(newgr, 0);
  ismask = GridData<int>(newgr, 0);
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
  calcLandMasses();
  calcIsMask();
  calcBBoxes();
  isles.clear();
  calcIslands();
}


// Set island state of landmass around a given cell.

void IslaModel::setIsIsland(int cr, int cc, bool val)
{
  if (!mask(cr, cc)) return;
  LMass lm = landmass(cr, cc);
  bool found = false, before = false;
  int nlat = gr->nlat(), nlon = gr->nlon();
  for (int r = 0; r < nlat; ++r)
    for (int c = 0; c < nlon; ++c)
      if (landmass(r, c) == lm) {
        if (!found) {
          before = is_island(r, c);
          if (before == val) return;
          found = true;
        }
        is_island(r, c) = val;
      }
  if (before) isles.erase(lm);
  else calcIsland(lm);
}


// Index land masses using flood fill.

template<typename T>
static void floodFill(GridData<T> &res, int r0, int c0, T val, T empty,
                      const GridData<bool> &mask)
{
  int nc = res.grid()->nlon(), nr = res.grid()->nlat();
  typedef pair<int,int> Cell;
  stack<Cell> st;
  st.push(Cell(r0, c0));
  while (!st.empty()) {
    Cell chk = st.top();
    st.pop();
    int r = chk.first, c = chk.second;
    if (mask(r, c % nc) && res(r, c % nc) == empty) {
      res(r, c % nc) = val;
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


// Determine landmasses and classify as island/not-island based on
// area threshold.

void IslaModel::calcLandMasses(void)
{
  // Determine initial land mass extents by flood fill.
  int nlon = gr->nlon(), nlat = gr->nlat();
  nlandmass = 0;
  LMass marker = numeric_limits<unsigned int>::max();
  landmass = marker;
  for (int r = 0; r < nlat; ++r)
    for (int c = 0; c < nlon; ++c) {
      if (landmass(r, c) != marker) continue;
      if (!mask(r, c)) { landmass(r, c) = 0; continue; }
      floodFill(landmass, r, c, ++nlandmass, marker, mask);
    }

  // Calculate land mass areas.
  lmsizes.clear();   lmsizes.resize(nlandmass+1, 0.0);
  lmcounts.clear();  lmcounts.resize(nlandmass+1, 0);
  for (int r = 0; r < nlat; ++r)
    for (int c = 0; c < nlon; ++c)
      if (landmass(r, c) >= 0) {
        ++lmcounts[landmass(r, c)];
        lmsizes[landmass(r, c)] += gr->cellArea(r, c);
      }

  // Filter for islands based on size threshold.
  set<LMass> island_regions;
  double island_threshold = IslaPreferences::get()->getIslandThreshold();
  for (LMass i = 1; i < nlandmass; ++i)
    if (lmsizes[i] <= island_threshold) island_regions.insert(i);

  // Mark island regions.
  is_island = false;
  for (int r = 0; r < nlat; ++r)
    for (int c = 0; c < nlon; ++c)
      is_island(r, c) =
        island_regions.find(landmass(r, c)) != island_regions.end();
}


// Calculate ISMASK field for island boundary calculations.  Also
// extends landmass values into all cells with ISMASK != 0.

void IslaModel::calcIsMask(void)
{
  int nlon = gr->nlon(), nlat = gr->nlat();
  for (int r = 0; r < nlat; ++r)
    for (int c = 0; c < nlon; ++c) {
      int v0 = mask(r, c);
      int vs = (r == 0) || mask(r - 1, c);
      int vw = mask(r, (c - 1 + nlon) % nlon);
      int vsw = (r == 0) || mask(r - 1, (c - 1 + nlon) % nlon);
      switch (v0 + vs + vw + vsw) {
      case 4: ismask(r, c) = 2; break;
      case 0: ismask(r, c) = 0; break;
      default: {
        ismask(r, c) = 1;
        if (!v0) {
          LMass lm = 0;
          if (vs && r != 0) lm = landmass(r - 1, c);
          if (!lm && vw) lm = landmass(r, (c - 1 + nlon) % nlon);
          if (!lm && r != 0 && vsw) lm = landmass(r - 1, (c - 1 + nlon) % nlon);
          landmass(r, c) = lm;
        }
        break;
      }
      }
    }
}


// Calculate bounding boxes for landmasses.

void IslaModel::calcBBoxes(void)
{
  int nc = gr->nlon(), nr = gr->nlat();
  lmbbox.clear();
  for (LMass lm = 1; lm <= nlandmass; ++lm) {
    bool found = false;
    int c;
    for (c = 2; c <= nc + 1; ++c) {
      for (int r = 0; r < nr; ++r)
        if (landmass(r, c % nc) == lm) { found = true; break; }
      if (found) break;
    }
    if (!found) throw logic_error("can't find landmass that should be there!");
    int minc = c, maxc = c, minr = nr - 1, maxr = 0;
    for (; maxc <= nc + 1; ++maxc) {
      bool found = false;
      for (int r = 0; r < nr; ++r) {
        if (landmass(r, maxc % nc) == lm) {
          found = true;  minr = min(minr, r);  maxr = max(maxr, r);
        }
      }
      if (!found) { --maxc;  break; }
    }
    BBox bbox;
    bbox.b1 = wxRect(minc, minr, minc != maxc && maxc % nc == minc ?
                     nc : maxc-minc+1, maxr-minr+1);
    if (minc == 2 && bbox.b1.width != nc) {
      bool found = false;
      int c;
      for (c = nc + 1; c > 2; --c) {
        for (int r = 0; r < nr; ++r)
          if (landmass(r, c % nc) == lm) { found = true; break; }
        if (found) break;
      }
      if (found) {
        int minc = c, maxc = c, minr = nr - 1, maxr = 0;
        for (; minc > 2; --minc) {
          bool found = false;
          for (int r = 0; r < nr; ++r) {
            if (landmass(r, minc % nc) == lm) {
              found = true;  minr = min(minr, r);  maxr = max(maxr, r);
            }
          }
          if (!found) { ++minc;  break; }
        }
        bbox.both = true;
        bbox.b2 = wxRect(minc, minr, maxc-minc+1, maxr-minr+1);
      }
    }
    lmbbox[lm] = bbox;
  }
}


// Recalculate island information for a single landmass.

bool IslaModel::calcIsland(LMass lm)
{
  IslaCompute compute(landmass, lmbbox, ismask);
  int startr = 0, startc = 0;
  bool found = false;
  int nlat = gr->nlat(), nlon = gr->nlon();
  for (startr = 0; startr < nlat; ++startr) {
    for (startc = 0; startc < nlon; ++startc)
      if (landmass(startr, startc) == lm) { found = true;  break; }
    if (found) break;
  }
  if (!is_island(startr, startc)) return false;

  // Set up island info.
  IslandInfo is;
  char tmp[15];
  sprintf(tmp, "Landmass %d", lm);
  is.name = tmp;
  if (isles.find(lm) != isles.end()) is.minsegs = isles[lm].minsegs;
  compute.segment(lm, isles[lm].minsegs, is.segments);
  IslaCompute::coincidence(is.segments, is.vcoinc, is.hcoinc);
  isles[lm] = is;
  return true;
}


// Recalculate island information for all landmasses.

void IslaModel::calcIslands(void)
{
  // For each island landmass...
  for (LMass lm = 1; lm < lmsizes.size(); ++lm)
    if (calcIsland(lm))
      isles[lm].absminsegs = isles[lm].segments.size();
}


// Segmentation control methods.

void IslaModel::coarsenIsland(int r, int c)
{
  LMass lm = landmass(r, c);
  unsigned int cursegs = isles[lm].segments.size();
  while (isles[lm].segments.size() == cursegs &&
         isles[lm].minsegs > isles[lm].absminsegs) {
    isles[lm].minsegs = max(isles[lm].minsegs - 1, isles[lm].absminsegs);
    calcIsland(lm);
  }
}

void IslaModel::refineIsland(int r, int c)
{
  LMass lm = landmass(r, c);
  unsigned int cursegs = isles[lm].segments.size();
  while (isles[lm].segments.size() == cursegs &&
         isles[lm].minsegs < lmcounts[lm]) {
    isles[lm].minsegs = min(isles[lm].minsegs + 1, lmcounts[lm]);
    calcIsland(lm);
  }
}

void IslaModel::resetIsland(int r, int c)
{
  LMass lm = landmass(r, c);
  isles[lm].minsegs = isles[lm].absminsegs;
  calcIsland(lm);
}


// File handling functions.

static void swap_bytes(void *buf, int nbytes, int n)
{
  unsigned char *cbuf = static_cast<unsigned char *>(buf);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < nbytes / 2; ++j) {
      unsigned char temp = cbuf[j];
      cbuf[j] = cbuf[nbytes - j - 1];
      cbuf[nbytes - j - 1] = temp;
    }
    cbuf += nbytes;
  }
}

static void readIslandDataFromDump(int grid_nx, int grid_ny, wxFFile &fp,
                                   vector<IslaModel::IslandInfo> &isl)
{
  // Check on the dump file type.  We only support 64-bit IEEE files,
  // but we handle byte-swapping OK.
  // ==> THIS RELIES ON longs BEING 64 BITS!
  long ibuf[2];
  fp.Read(ibuf, 2 * sizeof(long));
  bool swapped = false;
  if (ibuf[1] < 1 || ibuf[1] > 4) {
    swapped = true;
    swap_bytes(ibuf, sizeof(long), 2);
    if (ibuf[1] < 1 || ibuf[1] > 4)
      throw runtime_error("Dump files must be 64-bit byte-swapped IEEE");
  }
  if (ibuf[1] != 2)
    throw runtime_error("Dump file is not an ocean dump");

  // Read the fixed header and find the offset for the integer
  // constants and island data.
  long fixhd[256];
  fp.Seek(0);
  fp.Read(fixhd, 256 * sizeof(long));
  if (swapped) swap_bytes(fixhd, sizeof(long), 256);
  long int_consts_offset = fixhd[100-1];
  long int_consts_length = fixhd[101-1];
  long extra_data_offset = fixhd[130-1];
  long extra_data_length = fixhd[131-1];

  // Read the integer constants data and check the grid size.
  long int_data[int_consts_length];
  fp.Seek((int_consts_offset - 1) * sizeof(long));
  fp.Read(int_data, int_consts_length * sizeof(long));
  if (swapped) swap_bytes(int_data, sizeof(long), int_consts_length);
  long dump_nx = int_data[6-1], dump_ny = int_data[7-1];
  if (dump_nx != grid_nx + 2 || dump_ny != grid_ny)
    throw runtime_error("Dump file grid does not match land/sea mask");

  // Read the extra constants data.
  double data[extra_data_length];
  fp.Seek((extra_data_offset - 1) * sizeof(double));
  fp.Read(data, extra_data_length * sizeof(double));
  if (swapped) swap_bytes(data, sizeof(double), extra_data_length);

  // Set up island data.
  isl.resize(static_cast<unsigned int>(data[0]));
  int idata = 1;
  vector<int> isis, ieis, jsis, jeis;
  for (unsigned int iisl = 0; iisl < isl.size(); ++iisl){
    char tmp[32];
    sprintf(tmp, "Island %d", iisl + 1);
    isl[iisl].name = tmp;
    unsigned int nseg = static_cast<unsigned int>(data[idata++]);
    if (idata + nseg * 4 > extra_data_length)
      throw runtime_error("Format error in dump file island data");
    isis.clear();  ieis.clear();  jsis.clear();  jeis.clear();
    for (unsigned int i = 0; i < nseg; ++i)
      isis.push_back(static_cast<unsigned int>(data[idata++]));
    for (unsigned int i = 0; i < nseg; ++i)
      ieis.push_back(static_cast<unsigned int>(data[idata++]));
    for (unsigned int i = 0; i < nseg; ++i)
      jsis.push_back(static_cast<unsigned int>(data[idata++]));
    for (unsigned int i = 0; i < nseg; ++i)
      jeis.push_back(static_cast<unsigned int>(data[idata++]));
    isl[iisl].segments.resize(nseg);
    for (unsigned int i = 0; i < nseg; ++i)
      isl[iisl].segments[i] =
        wxRect(isis[i], jsis[i], ieis[i]-isis[i]+1, jeis[i]-jsis[i]+1);
  }
}

static bool parseASCIIIslands(int grid_nx, int grid_ny, wxString fname,
                              vector<IslaModel::IslandInfo> &isl)
{
  // Read whole file contents.
  wxTextFile fp(fname);
  if (!fp.Open())
    throw runtime_error(string("Cannot open file: ") +
                        string(fname.char_str()));

  // Process line by line.
  enum State { BEFORE_COUNT, BEFORE_SEGS, READING_SEGS };
  State state = BEFORE_COUNT;
  int iisl = 0, istep = 0, iseg = 0, nseg = 0;
  wxString islandname = _("");
  vector<int> isis, ieis, jsis, jeis;
  bool bad = false;
  for (wxString line = fp.GetFirstLine(); !fp.Eof(); line = fp.GetNextLine()) {
    line = line.Trim();
    if (line.Len() == 0) continue;
    if (line[0] == wxChar('#')) {
      // Comment line: if this is in the "BEFORE_SEGS" state, we
      // assume that it's a comment giving the name of the island.
      if (state == BEFORE_SEGS) islandname = line.Mid(1).Trim();
    } else {
      // Should be a whitespace separated string of integers.
      wxStringTokenizer tok(line);
      while (tok.HasMoreTokens()) {
        long val;
        if (!tok.GetNextToken().ToLong(&val))
          throw runtime_error("Failed to convert integer");
        switch (state) {
        case BEFORE_COUNT:
          isl.resize(val);
          state = BEFORE_SEGS;
          break;
        case BEFORE_SEGS: {
          if (islandname.Len() == 0) {
            char tmp[32];
            sprintf(tmp, "Island %d", iisl + 1);
            isl[iisl].name = tmp;
          } else isl[iisl].name = islandname.ToAscii();
          islandname = _("");
          isis.clear();  ieis.clear();  jsis.clear();  jeis.clear();
          nseg = val;  istep = 0;  iseg = 0;
          state = READING_SEGS;
          break;
        }
        case READING_SEGS: {
          switch (istep) {
          case 0:
            if (val < 2) { bad = true; val = 2; }
            if (val > grid_nx + 1) { bad = true; val = grid_nx + 1; }
            isis.push_back(val);
            break;
          case 1:
            if (val < 2) { bad = true; val = 2; }
            if (val > grid_nx + 1) { bad = true; val = grid_nx + 1; }
            ieis.push_back(val);
            break;
          case 2:
            if (val < 1) { bad = true; val = 1; }
            if (val > grid_ny) { bad = true; val = grid_ny; }
            jsis.push_back(val);
            break;
          case 3:
            if (val < 1) { bad = true; val = 1; }
            if (val > grid_ny) { bad = true; val = grid_ny; }
            jeis.push_back(val);
            break;
          }
          if (++iseg == nseg) {
            iseg = 0;
            if (++istep == 4) {
              isl[iisl].segments.resize(nseg);
              for (int i = 0; i < nseg; ++i)
                isl[iisl].segments[i] =
                  wxRect(isis[i], jsis[i],
                         ieis[i]-isis[i]+1, jeis[i]-jsis[i]+1);
              ++iisl;
              state = BEFORE_SEGS;
            }
          }
          break;
        }
        }
      }
    }
  }
  return !bad;
}

void IslaModel::saveIslands(wxString fname)
{
  wxTextFile fp(fname);
  if (fp.Exists()) { fp.Open(); fp.Clear(); } else fp.Create();

  fp.AddLine(_("# Island file"));
  fp.AddLine(_(""));
  fp.AddLine(wxString::Format(_("%d"), isles.size()));
  for (map<LMass, IslandInfo>::const_iterator it = isles.begin();
       it != isles.end(); ++it) {
    const IslandInfo &is = it->second;
    fp.AddLine(_(""));
    fp.AddLine(wxString(_("# ")) + wxString::FromAscii(is.name.c_str()));
    const vector<wxRect> &segs = is.segments;
    fp.AddLine(wxString::Format(_("%d"), segs.size()));
    bool firstseg = true;
    wxString isis, ieis, jsis, jeis;
    for (unsigned int i = 0; i < segs.size(); ++i) {
      if (!firstseg) {
        isis += _(" "); ieis += _(" "); jsis += _(" "); jeis += _(" ");
      }
      firstseg = false;
      isis += wxString::Format(_("%d"), segs[i].x);
      ieis += wxString::Format(_("%d"), segs[i].x + segs[i].width - 1);
      jsis += wxString::Format(_("%d"), segs[i].y);
      jeis += wxString::Format(_("%d"), segs[i].y + segs[i].height - 1);
    }
    fp.AddLine(isis);
    fp.AddLine(ieis);
    fp.AddLine(jsis);
    fp.AddLine(jeis);
  }

  fp.Write();
}

bool IslaModel::loadIslands(wxString fname, vector<IslandInfo> &isles)
{
  // Check that the file exists.
  wxFFile fp(fname, _("rb"));
  if (!fp.IsOpened())
    throw runtime_error(string("Cannot open file: ") +
                        string(fname.char_str()));

  // Determine whether it's a binary file or an ASCII file.  This is a
  // bit hit-and-miss: just read 128 bytes from the file and check
  // whether any of the values are outside the ASCII 7-bit range.
  char buff[128];
  size_t nread = fp.Read(buff, 128);
  fp.Seek(0);
  bool binary = false;
  for (size_t i = 0; i < nread; ++i)
    if (buff[i] & 0x80) { binary = true; break; }

  // Read raw island data from dump file, checking that it's a
  // suitable ocean dump file as we do so, or parse an ASCII islands
  // file.
  vector<IslandInfo> isltmp;
  bool ok = true;
  if (binary)
    readIslandDataFromDump(gr->nlon(), gr->nlat(), fp, isltmp);
  else
    ok = parseASCIIIslands(gr->nlon(), gr->nlat(), fname, isltmp);

  // Compute coincidence line segments for island display.
  for (vector<IslandInfo>::iterator it = isltmp.begin();
       it != isltmp.end(); ++it)
    IslaCompute::coincidence(it->segments, it->vcoinc, it->hcoinc);

  // Check...
  // cout << "#islands = " << isltmp.size() << endl;
  // for (int i = 0; i < isltmp.size(); ++i) {
  //   vector<wxRect> &ss = isltmp[i].segments;
  //   cout << "  " << i+1 << ": " << isltmp[i].name
  //        << " (" << ss.size() << ")" << endl;
  //   for (int j = 0; j < ss.size(); ++j)
  //     cout << "    L:" << ss[j].x << " B:" << ss[j].y
  //          << " W:" << ss[j].width << " H:" << ss[j].height << endl;
  // }

  // Set up new island data.
  isles = isltmp;
  return ok;
}

