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
#include "wx/wxhtml.h"

#include "IslaModel.hh"
#include "IslaCanvas.hh"

class IslaFrame : public wxFrame {
public:
  IslaFrame();
  virtual ~IslaFrame() { canvas->SetFrame(0); }

  void UpdateUI();
  void SetLocation(double lon, double lat, int x, int y);

private:
  DECLARE_EVENT_TABLE()
  void OnMenu(wxCommandEvent &e);
  void OnOpen(wxCommandEvent &e);
  void OnLoadMask(wxCommandEvent &e);
  void OnSaveMask(wxCommandEvent &e);
  void OnExportIslands(wxCommandEvent &e);
  void OnLoadComparison(wxCommandEvent &e);
  void OnClearComparison(wxCommandEvent &e);
  void OnZoom(wxCommandEvent &e);
  void OnPreferences(wxCommandEvent &e);
  void OnExit(wxCommandEvent &e);
  void OnClose(wxCloseEvent &e) { Destroy(); }
  void OnShowChange(wxCommandEvent &e);
#ifdef ISLA_DEBUG
  void OnDebug(wxCommandEvent &e);
#endif

  IslaModel *model;
  IslaCanvas *canvas;
  wxMenu *menuView;
  wxMenu *menuTools;
  wxToolBar *toolBar;
  wxHtmlHelpController *helpCtrl;
};

#endif
