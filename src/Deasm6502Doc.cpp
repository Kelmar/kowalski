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

// Deasm6502Doc.cpp : implementation file
//

#include "StdAfx.h"
//#include "6502.h"
#include "Deasm6502Doc.h"
#include "DeasmSaveOptions.h"
#include <locale.h>
#include "Deasm.h"

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502Doc

CDeasm6502Doc::CDeasm6502Doc()
{
    m_nPointerAddr = CAsm::INVALID_ADDRESS;

    m_uStart  = 0x0000;
    m_uEnd    = 0xFFFFFF;
    m_uLength = 0x1000000;
    m_bSaveAsData = false;
}

bool CDeasm6502Doc::OnNewDocument()
{
    if (!wxDocument::OnNewDocument())
        return false;

    SetStart(wxGetApp().m_global.GetStartAddr());
 
    return true;
}
/*************************************************************************/

bool CDeasm6502Doc::GetSaveOptions()
{
    // Probably shouldn't be asking for this information here. -- B.Simonds (April 27, 2024)

    CDeasmSaveOptions dlg;
    
    dlg.m_uStart  = m_uStart;
    dlg.m_uEnd    = m_uEnd;
    dlg.m_uLength = m_uLength;
    dlg.m_bSaveAsData = m_bSaveAsData;

    if (dlg.ShowModal() == wxID_OK) // Disassembly options
    {
        m_uStart  = dlg.m_uStart;
        m_uEnd    = dlg.m_uEnd;
        m_uLength = dlg.m_uLength;
        m_bSaveAsData = dlg.m_bSaveAsData;

        return true;
    }
    
    return false; // Abort save
}

/*************************************************************************/

std::ostream &CDeasm6502Doc::SaveObject(std::ostream &stream)
{
    if (!GetSaveOptions())
        return stream; // Abort save

    CContext &ctx = wxGetApp().m_global.GetSimulator()->GetContext();
    DeassembleSave(stream, ctx, m_uStart, m_uEnd, 0);

    return stream;
}

/*************************************************************************/

void CDeasm6502Doc::SetStart(uint32_t addr, bool bDraw)
{
    UNUSED(bDraw);
    m_uStartAddr = addr;
}

// Drawing/erasing an arrow in the line
void CDeasm6502Doc::SetPointer(uint32_t addr, bool scroll/*= FALSE*/)
{
    ASSERT((addr == CAsm::INVALID_ADDRESS) || (addr <= 0xFFFF));
    UNUSED(scroll);

    m_nPointerAddr = addr;

    UpdateAllViews(); // Redrawing of indicators
}

/*************************************************************************/

void CDeasm6502Doc::DeassembleSave(std::ostream &stream, const CContext &ctx, uint32_t start, uint32_t end, int opt)
{
    UNUSED(stream);
    UNUSED(ctx);
    UNUSED(start);
    UNUSED(end);
    UNUSED(opt);

#if 0
    COutputMem info;
    info.ClearMem();
    enum Flag { NONE = 0, ZPG = 1, ABS = 2, TXT = 4, CODE = 8 };

    UINT ptr;

    for (ptr = start; ptr < end; )
    {
        UINT8 cmd= ctx.mem[ptr];

        switch (CodeToMode()[cmd]) // Phase of selecting used addresses
        {
        case A_IMP:   // implied
        case A_ACC:   // accumulator
        case A_IMP2:  // implied dla BRK
        case A_ILL:   // illegal
            break;

        case A_IMM:   // immediate
        case A_IMM2:  // immediate
            break;

        case A_ZPG:    // zero page
        case A_ZPG_X:  // zero page indexed X
        case A_ZPG_Y:  // zero page indexed Y
        case A_ZPGI:   // zero page indirect
        case A_ZPGI_X: // zero page indirect, indexed X
        case A_ZPGI_Y: // zero page indirect, indexed Y
        case A_ZPG2:   // zero page dla rozkaz�w RMB SMB z 6501
            info[ctx.mem[ptr+1 & 0xFFFF]] |= ZPG;
            break;

        case A_ABS:	   // absolute
        case A_ABS_X:  // absolute indexed X
        case A_ABS_Y:  // absolute indexed Y
        case A_ABSI:   // absolute indirect
        case A_ABSI_X: // absolute indirect: indexed X
        {
            uint16_t addr = (uint16_t)ctx.mem[ptr+2 & 0xFFFF] << uint16_t(8);
            addr += ctx.mem[ptr + 1 & 0xFFFF];
            if (addr >= start && addr <= end)
                info[addr] |= TXT;
            else
                info[addr] |= ABS;
            break;
        }

        case A_REL: // relative
        {
            uint8_t arg = ctx.mem[ptr + 1 & 0xFFFF];
            uint16_T addr = arg & 0x80 ? ptr + 2 - (0x100 - arg) : ptr + 2 + arg;
            info[addr] |= TXT;
            break;
        }

        case A_ZREL: // zero page / relative -> BBS i BBR z 6501
        {
            uint8_t arg = ctx.mem[ptr + 1 & 0xFFFF];
            info[ctx.mem[ptr + 1 & 0xFFFF]] |= ZPG;
            info[uint16_t((arg & 0x80) ? ptr + 3 - (0x100 - arg) : ptr + 3 + arg) ] |= TXT;
            break;
        }

        default:
            ASSERT(false);
            break;
        }

        info[ptr] |= CODE;
        ptr += cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];
    }

    setlocale(LC_ALL, "");
    wxString str;
    std::string strFormat;

    strFormat.LoadString(IDS_DISASM_FORMAT1);
    str.Printf(strFormat, int(start), int(end), CTime::GetCurrentTime().Format("%x"));

//  str.Format("; Deasemblacja programu od $%04X do $%04X \t%s\r\n",(int)start,(int)end,CTime::GetCurrentTime().Format("%x"));
    ar.WriteString(str);

    if (!m_bSaveAsData)
    {
        strFormat.LoadString(IDS_DISASM_FORMAT2);
        ar.WriteString(strFormat);
        //  ar.WriteString("\r\n; oznaczenia etykiet:\r\n;   znn - adresy na str. zerowej\r\n"
        //    ";   annnn - adresy absolutne\r\n;   ennnn - adresy u�ywane w rozkazach skok�w wzgl�dnych\r\n\r\n");

        if (theApp.m_global.m_bProc6502 == 2)  // 1.3.3 support for 24-bit addressing
            str.Printf("\t.ORG $%06X\r\n", (int)start);
        else
            str.Printf("\t.ORG $%04X\r\n", (int)start);

        ar.WriteString(str);
    }

    if (!m_bSaveAsData)
    {
        for (int i = 0; i < 0xFFFF; i++)
        {
            if (info[i])
            {
                if (info[i] & ZPG)		// etykieta adresu na str. zerowej
                {
                    ASSERT(i<256);
                    str.Printf("z%02X\t= $%02X\r\n", i, i);
                    ar.WriteString(str);
                }

                if (info[i] & ABS)		// etykieta adresu absolutnego
                {
                    str.Printf("a%04X\t= $%04X\r\n", i, i);
                    ar.WriteString(str);
                }

                if ( (info[i] & TXT) && (info[i] & CODE)==0 )	// skok wzgl�dny
                {
                    str.Printf("e%04X\t= $%04X\r\n", i, i);
                    ar.WriteString(str);
                }
            }
        }
    }

    ar.WriteString("\r\n");

    CDeasm deasm;

    for (ptr = start; ptr < end; )
    {
        if (m_bSaveAsData)
        {
            str.Printf("e%06X:\t.DB $%02X, $%02X, $%02X, $%02X, $%02X, $%02X, $%02X, $%02X\r\n", int(ptr),
                       int(ctx.mem[ptr + 0]), int(ctx.mem[ptr + 1]), int(ctx.mem[ptr + 2]), int(ctx.mem[ptr + 3]),
                       int(ctx.mem[ptr + 4]), int(ctx.mem[ptr + 5]), int(ctx.mem[ptr + 6]), int(ctx.mem[ptr + 7]) );
            ar.WriteString(str);
            ptr += 8;
        }
        else
        {
            if (info[ptr] & TXT)
            {
                if (theApp.m_global.m_bProc6502 == 2)  // 1.3.3 support for 24-bit addressing
                    str.Printf("e%06X:\r\n", (int)ptr);
                else
                    str.Printf("e%04X:\r\n", (int)ptr);

                ar.WriteString(str);
            }

            if (CodeToCommand()[ctx.mem[ptr]] == C_ILL || ctx.mem[ptr] == 0) // Illegal order or BRK?
            {
                str.Printf("\t.DB $%02X\r\n", int(ctx.mem[ptr]));
                ar.WriteString(str);
            }
            else
            {
                uint32_t p = ptr;
                str = '\t';
                str += deasm.DeasmInstr(ctx, CAsm::DF_LABELS, p);
                ar.WriteString(str + "\r\n");
            }

            ptr += ctx.mem[ptr] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[ptr]]];
        }
    }

    ar.WriteString("\r\n");
    ar.WriteString("\t.END\r\n");
#endif
}

/*************************************************************************/
