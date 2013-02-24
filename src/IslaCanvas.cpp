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


// Canvas event table.

BEGIN_EVENT_TABLE(IslaCanvas, wxWindow)
  EVT_PAINT            (IslaCanvas::OnPaint)
  EVT_SIZE             (IslaCanvas::OnSize)
  EVT_MOTION           (IslaCanvas::OnMouse)
  EVT_LEFT_DOWN        (IslaCanvas::OnMouse)
  EVT_LEFT_UP          (IslaCanvas::OnMouse)
  EVT_MOUSEWHEEL       (IslaCanvas::OnMouse)
  EVT_ERASE_BACKGROUND (IslaCanvas::OnEraseBackground)
END_EVENT_TABLE()


// Constructor sets up border sizing, all model-dependent values and
// canvas size parameters.

IslaCanvas::IslaCanvas(wxWindow *parent, IslaModel *m) :
  wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
           wxFULL_REPAINT_ON_RESIZE)
#ifdef ISLA_DEBUG
  , sizingOverlay(false), regionOverlay(false)
#endif
{
  // Calculate border width and border text offset.
  wxPaintDC dc(this);
  wxCoord th;
  dc.GetTextExtent(_("888"), &celllablimit, &th);
  bw = 1.5 * th;
  boff = 0.25 * th;

  // Model grid.
  GridPtr g = m->grid();

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

  // Reset view.
  clon = 180.0;
  clat = 0.0;

  // Set up model-dependent values.
  modelReset(m);

  // UI setup.
  SetSize(wxDefaultCoord, wxDefaultCoord, canw, canh);
  mouse = MOUSE_NOTHING;
  SetCursor(wxCursor(wxCURSOR_ARROW));
  SetBackgroundColour(*wxLIGHT_GREY);
}


// Change of model.

void IslaCanvas::modelReset(IslaModel *m)
{
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

  // Trigger other required canvas recalculations.
  SizeRecalc();
  Refresh();
}


// Main paint callback.  In order, renders: grid cells, grid (if
// visible), axes, borders and any debug overlays.
//
// THIS WILL NEED TO BE OPTIMISED.  AT THE MOMENT, IT DOES THE DUMBEST
// POSSIBLE THING...

void IslaCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
  // Setup: determine minimum region to redraw.
  GridPtr g = model->grid();
  int nlon = g->nlon(), nlat = g->nlat();
  int nhor = min(static_cast<double>(nlon), mapw / (minDlon * scale) + 1) + 1;
  int nver = min(static_cast<double>(nlat), maph / (minDlat * scale) + 2);
  int lon0 = XToLon(0), lat0 = YToLat(maph);
  int ilon0 = 0, ilat0 = 0;
  double loni = g->lon(ilon0), loni1 = g->lon((ilon0 + 1) % nlon);
  while (!(lon0 >= loni && lon0 < loni1 + (loni1 >= loni ? 0 : 360.0))) {
    ilon0 = (ilon0 + 1) % nlon;
    loni = g->lon(ilon0);
    loni1 = g->lon((ilon0 + 1) % nlon);
  }
  while (g->lat(ilat0) < lat0) ++ilat0;
  ilat0 = ilat0 > 0 ? ilat0 - 1 : ilat0;
  wxPaintDC dc(this);

  // Clear grid cell and axis areas.
  dc.SetBrush(*wxWHITE_BRUSH);
  dc.DrawRectangle(xoff, yoff - bw, mapw, maph + 2 * bw);
  dc.DrawRectangle(xoff - bw, yoff, mapw + 2 * bw, maph);

  // Set clip region for grid cells and grid.
  dc.SetClippingRegion(xoff, yoff, mapw, maph);

  // Fill grid cells.
  dc.SetPen(*wxTRANSPARENT_PEN);
  for (int i = 0, c = ilon0; i < nhor; ++i, c = (c + 1) % nlon)
    for (int j = 0, r = ilat0; j < nver && r < nlat; ++j, ++r)
      if (model->maskVal(r, c)) {
        if (model->isIsland(r, c))
          dc.SetBrush(*wxBLACK_BRUSH);
        else
          dc.SetBrush(*wxLIGHT_GREY_BRUSH);
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
  if (MinCellSize() >= 10) {
    dc.SetPen(*wxGREY_PEN);
    for (int i = 0, c = ilon0; i <= nhor; ++i, c = (c + 1) % nlon) {
      int x = lonToX(iclons[c]);
      if (x >= 0 && x <= mapw)
        dc.DrawLine(xoff + x, yoff, xoff + x, yoff + maph);
    }
    for (int i = 0, r = ilat0; i <= nver && r <= nlat; ++i, ++r) {
      int y = latToY(iclats[r]);
      if (y >= 0 && y <= maph)
        dc.DrawLine(xoff, yoff + y, xoff + mapw, yoff + y);
    }
  }

  // Draw axes.
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(taxis);
  axisLabels(dc, true, yoff - bw + boff, taxpos, taxlab);
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(baxis);
  axisLabels(dc, true, canh - yoff + boff, baxpos, baxlab);
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(laxis);
  axisLabels(dc, false, xoff - bw + 1.5 * boff, laxpos, laxlab);
  dc.DestroyClippingRegion();
  dc.SetClippingRegion(raxis);
  axisLabels(dc, false, canw - xoff + 1.5 * boff, raxpos, raxlab);

  // Draw borders.
  dc.DestroyClippingRegion();
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(*wxBLACK_PEN);
  dc.DrawRectangle(xoff, yoff - bw, mapw, maph + bw * 2);
  dc.DrawRectangle(xoff - bw, yoff, mapw + bw * 2, maph);

#ifdef ISLA_DEBUG
  // Debug overlays.
  if (regionOverlay) {
    wxCoord tw, th;
    dc.SetFont(*wxSWISS_FONT);
    dc.GetTextExtent(_("XX"), &tw, &th);
    int cs = MinCellSize();
    wxFont font(*wxSWISS_FONT);
    if (th > 0.9 * cs) {
      font.SetPointSize(font.GetPointSize() * 0.9 * cs / th);
      dc.SetFont(font);
    }
    dc.SetTextForeground(*wxBLUE);
    dc.SetClippingRegion(xoff, yoff, mapw, maph);
    wxString txt;
    GridPtr g = model->grid();
    for (int i = 0, c = ilon0; i < nhor; ++i, c = (c + 1) % nlon) {
      int x = xoff + lonToX(g->lon(c)) - tw / 2;
      for (int j = 0, r = ilat0; j < nver && r < nlat; ++j, ++r) {
        txt.Printf(_("%d"), model->landMass(r, c));
        int y = latToY(g->lat(r));
        dc.DrawText(txt, x, yoff + y - th / 2);
      }
    }
  }
  if (sizingOverlay) {
    wxCoord tw, th;
    dc.SetFont(*wxSWISS_FONT);
    dc.GetTextExtent(_("X"), &tw, &th);
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


// Mouse handler.  Deals with:
//
//  * Dragging in the axis borders: pans view.
//  * Mousewheel events anywhere in the canvas: pan view.
//  * Dragging anywhere in the canvas while the pan tool is active:
//    pans view.
//  * Dragging anywhere in the canvas while "zoom to selection" is
//    active: rubberbands zoom box then triggers zoom when done.

void IslaCanvas::ProcessPan(wxMouseEvent &event, bool xpan, bool ypan)
{
  int x = event.GetX(), y = event.GetY();
  if (event.LeftDown()) {
    // Start a new action.
    mousex = x;  mousey = y;
    if (panning && !xpan && !ypan)
      mouse = MOUSE_PAN_2D;
    else
      mouse = xpan ? MOUSE_PAN_X : MOUSE_PAN_Y;
  } else if (!event.LeftIsDown()) {
    mouse = MOUSE_NOTHING;
    return;
  } else {
    switch(mouse) {
    case MOUSE_NOTHING: return;
    case MOUSE_PAN_X:  Pan(x - mousex, 0);          break;
    case MOUSE_PAN_Y:  Pan(0, y - mousey);          break;
    case MOUSE_PAN_2D: Pan(x - mousex, y - mousey); break;
    }
  }
  mousex = x;  mousey = y;
}

void IslaCanvas::ProcessZoomSelection(wxMouseEvent &event)
{
  if (!event.LeftDown() && mouse == MOUSE_NOTHING) return;
  int x = event.GetX(), y = event.GetY();
  wxPaintDC dc(this);
  wxPen pen(*wxBLACK, 2, wxLONG_DASH);
  dc.SetLogicalFunction(wxINVERT);
  dc.SetPen(pen);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  if (event.LeftDown()) {
    // Start a new action.
    zoom_x0 = x;  zoom_y0 = y;
    mouse = MOUSE_ZOOM_SELECTION;
  } else {
    // Erase rubber band rectangle if this isn't the start of a
    // new drag.
    int l = min(mousex, zoom_x0), t = min(mousey, zoom_y0);
    int w = abs(zoom_x0 - mousex), h = abs(zoom_y0 - mousey);
    dc.DrawRectangle(l, t, w, h);
  }
  if (!event.LeftIsDown()) {
    // Zoom selection complete.
    zoom_selection = false;
    mouse = MOUSE_NOTHING;
    SetCursor(wxCursor(wxCURSOR_ARROW));
    DoZoomToSelection(zoom_x0, zoom_y0, x, y);
  } else {
    // Draw rubber band rectangle.
    int l = min(zoom_x0, x), t = min(zoom_y0, y);
    int w = abs(zoom_x0 - x), h = abs(zoom_y0 - y);
    dc.DrawRectangle(l, t, w, h);
    mousex = x;  mousey = y;
  }
}

void IslaCanvas::ProcessEdit(wxMouseEvent &event)
{
  int x = event.GetX(), y = event.GetY();
  if (x < bw || x > canw - bw || y < bw || y > canh - bw) return;
  if (event.LeftDown()) {
    double edlon = XToLon(x - bw), edlat = YToLat(y - bw);
    edcol = lonToCol(edlon);
    edrow = latToRow(edlat);
    model->SetMask(edrow, edcol, !model->maskVal(edrow, edcol));
    edval = model->maskVal(edrow, edcol);
    mouse = MOUSE_EDIT;
    Refresh();
  } else if (event.LeftIsDown() && mouse == MOUSE_EDIT) {
    double edlon = XToLon(x - bw), edlat = YToLat(y - bw);
    int newedcol = lonToCol(edlon), newedrow = latToRow(edlat);
    if (newedcol != edcol || newedrow != edrow) {
      edcol = newedcol;  edrow = newedrow;
      model->SetMask(edrow, edcol, edval);
      Refresh();
    }
  } else { mouse = MOUSE_NOTHING;  return; }
}

void IslaCanvas::OnMouse(wxMouseEvent &event)
{
  int x = event.GetX(), y = event.GetY();
  bool ypanevent = laxis.Contains(x, y) || raxis.Contains(x, y);
  bool xpanevent = taxis.Contains(x, y) || baxis.Contains(x, y);
  if (event.GetEventType() == wxEVT_MOUSEWHEEL) {
    if (xpanevent) Pan(0.25 * event.GetWheelRotation(), 0);
    else           Pan(0, 0.25 * event.GetWheelRotation());
  } else if (xpanevent || ypanevent || panning)
    ProcessPan(event, xpanevent, ypanevent);
  else if (zoom_selection) ProcessZoomSelection(event);
  else if (edit)           ProcessEdit(event);
}


// Resize handler.  Just hands off to SizeRecalc then triggers a
// refresh.

void IslaCanvas::OnSize(wxSizeEvent &event)
{
  wxSize sz = event.GetSize();
  canw = sz.GetWidth();
  canh = sz.GetHeight();
  SizeRecalc();
  Refresh();
}


// Set up border axis label positions and strings.

void IslaCanvas::SetupAxes(bool dox, bool doy)
{
  GridPtr g = model->grid();
  int nlon = g->nlon(), nlat = g->nlat();
  wxPaintDC dc(this);
  wxString txt;
  int tw, th;

  if (dox) {
    taxis = wxRect(xoff, yoff - bw, mapw, bw);
    baxis = wxRect(xoff, canh - yoff, mapw, bw);
    vector<int> tws(nlon);
    vector<int> poss(nlon);
    for (int i = 0; i < nlon; ++i) {
      txt.Printf(_("%d"), i == 0 ? nlon : i);
      dc.GetTextExtent(txt, &tw, &th);
      tws[i] = tw;
      poss[i] = xoff + lonToX(g->lon((i - 1 + nlon) % nlon));
    }
    int mindpos, maxtw;
    for (int i = 0; i < nlon; ++i) {
      int dpos = fabs(poss[(i+1)%nlon] - poss[i]);
      if (i == 0 || dpos > 0 && dpos < mindpos) mindpos = dpos;
      if (i == 0 || tws[i] > maxtw) maxtw = tws[i];
    }
    int skip = 2;
    while (skip * mindpos < 3 * maxtw) skip += 2;
    taxpos.resize(nlon / skip);
    taxlab.resize(nlon / skip);
    for (int i = 0; i < nlon / skip; ++i) {
      taxpos[i] = xoff + lonToX(g->lon((i * skip - 1 + nlon) % nlon));
      taxlab[i].Printf(_("%d"), i * skip == 0 ? nlon : i * skip);
    }
    baxpos.resize(9);
    baxlab.resize(9);
    for (int i = 0; i < 9; ++i) baxpos[i] = xoff + lonToX(i * 45.0);
    baxlab[0] = _("0"); baxlab[1] = _("45E");  baxlab[2] = _("90E");
    baxlab[3] = _("135E"); baxlab[4] = _("180"); baxlab[5] = _("135W");
    baxlab[6] = _("90W"); baxlab[7] = _("45W"); baxlab[8] = _("0");
  }

  if (doy) {
    laxis = wxRect(xoff - bw, yoff, bw, maph);
    raxis = wxRect(canw - xoff, yoff, bw, maph);
    vector<int> tws(nlat);
    vector<int> poss(nlat);
    for (int i = 0; i < nlat; ++i) {
      txt.Printf(_("%d"), i + 1);
      dc.GetTextExtent(txt, &tw, &th);
      tws[i] = tw;
      poss[i] = xoff + latToY(g->lat(i));
    }
    int mindpos, maxtw;
    for (int i = 0; i < nlat; ++i) {
      int dpos = fabs(poss[i+1] - poss[i]);
      if (i == 0 || dpos > 0 && dpos < mindpos) mindpos = dpos;
      if (i == 0 || tws[i] > maxtw) maxtw = tws[i];
    }
    int skip = 2;
    while (skip * mindpos < 3 * maxtw) skip += 2;
    laxpos.resize(nlat / skip - (nlat % skip == 0 ? 1 : 0));
    laxlab.resize(nlat / skip - (nlat % skip == 0 ? 1 : 0));
    for (int i = skip; i < nlat; i += skip) {
      laxpos[i/skip-1] = yoff + latToY(g->lat(i - 1));
      laxlab[i/skip-1].Printf(_("%d"), i);
    }
    raxpos.resize(5);
    raxlab.resize(5);
    for (int i = 0; i < 5; ++i) raxpos[i] = yoff + latToY(60 - i * 30.0);
    raxlab[0] = _("60N"); raxlab[1] = _("30N"); raxlab[2] = _("0");
    raxlab[3] = _("30S"); raxlab[4] = _("60S");
  }
}


// Render axis labels for one axis.

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


// Find cell coordinates.

int IslaCanvas::lonToCol(double lon)
{
  lon = fmod(360.0 + lon, 360.0);
  int n = model->grid()->nlon();
  if (lon >= iclons[n]) return 0;
  for (int i = 0; i < n; ++i)
    if (lon >= iclons[i] && lon < iclons[i+1])
      return i;
  return -1;
}
int IslaCanvas::latToRow(double lat)
{
  for (int i = 0; i < iclats.size() - 1; ++i)
    if (iclats[i] <= lat && lat < iclats[i + 1]) return i;
  return -1;
}


// Pan handler.

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
  SetupAxes(dx != 0, dy != 0);
  Refresh();
}


// Zoom in or out by given scale factor.

void IslaCanvas::ZoomScale(double zfac)
{
  scale *= zfac;
  if (MinCellSize() < 2) SetMinCellSize(2);
  if (MinCellSize() > 64) SetMinCellSize(64);
  SizeRecalc();
  Refresh();
}


// Zoom to fit map within available canvas area.

void IslaCanvas::ZoomToFit(void)
{
  GridPtr g = model->grid();
  double possmapw = canw - 2 * bw, possmaph = canh - 2 * bw;
  double fitwscale = possmapw / 360.0;
  double fithscale = possmaph / (180.0 + g->lat(1) - g->lat(0));
  scale = min(fitwscale, fithscale);
  SizeRecalc();
  Refresh();
}


// Zoom to a given x,y rectangle (in canvas coordinates).

void IslaCanvas::DoZoomToSelection(int x0, int y0, int x1, int y1)
{
  // Adjust for border offsets and limit longitude to within canvas.
  x0 -= bw;  x0 = max(0.0, min(static_cast<double>(x0), mapw));
  x1 -= bw;  x1 = max(0.0, min(static_cast<double>(x1), mapw));
  y0 -= bw;  y1 -= bw;

  // Convert latitude to model (lat/lon) coordinate and limit to
  // within the acceptable range.
  double lat0 = YToLat(y0), lat1 = YToLat(y1);
  lat0 = min(max(lat0, iclats[0]), iclats[iclats.size()-1]);
  lat1 = min(max(lat1, iclats[0]), iclats[iclats.size()-1]);

  // Calculate new centre point and scale and redisplay.  The
  // longitude calculation is done in canvas coordinates to avoid
  // problems with wraparound, and the latitude calculation is done in
  // model coordinates to take account of the extra half-cell padding
  // at the top and bottom of the map.
  clon = XToLon((x0 + x1) / 2);  clat = (lat0 + lat1) / 2;
  double possmapw = canw - 2 * bw, possmaph = canh - 2 * bw;
  double fitwscale = possmapw / (fabs(x1 - x0) / scale);
  double fithscale = possmaph / fabs(lat1 - lat0);
  scale = min(fitwscale, fithscale);
  if (MinCellSize() < 2) SetMinCellSize(2);
  if (MinCellSize() > 64) SetMinCellSize(64);
  SizeRecalc();
  Refresh();
}

// Recalculate scaling information for canvas after resize, zoom, or
// other event.

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
  SetupAxes();
}
