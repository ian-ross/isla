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
  ID_LOAD_MASK = wxID_HIGHEST,
  ID_SAVE_MASK,
  ID_IMPORT_ISLCMP_DATA,
  ID_CLEAR_ISLCMP_DATA,
  ID_EXPORT_ISLAND_DATA,

  // View menu
  ID_ZOOM_SELECTION,
  ID_PAN,

  // Tools menu
  ID_SELECT,
  ID_EDIT_LANDSEA_MASK
};

#endif
