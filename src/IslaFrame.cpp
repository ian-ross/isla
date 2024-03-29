//----------------------------------------------------------------------
// FILE:   IslaFrame.cpp
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Main frame class for Isla island editor.
//----------------------------------------------------------------------

#include <string>
using namespace std;

#include "wx/wx.h"
#include "wx/stockitem.h"
#include "wx/statline.h"
#include "wx/clrpicker.h"
#include "wx/wfstream.h"
#include "wx/stdpaths.h"
#include "wx/fs_arc.h"

#include "IslaFrame.hh"
#include "IslaModel.hh"
#include "IslaCanvas.hh"
#include "IslaPreferences.hh"
#include "Dialogues.hh"
#include "ids.hh"

#include "GridData.hh"
using namespace netCDF;


// Resources

#if defined(__WXGTK__) || defined(__WXX11__)
// Application icon
#include "isla.xpm"

// Bitmap buttons for toolbar
#include "bitmaps/zoom-in.xpm"
#include "bitmaps/zoom-out.xpm"
#include "bitmaps/zoom-fit.xpm"
#include "bitmaps/zoom-select.xpm"
#include "bitmaps/pan.xpm"
#include "bitmaps/select.xpm"
#include "bitmaps/edit.xpm"
#endif


// Event table
BEGIN_EVENT_TABLE(IslaFrame, wxFrame)
  EVT_MENU  (wxID_NEW,              IslaFrame::OnMenu)
  EVT_MENU  (wxID_OPEN,             IslaFrame::OnLoadMask)
  EVT_MENU  (wxID_SAVEAS,           IslaFrame::OnSaveMask)
  EVT_MENU  (ID_EXPORT_ISLAND_DATA, IslaFrame::OnExportIslands)
  EVT_MENU  (ID_IMPORT_ISLCMP_DATA, IslaFrame::OnLoadComparison)
  EVT_MENU  (ID_CLEAR_ISLCMP_DATA,  IslaFrame::OnClearComparison)
  EVT_MENU  (wxID_PREFERENCES,      IslaFrame::OnPreferences)
  EVT_MENU  (wxID_EXIT,             IslaFrame::OnExit)

  EVT_MENU  (wxID_ZOOM_IN,          IslaFrame::OnZoom)
  EVT_MENU  (wxID_ZOOM_OUT,         IslaFrame::OnZoom)
  EVT_MENU  (wxID_ZOOM_FIT,         IslaFrame::OnZoom)
  EVT_MENU  (ID_ZOOM_SELECTION,     IslaFrame::OnZoom)
  EVT_MENU  (ID_PAN,                IslaFrame::OnZoom)
  EVT_MENU  (ID_SHOW_ISLANDS,       IslaFrame::OnShowChange)
  EVT_MENU  (ID_SHOW_COMPARISON,    IslaFrame::OnShowChange)

  EVT_MENU  (ID_SELECT,             IslaFrame::OnMenu)
  EVT_MENU  (ID_EDIT_MASK,          IslaFrame::OnMenu)

  EVT_MENU  (wxID_HELP_CONTENTS,    IslaFrame::OnMenu)
  EVT_MENU  (wxID_ABOUT,            IslaFrame::OnMenu)
  EVT_CLOSE (                       IslaFrame::OnClose)

#ifdef ISLA_DEBUG
  EVT_MENU  (ID_DEBUG_SIZE_OVERLAY,     IslaFrame::OnDebug)
  EVT_MENU  (ID_DEBUG_REGION_OVERLAY,   IslaFrame::OnDebug)
  EVT_MENU  (ID_DEBUG_ISMASK_OVERLAY,   IslaFrame::OnDebug)
  EVT_MENU  (ID_DEBUG_ISISLAND_OVERLAY, IslaFrame::OnDebug)
#endif
END_EVENT_TABLE()


IslaFrame::IslaFrame() :
  wxFrame((wxFrame *)NULL, wxID_ANY, _("Isla"), wxDefaultPosition)
{
  SetIcon(wxICON(isla));

  wxMenu *menuFile = new wxMenu(wxMENU_TEAROFF);
  menuView = new wxMenu(wxMENU_TEAROFF);
#ifdef ISLA_EDIT
  menuTools = new wxMenu(wxMENU_TEAROFF);
#endif
  wxMenu *menuHelp = new wxMenu(wxMENU_TEAROFF);

  menuFile->Append(wxID_NEW, _("&Clear grid"),
                   _("Clear land/sea mask and island data"));
  menuFile->Append(wxID_OPEN, _("&Load land/sea mask..."),
                   _("Load land/sea mask data from a NetCDF file"));
#ifdef ISLA_EDIT
  menuFile->Append(wxID_SAVE, _("Sa&ve land/sea mask..."),
                   _("Save current land/sea mask data to a NetCDF file"));
#endif
  menuFile->Append(ID_EXPORT_ISLAND_DATA, _("&Export island data...\tCtrl-E"),
                   _("Write new island data file"));
  menuFile->AppendSeparator();
  menuFile->Append(ID_IMPORT_ISLCMP_DATA,
                   _("&Import island comparison data...\tCtrl-I"),
                   _("Import island data file for comparison purposes"));
  menuFile->Append(ID_CLEAR_ISLCMP_DATA,
                   _("&Clear island comparison data\tCtrl-C"),
                   _("Clear island data file used for comparison purposes"));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_PREFERENCES);
  menuFile->Append(wxID_EXIT);

  menuView->Append(wxID_ZOOM_IN, _("Zoom in\t+"), _("Zoom in"));
  menuView->Append(wxID_ZOOM_OUT, _("Zoom out\t-"), _("Zoom out"));
  menuView->Append(wxID_ZOOM_FIT, _("Zoom to fit\tF"), _("Zoom to fit"));
  wxMenuItem *zoomsel_item =
    new wxMenuItem(menuView, ID_ZOOM_SELECTION, _("Zoom to &selection\tZ"),
                   _("Zoom to selection"));
  zoomsel_item->SetBitmap(wxBITMAP(zoom_select));
  menuView->Append(zoomsel_item);
  menuView->AppendCheckItem(ID_PAN, _("&Pan\tP"), _("Pan view"));
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_SHOW_ISLANDS, _("Show islands\tI"),
                            _("Display island outlines on map"));
  menuView->AppendCheckItem(ID_SHOW_COMPARISON,
                            _("Show comparison\tC"),
                            _("Display island comparison data on map"));

#ifdef ISLA_EDIT
  menuTools->AppendCheckItem(ID_SELECT, _("&Select"), _("Select items"));
  menuTools->AppendCheckItem(ID_EDIT_MASK, _("&Edit land/sea mask"),
                             _("Edit land/sea mask by "
                               "toggling state of cells"));
#endif

  menuHelp->Append(wxID_HELP_CONTENTS, _("&Contents\tCtrl-H"),
                   _("Show help contents"));
  menuHelp->Append(wxID_ABOUT, _("&About\tCtrl-A"), _("Show about dialogue"));


  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Append(menuFile, _("&File"));
  menuBar->Append(menuView, _("&View"));
#ifdef ISLA_EDIT
  menuBar->Append(menuTools, _("&Tools"));
#endif
  menuBar->Append(menuHelp, _("&Help"));
#ifdef ISLA_DEBUG
  wxMenu *menuDebug = new wxMenu(wxMENU_TEAROFF);
  menuDebug->AppendCheckItem(ID_DEBUG_SIZE_OVERLAY, _("Sizing overlay"),
                             _("Toggle size overlay"));
  menuDebug->AppendCheckItem(ID_DEBUG_REGION_OVERLAY, _("Region overlay"),
                             _("Toggle region overlay"));
  menuDebug->AppendCheckItem(ID_DEBUG_ISMASK_OVERLAY, _("ISMASK overlay"),
                             _("Toggle ISMASK overlay"));
  menuDebug->AppendCheckItem(ID_DEBUG_ISISLAND_OVERLAY,
                             _("\"Is island?\" overlay"),
                             _("Toggle \"is island?\" overlay"));
  menuBar->Append(menuDebug, _("Debug"));
#endif

  SetMenuBar(menuBar);

  // Tool bar.
  toolBar = CreateToolBar();
  toolBar->SetMargins(5, 5);
  toolBar->SetToolBitmapSize(wxSize(16, 16));

  toolBar->AddTool(wxID_ZOOM_IN,
                   wxGetStockLabel(wxID_ZOOM_IN, wxSTOCK_NOFLAGS),
                   wxBITMAP(zoom_in), wxString(_("Zoom in")));
  toolBar->AddTool(wxID_ZOOM_OUT,
                   wxGetStockLabel(wxID_ZOOM_OUT, wxSTOCK_NOFLAGS),
                   wxBITMAP(zoom_out), wxString(_("Zoom out")));
  toolBar->AddTool(wxID_ZOOM_FIT,
                   wxGetStockLabel(wxID_ZOOM_FIT, wxSTOCK_NOFLAGS),
                   wxBITMAP(zoom_fit), wxString(_("Zoom to fit")));
  toolBar->AddTool(ID_ZOOM_SELECTION,
                   _("Zoom to selection"),
                   wxBITMAP(zoom_select), wxString(_("Zoom to selection")));
  toolBar->AddCheckTool(ID_PAN, _("Pan"),
                        wxBITMAP(pan), wxNullBitmap, wxString(_("Pan view")));
#ifdef ISLA_EDIT
  toolBar->AddSeparator();
  toolBar->AddCheckTool(ID_SELECT, _("Select"),
                        wxBITMAP(select), wxNullBitmap,
                        wxString(_("Select items")));
  toolBar->AddCheckTool(ID_EDIT_MASK, _("Edit land/sea mask"),
                        wxBITMAP(edit), wxNullBitmap,
                        wxString(_("Edit land/sea mask")));
#endif
  toolBar->Realize();
  toolBar->EnableTool(wxID_ZOOM_IN, false);
  toolBar->EnableTool(wxID_ZOOM_OUT, false);
#ifdef ISLA_EDIT
  toolBar->ToggleTool(ID_SELECT, true);
  menuView->Check(ID_SELECT, true);
  toolBar->ToggleTool(ID_EDIT_MASK, false);
  menuView->Check(ID_EDIT_MASK, false);
#endif
  menuView->Check(ID_SHOW_ISLANDS, true);
  menuView->Check(ID_SHOW_COMPARISON, true);

  // Create status bar: one field for menu help items, one field for
  // grid cell display.
  CreateStatusBar(6);
  wxStatusBar *sb = GetStatusBar();
  wxPaintDC sdc(sb);
  wxCoord sbw, sbh;
  int widths[6];  widths[0] = -1;
  sdc.GetTextExtent(_(" LAT:88.8N "), &sbw, &sbh);   widths[1] = sbw;
  sdc.GetTextExtent(_(" LON:888.8W "), &sbw, &sbh);  widths[2] = sbw;
  sdc.GetTextExtent(_(" X:888 "), &sbw, &sbh);       widths[3] = sbw;
  sdc.GetTextExtent(_(" Y:888 "), &sbw, &sbh);       widths[4] = sbw;
  sdc.GetTextExtent(_("XXXX"), &sbw, &sbh);            widths[5] = sbw;
  sb->SetStatusWidths(6, widths);
  SetStatusText(_("Welcome to Isla!"), 0);

  // Help controller.
  wxFileSystem::AddHandler(new wxArchiveFSHandler);
  wxString exepath = wxStandardPaths::Get().GetExecutablePath();
  wxFileName helpfile(wxPathOnly(exepath) + wxString(_("/../share/isla")),
                      _("isla.htb"));
  helpCtrl = new wxHtmlHelpController();
  helpCtrl->UseConfig(wxConfigBase::Get());
  if (!helpCtrl->AddBook(helpfile, false))
    wxMessageBox(_("Failed adding book share/isla/isla.htb"));

  // Model.
  model = new IslaModel();

  // We use two different panels to reduce flicker in wxGTK, because
  // some widgets (like wxStaticText) don't have their own X11 window,
  // and thus updating the text would result in a refresh of the
  // canvas if they belong to the same parent.

  wxPanel *panel1 = new wxPanel(this, wxID_ANY);
  wxPanel *panel2 = new wxPanel(this, wxID_ANY);

  // canvas
  canvas = new IslaCanvas(panel1, model);
  canvas->SetFrame(this);

  // component layout
  wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *sizer2 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *sizer3 = new wxBoxSizer(wxVERTICAL);

  sizer1->Add(new wxStaticLine(panel1, wxID_ANY), 0, wxGROW);
  sizer1->Add(canvas, 1, wxGROW | wxALL, 2);
  sizer1->Add(new wxStaticLine(panel1, wxID_ANY), 0, wxGROW);
  panel1->SetSizer(sizer1);
  sizer1->Fit(panel1);

  panel2->SetSizer(sizer2);
  sizer2->Fit(panel2);

  sizer3->Add(panel1, 1, wxGROW);
  sizer3->Add(panel2, 0, wxGROW);
  SetSizer(sizer3);

  sizer3->Fit(this);
  sizer3->SetSizeHints(this);

  UpdateUI();
  canvas->SetFocus();
}

// Enable or disable tools and menu entries according to the current
// state. See also wxEVT_UPDATE_UI events for a slightly different
// way to do this.
void IslaFrame::UpdateUI()
{
  bool outok = canvas->ZoomOutOK(), inok = canvas->ZoomInOK();
  GetToolBar()->EnableTool(wxID_ZOOM_IN, inok);
  GetToolBar()->EnableTool(wxID_ZOOM_OUT, outok);
  GetMenuBar()->Enable(wxID_ZOOM_IN,  inok);
  GetMenuBar()->Enable(wxID_ZOOM_OUT, outok);
  toolBar->ToggleTool(ID_PAN, canvas->Panning());
  menuView->Check(ID_PAN, canvas->Panning());
#ifdef ISLA_EDIT
  toolBar->ToggleTool(ID_EDIT_MASK, canvas->Editing());
  menuTools->Check(ID_EDIT_MASK, canvas->Editing());
  toolBar->ToggleTool(ID_SELECT, !canvas->Editing());
  menuTools->Check(ID_SELECT, !canvas->Editing());
#endif
  menuView->Check(ID_SHOW_ISLANDS, canvas->ShowIslands());
  menuView->Check(ID_SHOW_COMPARISON, canvas->ShowComparison());
}

// Show mouse cursor location in status bar.
void IslaFrame::SetLocation(double lon, double lat, int x, int y)
{
  double tmplon = fmod(lon + 360.0, 360.0);
  double slon = tmplon <= 180.0 ? tmplon : 360.0 - tmplon;
  double slat = fabs(lat);
  wxString latlab = lat >= 0 ? _("N") : _("S");
  wxString lonlab = tmplon <= 180.0 ? _("E") : _("W");
  wxString latstr, lonstr, xstr, ystr;
  latstr.Printf(_("%2.1f"), slat);  lonstr.Printf(_("%3.1f"), slon);
  xstr.Printf(_("%3d"), x);  ystr.Printf(_("%3d"), y);
  wxString s1, s2, s3, s4;
  s1 << _("LAT:") << latstr << latlab;  SetStatusText(s1, 1);
  s2 << _("LON:") << lonstr << lonlab;  SetStatusText(s2, 2);
  s3 << _("X:") << xstr;                SetStatusText(s3, 3);
  s4 << _("Y:") << ystr;                SetStatusText(s4, 4);
}


// Event handlers -----------------------------------------------------------

// OnMenu handles all events which don't have their own event handler
void IslaFrame::OnMenu(wxCommandEvent &e)
{
  switch (e.GetId()) {
  case wxID_NEW: model->reset();  canvas->ModelReset(model);  break;
  case wxID_HELP_CONTENTS: helpCtrl->Display(_("Test HELPFILE"));  break;
  case wxID_ABOUT: { IslaAboutDialogue d(this);  d.ShowModal();  break; }
  case ID_SELECT: canvas->SetSelect();  UpdateUI(); break;
  case ID_EDIT_MASK: canvas->SetEdit(); UpdateUI(); break;
  }
}

// Debug stuff
#ifdef ISLA_DEBUG
void IslaFrame::OnDebug(wxCommandEvent &e)
{
  switch (e.GetId()) {
  case ID_DEBUG_SIZE_OVERLAY:
    canvas->sizingOverlay = !canvas->sizingOverlay;
    canvas->Refresh();
    break;
  case ID_DEBUG_REGION_OVERLAY:
    canvas->regionOverlay = !canvas->regionOverlay;
    canvas->Refresh();
    break;
  case ID_DEBUG_ISMASK_OVERLAY:
    canvas->ismaskOverlay = !canvas->ismaskOverlay;
    canvas->Refresh();
    break;
  case ID_DEBUG_ISISLAND_OVERLAY:
    canvas->isIslandOverlay = !canvas->isIslandOverlay;
    canvas->Refresh();
    break;
  }
}
#endif

void IslaFrame::OnExit(wxCommandEvent &e)
{
  if (model->hasGridChanges() || model->hasIslandChanges()) {
    wxMessageDialog qdlg(this,
                         _("The current mask has unsaved changes.  "
                           "Are you sure you want to quit?"),
                         _("Confirm quit"), wxOK | wxCANCEL);
    if (qdlg.ShowModal() == wxID_CANCEL) return;
  }
  Close(true);
}

void IslaFrame::OnPreferences(wxCommandEvent &e)
{
  IslaPrefDialogue d(this);
  if (d.ShowModal() == wxID_OK) {
    IslaPreferences *p = IslaPreferences::get();
    p->setGrid(d.grid());
    p->setIslandThreshold(d.islandThreshold());
    p->setOceanColour(d.oceanColour());
    p->setLandColour(d.landColour());
    p->setIslandColour(d.islandColour());
    p->setGridColour(d.gridColour());
    p->setIslandOutlineColour(d.islandOutlineColour());
    p->setCompOutlineColour(d.compOutlineColour());
    canvas->Refresh();
  }
}

void IslaFrame::OnLoadMask(wxCommandEvent &WXUNUSED(e))
{
  if (model->hasGridChanges() || model->hasIslandChanges()) {
    wxMessageDialog qdlg(this,
                         _("The current mask has unsaved changes.  "
                           "Are you sure you want to load a new mask?"),
                         _("Confirm load mask"), wxOK | wxCANCEL);
    if (qdlg.ShowModal() == wxID_CANCEL) return;
  }

  wxFileDialog filedlg(this,
                       _("Choose a file to open"),
                       wxEmptyString,
                       wxEmptyString,
                       _("NetCDF files (*.nc)|*.nc|All files (*.*)|*.*"),
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (filedlg.ShowModal() == wxID_CANCEL) return;

  string nc_file(filedlg.GetPath().char_str());
  NcFile *nc = 0;
  string maskvar = "";
  try {
    nc = new NcFile(nc_file, NcFile::read);
    multimap<string, NcVar> vars = nc->getVars();
    wxArrayString choices;
    for (multimap<string, NcVar>::const_iterator it = vars.begin();
         it != vars.end(); ++it)
      if (it->first != "lat" && it->first != "latitude" &&
          it->first != "lon" && it->first != "longitude")
        choices.Add(wxString::From8BitData(it->first.c_str()));
    if (choices.GetCount() > 1) {
      wxSingleChoiceDialog choice(this, _("Select NetCDF variable"),
                                  _("NetCDF mask variable selection"),
                                  choices);
      if (choice.ShowModal() == wxID_OK)
        maskvar = (const char *)(choices[choice.GetSelection()].To8BitData());
    } else
      maskvar = (const char *)(choices[0].To8BitData());
  } catch (std::exception &e) {
    wxString excmsg = wxString::FromAscii(e.what());
    wxMessageDialog msg(this, _("Failed to open NetCDF file\n\n") + excmsg,
                        _("NetCDF error"), wxICON_ERROR);
    msg.ShowModal();
  }
  if (maskvar != "") {
    try {
      model->loadMask(nc_file, maskvar);
      canvas->ModelReset(model);
    } catch (std::exception &e) {
      wxString excmsg = wxString::FromAscii(e.what());
      wxMessageDialog msg(this,
                          _("Failed to read mask data from NetCDF file\n\n") +
                          excmsg, _("NetCDF error"), wxICON_ERROR);
      msg.ShowModal();
    }
  }
  delete nc;
  nc = 0;
}

void IslaFrame::OnSaveMask(wxCommandEvent &WXUNUSED(e))
{
  wxFileDialog filedlg(this,
                       _("Choose a file to save to"),
                       wxEmptyString,
                       wxEmptyString,
                       _("NetCDF files (*.nc)|*.nc|All files (*.*)|*.*"),
                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (filedlg.ShowModal() == wxID_CANCEL) return;

  try {
    model->saveMask(string(filedlg.GetPath().char_str()));
  } catch (std::exception &e) {
    wxString excmsg = wxString::FromAscii(e.what());
    wxMessageDialog msg(this, _("Failed to write NetCDF file\n\n") + excmsg,
                        _("NetCDF error"), wxICON_ERROR);
    msg.ShowModal();
  }
}

void IslaFrame::OnExportIslands(wxCommandEvent &WXUNUSED(e))
{
  wxFileDialog filedlg(this,
                       _("Choose a file to export to"),
                       wxEmptyString,
                       wxEmptyString,
                       _("Island files (*.isl)|*.isl|All files (*.*)|*.*"),
                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (filedlg.ShowModal() == wxID_CANCEL) return;

  try {
    model->saveIslands(filedlg.GetPath());
  } catch (std::exception &e) {
    wxString excmsg = wxString::FromAscii(e.what());
    wxMessageDialog msg(this, _("Failed to write island file\n\n") + excmsg,
                        _("I/O error"), wxICON_ERROR);
    msg.ShowModal();
  }
}

void IslaFrame::OnLoadComparison(wxCommandEvent &WXUNUSED(e))
{
  wxFileDialog filedlg(this,
                       _("Choose a file to load comparison data from "
                         "(either a UM ocean dump file or an ASCII island "
                         "data file)"),
                       wxEmptyString,
                       wxEmptyString,
                       _("Island data files (*.isl;*.dat)|*.isl;*.dat|"
                         "All files (*.*)|*.*"),
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (filedlg.ShowModal() == wxID_CANCEL) return;

  wxString island_file(filedlg.GetPath());
  try {
    canvas->loadComparisonIslands(island_file);
  } catch (std::exception &e) {
    wxString excmsg = wxString::FromAscii(e.what());
    wxMessageDialog msg(this, _("Failed to load island data\n\n") + excmsg,
                        _("File access error"), wxICON_ERROR);
    msg.ShowModal();
  }
}

void IslaFrame::OnClearComparison(wxCommandEvent &WXUNUSED(e))
{
  canvas->clearComparisonIslands();
}

void IslaFrame::OnZoom(wxCommandEvent &e)
{
  switch (e.GetId()) {
  case wxID_ZOOM_IN:  canvas->ZoomIn();               break;
  case wxID_ZOOM_OUT: canvas->ZoomOut();              break;
  case wxID_ZOOM_FIT: canvas->ZoomToFit();            break;
  case ID_ZOOM_SELECTION: canvas->ZoomToSelection();  break;
  case ID_PAN: canvas->SetPanning(!canvas->Panning());
  }
  UpdateUI();
}

void IslaFrame::OnShowChange(wxCommandEvent &e)
{
  switch(e.GetId()) {
  case ID_SHOW_ISLANDS:
    canvas->SetShowIslands(!canvas->ShowIslands());
    break;
  case ID_SHOW_COMPARISON:
    canvas->SetShowComparison(!canvas->ShowComparison());
    break;
  default: return;
  }
  UpdateUI();
  canvas->Refresh();
}

