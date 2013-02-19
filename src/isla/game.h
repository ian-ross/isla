// FILE: game.h

#ifndef _ISLA_GAME_H_
#define _ISLA_GAME_H_

#include "wx/wx.h"

// Isla

// A struct used to pass cell coordinates around
struct IslaCell { wxInt32 i; wxInt32 j; };

// A private class that contains data about a block of cells
class IslaCellBox;

// A class that models a Isla game instance
class Isla {
public:
  Isla();
  ~Isla();

  wxUint32 GetNumCells() const    { return m_numcells; };
  bool IsAlive(wxInt32 x, wxInt32 y);
  void SetCell(wxInt32 x, wxInt32 y, bool alive = true);

  void Clear();
  bool NextTic();

  IslaCell FindNorth();
  IslaCell FindSouth();
  IslaCell FindWest();
  IslaCell FindEast();
  IslaCell FindCenter();

  // The following functions find cells within a given viewport; either
  // all alive cells, or only those cells which have changed since last
  // generation. You first call BeginFind() to specify the viewport,
  // then keep calling FindMore() until it returns true.
  //
  // BeginFind:
  //  Specify the viewport and whether to look for alive cells or for
  //  cells which have changed since the last generation and thus need
  //  to be repainted. In this latter case, there is no distinction
  //  between newborn or just-dead cells.
  //
  // FindMore:
  //  Fills an array with cells that match the specification given with
  //  BeginFind(). The array itself belongs to the Isla object and must
  //  not be modified or freed by the caller. If this function returns
  //  false, then the operation is not complete: just process all cells
  //  and call FillMore() again.
  //
  void BeginFind(wxInt32 x0, wxInt32 y0,
                 wxInt32 x1, wxInt32 y1,
                 bool changed);
  bool FindMore(IslaCell *cells[], size_t *ncells);

private:
  // cellbox-related
  IslaCellBox *CreateBox(wxInt32 x, wxInt32 y, wxUint32 hv);
  IslaCellBox *LinkBox(wxInt32 x, wxInt32 y, bool create = true);
  void KillBox(IslaCellBox *c);

  // helper for BeginFind & FindMore
  void DoLine(wxInt32 x, wxInt32 y, wxUint32 alive, wxUint32 old = 0);


  // pattern data
  IslaCellBox   *m_head;          // list of alive boxes
  IslaCellBox   *m_available;     // list of reusable dead boxes
  IslaCellBox  **m_boxes;         // hash table of alive boxes
  wxUint32   m_numcells;      // population (number of alive cells)

  // state vars for BeginFind & FindMore
  IslaCell  *m_cells;         // array of cells
  size_t     m_ncells;        // number of valid entries in m_cells
  wxInt32    m_x, m_y,        // counters and search mode
             m_x0, m_y0,
             m_x1, m_y1;
  bool       m_changed;
  bool       m_findmore;
};

#endif  // _ISLA_GAME_H_
