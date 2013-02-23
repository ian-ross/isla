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
  DECLARE_EVENT_TABLE()
  void OnPaint(wxPaintEvent &e);
  void OnMouse(wxMouseEvent &e);
  void OnSize(wxSizeEvent &e);
  void OnScroll(wxScrollWinEvent &e);
  void OnEraseBackground(wxEraseEvent &e);

  // Draw a cell (parametrized by DC)
  void DrawCell(wxInt32 i, wxInt32 j, wxDC &dc);

  // Conversion between model (lat/lon) and canvas coordinates.
  int latToY(double lat) const { return canh / 2 - (lat - clat) * scale; }
  int lonToX(double lon) const { return canw / 2 + (lon - clon) * scale; }

  // Conversion between cell and screen coordinates
  wxInt32 XToCell(wxCoord x) const { return 0; }
  wxInt32 YToCell(wxCoord y) const { return 0; }
  wxCoord CellToX(wxInt32 i) const { return 0; }
  wxCoord CellToY(wxInt32 j) const { return 0; }

  // what is the user doing?
  enum MouseStatus {
    MOUSE_NOACTION,
    MOUSE_DRAWING,
    MOUSE_ERASING
  };

  MouseStatus  _status;          // what is the user doing?
  wxInt32      _viewportX;       // first visible cell (x coord)
  wxInt32      _viewportY;       // first visible cell (y coord)
  wxInt32      _viewportW;       // number of visible cells (w)
  wxInt32      _viewportH;       // number of visible cells (h)
  wxInt32      _mi, _mj;         // last mouse position

  // Window layout parameters.
  int canw, canh;               // Canvas dimensions.
  double clat, clon;            // Centre coordinates (lat/lon).

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
