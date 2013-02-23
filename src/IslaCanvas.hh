//----------------------------------------------------------------------
// FILE:   IslaCanvas.hh
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Canvas class for main map view of Isla island editor.
//----------------------------------------------------------------------

#ifndef _H_ISLACANVAS_
#define _H_ISLACANVAS_

#include "wx/wx.h"

class IslaModel;

// Note that in IslaCanvas, all cell coordinates are
// named i, j, while screen coordinates are named x, y.

class IslaCanvas : public wxWindow {
public:
  IslaCanvas(wxWindow *parent, IslaModel *m);
  virtual ~IslaCanvas() { }

  // View management
  int  GetCellSize() const { return _cellsize; };
  void SetCellSize(int cellsize);
  void Recenter(wxInt32 i, wxInt32 j);

  // Drawing
  void DrawAll();
  void DrawCell(wxInt32 i, wxInt32 j, bool alive);

private:
  DECLARE_EVENT_TABLE()
  void OnPaint(wxPaintEvent &e);
  void OnMouse(wxMouseEvent &e);
  void OnSize(wxSizeEvent &e);
  void OnScroll(wxScrollWinEvent &e);
  void OnEraseBackground(wxEraseEvent &e);

  // Draw a cell (parametrized by DC)
  void DrawCell(wxInt32 i, wxInt32 j, wxDC &dc);

  // Conversion between cell and screen coordinates
  wxInt32 XToCell(wxCoord x) const { return (x / _cellsize) + _viewportX; };
  wxInt32 YToCell(wxCoord y) const { return (y / _cellsize) + _viewportY; };
  wxCoord CellToX(wxInt32 i) const { return (i - _viewportX) * _cellsize; };
  wxCoord CellToY(wxInt32 j) const { return (j - _viewportY) * _cellsize; };

  // what is the user doing?
  enum MouseStatus {
    MOUSE_NOACTION,
    MOUSE_DRAWING,
    MOUSE_ERASING
  };

  IslaModel *model;              // Isla model.
  int          _cellsize;        // current cell size, in pixels
  MouseStatus  _status;          // what is the user doing?
  wxInt32      _viewportX;       // first visible cell (x coord)
  wxInt32      _viewportY;       // first visible cell (y coord)
  wxInt32      _viewportW;       // number of visible cells (w)
  wxInt32      _viewportH;       // number of visible cells (h)
  int          _thumbX;          // horiz. scrollbar thumb position
  int          _thumbY;          // vert. scrollbar thumb position
  wxInt32      _mi, _mj;         // last mouse position
};

#endif
