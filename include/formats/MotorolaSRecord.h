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

#ifndef FMT_MOTOROLA_S_6502_H__
#define FMT_MOTOROLA_S_6502_H__

class COutputMem;
class CMarkArea;

#include "Archive.h"

class CMotorolaSRecord
{
    UINT geth(const char *&ptr, UINT &sum); // Interpretation of a two-digit hex number
    UINT row;

public:
    void SaveHexFormat(Archive &archive, COutputMem &mem, CMarkArea &area, int prog_start = -1);

    void LoadHexFormat(Archive &archive, COutputMem &mem, CMarkArea &area, int &prog_start);

    class CMotorolaSRecordException //: public CException
    {
    public:
        enum Err { E_NONE, E_BAD_FORMAT, E_CHKSUM, E_FORMAT };
        
    private:
        Err error;
        UINT row;

    public:
        CMotorolaSRecordException(Err err= E_NONE, UINT row= 0) : error(err), row(row)
        {
        }

        virtual ~CMotorolaSRecordException()
        {
        }

        virtual bool GetErrorMessage(char *lpszError, UINT nMaxError, UINT *pnHelpContext = nullptr);
    };
};

#endif /* FMT_MOTOROLA_S_6502_H__ */
