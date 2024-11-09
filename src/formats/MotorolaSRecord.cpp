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

// Odczyt i zapis kodu wynikowego w postaci Intel-HEX

#include "StdAfx.h"
#include "sim.h"

#include "formats/MotorolaSRecord.h"
#include "resource.h"
#include "MarkArea.h"

void CMotorolaSRecord::SaveHexFormat(Archive &archive, COutputMem &mem, CMarkArea &area, int prog_start)
{
    UNUSED(archive);
    UNUSED(mem);
    UNUSED(area);
    UNUSED(prog_start);


#if REWRITE_TO_WX_WIDGET
    char buf[80], *ptr;

    for (UINT part = 0; part < area.GetSize(); part++)
    {
        int start, end;

        area.GetPartition(part, start, end);
        ASSERT(start >= 0 && start <= 0xFFFF);
        ASSERT(end >= 0 && end <= 0xFFFF);
        ASSERT(start <= end);

        const int STEP = 0x10;

        for (int i = start; i <= end; i += STEP)
        {
            int sum = 0; // checksum
            int lim = min(i + STEP - 1, end);
            // beginning of line: amount of data, address (hi, lo), zero
            int cnt = i + STEP <= end ? STEP : end - i + 1; // number of bytes to send (per line)

            if (start > 0xFFFF)  // 1.3.3 support for 24-bit addressing
            {
                ptr = buf + wsprintf(buf, "S2%02X%02X%02X%02X",cnt+4,(i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF);
                sum += cnt+4 + ((i >> 16) & 0xFF) +((i >> 8) & 0xFF) + (i & 0xFF);
            }
            else
            {
                ptr = buf + wsprintf(buf, "S1%02X%02X%02X", cnt + 3, (i >> 8) & 0xFF, i & 0xFF);
                sum += cnt+3 + ((i >> 8) & 0xFF) + (i & 0xFF);
            }

            for (int j = i; j <= lim; j++)
            {
                ptr += wsprintf(ptr, _T("%02X"), mem[j]);
                sum += mem[j];
            } // the sum of all bytes in a line must equal zero

            ptr += wsprintf(ptr, "%02X\r\n", ~sum & 0xFF); // Write checksum
            archive.Write(buf, sizeof(char) * (ptr - buf));
        }
    }

    if (prog_start == -1)
        prog_start = 0;

    {
        ASSERT(prog_start >= 0 && prog_start <= 0xFFFF);

        if (prog_start > 0xFFFF)  // 1.3.3 support for 24-bit addressing for prog_start
        {
            int sum = ~( ((prog_start >> 16) & 0xFF) + ((prog_start >> 8) & 0xFF) + (prog_start & 0xFF) + 4 ) & 0xFF;
            ptr = buf + wsprintf(buf, "S8%02X%02X%02X%02X%02X\r\n", 4,
                                 (prog_start>>16) & 0xFF, (prog_start >> 8) & 0xFF, prog_start & 0xFF, sum);
        }
        else
        {
            int sum= ~(((prog_start >> 8) & 0xFF) + (prog_start & 0xFF) + 3) & 0xFF;
            ptr = buf + wsprintf(buf, "S9%02X%02X%02X%02X\r\n", 3,
                                 (prog_start >> 8) & 0xFF, prog_start & 0xFF, sum);
        }
    }

    archive.Write(buf, sizeof(char) * (ptr - buf));
#endif
}

//-----------------------------------------------------------------------------

UINT CMotorolaSRecord::geth(const char *&ptr, UINT &sum)
{
    UINT res = 0;
    
    for (int i = 0; i < 2; ++i, ++ptr)
    {
        res <<= 4;

        if (*ptr >= '0' && *ptr <= '9')
            res += *ptr - '0';
        else if (*ptr >= 'A' && *ptr <= 'F')
            res += *ptr - 'A' + 10;
        else if (*ptr >= 'a' && *ptr <= 'f')
            res += *ptr - 'a' + 10;
        else
            throw CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row);
    }

    sum += res; // Calculate the checksum
    sum &= 0xFF;

    return res;
}

//-----------------------------------------------------------------------------

void CMotorolaSRecord::LoadHexFormat(Archive &archive, COutputMem &mem, CMarkArea &area, int &prog_start)
{
    UNUSED(archive);
    UNUSED(mem);
    UNUSED(area);
    UNUSED(prog_start);

#if REWRITE_TO_WX_WIDGET
    char buf[256];

    for (row = 1; ; row++)
    {
        if (!archive.ReadString(buf, sizeof(buf)))
            break;

        if (strlen(buf) == sizeof(buf) - 1) // Line too long
            CMotorolaSRecordException(CMotorolaSRecordException::E_BAD_FORMAT,row);

        const char *ptr = buf;
        if (*ptr++ != 'S') // Unrecognized format?
            throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_BAD_FORMAT, row));

        UINT info = *ptr++; // information byte
        UINT sum = 0; // variable for calculating the checksum
        UINT cnt = geth(ptr, sum); // number of data bytes

        if (cnt < 3) // Unrecognized format?
            throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row));

        cnt -= 3;
        UINT addr = geth(ptr, sum);
        addr <<= 8;
        addr += geth(ptr, sum);	// data address

        switch (info)
        {
        case '1': // program code (S1)
        {
            if (cnt)
                area.SetStart(addr);

            for (UINT i = 0; i < cnt; i++)
            {
                if (addr > 0xFFFF)	// za du�y adres?
                    throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row));

                mem[addr++] = (UINT8)geth(ptr, sum);
            }

            geth(ptr, sum); // checksum byte
            if (sum != 0xFF) // checksum error
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_CHKSUM, row));

            if (cnt)
                area.SetEnd(addr - 1);

            break;
        }

        case '2': // 1.3.3 support for 24-bit addressing
        {
            if (wxGetApp().m_global.m_bProc6502 != 2)
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row));

            addr <<= 8;
            addr += geth(ptr, sum); // data address
            cnt -= 1;

            if (cnt)
                area.SetStart(addr);

            for (UINT i = 0; i < cnt; i++)
                mem[addr++] = (UINT8)geth(ptr, sum);

            geth(ptr, sum); // checksum byte
            if (sum != 0xFF) // checksum error
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_CHKSUM, row));

            if (cnt)
                area.SetEnd(addr - 1);
            break;
        }
        case '8': // 1.3.3 support for 24-bit addressing for prog_start
            cnt -=1;
            addr <<= 8;
            addr += geth(ptr, sum); // data address

            if (cnt) // Unexpected data?
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row));

            geth(ptr, sum); // checksum byte
            if (sum != 0xFF) // checksum error
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_CHKSUM, row));

            prog_start = (int)addr;
            break;

        case '9': // end, possibly startup address (S9)
            if (cnt) // Unexpected data?
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row));

            geth(ptr, sum); // checksum byte
            if (sum != 0xFF) // checksum error
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_CHKSUM, row));

            if (addr > 0xFFFF) // address too big?
                throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row));

            prog_start = (int)addr;
            break;

        default: // unknown value
            throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT, row));
        }
    }
#endif
}

//-----------------------------------------------------------------------------

/*virtual*/bool CMotorolaSRecord::CMotorolaSRecordException::GetErrorMessage(char *errBuf,
        UINT maxError, UINT *pnHelpContext/*= NULL*/)
{
    UNUSED(errBuf);
    UNUSED(maxError);
    UNUSED(pnHelpContext);

#if REWRITE_TO_WX_WIDGET
    std::string msg;
    char num[16];

    if (pnHelpContext != nullptr)
        *pnHelpContext = 0;

    snprintf(num, sizeof(num), "%u", row);

    switch (error)
    {
    case E_BAD_FORMAT:		// b��dny format pliku
        msg.LoadString(IDS_INTEL_HEX_ERR_2);
        break;

    case E_CHKSUM:		// b��d sumy kontrolnej
        AfxFormatString1(msg, IDS_INTEL_HEX_ERR_1, num);
        break;

    case E_FORMAT:		// b��dny format danych
        AfxFormatString1(msg, IDS_INTEL_HEX_ERR_3, num);
        break;

    default:
        return FALSE;
    }

    strncpy(lpszError, msg.c_str(), nMaxError);
#endif

    return true;
}
