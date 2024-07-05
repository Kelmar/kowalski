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

#ifndef GLOBAL_H__
#define GLOBAL_H__

#include "Broadcast.h"
#include "Asm.h"
#include "Sym6502.h"
#include "MarkArea.h"
#include "IntGenerator.h"
#include "LoadCodeOptions.h"

// Temporary place holder for now
class CArchive;

class CGlobal
{
private:
    UINT m_uAddrBusWidth;           // width of the address bus
    bool m_bCodePresent;            // true -> after successful assembly
    COutputMem m_ProgMem;           // memory written in the assembly process
    CDebugInfo m_Debug;             // startup information for the simulator
    uint32_t m_uOrigin;             // start of program 6502
    CSym6502 *m_pSym6502;           // simulator
    CAsm::Finish m_SymFinish;       // how the simulator ends the program
    CMarkArea m_MarkArea;           // designation of fragments of memory containing the object code

public:

    /// Selected processor type
    ProcessorType m_procType;
    
    uint8_t m_bHelpFile;            // ^^ help file type
    COutputMem m_Mem;               // memory for the object code and the simulator
    bool m_bGenerateListing;        // generate listing during assembly?
    std::string m_strListingFile;   // listing file
    CIntGenerator m_IntGenerator;   // interrupt request generator data

    CGlobal()
        : m_bCodePresent(false)
        , m_pSym6502(nullptr)
    {
        SetAddrBusWidth(16);
    }

    ~CGlobal()
    {
        delete m_pSym6502;
    }

    void SetAddrBusWidth(UINT w)
    {
        m_uAddrBusWidth = w;
        if (m_pSym6502)
            m_pSym6502->set_addr_bus_width(w);
    }

    CDebugInfo *GetDebug()
    {
        return &m_Debug;
    }

    COutputMem *GetMemForAsm() // Memory for the object code (assembly)
    {
        m_ProgMem.Clear();
        return &m_ProgMem;
    }

    COutputMem *GetMemForSym() // Memory with object code (simulator)
    {
        if (m_bCodePresent)
        {
            m_Mem = m_ProgMem;
            return &m_Mem;
        }
        return NULL;
    }

    CMarkArea *GetMarkArea()
    {
        return &m_MarkArea;
    }

    COutputMem *GetMem() // Memory with the result code
    {
        return &m_Mem;
    }

    uint32_t GetStartAddr() // Beginning of the program
    {
        return m_uOrigin;
    }

    bool IsCodePresent()
    {
        return m_bCodePresent;
    }

    bool IsDebugInfoPresent()
    {
        return m_bCodePresent; // to improve
    }

    bool IsDebugger()
    {
        return m_pSym6502 != NULL;
    }

    bool IsProgramRunning()
    {
        return m_pSym6502 ? m_pSym6502->IsRunning() : false;
    }

    bool IsProgramFinished()
    {
        return m_pSym6502 ? m_pSym6502->IsFinished() : false;
    }

    void SetCodePresence(bool present)
    {
        if (present)
            m_Mem = m_ProgMem;
        m_bCodePresent = present;
    }

    void StartDebug();

    void RestartProgram()
    {
        StartDebug();
    }

    void ExitDebugger();

    void SetStart(uint32_t prog_start)
    {
        m_uOrigin = prog_start;
    }

    CSym6502 *GetSimulator()
    {
        return m_pSym6502;
    }

    std::string GetStatMsg()
    {
        return m_pSym6502->GetLastStatMsg();
    }

    CAsm::Finish GetSymFinish()
    {
        ASSERT(m_pSym6502 == NULL || m_pSym6502->finish == m_SymFinish);
        return m_SymFinish;
    }

    void SetSymFinish(CAsm::Finish fin)
    {
        m_SymFinish = fin;
        if (m_pSym6502) m_pSym6502->finish = fin;
    }

    CAsm::Breakpoint SetBreakpoint(int line, const std::string &doc_title);
    CAsm::Breakpoint GetBreakpoint(int line, const std::string &doc_title);
    CAsm::Breakpoint ModifyBreakpoint(int line, const std::string &doc_title, CAsm::Breakpoint bp);
    void ClrBreakpoint(int line, const std::string &doc_title);
    CAsm::DbgFlag GetLineDebugFlags(int line, const std::string &doc_title);
    uint32_t GetLineCodeAddr(int line, const std::string &doc_title);
    bool SetTempExecBreakpoint(int line, const std::string &doc_title);

    void AbortProg()
    {
        if (m_pSym6502 != NULL)
            m_pSym6502->AbortProg();
    }

    //---------------------------------------------------------------------------

    ProcessorType GetProcType()
    {
        return m_procType;
    }

    void SetProcType(ProcessorType newType)
    {
        m_procType = newType;
    }

    uint8_t GetHelpType() //^^ Help
    {
        return m_bHelpFile;
    }

    void SetHelpType(uint8_t bHelp) //^^ Help
    {
        m_bHelpFile = bHelp;
    }

    //---------------------------------------------------------------------------

    bool CreateDeasm(); // New disassembler window

    CAsm::Breakpoint GetBreakpoint(uint32_t addr) // Get the interrupt at the given address
    {
        return m_Debug.GetBreakpoint(addr);
    }

    //---------------------------------------------------------------------------

    void SaveCode(CArchive &archive, uint32_t start, uint32_t end, int info);
    void LoadCode(CArchive &archive, uint32_t start, uint32_t end, int info, int nClear = 0);

    void LoadCode(const LoadCodeState &state);
};

#endif /* GLOBAL_H__ */
