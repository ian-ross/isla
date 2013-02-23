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
#include "wx/wfstream.h"

#include "IslaFrame.hh"
#include "IslaModel.hh"
#include "IslaCanvas.hh"
#include "Dialogues.hh"
#include "ids.hh"

#include "GridData.hh"
using namespace netCDF;


// Resources

#if defined(__WXGTK__) || defined(__WXX11__)
// Application icon
#include "isla.xpm"

// Bitmap buttons for toolbar
#include "bitmaps/reset.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/play.xpm"
#include "bitmaps/stop.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
#include "bitmaps/info.xpm"
#endif


// Event table
BEGIN_EVENT_TABLE(IslaFrame, wxFrame)
  EVT_MENU  (wxID_NEW,              IslaFrame::OnMenu)
  EVT_MENU  (wxID_OPEN,             IslaFrame::OnOpen)
  EVT_MENU  (wxID_SAVE,             IslaFrame::OnMenu)
  EVT_MENU  (wxID_SAVEAS,           IslaFrame::OnMenu)
  EVT_MENU  (ID_LOAD_MASK,          IslaFrame::OnLoadMask)
  EVT_MENU  (ID_SAVE_MASK,          IslaFrame::OnMenu)
  EVT_MENU  (ID_IMPORT_ISLCMP_DATA, IslaFrame::OnMenu)
  EVT_MENU  (ID_CLEAR_ISLCMP_DATA, IslaFrame::OnMenu)
  EVT_MENU  (ID_EXPORT_ISLAND_DATA, IslaFrame::OnMenu)
  EVT_MENU  (wxID_EXIT,             IslaFrame::OnMenu)

  EVT_MENU  (wxID_ZOOM_IN,          IslaFrame::OnZoom)
  EVT_MENU  (wxID_ZOOM_OUT,         IslaFrame::OnZoom)
  EVT_MENU  (wxID_ZOOM_FIT,         IslaFrame::OnZoom)
  EVT_MENU  (ID_ZOOM_SELECTION,     IslaFrame::OnZoom)
  EVT_MENU  (ID_PAN,                IslaFrame::OnZoom)

  EVT_MENU  (ID_SELECT,             IslaFrame::OnMenu)
  EVT_MENU  (ID_EDIT_LANDSEA_MASK,  IslaFrame::OnMenu)

  EVT_MENU  (wxID_ABOUT,            IslaFrame::OnMenu)
  EVT_CLOSE (                       IslaFrame::OnClose)
END_EVENT_TABLE()


#define ADD_TOOL(id, bmp, tooltip, help) \
    toolBar->AddTool(id, bmp, wxNullBitmap, false, \
                     wxDefaultCoord, wxDefaultCoord, \
                     (wxObject *)NULL, tooltip, help)


// --------------------------------------------------------------------------
//
//  IslaFrame
//

IslaFrame::IslaFrame() :
  wxFrame((wxFrame *)NULL, wxID_ANY, _("Isla"), wxDefaultPosition )
{
  SetIcon(wxICON(isla));

  wxMenu *menuFile = new wxMenu(wxMENU_TEAROFF);
  wxMenu *menuView = new wxMenu(wxMENU_TEAROFF);
  wxMenu *menuTools = new wxMenu(wxMENU_TEAROFF);
  wxMenu *menuHelp = new wxMenu(wxMENU_TEAROFF);

  menuFile->Append(wxID_NEW, _("&New project"),
                   _("Start a new Isla project"));
  menuFile->Append(wxID_OPEN, _("&Open project..."),
                   _("Open an existing Isla project"));
  menuFile->Append(wxID_SAVE, _("&Save project"),
                   _("Save current project"));
  menuFile->Append(wxID_SAVEAS, _("Save project &as..."),
                   _("Save current project under a different name"));
  menuFile->AppendSeparator();
  menuFile->Append(ID_LOAD_MASK, _("&Load land/sea mask..."),
                   _("Load land/sea mask data from a NetCDF file"));
  menuFile->Append(ID_SAVE_MASK, _("Sa&ve land/sea mask..."),
                   _("Save current land/sea mask data to a NetCDF file"));
  menuFile->AppendSeparator();
  menuFile->Append(ID_IMPORT_ISLCMP_DATA,
                   _("&Import island comparison data..."),
                   _("Import island data file for comparison purposes"));
  menuFile->Append(ID_CLEAR_ISLCMP_DATA, _("&Clear island comparison data"),
                   _("Clear island data file used for comparison purposes"));
  menuFile->Append(ID_EXPORT_ISLAND_DATA, _("&Export island data..."),
                   _("Write new island data file"));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  menuView->Append(wxID_ZOOM_IN, wxEmptyString, _("Zoom in"));
  menuView->Append(wxID_ZOOM_OUT, wxEmptyString, _("Zoom out"));
  menuView->Append(wxID_ZOOM_FIT, wxEmptyString, _("Zoom to fit"));
  menuView->Append(ID_ZOOM_SELECTION, _("Zoom to &selection"),
                   _("Zoom to selection"));
  menuView->Append(ID_PAN, _("&Pan"), _("Pan view"));

  menuTools->Append(ID_SELECT, _("&Select\tCtrl-S"),
                    _("Select items"));
  menuTools->Append(ID_EDIT_LANDSEA_MASK, _("&Edit land/sea mask\tCtrl-E"),
                    _("Edit land/sea mask by toggling state of cells"));

  menuHelp->Append(wxID_ABOUT, _("&About\tCtrl-A"), _("Show about dialogue"));

  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Append(menuFile, _("&File"));
  menuBar->Append(menuView, _("&View"));
  menuBar->Append(menuTools, _("&Tools"));
  menuBar->Append(menuHelp, _("&Help"));
  SetMenuBar(menuBar);

  // tool bar
  wxBitmap tbBitmaps[7];

  tbBitmaps[0] = wxBITMAP(reset);
  tbBitmaps[1] = wxBITMAP(open);
  tbBitmaps[2] = wxBITMAP(zoomin);
  tbBitmaps[3] = wxBITMAP(zoomout);
  tbBitmaps[4] = wxBITMAP(info);
  tbBitmaps[5] = wxBITMAP(play);
  tbBitmaps[6] = wxBITMAP(stop);

  wxToolBar *toolBar = CreateToolBar();
  toolBar->SetMargins(5, 5);
  toolBar->SetToolBitmapSize(wxSize(16, 16));

  ADD_TOOL(wxID_NEW, tbBitmaps[0],
           wxGetStockLabel(wxID_NEW, wxSTOCK_NOFLAGS),
           _("Start a new game"));
  ADD_TOOL(wxID_OPEN, tbBitmaps[1],
           wxGetStockLabel(wxID_OPEN, wxSTOCK_NOFLAGS),
           _("Open an existing Isla pattern"));
  toolBar->AddSeparator();
  ADD_TOOL(wxID_ZOOM_IN, tbBitmaps[2],
           wxGetStockLabel(wxID_ZOOM_IN, wxSTOCK_NOFLAGS), _("Zoom in"));
  ADD_TOOL(wxID_ZOOM_OUT, tbBitmaps[3],
           wxGetStockLabel(wxID_ZOOM_OUT, wxSTOCK_NOFLAGS), _("Zoom out"));
  toolBar->Realize();
  toolBar->EnableTool(wxID_ZOOM_IN, false);
  toolBar->EnableTool(wxID_ZOOM_OUT, false);

  CreateStatusBar(2);
  SetStatusText(_("Welcome to Isla!"));

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
}

// Enable or disable tools and menu entries according to the current
// state. See also wxEVT_UPDATE_UI events for a slightly different
// way to do this.
void IslaFrame::UpdateUI()
{
  // zooming
  int cellsize = canvas->GetCellSize();
  GetToolBar()->EnableTool(wxID_ZOOM_IN,  cellsize < 32);
  GetToolBar()->EnableTool(wxID_ZOOM_OUT, cellsize > 1);
  GetMenuBar()->Enable(wxID_ZOOM_IN,  cellsize < 32);
  GetMenuBar()->Enable(wxID_ZOOM_OUT, cellsize > 1);
}

// Event handlers -----------------------------------------------------------

// OnMenu handles all events which don't have their own event handler
void IslaFrame::OnMenu(wxCommandEvent &e)
{
  switch (e.GetId()) {
  case wxID_NEW: {
    // stop if it was running
    model->reset();
    canvas->Recenter(0, 0);
    break;
  }
  case wxID_ABOUT: {
    IslaAboutDialog dialog(this);
    dialog.ShowModal();
    break;
  }
  case wxID_EXIT:
    Close(true);
    break;
  }
}

void IslaFrame::OnOpen(wxCommandEvent &WXUNUSED(e))
{
  wxFileDialog filedlg(this,
                       _("Choose a file to open"),
                       wxEmptyString,
                       wxEmptyString,
                       _("Isla patterns (*.lif)|*.lif|All files (*.*)|*.*"),
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (filedlg.ShowModal() == wxID_OK) {
    wxFileInputStream stream(filedlg.GetPath());
  }
}

void IslaFrame::OnLoadMask(wxCommandEvent &WXUNUSED(e))
{
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
    wxMessageDialog msg(this, _("Failed to open NetCDF file"),
                        _("NetCDF error"), wxICON_ERROR);
    msg.ShowModal();
    cout << "EXCEPTION: " << e.what() << endl;
  }
  if (maskvar != "") {
    cout << nc_file << endl;
    cout << maskvar << endl;
    try {
      model->LoadMask(nc_file, maskvar);
    } catch (std::exception &e) {
      cout << "EXCEPTION: " << e.what() << endl;
    }
  }
  delete nc;
  nc = 0;
}

void IslaFrame::OnZoom(wxCommandEvent &e)
{
  int cellsize = canvas->GetCellSize();
  if (e.GetId() == wxID_ZOOM_IN && cellsize < 32) {
    canvas->SetCellSize(cellsize * 2);
    UpdateUI();
  } else if (e.GetId() == wxID_ZOOM_OUT && cellsize > 1) {
    canvas->SetCellSize(cellsize / 2);
    UpdateUI();
  }
}

void IslaFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
    // Stop if it was running; this is absolutely needed because
    // the frame won't be actually destroyed until there are no
    // more pending events, and this in turn won't ever happen
    // if the timer is running faster than the window can redraw.
    Destroy();
}
