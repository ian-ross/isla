//----------------------------------------------------------------------
// FILE:   IslaCanvas.hh
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Canvas class for main map view of Isla island editor.
//----------------------------------------------------------------------

#ifndef _H_ISLACANVAS_
#define _H_ISLACANVAS_

#include <vector>

#include "wx/wx.h"

class IslaModel;

// Note that in IslaCanvas, all cell coordinates are
// named i, j, while screen coordinates are named x, y.

class IslaCanvas: public wxWindow {
public:
  IslaCanvas(wxWindow *parent, IslaModel *m);
  virtual ~IslaCanvas() { }

  // Pan and zoom behaviour.
  void Pan(int dx, int dy);
  void ZoomIn(void) { ZoomScale(1.25); }
  void ZoomOut(void) { ZoomScale(1.0/1.25); }
  void ZoomToFit(void);
  void ZoomToSelection(void) {
    zoom_selection = true;
    panning = false;
    SetCursor(wxCursor(wxCURSOR_CROSS));
  }
  bool ZoomInOK(void) const { return MinCellSize() < 64; }
  bool ZoomOutOK(void) const { return MinCellSize() > 2; }
  bool Panning(void) const { return panning; }
  void SetPanning(bool pan) {
    panning = pan;
    zoom_selection = false;
    SetCursor(wxCursor(panning ? wxCURSOR_HAND : wxCURSOR_ARROW));
  }

  // View management
  double GetScale(void) const { return scale; }

  // Model changed...
  void modelReset(IslaModel *m);

#ifdef ISLA_DEBUG
  bool sizingOverlay;           // Display sizing information?
  bool regionOverlay;           // Display region information?
#endif

private:
  // Draw axis labels.
  void axisLabels(wxDC &dc, bool horiz, int yOrX,
                  const std::vector<int> &xOrYs,
                  const std::vector<wxString> &labs);

  void ZoomScale(double zfac);  // Rescale.
  void SizeRecalc(void);        // Recalculate sizing information.

  // Do zoom to selection.
  void DoZoomToSelection(int x0, int y0, int x1, int y1);

  // Axis position and label calculation.
  void SetupAxes(bool dox = true, bool doy = true);

  // Find minimum cell size in either direction.
  double MinCellSize(void) const {
    return min(minDlon * scale, minDlat * scale);
  }

  // Set minimum cell size in either direction.
  void SetMinCellSize(int cs) { scale = cs / min(minDlon, minDlat); }


  DECLARE_EVENT_TABLE()
  void OnPaint(wxPaintEvent &e);
  void OnMouse(wxMouseEvent &e);
  void OnSize(wxSizeEvent &e);
  void OnEraseBackground(wxEraseEvent &e) { }

  // Draw a cell (parametrized by DC)
  void DrawCell(wxInt32 i, wxInt32 j, wxDC &dc);

  // Conversion from model (lat/lon) to canvas coordinates.
  double latToY(double lat) const { return maph / 2 - (lat - clat) * scale; }
  double lonToX(double lon) const {
    bool right = fmod(lon - clon + 360.0, 360.0) <= 180.0;
    double dlon = right ?
      fmod(lon - clon + 360.0, 360.0) : -fmod(clon - lon + 360.0, 360.0);
    return mapw / 2 + dlon * scale;
  }

  // Conversion from canvas to model (lat/lon) coordinates.
  double YToLat(double y) const { return clat - (y - maph / 2) / scale; }
  double XToLon(double x) const {
    return fmod(360.0 + clon + (x - mapw / 2) / scale, 360.0);
  }

  enum MouseState {
    MOUSE_NOTHING,
    MOUSE_PAN_X,
    MOUSE_PAN_Y,
    MOUSE_PAN_2D,
    MOUSE_ZOOM_SELECTION
  };

  // Window layout parameters.  Dimensions are stored as exact double
  // values to avoid rounding problems.
  double canw, canh;            // Canvas dimensions.
  double mapw, maph;            // Map dimensions (i.e. without
                                // borders).
  int xoff, yoff;               // Map offsets within canvas.
  double clat, clon;            // Centre coordinates (lat/lon).
  wxRect baxis, taxis;          // Axis rectangles for mouse event
  wxRect laxis, raxis;          // handling.

  // Mouse information.
  int mousex, mousey;           // Start coordinates for drags.
  MouseState mouse;             // What are we mousing?
  bool panning;                 // Is panning enabled?
  bool zoom_selection;          // Zoom to selection enabled?
  int zoom_x0, zoom_y0;         // Zoom selection start coordinates.

  // Model depedent members.
  IslaModel *model;             // Isla model.
  std::vector<double> iclats;   // Inter-cell latitude values.
  std::vector<double> iclons;   // Inter-cell longitude values.
  double scale;                 // Degrees of longitude (X-direction)
                                // or latitude (Y-direction) per pixel
                                // in the current view.
  double minDlat, minDlon;      // Minimum latitude and longitude
                                // sizes of cells in the current grid.

  int bw;                       // Width (px) of axis borders.
                                // Calculated from font size used to
                                // display cell and lat/lon axis
                                // labels.
  int boff;                     // Text offset for axis borders.
  int celllablimit;             // Maximum label width for axis
                                // borders, calculated as the text
                                // extent for the string "888".

  // Current axis label positions and text.
  std::vector<int> laxpos;  std::vector<wxString> laxlab;
  std::vector<int> raxpos;  std::vector<wxString> raxlab;
  std::vector<int> taxpos;  std::vector<wxString> taxlab;
  std::vector<int> baxpos;  std::vector<wxString> baxlab;
};

#endif
