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

//#if !defined(AFX_INTREQUESTGENERATORDLG_H__1AEFC032_2D6B_4A3D_8DE5_5BB89CB76F80__INCLUDED_)
//#define AFX_INTREQUESTGENERATORDLG_H__1AEFC032_2D6B_4A3D_8DE5_5BB89CB76F80__INCLUDED_


#ifndef GOTO_LINE_H__
#define GOTO_LINE_H__

// CGotoLineNumber.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntRequestGeneratorDlg dialog

class CGotoLineNumber : public wxDialog
{
public:
    /* constructor */ CGotoLineNumber();
    virtual ~CGotoLineNumber();

    UINT m_uLineNo;
    std::string m_sPrompt;
};

#endif /* GOTO_LINE_H__ */
