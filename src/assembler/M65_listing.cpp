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

// Assembler for M65XX and M65C02 microprocessors

//#include <ctime>
#include "StdAfx.h"

#include "M6502.h"


/*=======================================================================*/

CAsm6502::CListing::CListing(const char *fname)
    : CListing()
{
    if (fname && *fname)
    {
        m_fileName = fname;

        Open(fname);

        //% Bug fix 1.2.14.2 - bad listing file crashes system
        if (!m_file)
        {
#if REWRITE_TO_WX_WIDGET
            MessageBoxA(
                nullptr,
                "Listing file name or file path trouble.  No listing file will be generated.\n\nPlease go to Assembler Options to correct it.",
                "Warning",
                MB_OK);
#endif
        }
    }
}

/*=======================================================================*/

void CAsm6502::CListing::Remove()
{
    ASSERT(IsOpen()); // The file must be open

    // Probably should warn if we can't remove the file.
    std::remove(m_fileName.c_str());
}

void CAsm6502::CListing::NextLine()
{
    ASSERT(IsOpen()); // The file must be open

    if (m_lineNumber != 0)
    {
        // Looks like we're trying to play some games with line endings, we should let the OS handle this. -- B.Simonds (April 29, 2024)
#if 0

        if (m_str.Replace(0xd, '\n') == 0 && m_str.size() > 0 && m_str[m_str.size() - 1] != '\n')
            m_str += '\n';
#endif

        fputs(m_str.c_str(), m_file);
    }

    m_lineNumber++;

    //% Bug Fix 1.2.13.2 - remove extra space from list report

    char buf[32];
    snprintf(buf, sizeof(buf), "%05d  ", m_lineNumber);

    m_str = buf;
}

void CAsm6502::CListing::AddCodeBytes(uint32_t addr, int code1/*= -1*/, int code2/*= -1*/, int code3/*= -1*/, int code4/*= -1*/)
{
    ASSERT(IsOpen()); // The file must be open
    char buf[32];

    //% Bug Fix 1.2.13.2 - remove extra space from list report
    if (code4 != -1)
        snprintf(buf, sizeof(buf), "%06X  %02X %02X %02X %02X   ", (int)addr, (int)code1, code2, code3, code4);
    else if (code3 != -1)
        snprintf(buf, sizeof(buf), "%06X  %02X %02X %02X      ", (int)addr, (int)code1, code2, code3);
    else if (code2 != -1)
        snprintf(buf, sizeof(buf), "%06X  %02X %02X         ", (int)addr, (int)code1, code2);
    else if (code1 != -1)
        snprintf(buf, sizeof(buf), "%06X  %02X            ", (int)addr, (int)code1);
    else
        snprintf(buf, sizeof(buf), "%06X                ", (int)addr);

    m_str += buf;
}

void CAsm6502::CListing::AddValue(uint32_t val)
{
    ASSERT(IsOpen()); // The file must be open
    char buf[32];

    //% Bug Fix 1.2.13.2 - remove extra space from list report

    if (val > 0xFFFFFF)
        snprintf(buf, sizeof(buf), "  %08X          ", val);
    else if (val > 0xFFFF)
        snprintf(buf, sizeof(buf), "  %06X            ", val);
    else
        snprintf(buf, sizeof(buf), "  %04X              ", val);

    m_str += buf;
}

void CAsm6502::CListing::AddBytes(uint32_t addr, uint16_t mask, const uint8_t mem[], int len)
{
    ASSERT(IsOpen()); // The file must be open
    ASSERT(len > 0);

    wxString buf;

    for (int i = 0; i < len; i += 4)
    {
        switch ((len - i) % 4)
        {
            //% Bug Fix 1.2.13.2 - remove extra space from list report
        case 1:
            buf.Printf("%06X  %02X        ", int(addr), int(mem[addr & mask]));
            break;

        case 2:
            buf.Printf("%06X  %02X %02X     ", int(addr), int(mem[addr & mask]),
                int(mem[(addr + 1) & mask]));
            break;

        case 3:
            buf.Printf("%06X  %02X %02X %02X  ", int(addr), int(mem[addr & mask]),
                int(mem[(addr + 1) & mask]), int(mem[(addr + 2) & mask]));
            break;

        case 0:
            buf.Printf("%06X  %02X %02X %02X %02X  ", int(addr), int(mem[addr & mask]),
                int(mem[(addr + 1) & mask]), int(mem[(addr + 2) & mask]), int(mem[(addr + 3) & mask]));
            break;

        default:
            ASSERT(false);
            return;
        }

        m_str += buf;
        addr = (addr + 4) & mask;
        NextLine();
    }
}

void CAsm6502::CListing::AddSourceLine(const std::string &line)
{
    ASSERT(IsOpen()); // The file must be open
    m_str += line;
}

/*=======================================================================*/