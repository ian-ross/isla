//----------------------------------------------------------------------
// FILE:   Dialogues.hh
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Dialogue classes for Isla island editor.
//----------------------------------------------------------------------

#ifndef _ISLA_DIALOGUES_H_
#define _ISLA_DIALOGUES_H_

#include "IslaModel.hh"
#include "wx/wx.h"
#include "wx/clrpicker.h"

class IslaAboutDialogue: public wxDialog {
public:
  IslaAboutDialogue(wxWindow *parent);
};

class IslaPrefDialogue: public wxDialog {
public:
  IslaPrefDialogue(wxWindow *parent);
  IslaModel::GridType grid(void) const;
  double islandThreshold(void) const;
  wxColour oceanColour(void) const { return colours[0]->GetColour(); }
  wxColour gridColour(void) const { return colours[1]->GetColour(); }
  wxColour landColour(void) const { return colours[2]->GetColour(); }
  wxColour islandOutlineColour(void) const { return colours[3]->GetColour(); }
  wxColour islandColour(void) const { return colours[4]->GetColour(); }
  wxColour compOutlineColour(void) const { return colours[5]->GetColour(); }

private:
  DECLARE_EVENT_TABLE()
  void OnReset(wxCommandEvent &event);

  wxChoice *grid_choice;
  wxTextCtrl *island_threshold;
  wxColourPickerCtrl *colours[6];
};

#endif
