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

// ColorButton.h : header file
//

#ifndef COLOR_BUTTON_H__
#define COLOR_BUTTON_H__

/////////////////////////////////////////////////////////////////////////////
// CColorButton window

/*
 * wxWidgets suggests that we not try and override a platform's default paint
 * method for buttons.  For now we're deriving from wxControl so we can paint
 * ourselves in a controlled way.
 * 
 *              -- B.Simonds (April 27, 2024)
 */

class CColorButton : public wxControl // CButton
{
private:
    wxColour m_color;

    int m_nDx;
    int m_nDy;

    void PaintIt(int offset);

    // Construction
public:
    /* constructor */ CColorButton();
    virtual          ~CColorButton() { }

public:
    void SetColorRef(const wxColour &color)
    {
        m_color = color;
        Refresh();
    }

protected:
    afx_msg void OnPaint();
};

#endif /* COLOR_BUTTON_H__ */
