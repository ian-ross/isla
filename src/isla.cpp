//----------------------------------------------------------------------
// FILE:   isla.cpp
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Main program for Isla island editor.
//----------------------------------------------------------------------

#include "wx/wx.h"
#include "IslaFrame.hh"
#include "IslaPreferences.hh"

class IslaApp: public wxApp {
public:
  virtual bool OnInit();
};

IMPLEMENT_APP(IslaApp)

bool IslaApp::OnInit()
{
  SetVendorName(_("skybluetrades"));
  SetAppName(_("isla"));
  IslaFrame *frame = new IslaFrame();
  frame->Show(true);
  SetTopWindow(frame);
  return true;
}
