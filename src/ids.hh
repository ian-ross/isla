//----------------------------------------------------------------------
// FILE:   ids.hh
// DATE:   23-FEB-2013
// AUTHOR: Ian Ross
//
// Non-standard IDs for controls and menu commands.
//----------------------------------------------------------------------

#ifndef _H_IDS_
#define _H_IDS_

enum {
  // File menu
  ID_IMPORT_ISLCMP_DATA = wxID_HIGHEST,
  ID_CLEAR_ISLCMP_DATA,
  ID_EXPORT_ISLAND_DATA,

  // View menu
  ID_ZOOM_SELECTION,
  ID_PAN,

  // Tools menu
  ID_SELECT,
  ID_EDIT_MASK,

  // Dialogues
  ID_PREFS_GRID_CHOICE,
  ID_PREFS_ISLAND_THRESHOLD,
  ID_PREFS_COL_OCEAN,
  ID_PREFS_COL_LAND,
  ID_PREFS_COL_ISLAND,
  ID_PREFS_COL_GRID,
  ID_PREFS_COL_OUTLINE,
  ID_PREFS_COL_COMP,

  // Context menu
  ID_CTX_TOGGLE_ISLAND,

#ifdef ISLA_DEBUG
  // Debug menu
  ID_DEBUG_SIZE_OVERLAY,
  ID_DEBUG_REGION_OVERLAY,
  ID_DEBUG_ISMASK_OVERLAY,
  ID_DEBUG_ISISLAND_OVERLAY,
#endif
};

#endif
