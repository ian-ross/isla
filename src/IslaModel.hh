//----------------------------------------------------------------------
// FILE:   IslaModel.hh
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Model class for Isla island editor.
//----------------------------------------------------------------------

#ifndef _H_ISLAMODEL_
#define _H_ISLAMODEL_

#include <string>
#include <vector>
#include <map>

#include <wx/gdicmn.h>
#include "GridData.hh"

// Here, "mask" means a boolean land/sea mask (with true for land,
// false for ocean).

typedef int LMass;

class IslaModel {
public:
  enum GridType { HadCM3L, HadCM3, HadGEM2 };

  // Bounding box type -- can't just use a simple rectangle because
  // islands can extend across prime meridian.
  struct BBox {
    BBox() : both(false) { }
    bool both;
    wxRect b1, b2;
  };

  // Structures used for recording island information.
  typedef std::multimap<int, std::pair<int, int> > CoincInfo;
  struct IslandInfo {
    IslandInfo(): minsegs(1), absminsegs(1) { }
    std::string name;
    int minsegs, absminsegs;
    std::vector<wxRect> segments;
    CoincInfo vcoinc;
    CoincInfo hcoinc;
  };


  // Create a default model: HadCM3L grid, no land, island threshold
  // set at 8.0E6 km^2 (big enough to include Australia).
  IslaModel();
  ~IslaModel() { }

  // Access grid.
  GridPtr grid(void) { return gr; }

  // Reset to original empty mask.
  void reset(void);

  // Load a new mask from a NetCDF file.
  void loadMask(std::string file, std::string var);

  // Save current mask to NetCDF file.
  void saveMask(std::string file);

  // Extract data values.
  bool maskVal(int r, int c) { return mask(r, c); }
  bool origMaskVal(int r, int c) { return orig_mask(r, c); }
  bool isIsland(int r, int c) { return is_island(r, c); }
  LMass landMass(int r, int c) { return landmass(r, c); }
  int isMask(int r, int c) { return ismask(r, c); }
  const std::map<int, IslandInfo> islands(void) const { return isles; }

  // Change data values.
  void setMask(int r, int c, bool val) {
    bool orig = orig_mask(r, c), old = mask(r, c);
    if (old == orig && val != orig) ++grid_changes;
    else if (old != orig && val == orig) --grid_changes;
    mask(r, c) = val;
  }
  void setIsIsland(int cr, int cc, bool val);

  // Check for changes in grid or islands from the values generated
  // from the originally loaded mask data.  These are used as
  // indicators that there are changes that might need to be saved
  // before certain actions.
  bool hasGridChanges(void) const { return grid_changes > 0; }
  bool hasIslandChanges(void) const { return false; }

  // Recalculate everything: land masses, ISMASK, islands.
  void recalcAll(void);

  // Individual recalculation methods.
  void calcLandMasses(void);    // Index land masses.
  void calcIsMask(void);        // Calculate ISMASK.
  void calcBBoxes(void);        // Calculate landmass bounding boxes.
  void calcIsland(LMass lm);    // Analyse single island.
  void calcIslands(void);       // Determine islands from scratch.

  // Control island segmentation level of detail.
  void coarsenIsland(int r, int c);
  void refineIsland(int r, int c);
  void resetIsland(int r, int c);

  // Return landmass bounding box map.
  const std::map<LMass,BBox> landMassBBox(void) const { return lmbbox; }

  // Export current island data.
  void saveIslands(wxString file);

  // Load comparison island data.
  void loadIslands(wxString fname, std::vector<IslandInfo> &isles);

private:
  static GridPtr makeGrid(GridType g);

  std::string maskfile;         // Input mask NetCDF file.
  std::string maskvar;          // Input mask NetCDF variable name.
  GridPtr gr;                   // Working grid.
  GridData<bool> orig_mask;     // Original mask data.
  GridData<bool> mask;          // Current mask data.
  int grid_changes;             // Changes between original and
                                // current mask.

  GridData<LMass> landmass;     // Land mass index for current mask.
  int nlandmass;                // Number of landmasses.
  std::map<LMass, BBox> lmbbox;
  std::vector<int> lmcounts;    // Land mass box counts.
  std::vector<double> lmsizes;  // Land mass sizes.
  GridData<bool> is_island;     // Are land points part of an island?
  GridData<int> ismask;         // UM ISMASK for current mask.

  // Map from landmass ID to island information.
  std::map<LMass, IslandInfo> isles;
};

#endif
