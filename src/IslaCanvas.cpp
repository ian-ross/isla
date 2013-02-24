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
  // the space required for the axis borders, then calculate a map
  // aspect ratio based on the padding we need to add to have full
  // grid cells at the top and bottom.  We can then use this to
  // calculate the map height, and add the border widths to get
  // requested canvas dimensions.
  int nomw = 1000;
  mapw = nomw - 2 * bw;
  scale = mapw / 360.0;
  double maplon = 360.0;
  double maplat = 180.0 + g->lat(1) - g->lat(0);
  maph = maplat * scale;
  canw = mapw + 2 * bw;      canh = maph + 2 * bw;
  xoff = (canw - mapw) / 2;  yoff = (canh - maph) / 2;
  SetSize(wxDefaultCoord, wxDefaultCoord, canw, canh);

  // Reset view.
  clon = 180.0;
  clat = 0.0;

  // UI setup.
  mouse = MOUSE_NOTHING;
  SetCursor(*wxCROSS_CURSOR);
  SetBackgroundColour(*wxLIGHT_GREY);
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
  int n = g->nlon();
  iclons.resize(n + 1);
  for (int i = 0; i < n; ++i)
    iclons[i+1] =
      g->lon(i) + fmod(360.0 + g->lon((i+1) % n) - g->lon(i), 360.0) / 2;
  iclons[0] = -fmod(360.0 + g->lon(0) - g->lon(n-1), 360.0) / 2;
  iclats.resize(g->nlat() + 1);
  n = g->nlat();
  for (int i = 0; i < n-1; ++i)
    iclats[i+1] = (g->lat(i) + g->lat(i+1)) / 2;
  iclats[0] = g->lat(0) - (iclats[2] - iclats[1]) / 2;
  iclats[n] = g->lat(n-1) + (iclats[n-1] - iclats[n-2]) / 2;
}

void IslaCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
  // THIS WILL NEED TO BE OPTIMISED.  AT THE MOMENT, IT DOES THE
  // DUMBEST POSSIBLE THING...

  GridPtr g = model->grid();
  int nlon = g->nlon(), nlat = g->nlat();
  wxPaintDC dc(this);
  wxCoord tw1, tw2, th;
  dc.GetTextExtent(_("88"), &tw1, &th);
  dc.GetTextExtent(_("XXXW"), &tw2, &th);

  // Clear grid cell and axis areas.
  dc.SetBrush(*wxWHITE_BRUSH);
  dc.DrawRectangle(xoff, yoff - bw, mapw, maph + 2 * bw);
  dc.DrawRectangle(xoff - bw, yoff, mapw + 2 * bw, maph);

  // Set clip region for grid cells and grid.
  dc.SetClippingRegion(xoff, yoff, mapw, maph);

  // Fill grid cells.
  // LONGITUDE WRAPAROUND ISSUES...
  dc.SetBrush(*wxLIGHT_GREY_BRUSH);
  dc.SetPen(*wxTRANSPARENT_PEN);
  for (int c = 0; c < nlon; ++c)
    for (int r = 0; r < nlat; ++r)
      if (model->maskVal(r, c)) {
        int xl = lonToX(iclons[c]), xr = lonToX(iclons[(c+1)%nlon]);
        int yt = max(0.0, latToY(iclats[r]));
        int yb = min(latToY(iclats[r+1]), canh);
        if (xl <= xr)
          dc.DrawRectangle(xoff + xl, yoff + yt, xr-xl, yb-yt);
        else {
          dc.DrawRectangle(xoff + xl, yoff + yt, mapw-xl, yb-yt);
          dc.DrawRectangle(xoff, yoff + yt, xr, yb-yt);
        }
      }
  dc.SetBrush(*wxTRANSPARENT_BRUSH);

  // Draw grid.
  dc.SetPen(*wxGREY_PEN);
  for (int i = 0; i < nlon + 1; ++i) {
    int x = lonToX(iclons[i]);
    if (x >= 0 && x <= mapw)
      dc.DrawLine(xoff + x, yoff, xoff + x, yoff + maph);
  }
  for (int i = 0; i < nlat + 1; ++i) {
    int y = latToY(iclats[i]);
    if (y >= 0 && y <= maph)
      dc.DrawLine(xoff, yoff + y, xoff + mapw, yoff + y);
  }

  // Top horizontal axis.
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(taxis);
  vector<int> pos(nlon / 4);
  vector<wxString> labs(nlon / 4);
  for (int i = 0; i < nlon / 4; ++i) {
    pos[i] = xoff + lonToX(g->lon((i * 4 - 1 + nlon) % nlon));
    labs[i].Printf(_("%d"), i * 4 == 0 ? nlon : i * 4);
  }
  axisLabels(dc, true, yoff - bw + 2, pos, labs);

  // Bottom horizontal axis.
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(baxis);
  pos.resize(9);
  labs.resize(9);
  for (int i = 0; i < 9; ++i) pos[i] = xoff + lonToX(i * 45.0);
  labs[0] = _("0"); labs[1] = _("45E");  labs[2] = _("90E");
  labs[3] = _("135E"); labs[4] = _("180"); labs[5] = _("135W");
  labs[6] = _("90W"); labs[7] = _("45W"); labs[8] = _("0");
  axisLabels(dc, true, canh - yoff + 2, pos, labs);

  // Left vertical axis.
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(laxis);
  pos.resize(nlat / 4);
  labs.resize(nlat / 4);
  for (int i = 0; i < nlat / 4; ++i) {
    pos[i] = yoff + latToY(g->lat((i + 1) * 4 - 1));
    labs[i].Printf(_("%d"), (i + 1) * 4);
  }
  axisLabels(dc, false, xoff - bw + 2, pos, labs);

  // Right vertical axis.
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(raxis);
  pos.resize(5);
  labs.resize(5);
  for (int i = 0; i < 5; ++i) pos[i] = yoff + latToY(60 - i * 30.0);
  labs[0] = _("60N"); labs[1] = _("30N"); labs[2] = _("0");
  labs[3] = _("30S"); labs[4] = _("60S");
  axisLabels(dc, false, canw - xoff + 2, pos, labs);

  // Draw borders.
  dc.DestroyClippingRegion();
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(*wxBLACK_PEN);
  dc.DrawRectangle(xoff, yoff - bw, mapw, maph + bw * 2);
  dc.DrawRectangle(xoff - bw, yoff, mapw + bw * 2, maph);

#ifdef ISLA_DEBUG
  // Debug overlays.
  if (sizingOverlay) {
    dc.SetFont(*wxSWISS_FONT);
    dc.SetTextForeground(*wxRED);
    int x = 50, y = 30, l = 0;
    wxString txt;
    txt.Printf(_("nlon=%d nlat=%d"), g->nlon(), g->nlat());
    dc.DrawText(txt, x, y + th * l++);
    txt.Printf(_("minDlon=%.2f minDlat=%.2f"), minDlon, minDlat);
    dc.DrawText(txt, x, y + th * l++);
    txt.Printf(_("canw=%d canh=%d"), canw, canh);
    dc.DrawText(txt, x, y + th * l++);
    txt.Printf(_("mapw=%d maph=%d"), mapw, maph);
    dc.DrawText(txt, x, y + th * l++);
  }
#endif
}

void IslaCanvas::axisLabels(wxDC &dc, bool horiz, int yOrX,
                            const vector<int> &xOrYs,
                            const vector<wxString> &labs)
{
  wxCoord tw, th;
  for (int i = 0; i < xOrYs.size(); ++i) {
    dc.GetTextExtent(labs[i], &tw, &th);
    if (horiz) {
      dc.DrawText(labs[i], xOrYs[i] - tw / 2, yOrX);
    } else {
      dc.DrawRotatedText(labs[i], yOrX, xOrYs[i] + tw / 2, 90.0);
    }
  }
}

void IslaCanvas::Pan(int dx, int dy)
{
  GridPtr g = model->grid();
  double maplat = maph / scale;
  double halfh = maph / 2 / scale;
  double clatb = clat;
  clon = fmod(360.0 + clon - dx / scale, 360.0);
  clat += dy / scale;
  clat = max(clat, iclats[0] + halfh);
  clat = min(clat, iclats[iclats.size()-1] - halfh);
  if (fabs(clat - clatb) * scale < 1) clat = clatb;
  Refresh();
}

void IslaCanvas::ZoomScale(double zfac)
{
  scale *= zfac;
  SizeRecalc();
  Refresh();
}

void IslaCanvas::OnMouse(wxMouseEvent& event)
{
  if (!event.LeftIsDown()) { mouse = MOUSE_NOTHING; return; }

  int x = event.GetX(), y = event.GetY();

  if (event.LeftDown()) {
    // Start a new action.
    bool ypanevent = laxis.Contains(x, y) || raxis.Contains(x, y);
    bool xpanevent = taxis.Contains(x, y) || baxis.Contains(x, y);
    if (!xpanevent && !ypanevent) { mouse = MOUSE_NOTHING; return; }
    mousex = x;  mousey = y;
    if (xpanevent) mouse = MOUSE_PAN_X;
    else if (ypanevent) mouse = MOUSE_PAN_Y;
  }

  if (mouse == MOUSE_PAN_X) Pan(x - mousex, 0);
  else Pan(0, y - mousey);
  mousex = x;  mousey = y;
}

void IslaCanvas::SizeRecalc(void)
{
  mapw = min(360.0 * scale, canw - bw * 2);
  xoff = (canw - mapw) / 2;
  GridPtr g = model->grid();
  double maphb = maph;
  maph = min((180.0 + g->lat(1) - g->lat(0)) * scale, canh - bw * 2.0);
  yoff = (canh - maph) / 2;
  double halfh = maph / 2 / scale;
  clat = max(clat, iclats[0] + halfh);
  clat = min(clat, iclats[iclats.size()-1] - halfh);
  taxis = wxRect(xoff, yoff - bw, mapw, bw);
  baxis = wxRect(xoff, canh - yoff, mapw, bw);
  laxis = wxRect(xoff - bw, yoff, bw, maph);
  raxis = wxRect(canw - xoff, yoff, bw, maph);
}

void IslaCanvas::OnSize(wxSizeEvent &event)
{
  wxSize sz = event.GetSize();
  canw = sz.GetWidth();
  canh = sz.GetHeight();
  SizeRecalc();
  Refresh();
}
