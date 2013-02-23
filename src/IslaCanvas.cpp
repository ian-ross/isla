//----------------------------------------------------------------------
// FILE:   IslaCanvas.cpp
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Canvas class for main map view of Isla island editor.
//----------------------------------------------------------------------

#include <wx/wx.h>
#include <wx/colour.h>

#include <iostream>
using namespace std;

#include "IslaCanvas.hh"
#include "IslaModel.hh"
#include "game.hh"

// Event table
BEGIN_EVENT_TABLE(IslaCanvas, wxWindow)
  EVT_PAINT            (IslaCanvas::OnPaint)
  EVT_SCROLLWIN        (IslaCanvas::OnScroll)
  EVT_SIZE             (IslaCanvas::OnSize)
  EVT_MOTION           (IslaCanvas::OnMouse)
  EVT_LEFT_DOWN        (IslaCanvas::OnMouse)
  EVT_LEFT_UP          (IslaCanvas::OnMouse)
  EVT_LEFT_DCLICK      (IslaCanvas::OnMouse)
  EVT_ERASE_BACKGROUND (IslaCanvas::OnEraseBackground)
END_EVENT_TABLE()

// --------------------------------------------------------------------------
// IslaCanvas
// --------------------------------------------------------------------------

IslaCanvas::IslaCanvas(wxWindow *parent, IslaModel *m) :
  wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
           wxFULL_REPAINT_ON_RESIZE)
#ifdef ISLA_DEBUG
  , sizingOverlay(false)
#endif
{
  // ===> THE BORDER WIDTH SHOULD BE SET DEPENDENDING ON THE FONT USED
  //      FOR THE AXES
  bw = 20;

  // Set up model-dependent values.
  modelReset(m);

  // Model grid.
  GridPtr g = model->grid();

  // Set up initial sizing.  We start with a nominal width, take off
  // the space required for the axis borders, calculate a mean cell
  // width and height from the grid size, then use this to set the
  // initial window size.
  int nomw = 1000;
  int mapw = nomw - 2 * bw;
  int meancellw = mapw / g->nlon();
  double dlon = 360.0 / g->nlon(), dlat = 180.0 / g->nlat();
  double aspect = dlat / dlon;
  int meancellh = aspect * meancellw;
  canw = meancellw * g->nlon() + 2 * bw;
  canh = meancellh * g->nlat() + 2 * bw;
  SetSize(wxDefaultCoord, wxDefaultCoord, canw, canh);

  // Reset view.
  clon = 180.0;
  clat = 0.0;
  double lonscale = (canw - 2 * bw) / 360.0;
  double latscale = (canh - 2 * bw) / 180.0;
  cout << "lonscale=" << lonscale << " latscale=" << latscale << endl;
  scale = lonscale;

  SetCursor(*wxCROSS_CURSOR);

    // // reduce flicker if wxEVT_ERASE_BACKGROUND is not available
    // SetBackgroundColour(*wxWHITE);
}


void IslaCanvas::modelReset(IslaModel *m)
{
  // NEED TO DEAL WITH THE ISSUE OF WHAT HAPPENS IS THE NEW MODEL GRID
  // IS DIFFERENT...
  model = m;
  GridPtr g = model->grid();

  // Find minimum longitude and latitude step used in the model grid
  // (used for determining scales for enabling and disabling zoom
  // in/out).
  for (int i = 0; i < g->nlon(); ++i) {
    double dlon = fabs(fmod(g->lon((i+1)%g->nlon()) - g->lon(i), 360.0));
    minDlon = i == 0 ? dlon : min(minDlon, dlon);
  }
  for (int i = 0; i < g->nlat()-1; ++i) {
    double dlat = fabs(g->lat(i+1) - g->lat(i));
    minDlat = i == 0 ? dlat : min(minDlat, dlat);
  }

  // Determine inter-grid cell latitudes and longitudes (used for
  // bounds of grid cells and for drawing grid lines).
  iclons.resize(g->nlon());
  int n = g->nlon();
  for (int i = 0; i < n; ++i)
    iclons[(i+1) % n] =
      g->lon(i) + fmod(360.0 + g->lon((i+1) % n) - g->lon(i), 360.0) / 2;
  iclats.resize(g->nlat() + 1);
  n = g->nlat();
  for (int i = 0; i < n-1; ++i)
    iclats[i+1] = (g->lat(i) + g->lat(i+1)) / 2;
  iclats[0] = g->lat(0) - (iclats[2] - iclats[1]) / 2;
  iclats[n] = g->lat(n-1) + (iclats[n-1] - iclats[n-2]) / 2;
}

int IslaCanvas::GetMinCellSize(void) const
{


}


// recenter at the given position
void IslaCanvas::Recenter(wxInt32 i, wxInt32 j)
{
    // _viewportX = i - _viewportW / 2;
    // _viewportY = j - _viewportH / 2;

    // // redraw everything
    // Refresh(false);
}

// set the cell size and refresh display
// void IslaCanvas::SetCellSize(int cellsize)
// {
    // _cellsize = cellsize;

    // // find current center
    // wxInt32 cx = _viewportX + _viewportW / 2;
    // wxInt32 cy = _viewportY + _viewportH / 2;

    // // get current canvas size and adjust viewport accordingly
    // int w, h;
    // GetClientSize(&w, &h);
    // _viewportW = (w + _cellsize - 1) / _cellsize;
    // _viewportH = (h + _cellsize - 1) / _cellsize;

    // // recenter
    // _viewportX = cx - _viewportW / 2;
    // _viewportY = cy - _viewportH / 2;

    // // adjust scrollbars
    // SetScrollbar(wxHORIZONTAL, _viewportW, _viewportW, 3 * _viewportW);
    // SetScrollbar(wxVERTICAL,   _viewportH, _viewportH, 3 * _viewportH);
    // _thumbX = _viewportW;
    // _thumbY = _viewportH;

    // Refresh(false);
//}

// draw a cell
void IslaCanvas::DrawCell(wxInt32 i, wxInt32 j, bool alive)
{
    // wxClientDC dc(this);

    // dc.SetPen(alive? *wxBLACK_PEN : *wxWHITE_PEN);
    // dc.SetBrush(alive? *wxBLACK_BRUSH : *wxWHITE_BRUSH);

    // DrawCell(i, j, dc);
}

void IslaCanvas::DrawCell(wxInt32 i, wxInt32 j, wxDC &dc)
{
    // wxCoord x = CellToX(i);
    // wxCoord y = CellToY(j);

    // // if cellsize is 1 or 2, there will be no grid
    // switch (_cellsize)
    // {
    //     case 1:
    //         dc.DrawPoint(x, y);
    //         break;
    //     case 2:
    //         dc.DrawRectangle(x, y, 2, 2);
    //         break;
    //     default:
    //         dc.DrawRectangle(x + 1, y + 1, _cellsize - 1, _cellsize - 1);
    // }
}

void IslaCanvas::DrawAll()
{
  // wxClientDC dc(this);

  //   size_t ncells;
  //   IslaCell *cells;
  //   bool done = false;

  //   // _isla->BeginFind(_viewportX,
  //   //                   _viewportY,
  //   //                   _viewportX + _viewportW,
  //   //                   _viewportY + _viewportH,
  //   //                   true);

  //   if (_cellsize == 1)
  //   {
  //       dc.SetPen(*wxBLACK_PEN);
  //   }
  //   else
  //   {
  //       dc.SetPen(*wxTRANSPARENT_PEN);
  //       dc.SetBrush(*wxBLACK_BRUSH);
  //   }
  //   dc.SetLogicalFunction(wxINVERT);

  //   while (!done)
  //   {
  //       // done = _isla->FindMore(&cells, &ncells);

  //       for (size_t m = 0; m < ncells; m++)
  //           DrawCell(cells[m].i, cells[m].j, dc);
  //   }
}

// event handlers
void IslaCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
  // THIS WILL NEED TO BE OPTIMISED.  AT THE MOMENT, IT DOES THE
  // DUMBEST POSSIBLE THING...

  GridPtr g = model->grid();
  wxPaintDC dc(this);

  // Clear grid cell and axis areas.
  dc.SetBrush(*wxWHITE_BRUSH);
  dc.DrawRectangle(bw, 0, canw - 2 * bw, canh);
  dc.DrawRectangle(0, bw, canw, canh - 2 * bw);

  // Set clip region for grid cells and grid.
  dc.SetClippingRegion(bw, bw, canw - 2 * bw, canh - 2 * bw);

  // Fill grid cells.


  // Draw grid.
  dc.SetPen(*wxGREY_PEN);
  for (int i = 0; i < g->nlon(); ++i) {
    int x = lonToX(iclons[i]);
    if (x >= 0 && x <= canw) dc.DrawLine(x, 0, x, canh);
  }
  for (int i = 0; i < g->nlat() + 1; ++i) {
    int y = latToY(iclats[i]);
    if (y >= 0 && y <= canh) dc.DrawLine(0, y, canw, y);
  }

  // Draw axis labels.
  dc.DestroyClippingRegion();


  // Draw borders.
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(*wxBLACK_PEN);
  dc.DrawRectangle(bw, 0, canw - 2 * bw, canh);
  dc.DrawRectangle(0, bw, canw, canh - 2 * bw);

#ifdef ISLA_DEBUG
  // Debug overlays.
  if (sizingOverlay) {
    dc.SetFont(*wxSWISS_FONT);
    wxCoord tw, th;
    dc.GetTextExtent(_("X"), &tw, &th);
    dc.SetTextForeground(*wxRED);
    int x = 50, y = 30, l = 0;
    wxString txt;
    txt.Printf(_("nlon=%d nlat=%d"), g->nlon(), g->nlat());
    dc.DrawText(txt, x, y + th * l++);
    txt.Printf(_("minDlon=%.2f minDlat=%.2f"), minDlon, minDlat);
    dc.DrawText(txt, x, y + th * l++);
    txt.Printf(_("w x h = %d x %d"), canw, canh);
    dc.DrawText(txt, x, y + th * l++);
  }
#endif


    // wxRect  rect = GetUpdateRegion().GetBox();
    // wxCoord x, y, w, h;
    // wxInt32 i0, j0, i1, j1;

    // // find damaged area
    // x = rect.GetX();
    // y = rect.GetY();
    // w = rect.GetWidth();
    // h = rect.GetHeight();

    // i0 = XToCell(x);
    // j0 = YToCell(y);
    // i1 = XToCell(x + w - 1);
    // j1 = YToCell(y + h - 1);

    // size_t ncells;
    // IslaCell *cells;

    // // _isla->BeginFind(i0, j0, i1, j1, false);
    // // bool done = _isla->FindMore(&cells, &ncells);
    // bool done;

    // // erase all damaged cells and draw the grid
    // dc.SetBrush(*wxWHITE_BRUSH);

    // if (_cellsize <= 2)
    // {
    //    // no grid
    //    dc.SetPen(*wxWHITE_PEN);
    //    dc.DrawRectangle(x, y, w, h);
    // }
    // else
    // {
    //     x = CellToX(i0);
    //     y = CellToY(j0);
    //     w = CellToX(i1 + 1) - x + 1;
    //     h = CellToY(j1 + 1) - y + 1;

    //     dc.SetPen(*wxLIGHT_GREY_PEN);
    //     for (wxInt32 yy = y; yy <= (y + h - _cellsize); yy += _cellsize)
    //         dc.DrawRectangle(x, yy, w, _cellsize + 1);
    //     for (wxInt32 xx = x; xx <= (x + w - _cellsize); xx += _cellsize)
    //         dc.DrawLine(xx, y, xx, y + h);
    // }

    // // draw all alive cells
    // dc.SetPen(*wxBLACK_PEN);
    // dc.SetBrush(*wxBLACK_BRUSH);

    // while (!done)
    // {
    //     for (size_t m = 0; m < ncells; m++)
    //         DrawCell(cells[m].i, cells[m].j, dc);

    //     // done = _isla->FindMore(&cells, &ncells);
    // }

    // // last set
    // for (size_t m = 0; m < ncells; m++)
    //     DrawCell(cells[m].i, cells[m].j, dc);
}

void IslaCanvas::OnMouse(wxMouseEvent& event)
{
//     // which cell are we pointing at?
//     wxInt32 i = XToCell( event.GetX() );
//     wxInt32 j = YToCell( event.GetY() );

// #if wxUSE_STATUSBAR
//     // // set statusbar text
//     // wxString msg;
//     // msg.Printf(_("Cell: (%d, %d)"), i, j);
//     // ((IslaFrame *) wxGetApp().GetTopWindow())->SetStatusText(msg, 1);
// #endif // wxUSE_STATUSBAR

//     // NOTE that wxMouseEvent::LeftDown() and wxMouseEvent::LeftIsDown()
//     // have different semantics. The first one is used to signal that the
//     // button was just pressed (i.e., in "button down" events); the second
//     // one just describes the current status of the button, independently
//     // of the mouse event type. LeftIsDown is typically used in "mouse
//     // move" events, to test if the button is _still_ pressed.

//     // is the button down?
//     if (!event.LeftIsDown())
//     {
//         _status = MOUSE_NOACTION;
//         return;
//     }

//     // was it pressed just now?
//     if (event.LeftDown())
//     {
//         // yes: start a new action and toggle this cell
//         // _status = (_isla->IsAlive(i, j)? MOUSE_ERASING : MOUSE_DRAWING);

//         _mi = i;
//         _mj = j;
//         // _isla->SetCell(i, j, _status == MOUSE_DRAWING);
//         DrawCell(i, j, _status == MOUSE_DRAWING);
//     }
//     else if ((_mi != i) || (_mj != j))
//     {
//         // no: continue ongoing action
//         bool alive = (_status == MOUSE_DRAWING);

//         // prepare DC and pen + brush to optimize drawing
//         wxClientDC dc(this);
//         dc.SetPen(alive? *wxBLACK_PEN : *wxWHITE_PEN);
//         dc.SetBrush(alive? *wxBLACK_BRUSH : *wxWHITE_BRUSH);

//         // draw a line of cells using Bresenham's algorithm
//         wxInt32 d, ii, jj, di, ai, si, dj, aj, sj;
//         di = i - _mi;
//         ai = abs(di) << 1;
//         si = (di < 0)? -1 : 1;
//         dj = j - _mj;
//         aj = abs(dj) << 1;
//         sj = (dj < 0)? -1 : 1;

//         ii = _mi;
//         jj = _mj;

//         if (ai > aj)
//         {
//             // iterate over i
//             d = aj - (ai >> 1);

//             while (ii != i)
//             {
//                 // _isla->SetCell(ii, jj, alive);
//                 DrawCell(ii, jj, dc);
//                 if (d >= 0)
//                 {
//                     jj += sj;
//                     d  -= ai;
//                 }
//                 ii += si;
//                 d  += aj;
//             }
//         }
//         else
//         {
//             // iterate over j
//             d = ai - (aj >> 1);

//             while (jj != j)
//             {
//                 // _isla->SetCell(ii, jj, alive);
//                 DrawCell(ii, jj, dc);
//                 if (d >= 0)
//                 {
//                     ii += si;
//                     d  -= aj;
//                 }
//                 jj += sj;
//                 d  += ai;
//             }
//         }

//         // last cell
//         // _isla->SetCell(ii, jj, alive);
//         DrawCell(ii, jj, dc);
//         _mi = ii;
//         _mj = jj;
//     }
}

void IslaCanvas::OnSize(wxSizeEvent &event)
{
  wxSize sz = event.GetSize();
  canw = sz.GetWidth();
  canh = sz.GetHeight();
  Refresh();

    // // find center
    // wxInt32 cx = _viewportX + _viewportW / 2;
    // wxInt32 cy = _viewportY + _viewportH / 2;

    // // get new size
    // wxCoord w = event.GetSize().GetX();
    // wxCoord h = event.GetSize().GetY();
    // _viewportW = (w + _cellsize - 1) / _cellsize;
    // _viewportH = (h + _cellsize - 1) / _cellsize;

    // // recenter
    // _viewportX = cx - _viewportW / 2;
    // _viewportY = cy - _viewportH / 2;

    // // scrollbars
    // SetScrollbar(wxHORIZONTAL, _viewportW, _viewportW, 3 * _viewportW);
    // SetScrollbar(wxVERTICAL,   _viewportH, _viewportH, 3 * _viewportH);
    // _thumbX = _viewportW;
    // _thumbY = _viewportH;

    // // allow default processing
    // event.Skip();
}

void IslaCanvas::OnScroll(wxScrollWinEvent& event)
{
//     WXTYPE type = (WXTYPE)event.GetEventType();
//     int pos     = event.GetPosition();
//     int orient  = event.GetOrientation();

//     // calculate scroll increment
//     int scrollinc = 0;
//     if (type == wxEVT_SCROLLWIN_TOP)
//     {
//         if (orient == wxHORIZONTAL)
//             scrollinc = -_viewportW;
//         else
//             scrollinc = -_viewportH;
//     }
//     else
//     if (type == wxEVT_SCROLLWIN_BOTTOM)
//     {
//         if (orient == wxHORIZONTAL)
//             scrollinc = _viewportW;
//         else
//             scrollinc = _viewportH;
//     }
//     else
//     if (type == wxEVT_SCROLLWIN_LINEUP)
//     {
//         scrollinc = -1;
//     }
//     else
//     if (type == wxEVT_SCROLLWIN_LINEDOWN)
//     {
//         scrollinc = +1;
//     }
//     else
//     if (type == wxEVT_SCROLLWIN_PAGEUP)
//     {
//         scrollinc = -10;
//     }
//     else
//     if (type == wxEVT_SCROLLWIN_PAGEDOWN)
//     {
//         scrollinc = +10;
//     }
//     else
//     if (type == wxEVT_SCROLLWIN_THUMBTRACK)
//     {
//         if (orient == wxHORIZONTAL)
//         {
//             scrollinc = pos - _thumbX;
//             _thumbX = pos;
//         }
//         else
//         {
//             scrollinc = pos - _thumbY;
//             _thumbY = pos;
//         }
//     }
//     else
//     if (type == wxEVT_SCROLLWIN_THUMBRELEASE)
//     {
//         _thumbX = _viewportW;
//         _thumbY = _viewportH;
//     }

// #if defined(__WXGTK__) || defined(__WXMOTIF__)
//     // wxGTK and wxMotif update the thumb automatically (wxMSW doesn't);
//     // so reset it back as we always want it to be in the same position.
//     if (type != wxEVT_SCROLLWIN_THUMBTRACK)
//     {
//         SetScrollbar(wxHORIZONTAL, _viewportW, _viewportW, 3 * _viewportW);
//         SetScrollbar(wxVERTICAL,   _viewportH, _viewportH, 3 * _viewportH);
//     }
// #endif

//     if (scrollinc == 0) return;

//     // scroll the window and adjust the viewport
//     if (orient == wxHORIZONTAL)
//     {
//         _viewportX += scrollinc;
//         ScrollWindow( -_cellsize * scrollinc, 0, (const wxRect *) NULL);
//     }
//     else
//     {
//         _viewportY += scrollinc;
//         ScrollWindow( 0, -_cellsize * scrollinc, (const wxRect *) NULL);
//     }
}

void IslaCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    // do nothing. I just don't want the background to be erased, you know.
}
