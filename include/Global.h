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
#include "sim.h"

#include "MarkArea.h"
#include "IntGenerator.h"
#include "LoadCodeOptions.h"

class CGlobal
{
private:
    friend class SimulatorController;

    bool m_isCodePresent;     // true -> after successful assembly
    COutputMem m_memory;      // memory written in the assembly process
    CDebugInfo m_debugInfo;   // startup information for the simulator

    /// Start address override
    sim_addr_t m_startAddress;
    
    /// I/O address override
    sim_addr_t m_ioAddress;

    CAsm::Finish m_simFinish; // how the simulator ends the program
    CMarkArea m_markArea;     // designation of fragments of memory containing the object code

    PSym6502 Simulator() const;

public:
    bool m_bBank; // Flag for member above bank 0 for deasm view
    uint8_t m_bPBR; // PBR register for deasm view
    uint16_t m_bSRef; // Stack pointer reference
    
    uint8_t m_bHelpFile;            // ^^ help file type
    bool m_bGenerateListing;        // generate listing during assembly?
    std::string m_strListingFile;   // listing file
    CIntGenerator m_IntGenerator;   // interrupt request generator data

    CGlobal()
        : m_isCodePresent(false)
        , m_memory()
        , m_debugInfo()
        , m_startAddress(sim::INVALID_ADDRESS)
        , m_ioAddress(sim::INVALID_ADDRESS)
        , m_simFinish(CAsm::Finish::FIN_BY_BRK)
        , m_markArea()
    {
    }

    ~CGlobal()
    {
    }

    CDebugInfo *GetDebug()
    {
        return &m_debugInfo;
    }

    CMarkArea *GetMarkArea()
    {
        return &m_markArea;
    }

    COutputMem &GetMemory() { return m_memory; }

    const COutputMem &GetMemory() const { return m_memory; }

    uint32_t GetStartAddr() // Beginning of the program
    {
        return m_startAddress;
    }

    void ioAddress(sim_addr_t address)
    {
        m_ioAddress = address;
    }

    sim_addr_t ioAddress() const
    {
        // TODO: Load value from config
        return (m_ioAddress == sim::INVALID_ADDRESS) ? 0xE000 : m_ioAddress;
    }

    bool IsCodePresent()
    {
        return m_isCodePresent;
    }

    bool IsDebugInfoPresent()
    {
        return m_isCodePresent; // to improve
    }

    void SetCodePresence(bool present)
    {
        m_isCodePresent = present;
    }

    void SetStart(sim_addr_t address)
    {
        m_startAddress = address;
    }

    std::string GetStatMsg()
    {
        return Simulator()->GetLastStatMsg();
    }

    CAsm::Finish GetSymFinish()
    {
        ASSERT(!Simulator() || Simulator()->finish == m_simFinish);
        return m_simFinish;
    }

    void SetSymFinish(CAsm::Finish fin)
    {
        m_simFinish = fin;

        if (Simulator())
            Simulator()->finish = fin;
    }

    CAsm::Breakpoint SetBreakpoint(int line, const std::string &doc_title);
    CAsm::Breakpoint GetBreakpoint(int line, const std::string &doc_title);
    CAsm::Breakpoint ModifyBreakpoint(int line, const std::string &doc_title, CAsm::Breakpoint bp);
    void ClrBreakpoint(int line, const std::string &doc_title);
    CAsm::DbgFlag GetLineDebugFlags(int line, const std::string &doc_title);
    sim_addr_t GetLineCodeAddr(int line, const std::string &doc_title);
    void SetTempExecBreakpoint(sim_addr_t address);

    //---------------------------------------------------------------------------

    ProcessorType GetProcType() const;

    //---------------------------------------------------------------------------

    bool CreateDeasm(); // New disassembler window

    CAsm::Breakpoint GetBreakpoint(uint32_t addr) // Get the interrupt at the given address
    {
        return m_debugInfo.GetBreakpoint(addr);
    }

    //---------------------------------------------------------------------------

    void LoadCode(const LoadCodeState &state);
};

#endif /* GLOBAL_H__ */
