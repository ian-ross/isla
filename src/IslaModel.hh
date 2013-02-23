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

#include "GridData.hh"

// Here, "mask" means a boolean land/sea mask (with true for land,
// false for ocean).

class IslaModel {
public:
  // Create a default model: HadCM3 grid, no land.
  IslaModel();
  ~IslaModel() { }

  // Access grid.
  GridPtr grid(void) { return gr; }

  // Reset to original empty mask.
  void reset(void);

  // Load a new mask from a NetCDF file.
  void LoadMask(std::string file, std::string var);

  // Check for changes in grid or islands from the values generated
  // from the originally loaded mask data.  These are used as
  // indicators that there are changes that might need to be saved
  // before certain actions.
  bool hasGridChanges(void) const;
  bool hasIslandChanges(void) const { return false; }

  // Get and set current island size threshold value.  Changing this
  // value may trigger recalculations.
  int islandThreshold(void) const { return island_threshold; }
  void setIslandThreshold(int thresh);

  // Recalculate everything: land masses, ISMASK, islands.
  void recalcAll(void);

  // Index land masses.
  void calcLandMasses(void);

private:
  std::string maskfile;         // Input mask NetCDF file.
  std::string maskvar;          // Input mask NetCDF variable name.
  GridPtr gr;                   // Working grid.
  GridData<bool> orig_mask;     // Original mask data.
  GridData<bool> mask;          // Current mask data.

  int island_threshold;         // Maximum land mass size for an island.
  GridData<int> landmass;       // Land mass index for current mask.
  GridData<int> ismask;         // UM ISMASK for current mask.
};

#endif
