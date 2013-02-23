//----------------------------------------------------------------------
// FILE:   isla.cpp
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Main program for Isla island editor.
//----------------------------------------------------------------------

#include "wx/wx.h"
#include "IslaFrame.hh"

class IslaApp: public wxApp {
public:
  virtual bool OnInit();
};

IMPLEMENT_APP(IslaApp)

bool IslaApp::OnInit()
{
  IslaFrame *frame = new IslaFrame();
  frame->Show(true);
  SetTopWindow(frame);
  return true;
}