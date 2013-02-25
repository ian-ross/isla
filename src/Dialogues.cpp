//----------------------------------------------------------------------
// FILE:   Dialogues.cpp
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Dialogue classes for Isla island editor.
//----------------------------------------------------------------------

#include <iostream>
using namespace std;

#include <wx/wx.h>
#include <wx/statline.h>
#include <wx/clrpicker.h>

#include "Dialogues.hh"
#include "IslaPreferences.hh"
#include "ids.hh"

#include "bitmaps/isla.xpm"

//----------------------------------------------------------------------
//
//  ABOUT DIALOGUE
//

IslaAboutDialogue::IslaAboutDialogue(wxWindow *p) :
  wxDialog(p, wxID_ANY, _("About Isla"), wxDefaultPosition, wxDefaultSize)
{
  wxStaticBitmap *sbmp = new wxStaticBitmap(this, wxID_ANY, wxBitmap(isla_xpm));
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(sbmp, 0, wxCENTRE | wxALL, 10);
  sizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxGROW | wxLEFT | wxRIGHT, 5);
  sizer->Add(CreateTextSizer(_("Isla version 0.0 for wxWidgets\n\n"
                               "(c) 2013 Ian Ross\n\n"
                               "<ian@skybluetrades.net>")),
             0, wxCENTRE | wxRIGHT|wxLEFT|wxTOP, 20);
  wxSizer *sizerBtns = CreateButtonSizer(wxOK);
  if (sizerBtns) sizer->Add(sizerBtns, wxSizerFlags().Expand().Border());
  SetSizer(sizer);
  sizer->SetSizeHints(this);
  sizer->Fit(this);
  Centre(wxBOTH);
}


//----------------------------------------------------------------------
//
//  PREFERENCES DIALOGUE
//

BEGIN_EVENT_TABLE(IslaPrefDialogue, wxDialog)
  EVT_BUTTON(wxID_RESET, IslaPrefDialogue::OnReset)
END_EVENT_TABLE()

IslaPrefDialogue::IslaPrefDialogue(wxWindow *p) :
  wxDialog(p, wxID_ANY, _("Isla Preferences"), wxDefaultPosition, wxDefaultSize)
{
  wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);

  // General preferences box.
  wxStaticBox *b1 = new wxStaticBox(this, wxID_ANY, _("General preferences"));
  wxBoxSizer *b1sizer = new wxStaticBoxSizer(b1, wxVERTICAL);

  //   Grid choice list.
  wxBoxSizer *b1_item1sizer = new wxBoxSizer(wxHORIZONTAL);
  wxArrayString grid_choices;
  grid_choices.Add(_("HadCM3L"));
  grid_choices.Add(_("HadCM3"));
  grid_choices.Add(_("HadGEM2"));
  grid_choice = new wxChoice(this, ID_PREFS_GRID_CHOICE,
                             wxDefaultPosition, wxDefaultSize, grid_choices);
  b1_item1sizer->Add(new wxStaticText(this, wxID_ANY, _("Grid at startup:")),
                     0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  b1_item1sizer->Add(5, 5, 1, wxALL, 0);
  b1_item1sizer->Add(grid_choice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  b1sizer->Add(b1_item1sizer, 0, wxGROW|wxALL, 5);

  //   Island threshold text entry.
  wxBoxSizer *b1_item2sizer = new wxBoxSizer(wxHORIZONTAL);
  int thr = static_cast<int>(IslaPreferences::get()->getIslandThreshold());
  wxString init_val;
  init_val.Printf(_("%d"), thr);
  int tw, th;
  wxPaintDC dc(this);
  dc.GetTextExtent(_("000000000"), &tw, &th);
  island_threshold = new wxTextCtrl(this, ID_PREFS_ISLAND_THRESHOLD, init_val,
                                    wxDefaultPosition, wxSize(1.1 * tw, -1));
  b1_item2sizer->Add(new wxStaticText(this, wxID_ANY,
                                      _("Island threshold (sq. km):")),
                     0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  b1_item2sizer->Add(5, 5, 1, wxALL, 0);
  b1_item2sizer->Add(island_threshold, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  b1sizer->Add(b1_item2sizer, 0, wxGROW|wxALL, 5);

  top_sizer->Add(b1sizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 5);


  // Colours box.
  wxString labels[] = { _("Ocean"), _("Grid"), _("Land"),
                        _("Island outline"), _("Island"),
                        _("Comparison") };
  wxWindowID ids[] = { ID_PREFS_COL_OCEAN, ID_PREFS_COL_GRID,
                       ID_PREFS_COL_LAND, ID_PREFS_COL_OUTLINE,
                       ID_PREFS_COL_ISLAND, ID_PREFS_COL_COMP };
  wxColour cols[] = { IslaPreferences::get()->getOceanColour(),
                      IslaPreferences::get()->getGridColour(),
                      IslaPreferences::get()->getLandColour(),
                      IslaPreferences::get()->getIslandOutlineColour(),
                      IslaPreferences::get()->getIslandColour(),
                      IslaPreferences::get()->getCompOutlineColour() };
  wxStaticBox *b2 = new wxStaticBox(this, wxID_ANY, _("Colours"));
  wxBoxSizer *b2sizer = new wxStaticBoxSizer(b2, wxVERTICAL);
  wxFlexGridSizer *b2_itemsizer = new wxFlexGridSizer(3, 4, 5, 5);
  for (int i = 0; i < 6; ++i) {
    b2_itemsizer->Add(new wxStaticText(this, wxID_ANY, labels[i]),
                      0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    colours[i] = new wxColourPickerCtrl(this, ids[i], cols[i]);
    b2_itemsizer->Add(colours[i], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  }
  b2sizer->Add(b2_itemsizer, 0, wxGROW|wxALL, 5);
  top_sizer->Add(b2sizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5);


  // Buttons.
  wxSizer *btn_sizer = CreateButtonSizer(wxOK|wxCANCEL);
  btn_sizer->Insert(0, new wxButton(this, wxID_RESET, _("Restore defaults")));
  top_sizer->Add(btn_sizer, wxSizerFlags().Expand().Border());

  SetSizer(top_sizer);
  top_sizer->SetSizeHints(this);
  top_sizer->Fit(this);
  Centre(wxBOTH);
}

IslaModel::GridType IslaPrefDialogue::grid(void) const
{
  switch (grid_choice->GetSelection()) {
  case 0: return IslaModel::HadCM3L;
  case 1: return IslaModel::HadCM3;
  case 2: return IslaModel::HadGEM2;
  }
}

double IslaPrefDialogue::islandThreshold(void) const
{
  double tmp = -1;
  island_threshold->GetValue().ToDouble(&tmp);
  return tmp;
}

void IslaPrefDialogue::OnReset(wxCommandEvent &event)
{
  grid_choice->SetSelection(0);
  island_threshold->SetValue(_("8000000"));
  colours[0]->SetColour(wxColour(_("white")));
  colours[1]->SetColour(wxColour(_("light grey")));
  colours[2]->SetColour(wxColour(_("light grey")));
  colours[3]->SetColour(wxColour(_("orange")));
  colours[4]->SetColour(wxColour(_("black")));
  colours[5]->SetColour(wxColour(_("steel blue")));
}
