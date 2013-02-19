// FILE: dialogs.h

#ifndef _ISLA_DIALOGS_H_
#define _ISLA_DIALOGS_H_

// for compilers that support precompilation, includes "wx/wx.h"
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "isla.h"
#include "game.h"


// --------------------------------------------------------------------------
// IslaAboutDialog
// --------------------------------------------------------------------------

class IslaAboutDialog : public wxDialog
{
public:
    // ctor
    IslaAboutDialog(wxWindow *parent);
};


#endif  // _ISLA_DIALOGS_H_
