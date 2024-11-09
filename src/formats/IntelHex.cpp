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

#include "formats/IntelHex.h"
#include "resource.h"
#include "MarkArea.h"

void CIntelHex::SaveHexFormat(Archive &archive, COutputMem &mem, CMarkArea &area, int prog_start/*= -1*/)
{
    UNUSED(archive);
    UNUSED(mem);
    UNUSED(area);
    UNUSED(prog_start);

#if 0
    char buf[80], *ptr;

    for (UINT part = 0; part < area.GetSize(); part++)
    {
        int start, end;

        area.GetPartition(part, start, end);
        ASSERT(start >= 0 && start <= 0xFFFF);
        ASSERT(end >= 0 && end <= 0xFFFF);
        ASSERT(start <= end);

        if (start > 0xFFFF) // 1.3.3  support for 24 bit addressing
        {
            int sum = -(((start >> 24) & 0xFF) + ((start >> 16) & 0xFF) + 6) & 0xFF;
            ptr = buf + sprintf(buf, ":%02X%02X%02X%02X%02X%02X%02X\r\n", 2, 0, 0, 4, // Generates bank address line
                                 0, (start>>16) & 0xFF, sum);
            archive.Write(buf, sizeof(char) * (ptr - buf));
        }

        const int STEP = 0x10;
        for (int i = start; i <= end; i += STEP)
        {
            int sum = 0; // Checksum
            int lim = std::min(i + STEP - 1, end);

            // Beginning of line: amount of data, address (hi, lo), zero
            int cnt = i + STEP <= end ? STEP : end - i + 1; // How many bytes to send (per line)

            ptr = buf + sprintf(buf, ":%02X%02X%02X%02X", cnt, (i >> 8) & 0xFF, i & 0xFF, 0);
            sum += cnt + ((i >> 8) & 0xFF) + (i & 0xFF);

            for (int j =i ; j <= lim; j++)
            {
                ptr += sprintf(ptr, "%02X", mem[j]);
                sum += mem[j];
            } // The sum of all bytes in a line must be zero

            ptr += sprintf(ptr, "%02X\r\n", -sum & 0xFF); // Generate a control byte
            archive.Write(buf, sizeof(char) * (ptr - buf));
        }
    }

    if (prog_start == -1)
        prog_start = 0;

    if (prog_start > 0xFFFF) // 1.3.3  added support for 24-bit start address
    {
        int sum = -( ((prog_start >> 16) & 0xFF) + ((prog_start >> 8) & 0xFF) + (prog_start & 0xFF) + 9) & 0xFF;
        ptr = buf + sprintf(buf, ":%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n:%02X%02X%02X%02X%02X\r\n", 4, 0, 0, 5, 0,
                             ((prog_start >> 16) & 0xFF), (prog_start >> 8) & 0xFF, prog_start & 0xFF, sum, 0, 0, 0, 1, 0xFF);
    }
    else
    {
        int sum = -( ((prog_start>>8)&0xFF) + (prog_start&0xFF) + 1 ) & 0xFF;
        ptr = buf + sprintf(buf, ":%02X%02X%02X%02X%02X\r\n", 0,
                             (prog_start >> 8) & 0xFF, prog_start & 0xFF, 1, sum);
    }

    archive.Write(buf, sizeof(char) * (ptr - buf));
#endif
}

//-----------------------------------------------------------------------------

// Interpretation of a two-digit hex number
UINT CIntelHex::geth(const char *&ptr, UINT &sum)
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
            throw CIntelHexException(CIntelHexException::E_FORMAT, row);
    }

    sum += res; // Calculate the checksum
    sum &= 0xFF;

    return res;
}

//-----------------------------------------------------------------------------

void CIntelHex::LoadHexFormat(Archive &archive, COutputMem &mem, CMarkArea &area, int &prog_start)
{
    UNUSED(archive);
    UNUSED(mem);
    UNUSED(area);
    UNUSED(prog_start);

#if 0
    char buf[256];
    int bank = 0;
    bool ps = false;  // 1.3.3 added flag to show 24 bit prog_start was used.

    for (row = 1; ; row++)
    {
        if (!archive.ReadString(buf, sizeof(buf))
            break;

        if (strlen(buf) == sizeof(buf) - 1)
            CIntelHexException(CIntelHexException::E_BAD_FORMAT, row); // Line too long

        const char *ptr = buf;

        if (*ptr++ != ':')
            throw CIntelHexException(CIntelHexException::E_BAD_FORMAT, row); // Unrecognized format

        UINT sum = 0; // Variable for counting the checksum
        UINT cnt = geth(ptr, sum); // Number of bytes of data
        UINT addr = geth(ptr, sum);
        addr <<= 8;
        addr += geth(ptr, sum); // Data address
        UINT info = geth(ptr, sum); // Information byte

        switch (info)
        {
        case 0: // Program code
        {
            if (cnt)
            {
                if (theApp.m_global.m_bProc6502 == 2) // More 65816 support it looks like. -- B.Simonds (April 28, 2024)
                    area.SetStart(((bank & 0xff) << 16) + addr);
                else
                    area.SetStart(addr);
            }

            for (UINT i = 0; i < cnt; i++)
            {
                if (addr > 0xFFFF)
                    throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Address too large
                    
                if (theApp.m_global.m_bProc6502==2)   // 1.3.3 added support to store 24-bit address range
                    mem[((bank&0xff)<<16) + addr++] = (UINT8)geth(ptr,sum);
                else
                    mem[addr++] = (UINT8)geth(ptr,sum);
            }

            geth(ptr, sum); // Checksum byte

            if (sum) // Checksum error
                throw CIntelHexException(CIntelHexException::E_CHKSUM, row);

            if (cnt)
            {
                if (theApp.m_global.m_bProc6502 == 2)  // 1.3.3 added support for 24-bit addressing
                    area.SetEnd((bank << 16) & 0xff + addr - 1);
                else
                    area.SetEnd(addr - 1);
            }
            break;
        }

        case 1: // Launch address
            if (cnt)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            geth(ptr, sum); // Checksum byte

            if (sum) // Checksum error
                throw CIntelHexException(CIntelHexException::E_CHKSUM,row);

            if (addr > 0xFFFF)
                throw CIntelHexException(CIntelHexException::E_FORMAT,row); // Address too large

            if (!ps)
                prog_start = (int)addr;
            break;

        case 4:  // 1.3.3 used to set new address bank
            if (theApp.m_global.m_bProc6502 != 2)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            if (cnt != 2)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            if (addr)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            addr= geth(ptr, sum);
            addr <<= 8;
            addr += geth(ptr, sum);

            if (addr > 0xFF)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            geth(ptr, sum);//checksum byte

            if (sum) // checksum error
                throw CIntelHexException(CIntelHexException::E_CHKSUM, row);

            bank = (int)addr;
            break;

        case 5:  // 1.3.3 used to set new prog start for 24 bit addresses
            if (theApp.m_global.m_bProc6502!=2)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            if (cnt != 4)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            if (addr)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            addr = geth(ptr,sum);
            if (addr)
                throw CIntelHexException(CIntelHexException::E_FORMAT, row); // Unexpected data

            addr = geth(ptr,sum);
            addr <<= 8;
            addr += geth(ptr,sum);
            addr <<= 8;
            addr += geth(ptr,sum);
            
            geth(ptr, sum); // checksum byte
            if (sum) // checksum error
                throw CIntelHexException(CIntelHexException::E_CHKSUM, row);

            prog_start = (int)addr;
            ps = true;
            break;

        default: // Unknown value
            throw CIntelHexException(CIntelHexException::E_FORMAT, row);
        }
    }
#endif
}

//-----------------------------------------------------------------------------

/*virtual*/ bool CIntelHex::CIntelHexException::GetErrorMessage(char *errBuf,
        UINT nMaxError, UINT *pnHelpContext/*= NULL*/)
{
    UNUSED(errBuf);
    UNUSED(nMaxError);
    UNUSED(pnHelpContext);

#if 0
    std::string msg;
    char num[16];

    if (pnHelpContext != NULL)
        *pnHelpContext = 0;

    sprintf(num, "%u", row);

    switch (error)
    {
    case E_BAD_FORMAT: // Invalid file format
        msg.LoadString(IDS_INTEL_HEX_ERR_2);
        break;

    case E_CHKSUM: // Checksum error
        AfxFormatString1(msg, IDS_INTEL_HEX_ERR_1, num);
        break;

    case E_FORMAT: // Invalid data format
        AfxFormatString1(msg, IDS_INTEL_HEX_ERR_3, num);
        break;

    default:
        return false;
    }

    strncpy(errBuf, msg, nMaxError);
#endif

    return true;
}
