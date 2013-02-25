//----------------------------------------------------------------------
// FILE:   IslaPreferences.hh
// DATE:   25-FEB-2013
// AUTHOR: Ian Ross
//
// Preferences information class for Isla island editor.
//----------------------------------------------------------------------

#ifndef _H_ISLAPREFS_
#define _H_ISLAPREFS_

#include <wx/wx.h>
#include <wx/config.h>

#include "IslaModel.hh"

class IslaPreferences {
public:
  static IslaPreferences *get(void);
  ~IslaPreferences() { delete cfg; }

  void reset(void);

  IslaModel::GridType getGrid(void) const { return grid; }
  double getIslandThreshold(void) const { return island_threshold; }
  wxColour getOceanColour(void) const { return ocean_colour; }
  wxColour getLandColour(void) const { return land_colour; }
  wxColour getIslandColour(void) const { return island_colour; }
  wxColour getGridColour(void) const { return grid_colour; }
  wxColour getIslandOutlineColour(void) const { return outline_colour; }
  wxColour getCompOutlineColour(void) const { return comp_outline_colour; }

  bool setGrid(IslaModel::GridType  gt);
  bool setIslandThreshold(double thr);
  bool setOceanColour(wxColour col);
  bool setLandColour(wxColour col);
  bool setIslandColour(wxColour col);
  bool setGridColour(wxColour col);
  bool setIslandOutlineColour(wxColour col);
  bool setCompOutlineColour(wxColour col);

private:

  // Singleton...
  IslaPreferences();
  IslaPreferences(const IslaPreferences &);
  IslaPreferences &operator=(const IslaPreferences &);
  static IslaPreferences *inst;

  bool writeColour(wxString k, wxColour c);

  IslaModel::GridType grid;
  double island_threshold;
  wxColour ocean_colour;
  wxColour land_colour;
  wxColour island_colour;
  wxColour grid_colour;
  wxColour outline_colour;
  wxColour comp_outline_colour;
  wxConfigBase *cfg;
};

#endif
