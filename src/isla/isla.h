// FILE: isla.h

#ifndef _ISLA_APP_H_
#define _ISLA_APP_H_

#include "wx/wx.h"
#include "wx/minifram.h"

#include "game.h"

#include "GridData.hh"

// IslaCanvas

// Note that in IslaCanvas, all cell coordinates are
// named i, j, while screen coordinates are named x, y.

class IslaCanvas : public wxWindow {
public:
  IslaCanvas(wxWindow *parent, Isla *isla, bool interactive = true);
  virtual ~IslaCanvas();

  // View management
  int  GetCellSize() const { return _cellsize; };
  void SetCellSize(int cellsize);
  void Recenter(wxInt32 i, wxInt32 j);

  // Drawing
  void DrawChanged();
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

  Isla         *_isla;           // Isla object
  int          _cellsize;        // current cell size, in pixels
  bool         _interactive;     // is this canvas interactive?
  MouseStatus  _status;          // what is the user doing?
  wxInt32      _viewportX;       // first visible cell (x coord)
  wxInt32      _viewportY;       // first visible cell (y coord)
  wxInt32      _viewportW;       // number of visible cells (w)
  wxInt32      _viewportH;       // number of visible cells (h)
  int          _thumbX;          // horiz. scrollbar thumb position
  int          _thumbY;          // vert. scrollbar thumb position
  wxInt32      _mi, _mj;         // last mouse position
};


// IslaFrame

class IslaFrame : public wxFrame {
public:
  IslaFrame();
  virtual ~IslaFrame() { }

  void UpdateUI();

private:
  DECLARE_EVENT_TABLE()
  void OnMenu(wxCommandEvent &e);
  void OnOpen(wxCommandEvent &e);
  void OnLoadMask(wxCommandEvent &e);
  void OnZoom(wxCommandEvent &e);
  void OnClose(wxCloseEvent &e);

  Isla           *_isla;
  IslaCanvas     *_canvas;
  GridPtr        _grid;
  GridData<bool> *_orig_mask;
};


// IslaApp

class IslaApp : public wxApp {
public:
  virtual bool OnInit();
};

#endif  // _ISLA_APP_H_
