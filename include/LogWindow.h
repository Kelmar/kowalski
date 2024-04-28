/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

#ifndef LOGWINDOW_H__
#define LOGWINDOW_H__

/////////////////////////////////////////////////////////////////////////////
// CLogWindow frame

class CLogWindow : public wxWindow //CMiniFrameWnd
{
protected:
    virtual void PostNcDestroy();

public:
    void SetText(const CommandLog& log);
    void Invalidate();

    bool m_bHidden;

    /* constructor */ CLogWindow();
    virtual          ~CLogWindow();

    bool Create();

    afx_msg int OnCtlColor(wxDC* dc, wxWindow* wnd, UINT ctlColor);
    afx_msg void OnClose();

    wxFont m_fntMono;
    wxBrush m_brBackground;

    afx_msg LRESULT OnChangeCode(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */);
    afx_msg LRESULT OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */);
};

#endif /* LOGWINDOW_H__ */
