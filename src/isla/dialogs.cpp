// FILE: dialogs.cpp

// HEADER FILES

#include "wx/wx.h"
#include "wx/statline.h"
#include "wx/minifram.h"
#include "wx/settings.h"

#include "dialogs.h"

#include "bitmaps/isla.xpm"


// IslaAboutDialog

IslaAboutDialog::IslaAboutDialog(wxWindow *parent)
  : wxDialog(parent, wxID_ANY, _("About Isla"),
             wxDefaultPosition, wxDefaultSize)
{
  wxStaticBitmap *sbmp = new wxStaticBitmap(this, wxID_ANY, wxBitmap(isla_xpm));
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(sbmp, 0, wxCENTRE | wxALL, 10);
  sizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxGROW | wxLEFT | wxRIGHT, 5);
  sizer->Add(CreateTextSizer(_("Isla version 0.0 for wxWidgets\n\n"
                               "(c) 2013 Ian Ross\n\n"
                               "<ian@skybluetrades.net>")),
             0, wxCENTRE | wxRIGHT|wxLEFT|wxTOP, 20);
  wxSizer *sizerBtns = CreateButtonSizer(wxOK);
  if (sizerBtns) sizer->Add(sizerBtns, wxSizerFlags().Expand().Border());
  SetSizer(sizer);
  sizer->SetSizeHints(this);
  sizer->Fit(this);
  Centre(wxBOTH | wxCENTRE_ON_SCREEN);
}
