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
#include "Deasm6502Doc.h"

#if 0
#include "IntelHex.h"
#include "MotorolaSRecord.h"
#include "AtariBin.h"
#include "Code65p.h"
#endif

CAsm::Breakpoint CGlobal::SetBreakpoint(int line, const std::string &doc_title)
{
    CAsm::FileUID fuid = m_debugInfo.GetFileUID(doc_title);
    return m_debugInfo.ToggleBreakpoint(line, fuid); // set/delete breakpoint
}

CAsm::Breakpoint CGlobal::GetBreakpoint(int line, const std::string &doc_title)
{
    CAsm::FileUID fuid = m_debugInfo.GetFileUID(doc_title);
    return m_debugInfo.GetBreakpoint(line, fuid); // set/delete breakpoint
}

CAsm::Breakpoint CGlobal::ModifyBreakpoint(int line, const std::string &doc_title, CAsm::Breakpoint bp)
{
    CAsm::FileUID fuid = m_debugInfo.GetFileUID(doc_title);
    return m_debugInfo.ModifyBreakpoint(line, fuid, bp); // Breakpoint setting
}

void CGlobal::ClrBreakpoint(int line, const std::string &doc_title)
{
    CAsm::FileUID fuid = m_debugInfo.GetFileUID(doc_title);
    m_debugInfo.ClrBreakpoint(line, fuid); // Clear the breakpoint
}

CAsm::DbgFlag CGlobal::GetLineDebugFlags(int line, const std::string &doc_title)
{
    CAsm::FileUID fuid = m_debugInfo.GetFileUID(doc_title); //File ID
    CDebugLine dl;
    m_debugInfo.GetAddress(dl, line, fuid); // Find the address corresponding to the line
    return (CAsm::DbgFlag)dl.flags; // Flags describing the program line
}

uint32_t CGlobal::GetLineCodeAddr(int line, const std::string &doc_title)
{
    CAsm::FileUID fuid = m_debugInfo.GetFileUID(doc_title); // File ID
    CDebugLine dl;
    m_debugInfo.GetAddress(dl, line, fuid); // Find the address corresponding to the line
    return dl.addr;
}

bool CGlobal::SetTempExecBreakpoint(int line, const std::string &doc_title)
{
    CAsm::FileUID fuid = m_debugInfo.GetFileUID(doc_title); // File ID
    CDebugLine dl;
    m_debugInfo.GetAddress(dl, line, fuid); // Find the address corresponding to the line

    if (dl.flags == CAsm::DBG_EMPTY || (dl.flags & CAsm::DBG_MACRO))
        return false; // There is no code in line 'line'

    m_debugInfo.SetTemporaryExecBreakpoint(dl.addr);

    return true;
}

bool CGlobal::CreateDeasm()
{
    ASSERT(m_simulator != nullptr);

#if 0

    CDeasm6502Doc *pDoc= (CDeasm6502Doc*)wxGetApp().m_pDocDeasmTemplate->OpenDocumentFile(nullptr);

    if (pDoc == nullptr)
        return false;
    /*
      pDoc->SetContext(m_pSym6502->GetContext());
      pDoc->SetStart(m_pSym6502->get_pc());
    */
    if (m_bBank)
        pDoc->SetPointer(m_pSym6502->get_pc() + (m_bPBR << 16));
    else
        pDoc->SetPointer(m_pSym6502->get_pc());

#endif

    return true;
}

//-----------------------------------------------------------------------------

void CGlobal::StartDebug()
{
    //if (wxGetApp().m_global.m_procType == ProcessorType::WDC65816) // 1.3.3 disable debugger for 65816
    //    return;

    bool restart;

    if (m_simulator == nullptr)
    {
        restart = false;
        m_simulator = new CSym6502(m_memory, &m_debugInfo, m_busWidth);
    }
    else
    {
        restart = true;
        m_simulator->Restart(m_memory);
    }

    m_simulator->finish = m_simFinish;
    m_simulator->SymStart(m_startAddress);

    /*
      struct { const std::wstring *pStr, const CContext *pCtx } data;
      std::wstring str= m_pSym6502->GetStatMsg(stat);
      data.pStr = &str;
      data.pCtx = m_pSym6502->GetContext();
    */

    Broadcast::ToViews(EVT_START_DEBUGGER, 0, 0);
    Broadcast::ToPopups(EVT_START_DEBUGGER, (WPARAM)restart, 0);
}

/*************************************************************************/

void CGlobal::ExitDebugger()
{
    if (m_simulator == nullptr)
        return;

    Broadcast::ToViews(EVT_EXIT_DEBUGGER, 0, 0);
    Broadcast::ToPopups(EVT_EXIT_DEBUGGER, 0, 0);

    delete m_simulator;
    m_simulator = nullptr;
}

/*************************************************************************/

void CGlobal::SaveCode(CArchive &archive, uint32_t start, uint32_t end, int info)
{
    UNUSED(archive);
    UNUSED(start);
    UNUSED(end);
    UNUSED(info);

#if 0
    //ASSERT(m_bCodePresent);

    switch (info)
    {
    case 0: // Intel-HEX format of the result code (*.65h/*.hex)
    {
        CIntelHex hex;
        hex.SaveHexFormat(archive, m_Mem, m_MarkArea, m_uOrigin);
        break;
    }

    case 1: // Motorola s-record format of the object code (*.65m/*.s9)
    {
        CMotorolaSRecord srec;
        srec.SaveHexFormat(archive, m_Mem, m_MarkArea, m_uOrigin);
        break;
    }

    case 2: // binary image of the object code (*.bin/*.65b)
        //m_Mem.Save(archive, start, end);
        break;

    case 3: // resulting program (*.65p)
        CCode65p code;
        if (m_MarkArea.GetSize() == 0)
            code.SaveCode65p(archive, m_Mem, start, end);
        else
            code.SaveCode65p(archive, m_Mem, m_MarkArea, m_uOrigin);
        break;

    default:
        ASSERT(false);
        break;
    }
#endif
}

void CGlobal::LoadCode(const LoadCodeState &state)
{
    m_memory = *state.Memory;

    SetCodePresence(true);

    uint32_t start = state.StartAddress;

    if (start == CAsm::INVALID_ADDRESS)
    {
        // TODO: This should be in the simulator and not here. -- B.Simonds (July 4, 2024)
        uint32_t vector = m_simulator->getVectorAddress(CSym6502::Vector::RESET);
        start = m_memory.getWord(vector);
    }

    SetStart(start);

    StartDebug();

    Broadcast::ToViews(EVT_PROG_MEM_CHANGED, (WPARAM)start, 0);
    Broadcast::ToPopups(EVT_PROG_MEM_CHANGED, (WPARAM)start, 0);
}

void CGlobal::LoadCode(CArchive &archive, uint32_t start, uint32_t end, int info, int nClear/*= 0*/)
{
    UNUSED(archive);
    UNUSED(start);
    UNUSED(end);
    UNUSED(info);
    UNUSED(nClear);

#if 0
    COutputMem mem; // Memory for the loaded program
    int prog_start = -1;

    if (nClear != -1)
        mem.ClearMem(nClear);
    else
        mem = m_ProgMem;

    switch (info)
    {
    case 0: // Intel-HEX format of the result code (*.65h/*.hex)
    {
        CIntelHex hex;
        hex.LoadHexFormat(archive, mem, m_MarkArea, prog_start);
        break;
    }

    case 1: // Motorola s-record format of the object code (*.65m/*.s9)
    {
        CMotorolaSRecord srec;
        srec.LoadHexFormat(archive, mem, m_MarkArea, prog_start);
        break;
    }

    case 2: // binary image of the result code (*.bin/*.65b)
        mem.Load(archive, start, end);
        break;

    case 3: // result program (*.65p)
        CCode65p code;
        code.LoadCode65p(archive, mem);
    break; // 1.3.3 added break - was missing and causing read past end of file error

    case 4: // Atari binary program
    {
        CAtariBin bin;
        bin.LoadAtaBinFormat(archive, mem, m_MarkArea, prog_start);
    }
    break;

    default:
        ASSERT(false);
    }

    m_ProgMem = mem; // program loaded successfully
    SetCodePresence(true);
    SetStart(prog_start != -1 ? prog_start : start);
    StartDebug();

    Broadcast::ToViews(EVT_PROG_MEM_CHANGED, (WPARAM)start, 0);
    Broadcast::ToPopups(EVT_PROG_MEM_CHANGED, (WPARAM)start, 0);
#endif
}
