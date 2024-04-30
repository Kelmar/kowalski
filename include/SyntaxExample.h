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

#ifndef SYNTAX_EXAMPLE_H__
#define SYNTAX_EXAMPLE_H__

/////////////////////////////////////////////////////////////////////////////
// CSyntaxExample window

class CSyntaxExample : public wxPanel
{
public:
    /* constructor */ CSyntaxExample();

    int m_nTabStop;
    wxFont m_hEditorFont;
    wxColour m_rgbBackground;
    wxColour m_rgbText;
    wxColour m_rgbInstruction;
    wxColour m_rgbDirective;
    wxColour m_rgbComment;
    wxColour m_rgbNumber;
    wxColour m_rgbString;
    wxColour m_rgbSelection;
    bool m_vbBold[5];

public:
    //virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

public:
    virtual ~CSyntaxExample();

protected:
    afx_msg bool OnEraseBkgnd(wxDC* pDC);
};

#endif /* SYNTAX_EXAMPLE_H__ */
