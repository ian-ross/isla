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
  void ZoomIn(void);
  void ZoomOut(void);
  bool ZoomInOK(void) const { return true; }
  bool ZoomOutOK(void) const { return true; }

  // View management
  double GetScale(void) const { return scale; }
  int GetMinCellSize(void) const;
  void Recenter(wxInt32 i, wxInt32 j);

  // Model changed...
  void modelReset(IslaModel *m);

  // Drawing
  void DrawAll();
  void DrawCell(wxInt32 i, wxInt32 j, bool alive);

#ifdef ISLA_DEBUG
  bool sizingOverlay;           // Display sizing information?
#endif

private:
  // Draw axis labels.
  void axisLabels(wxDC &dc, bool horiz, int yOrX,
                  const std::vector<int> &xOrYs,
                  const std::vector<wxString> &labs);

  DECLARE_EVENT_TABLE()
  void OnPaint(wxPaintEvent &e);
  void OnMouse(wxMouseEvent &e);
  void OnSize(wxSizeEvent &e);
  void OnScroll(wxScrollWinEvent &e);
  void OnEraseBackground(wxEraseEvent &e);

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

  // Conversion between cell and screen coordinates
  wxInt32 XToCell(wxCoord x) const { return 0; }
  wxInt32 YToCell(wxCoord y) const { return 0; }
  wxCoord CellToX(wxInt32 i) const { return 0; }
  wxCoord CellToY(wxInt32 j) const { return 0; }

  enum MouseState {
    MOUSE_NOTHING,
    MOUSE_PAN_X,
    MOUSE_PAN_Y,
    MOUSE_PAN_2D
  };

  wxInt32      _viewportX;       // first visible cell (x coord)
  wxInt32      _viewportY;       // first visible cell (y coord)
  wxInt32      _viewportW;       // number of visible cells (w)
  wxInt32      _viewportH;       // number of visible cells (h)
  wxInt32      _mi, _mj;         // last mouse position

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
