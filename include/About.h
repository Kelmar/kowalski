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

#ifndef ABOUT_DLG_H__
#define ABOUT_DLG_H__

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#include "resource.h"
//#include "StaticLink.h"

class CAboutDlg : public wxDialog
{
private:
    wxFont m_TitleFont;

public:
    /* constructor */ CAboutDlg();

    //CStatic m_wndTitle;
    std::string m_strVersion;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    virtual bool OnInitDialog();

    wxHyperlinkCtrl m_StaticLink;
};

#endif /* ABOUT_DLG_H__ */
