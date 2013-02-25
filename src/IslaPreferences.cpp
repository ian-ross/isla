//----------------------------------------------------------------------
// FILE:   IslaPreferences.cpp
// DATE:   25-FEB-2013
// AUTHOR: Ian Ross
//
// Preferences information class for Isla island editor.
//----------------------------------------------------------------------

#include <wx/config.h>

#include "IslaPreferences.hh"

IslaPreferences *IslaPreferences::inst = 0;

IslaPreferences *IslaPreferences::get(void) {
  if (!inst) inst = new IslaPreferences();
  return inst;
}

IslaPreferences::IslaPreferences()
{
  cfg = wxConfigBase::Get();
  cfg->SetRecordDefaults();
  wxString tmp;

  if (cfg->Read(_("/Isla/Grid"), &tmp)) {
    if (tmp == _("HadCM3L")) grid = IslaModel::HadCM3L;
    else if (tmp == _("HadCM3")) grid = IslaModel::HadCM3;
    else if (tmp == _("HadGEM2")) grid = IslaModel::HadGEM2;
    else grid = IslaModel::HadCM3L;
  } else grid = IslaModel::HadCM3L;

  double dtmp = 8.0E6;
  if (cfg->Read(_("/Isla/IslandThreshold"), &tmp)) tmp.ToDouble(&dtmp);
  island_threshold = dtmp;

  tmp = _("white");
  cfg->Read(_("/Isla/Colours/Ocean"), &tmp);
  ocean_colour = wxColour(tmp);

  tmp = _("light grey");
  cfg->Read(_("/Isla/Colours/Land"), &tmp);
  land_colour = wxColour(tmp);

  tmp = _("black");
  cfg->Read(_("/Isla/Colours/Island"), &tmp);
  island_colour = wxColour(tmp);

  tmp = _("light grey");
  cfg->Read(_("/Isla/Colours/Grid"), &tmp);
  grid_colour = wxColour(tmp);

  tmp = _("orange");
  cfg->Read(_("/Isla/Colours/IslandOutline"), &tmp);
  outline_colour = wxColour(tmp);

  tmp = _("steel blue");
  cfg->Read(_("/Isla/Colours/ComparisonOutline"), &tmp);
  comp_outline_colour = wxColour(tmp);
}

void IslaPreferences::reset(void)
{
  setGrid(IslaModel::HadCM3L);
  setIslandThreshold(8.0E6);
  setOceanColour(wxColour(_("white")));
  setLandColour(wxColour(_("light grey")));
  setIslandColour(wxColour(_("black")));
  setGridColour(wxColour(_("light grey")));
  setIslandOutlineColour(wxColour(_("orange")));
  setCompOutlineColour(wxColour(_("steel blue")));
}

bool IslaPreferences::setGrid(IslaModel::GridType  gt)
{
  grid = gt;
  wxString tmp;
  switch (gt) {
  case IslaModel::HadCM3L: tmp = _("HadCM3L"); break;
  case IslaModel::HadCM3:  tmp = _("HadCM3");  break;
  case IslaModel::HadGEM2: tmp = _("HadGEM2"); break;
  }
  return cfg->Write(_("/Isla/Grid"), tmp);
}


bool IslaPreferences::setIslandThreshold(double thr)
{
  island_threshold = thr;
  wxString tmp;
  tmp.Printf(_("%f"), thr);
  return cfg->Write(_("/Isla/IslandThreshold"), tmp);
}

bool IslaPreferences::writeColour(wxString k, wxColour c)
{
  return cfg->Write(_("/Isla/Colours/") + k, c.GetAsString(wxC2S_HTML_SYNTAX));
}

bool IslaPreferences::setOceanColour(wxColour col)
{ ocean_colour = col;  return writeColour(_("Ocean"), col); }

bool IslaPreferences::setLandColour(wxColour col)
{ land_colour = col;  return writeColour(_("Land"), col); }

bool IslaPreferences::setIslandColour(wxColour col)
{ island_colour = col;  return writeColour(_("Island"), col); }

bool IslaPreferences::setGridColour(wxColour col)
{ grid_colour = col;  return writeColour(_("Grid"), col); }

bool IslaPreferences::setIslandOutlineColour(wxColour col)
{ outline_colour = col;  return writeColour(_("IslandOutline"), col); }

bool IslaPreferences::setCompOutlineColour(wxColour col)
{ comp_outline_colour = col;  return writeColour(_("ComparisonOutline"), col); }
