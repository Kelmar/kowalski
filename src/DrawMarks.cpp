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

#include "StdAfx.h"
#include "DrawMarks.h"

// Arrow, break and error tracing functions
// Used in ...View classes

wxColour CMarks::s_rgbPointer = wxColour(255, 255, 0);
wxColour CMarks::s_rgbBreakpoint = wxColour(0, 0, 255);
wxColour CMarks::s_rgbError = wxColour(255, 0, 0);

//-----------------------------------------------------------------------------

void CMarks::draw_pointer(wxDC &dc, int x, int y, int h)
{
    static const wxPoint shape[] =
    { {-4, -3}, {0, -3}, {0, -7}, {7, 0}, {0, 7}, {0, 3}, {-4, 3}, {-4, -3} };

    const int size = sizeof(shape) / sizeof(wxPoint);
    wxPoint coords[size];

    x += (6 * h) / 15;
    y += (7 * h) / 15;
    
    for (int i = 0; i < size; i++)
    {
        coords[i].x = x + (shape[i].x * h) / 15; // Rescale and shift
        coords[i].y = y + (shape[i].y * h) / 15;
    }

    wxColour sysText = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOWTEXT);

    wxPen pen(sysText);
    wxBrush brush(s_rgbPointer);

    dc.SetPen(pen);
    dc.SetBrush(brush);

    dc.DrawPolygon(size, coords);
}

//-----------------------------------------------------------------------------

void CMarks::draw_breakpoint(wxDC &dc, int x, int y, int h, bool active)
{
    wxColour sysText = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOWTEXT);

    wxPen pen(sysText);
    wxBrush brush(s_rgbBreakpoint);

    dc.SetPen(pen);
    dc.SetBrush(active ? brush : dc.GetBackground());

    dc.DrawEllipse(x, y, x + h, y + h);
}

//-----------------------------------------------------------------------------

void CMarks::draw_mark(wxDC &dc, int x, int y, int h)
{
    static const wxPoint shape[] =
    { {-1, -7}, {0, -7}, {7, 0}, {0, 7}, {-1, 7}, {-1, -7} };
//  { {0,0}, {1,0}, {8,7}, {1,14}, {0,14}, {0,0} };
    const int size = sizeof shape / sizeof(wxPoint);
    wxPoint coords[size];

    x += (3 * h) / 15;
    y += (7 * h) / 15;

    for (int i = 0; i < size; i++)
    {
        coords[i].x = x + (shape[i].x * h) / 15;
        coords[i].y = y + (shape[i].y * h) / 15;
    }

    wxColour sysText = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOWTEXT);

    wxPen pen(sysText);
    wxBrush brush(s_rgbError);

    dc.SetPen(pen);
    dc.SetBrush(brush);

    dc.DrawPolygon(size, coords);
}

//-----------------------------------------------------------------------------
