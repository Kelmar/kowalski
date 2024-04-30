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
#include "FormatNums.h"

using namespace NumberFormats;

void IncEditField(wxControl *pCtrl, int iDelta, int iMin, int iMax)
{
    int num, old;
    NumberFormat fmt;

    old = num = ReadNumber(pCtrl, fmt);

    num += iDelta;

    if (num > iMax)
        num = iMax;
    else if (num < iMin)
        num = iMin;

    if (num != old)
        SetNumber(pCtrl, num, fmt);
}

int ReadNumber(wxControl *pCtrl, NumberFormat &fmt)
{
    if (!pCtrl)
        return 0;

    wxString buf = pCtrl->GetLabel().MakeUpper();

    if (buf.IsEmpty())
        return 0;

    int radix = 10;
    int start = 0;
    
    if (buf[0] == '$')
    {
        fmt = NumberFormat::DollarHex;
        radix = 16;
        start = 1;
    }
    else if (buf.StartsWith("0X"))
    {
        fmt = NumberFormat::IntelHex;
        radix = 16;
        start = 2;
    }
    else if (buf[0] >= '0' && buf[0] <= '9')
    {
        fmt = NumberFormat::Decimal;
    }
    else
    {
        fmt = NumberFormat::Error;
        return 0;
    }

    int num = 0;
    for (int i = start; i < buf.Length(); ++i)
    {
        char c = buf[i];
        int val;

        if ((c >= '0') && (c <= '9'))
            val = c - '0';
        else
            val = c - 'A' + 10;

        num = num * radix + val;
    }

    return num;
}

void SetNumber(wxControl *pCtrl, int num, NumberFormat fmt)
{
    if (!pCtrl)
        return;

    wxString buf;

    switch (fmt)
    {
    case NumberFormat::Error:
    case NumberFormat::IntelHex:
        buf.Printf("0x%04X", num);
        break;

    case NumberFormat::DollarHex:
        buf.Printf("$%04X", num);
        break;

    case NumberFormat::Decimal:
        buf.Printf("%d", num);
        break;

    default:
        ASSERT(false);
        break;
    }

    pCtrl->SetLabel(buf);
}
