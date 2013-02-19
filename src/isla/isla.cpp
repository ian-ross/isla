// FILE: isla.cpp

// HEADER FILES

#include "wx/wx.h"

#include "wx/statline.h"
#include "wx/wfstream.h"
#include "wx/filedlg.h"
#include "wx/stockitem.h"

#include "isla.h"
#include "game.h"
#include "dialogs.h"

#include <iostream>
#include <map>
#include <string>
using namespace std;

#include "GridData.hh"
using namespace netCDF;


// RESOURCES

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

// CONSTANTS

// Non-standard IDs for controls and menu commands.
enum {
  // File menu
  ID_LOAD_LANDSEA_MASK = wxID_HIGHEST,
  ID_SAVE_LANDSEA_MASK,
  ID_IMPORT_ISLAND_DATA,
  ID_EXPORT_ISLAND_DATA,

  // View menu
  ID_ZOOM_SELECTION,
  ID_PAN,

  // Tools menu
  ID_SELECT,
  ID_EDIT_LANDSEA_MASK
};

// EVENT TABLES

// Event tables
BEGIN_EVENT_TABLE(IslaFrame, wxFrame)
  EVT_MENU  (wxID_NEW,              IslaFrame::OnMenu)
  EVT_MENU  (wxID_OPEN,             IslaFrame::OnOpen)
  EVT_MENU  (wxID_SAVE,             IslaFrame::OnMenu)
  EVT_MENU  (wxID_SAVEAS,           IslaFrame::OnMenu)
  EVT_MENU  (ID_LOAD_LANDSEA_MASK,  IslaFrame::OnLoadMask)
  EVT_MENU  (ID_SAVE_LANDSEA_MASK,  IslaFrame::OnMenu)
  EVT_MENU  (ID_IMPORT_ISLAND_DATA, IslaFrame::OnMenu)
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

BEGIN_EVENT_TABLE(IslaCanvas, wxWindow)
  EVT_PAINT           (             IslaCanvas::OnPaint)
  EVT_SCROLLWIN       (             IslaCanvas::OnScroll)
  EVT_SIZE            (             IslaCanvas::OnSize)
  EVT_MOTION          (             IslaCanvas::OnMouse)
  EVT_LEFT_DOWN       (             IslaCanvas::OnMouse)
  EVT_LEFT_UP         (             IslaCanvas::OnMouse)
  EVT_LEFT_DCLICK     (             IslaCanvas::OnMouse)
  EVT_ERASE_BACKGROUND(             IslaCanvas::OnEraseBackground)
END_EVENT_TABLE()


// Create new application object.
IMPLEMENT_APP(IslaApp)


// IMPLEMENTATION

#define ADD_TOOL(id, bmp, tooltip, help) \
    toolBar->AddTool(id, bmp, wxNullBitmap, false, \
                     wxDefaultCoord, wxDefaultCoord, \
                     (wxObject *)NULL, tooltip, help)


// --------------------------------------------------------------------------
//
//  IslaApp
//

bool IslaApp::OnInit()
{
  IslaFrame *frame = new IslaFrame();
  frame->Show(true);
  SetTopWindow(frame);
  return true;
}


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
  menuFile->Append(ID_LOAD_LANDSEA_MASK, _("&Load land/sea mask..."),
                   _("Load land/sea mask data from a NetCDF file"));
  menuFile->Append(ID_SAVE_LANDSEA_MASK, _("Sa&ve land/sea mask..."),
                   _("Save current land/sea mask data to a NetCDF file"));
  menuFile->AppendSeparator();
  menuFile->Append(ID_IMPORT_ISLAND_DATA, _("&Import island data..."),
                   _("Import island data file for comparison purposes"));
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

  // game and timer
  _isla     = new Isla();

  // We use two different panels to reduce flicker in wxGTK, because
  // some widgets (like wxStaticText) don't have their own X11 window,
  // and thus updating the text would result in a refresh of the
  // canvas if they belong to the same parent.

  wxPanel *panel1 = new wxPanel(this, wxID_ANY);
  wxPanel *panel2 = new wxPanel(this, wxID_ANY);

  // canvas
  _canvas = new IslaCanvas(panel1, _isla);

  // component layout
  wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *sizer2 = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *sizer3 = new wxBoxSizer(wxVERTICAL);

  sizer1->Add(new wxStaticLine(panel1, wxID_ANY), 0, wxGROW);
  sizer1->Add(_canvas, 1, wxGROW | wxALL, 2);
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
  int cellsize = _canvas->GetCellSize();
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
    _isla->Clear();
    _canvas->Recenter(0, 0);
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
      _grid = GridPtr(new Grid(*nc));
      _orig_mask = new GridData<bool>(_grid, *nc, maskvar);
    } catch (std::exception &e) {
      cout << "EXCEPTION: " << e.what() << endl;
    }
  }
  delete nc;
  nc = 0;
}

void IslaFrame::OnZoom(wxCommandEvent &e)
{
  int cellsize = _canvas->GetCellSize();
  if (e.GetId() == wxID_ZOOM_IN && cellsize < 32) {
    _canvas->SetCellSize(cellsize * 2);
    UpdateUI();
  } else if (e.GetId() == wxID_ZOOM_OUT && cellsize > 1) {
    _canvas->SetCellSize(cellsize / 2);
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


// --------------------------------------------------------------------------
// IslaCanvas
// --------------------------------------------------------------------------

// canvas constructor
IslaCanvas::IslaCanvas(wxWindow *parent, Isla *life, bool interactive)
          : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(100, 100),
            wxFULL_REPAINT_ON_RESIZE
#if !defined(__SMARTPHONE__) && !defined(__POCKETPC__)
            |wxSUNKEN_BORDER
#else
            |wxSIMPLE_BORDER
#endif
            )
{
    _isla        = life;
    _interactive = interactive;
    _cellsize    = 8;
    _status      = MOUSE_NOACTION;
    _viewportX   = 0;
    _viewportY   = 0;
    _viewportH   = 0;
    _viewportW   = 0;

    if (_interactive)
        SetCursor(*wxCROSS_CURSOR);

    // reduce flicker if wxEVT_ERASE_BACKGROUND is not available
    SetBackgroundColour(*wxWHITE);
}

IslaCanvas::~IslaCanvas()
{
    delete _isla;
}

// recenter at the given position
void IslaCanvas::Recenter(wxInt32 i, wxInt32 j)
{
    _viewportX = i - _viewportW / 2;
    _viewportY = j - _viewportH / 2;

    // redraw everything
    Refresh(false);
}

// set the cell size and refresh display
void IslaCanvas::SetCellSize(int cellsize)
{
    _cellsize = cellsize;

    // find current center
    wxInt32 cx = _viewportX + _viewportW / 2;
    wxInt32 cy = _viewportY + _viewportH / 2;

    // get current canvas size and adjust viewport accordingly
    int w, h;
    GetClientSize(&w, &h);
    _viewportW = (w + _cellsize - 1) / _cellsize;
    _viewportH = (h + _cellsize - 1) / _cellsize;

    // recenter
    _viewportX = cx - _viewportW / 2;
    _viewportY = cy - _viewportH / 2;

    // adjust scrollbars
    if (_interactive)
    {
        SetScrollbar(wxHORIZONTAL, _viewportW, _viewportW, 3 * _viewportW);
        SetScrollbar(wxVERTICAL,   _viewportH, _viewportH, 3 * _viewportH);
        _thumbX = _viewportW;
        _thumbY = _viewportH;
    }

    Refresh(false);
}

// draw a cell
void IslaCanvas::DrawCell(wxInt32 i, wxInt32 j, bool alive)
{
    wxClientDC dc(this);

    dc.SetPen(alive? *wxBLACK_PEN : *wxWHITE_PEN);
    dc.SetBrush(alive? *wxBLACK_BRUSH : *wxWHITE_BRUSH);

    DrawCell(i, j, dc);
}

void IslaCanvas::DrawCell(wxInt32 i, wxInt32 j, wxDC &dc)
{
    wxCoord x = CellToX(i);
    wxCoord y = CellToY(j);

    // if cellsize is 1 or 2, there will be no grid
    switch (_cellsize)
    {
        case 1:
            dc.DrawPoint(x, y);
            break;
        case 2:
            dc.DrawRectangle(x, y, 2, 2);
            break;
        default:
            dc.DrawRectangle(x + 1, y + 1, _cellsize - 1, _cellsize - 1);
    }
}

// draw all changed cells
void IslaCanvas::DrawChanged()
{
    wxClientDC dc(this);

    size_t ncells;
    IslaCell *cells;
    bool done = false;

    _isla->BeginFind(_viewportX,
                      _viewportY,
                      _viewportX + _viewportW,
                      _viewportY + _viewportH,
                      true);

    if (_cellsize == 1)
    {
        dc.SetPen(*wxBLACK_PEN);
    }
    else
    {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(*wxBLACK_BRUSH);
    }
    dc.SetLogicalFunction(wxINVERT);

    while (!done)
    {
        done = _isla->FindMore(&cells, &ncells);

        for (size_t m = 0; m < ncells; m++)
            DrawCell(cells[m].i, cells[m].j, dc);
    }
}

// event handlers
void IslaCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    wxRect  rect = GetUpdateRegion().GetBox();
    wxCoord x, y, w, h;
    wxInt32 i0, j0, i1, j1;

    // find damaged area
    x = rect.GetX();
    y = rect.GetY();
    w = rect.GetWidth();
    h = rect.GetHeight();

    i0 = XToCell(x);
    j0 = YToCell(y);
    i1 = XToCell(x + w - 1);
    j1 = YToCell(y + h - 1);

    size_t ncells;
    IslaCell *cells;

    _isla->BeginFind(i0, j0, i1, j1, false);
    bool done = _isla->FindMore(&cells, &ncells);

    // erase all damaged cells and draw the grid
    dc.SetBrush(*wxWHITE_BRUSH);

    if (_cellsize <= 2)
    {
       // no grid
       dc.SetPen(*wxWHITE_PEN);
       dc.DrawRectangle(x, y, w, h);
    }
    else
    {
        x = CellToX(i0);
        y = CellToY(j0);
        w = CellToX(i1 + 1) - x + 1;
        h = CellToY(j1 + 1) - y + 1;

        dc.SetPen(*wxLIGHT_GREY_PEN);
        for (wxInt32 yy = y; yy <= (y + h - _cellsize); yy += _cellsize)
            dc.DrawRectangle(x, yy, w, _cellsize + 1);
        for (wxInt32 xx = x; xx <= (x + w - _cellsize); xx += _cellsize)
            dc.DrawLine(xx, y, xx, y + h);
    }

    // draw all alive cells
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);

    while (!done)
    {
        for (size_t m = 0; m < ncells; m++)
            DrawCell(cells[m].i, cells[m].j, dc);

        done = _isla->FindMore(&cells, &ncells);
    }

    // last set
    for (size_t m = 0; m < ncells; m++)
        DrawCell(cells[m].i, cells[m].j, dc);
}

void IslaCanvas::OnMouse(wxMouseEvent& event)
{
    if (!_interactive)
        return;

    // which cell are we pointing at?
    wxInt32 i = XToCell( event.GetX() );
    wxInt32 j = YToCell( event.GetY() );

#if wxUSE_STATUSBAR
    // set statusbar text
    wxString msg;
    msg.Printf(_("Cell: (%d, %d)"), i, j);
    ((IslaFrame *) wxGetApp().GetTopWindow())->SetStatusText(msg, 1);
#endif // wxUSE_STATUSBAR

    // NOTE that wxMouseEvent::LeftDown() and wxMouseEvent::LeftIsDown()
    // have different semantics. The first one is used to signal that the
    // button was just pressed (i.e., in "button down" events); the second
    // one just describes the current status of the button, independently
    // of the mouse event type. LeftIsDown is typically used in "mouse
    // move" events, to test if the button is _still_ pressed.

    // is the button down?
    if (!event.LeftIsDown())
    {
        _status = MOUSE_NOACTION;
        return;
    }

    // was it pressed just now?
    if (event.LeftDown())
    {
        // yes: start a new action and toggle this cell
        _status = (_isla->IsAlive(i, j)? MOUSE_ERASING : MOUSE_DRAWING);

        _mi = i;
        _mj = j;
        _isla->SetCell(i, j, _status == MOUSE_DRAWING);
        DrawCell(i, j, _status == MOUSE_DRAWING);
    }
    else if ((_mi != i) || (_mj != j))
    {
        // no: continue ongoing action
        bool alive = (_status == MOUSE_DRAWING);

        // prepare DC and pen + brush to optimize drawing
        wxClientDC dc(this);
        dc.SetPen(alive? *wxBLACK_PEN : *wxWHITE_PEN);
        dc.SetBrush(alive? *wxBLACK_BRUSH : *wxWHITE_BRUSH);

        // draw a line of cells using Bresenham's algorithm
        wxInt32 d, ii, jj, di, ai, si, dj, aj, sj;
        di = i - _mi;
        ai = abs(di) << 1;
        si = (di < 0)? -1 : 1;
        dj = j - _mj;
        aj = abs(dj) << 1;
        sj = (dj < 0)? -1 : 1;

        ii = _mi;
        jj = _mj;

        if (ai > aj)
        {
            // iterate over i
            d = aj - (ai >> 1);

            while (ii != i)
            {
                _isla->SetCell(ii, jj, alive);
                DrawCell(ii, jj, dc);
                if (d >= 0)
                {
                    jj += sj;
                    d  -= ai;
                }
                ii += si;
                d  += aj;
            }
        }
        else
        {
            // iterate over j
            d = ai - (aj >> 1);

            while (jj != j)
            {
                _isla->SetCell(ii, jj, alive);
                DrawCell(ii, jj, dc);
                if (d >= 0)
                {
                    ii += si;
                    d  -= aj;
                }
                jj += sj;
                d  += ai;
            }
        }

        // last cell
        _isla->SetCell(ii, jj, alive);
        DrawCell(ii, jj, dc);
        _mi = ii;
        _mj = jj;
    }
}

void IslaCanvas::OnSize(wxSizeEvent& event)
{
    // find center
    wxInt32 cx = _viewportX + _viewportW / 2;
    wxInt32 cy = _viewportY + _viewportH / 2;

    // get new size
    wxCoord w = event.GetSize().GetX();
    wxCoord h = event.GetSize().GetY();
    _viewportW = (w + _cellsize - 1) / _cellsize;
    _viewportH = (h + _cellsize - 1) / _cellsize;

    // recenter
    _viewportX = cx - _viewportW / 2;
    _viewportY = cy - _viewportH / 2;

    // scrollbars
    if (_interactive)
    {
        SetScrollbar(wxHORIZONTAL, _viewportW, _viewportW, 3 * _viewportW);
        SetScrollbar(wxVERTICAL,   _viewportH, _viewportH, 3 * _viewportH);
        _thumbX = _viewportW;
        _thumbY = _viewportH;
    }

    // allow default processing
    event.Skip();
}

void IslaCanvas::OnScroll(wxScrollWinEvent& event)
{
    WXTYPE type = (WXTYPE)event.GetEventType();
    int pos     = event.GetPosition();
    int orient  = event.GetOrientation();

    // calculate scroll increment
    int scrollinc = 0;
    if (type == wxEVT_SCROLLWIN_TOP)
    {
        if (orient == wxHORIZONTAL)
            scrollinc = -_viewportW;
        else
            scrollinc = -_viewportH;
    }
    else
    if (type == wxEVT_SCROLLWIN_BOTTOM)
    {
        if (orient == wxHORIZONTAL)
            scrollinc = _viewportW;
        else
            scrollinc = _viewportH;
    }
    else
    if (type == wxEVT_SCROLLWIN_LINEUP)
    {
        scrollinc = -1;
    }
    else
    if (type == wxEVT_SCROLLWIN_LINEDOWN)
    {
        scrollinc = +1;
    }
    else
    if (type == wxEVT_SCROLLWIN_PAGEUP)
    {
        scrollinc = -10;
    }
    else
    if (type == wxEVT_SCROLLWIN_PAGEDOWN)
    {
        scrollinc = +10;
    }
    else
    if (type == wxEVT_SCROLLWIN_THUMBTRACK)
    {
        if (orient == wxHORIZONTAL)
        {
            scrollinc = pos - _thumbX;
            _thumbX = pos;
        }
        else
        {
            scrollinc = pos - _thumbY;
            _thumbY = pos;
        }
    }
    else
    if (type == wxEVT_SCROLLWIN_THUMBRELEASE)
    {
        _thumbX = _viewportW;
        _thumbY = _viewportH;
    }

#if defined(__WXGTK__) || defined(__WXMOTIF__)
    // wxGTK and wxMotif update the thumb automatically (wxMSW doesn't);
    // so reset it back as we always want it to be in the same position.
    if (type != wxEVT_SCROLLWIN_THUMBTRACK)
    {
        SetScrollbar(wxHORIZONTAL, _viewportW, _viewportW, 3 * _viewportW);
        SetScrollbar(wxVERTICAL,   _viewportH, _viewportH, 3 * _viewportH);
    }
#endif

    if (scrollinc == 0) return;

    // scroll the window and adjust the viewport
    if (orient == wxHORIZONTAL)
    {
        _viewportX += scrollinc;
        ScrollWindow( -_cellsize * scrollinc, 0, (const wxRect *) NULL);
    }
    else
    {
        _viewportY += scrollinc;
        ScrollWindow( 0, -_cellsize * scrollinc, (const wxRect *) NULL);
    }
}

void IslaCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    // do nothing. I just don't want the background to be erased, you know.
}
