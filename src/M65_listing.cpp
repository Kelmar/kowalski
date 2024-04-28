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

#include "resource.h"
#include "MarkArea.h"
#include "IOWindow.h"	// this is sloppy, but right now there's no mechanism to let framework know about requested new terminal wnd size

//=============================================================================

void CAsm6502::CListing::Remove()
{
    ASSERT(m_nLine != -1);	// plik musi by� otwarty
    try
    {
        m_File.Remove(m_File.GetFilePath());
    }
    catch (CFileException *)
    {
    }
}

void CAsm6502::CListing::NextLine()
{
    ASSERT(m_nLine != -1);	// plik musi by� otwarty
    if (m_nLine != 0)
    {
        if (m_Str.Replace(0xd, '\n') == 0 && m_Str.GetLength() > 0 && m_Str[m_Str.GetLength() - 1] != '\n')
            m_Str += '\n';
        m_File.WriteString(m_Str);
    }
    m_nLine++;
    //% Bug Fix 1.2.13.2 - remove extra space from list report
    //m_Str.Format(_T("%05d    "), m_nLine);
    m_Str.Format(_T("%05d  "), m_nLine);
}

void CAsm6502::CListing::AddCodeBytes(uint32_t addr, int code1/*= -1*/, int code2/*= -1*/, int code3/*= -1*/, int code4/*= -1*/)
{
    ASSERT(m_nLine != -1);	// plik musi by� otwarty
    char buf[32];

    //% Bug Fix 1.2.13.2 - remove extra space from list report
    if (code4 != -1)
        //	wsprintf(buf,_T("%04X  %02X %02X %02X     "),(int)addr,(int)code1,code2,code3);
        wsprintf(buf,_T("%06X  %02X %02X %02X %02X   "),(int)addr,(int)code1,code2,code3,code4);
    else if (code3 != -1)
        //	wsprintf(buf,_T("%04X  %02X %02X %02X     "),(int)addr,(int)code1,code2,code3);
        wsprintf(buf,_T("%06X  %02X %02X %02X      "),(int)addr,(int)code1,code2,code3);
    else if (code2 != -1)
        //	wsprintf(buf,_T("%04X  %02X %02X        "),(int)addr,(int)code1,code2);
        wsprintf(buf,_T("%06X  %02X %02X         "),(int)addr,(int)code1,code2);
    else if (code1 != -1)
        //	wsprintf(buf,_T("%04X  %02X           "),(int)addr,(int)code1);
        wsprintf(buf,_T("%06X  %02X            "),(int)addr,(int)code1);
    else
        //	wsprintf(buf,_T("%04X               "),(int)addr);
        wsprintf(buf,_T("%06X                "),(int)addr);
    m_Str += buf;
}

void CAsm6502::CListing::AddValue(uint32_t val)
{
    ASSERT(m_nLine != -1);	// plik musi by� otwarty
    char buf[32];
    //% Bug Fix 1.2.13.2 - remove extra space from list report
    //wsprintf(buf,_T("  %04X             "),val);
    if (val > 0xFFFFFF)
        wsprintf(buf,_T("  %08X          "),val);
    else if (val > 0xFFFF)
        wsprintf(buf,_T("  %06X            "),val);
    else
        wsprintf(buf,_T("  %04X              "),val);
    m_Str += buf;
}

void CAsm6502::CListing::AddBytes(uint32_t addr, uint16_t mask, const uint8_t mem[], int len)
{
    ASSERT(m_nLine != -1);	// plik musi by� otwarty
    ASSERT(len > 0);
    char buf[32];
    for (int i=0; i<len; i+=4)
    {
        switch ((len-i) % 4)
        {
        //% Bug Fix 1.2.13.2 - remove extra space from list report
        case 1:
            //	wsprintf(buf,_T("%04X  %02X           "),int(addr),int(mem[addr & mask]));
            wsprintf(buf,_T("%06X  %02X        "),int(addr),int(mem[addr & mask]));
            break;
        case 2:
            //	wsprintf(buf,_T("%04X  %02X %02X        "),int(addr),int(mem[addr & mask]),
            wsprintf(buf,_T("%06X  %02X %02X     "),int(addr),int(mem[addr & mask]),
                     int(mem[addr+1 & mask]));
            break;
        case 3:
            //	wsprintf(buf,_T("%04X  %02X %02X %02X     "),int(addr),int(mem[addr & mask]),
            wsprintf(buf,_T("%06X  %02X %02X %02X  "),int(addr),int(mem[addr & mask]),
                     int(mem[addr+1 & mask]),int(mem[addr+2 & mask]));
            break;
        case 0:
            wsprintf(buf,_T("%06X  %02X %02X %02X %02X  "),int(addr),int(mem[addr & mask]),
                     int(mem[addr+1 & mask]),int(mem[addr+2 & mask]),int(mem[addr+3 & mask]));
            break;
        }
        m_Str += buf;
        addr = addr+4 & mask;
        NextLine();
    }
}

void CAsm6502::CListing::AddSourceLine(const char *line)
{
    ASSERT(m_nLine != -1);	// plik musi by� otwarty
    m_Str += line;
}

CAsm6502::CListing::CListing(const char *fname)
{
    if (fname && *fname)
    {
        Open(fname);
        //% Bug fix 1.2.14.2 - bad listing file crashes system
        if (m_nLine)
            m_nLine = 0;
        else
        {
            m_nLine = -1;
            CString cs;
            cs.Format("Listing file name or file path trouble.  No listing file will be generated.\n\nPlease go to Assembler Options to correct it.");
            MessageBoxA(NULL, cs, "Warning", MB_OK );
        }
    }
    else
        m_nLine = -1;
}
