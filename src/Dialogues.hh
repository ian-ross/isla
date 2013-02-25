//----------------------------------------------------------------------
// FILE:   Dialogues.hh
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Dialogue classes for Isla island editor.
//----------------------------------------------------------------------

#ifndef _ISLA_DIALOGUES_H_
#define _ISLA_DIALOGUES_H_

#include "wx/wx.h"
#include "IslaPreferences.hh"

class IslaAboutDialog: public wxDialog {
public:
  IslaAboutDialog(wxWindow *parent);
};

class IslaPreferencesDialog: public wxDialog {
public:
  IslaPreferencesDialog(wxWindow *parent, IslaPreferences &prefs);
};

#endif
