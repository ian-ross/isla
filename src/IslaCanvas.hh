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
  bool ZoomInOK(void) const { return true; }
  bool ZoomOutOK(void) const { return true; }

  // View management
  double GetScale(void) const { return scale; }

  // Model changed...
  void modelReset(IslaModel *m);

#ifdef ISLA_DEBUG
  bool sizingOverlay;           // Display sizing information?
#endif

private:
  // Draw axis labels.
  void axisLabels(wxDC &dc, bool horiz, int yOrX,
                  const std::vector<int> &xOrYs,
                  const std::vector<wxString> &labs);

  void ZoomScale(double zfac);  // Rescale.
  void SizeRecalc(void);        // Recalculate sizing information.

  DECLARE_EVENT_TABLE()
  void OnPaint(wxPaintEvent &e);
  void OnMouse(wxMouseEvent &e);
  void OnSize(wxSizeEvent &e);
  void OnEraseBackground(wxEraseEvent &e) { }

  // Draw a cell (parametrized by DC)
  void DrawCell(wxInt32 i, wxInt32 j, wxDC &dc);

  // Conversion between model (lat/lon) and canvas coordinates.
  int latToY(double lat) const { return maph / 2 - (lat - clat) * scale; }
  int lonToX(double lon) const {
    bool right = fmod(lon - clon + 360.0, 360.0) <= 180.0;
    double dlon = right ?
      fmod(lon - clon + 360.0, 360.0) : -fmod(clon - lon + 360.0, 360.0);
    return mapw / 2 + dlon * scale;
  }

  enum MouseState {
    MOUSE_NOTHING,
    MOUSE_PAN_X,
    MOUSE_PAN_Y,
    MOUSE_PAN_2D
  };

  // Window layout parameters.
  int canw, canh;               // Canvas dimensions.
  int mapw, maph;               // Map dimensions (i.e. without
                                // borders).
  int xoff, yoff;               // Map offsets within canvas.
  double clat, clon;            // Centre coordinates (lat/lon).
  wxRect baxis, taxis;          // Axis rectangles for mouse event
  wxRect laxis, raxis;          // handling.

  // Mouse information.
  int mousex, mousey;           // Start coordinates for drags.
  MouseState mouse;             // What are we mousing?

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
                                // diaplay cell and lat/lon axis
                                // labels.
};

#endif
