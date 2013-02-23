//----------------------------------------------------------------------
// FILE:   IslaFrame.hh
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Main frame class for Isla island editor.
//----------------------------------------------------------------------

#ifndef _H_ISLAFRAME_
#define _H_ISLAFRAME_

#include "wx/wx.h"

#include "IslaModel.hh"
class IslaCanvas;

class IslaFrame : public wxFrame {
public:
  IslaFrame();
  virtual ~IslaFrame() { delete model; }

  void UpdateUI();

private:
  DECLARE_EVENT_TABLE()
  void OnMenu(wxCommandEvent &e);
  void OnOpen(wxCommandEvent &e);
  void OnLoadMask(wxCommandEvent &e);
  void OnZoom(wxCommandEvent &e);
  void OnClose(wxCloseEvent &e);

  IslaModel *model;
  IslaCanvas *canvas;
};

#endif
