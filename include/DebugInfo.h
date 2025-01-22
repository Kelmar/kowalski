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

#ifndef DEBUG_INFO_6502_H__
#define DEBUG_INFO_6502_H__

/*=======================================================================*/

//#include "sim.h"

typedef uint32_t sim_addr_t;

#include "MapFile.h"
#include "Asm.h"
#include "Ident.h"

/*=======================================================================*/

struct CLine
{
    int ln;             // Line number in the source file
    CAsm::FileUID file; // File ID

    CLine(int ln, CAsm::FileUID file)
        : ln(ln), file(file)
    { }

    CLine()
        : ln(0), file(0)
    { }

    CLine(const CLine &rhs)
        : ln(rhs.ln)
        , file(rhs.file)
    {

    }

    CLine(CLine &&rhs)
        : ln(std::move(rhs.ln))
        , file(std::move(rhs.file))
    {
    }

    const CLine &operator =(const CLine &rhs)
    {
        ln = rhs.ln;
        file = rhs.file;

        return *this;
    }

    const CLine &operator =(CLine &&rhs)
    {
        ln = std::move(rhs.ln);
        file = std::move(rhs.file);

        return *this;
    }

    bool operator ==(const CLine &arg) const
    {
        return (ln == arg.ln) && (file == arg.file);
    }

    bool operator !=(const CLine &arg) const { return !(this->operator ==(arg)); }
};

template<>
struct std::hash<CLine>
{
    std::size_t operator()(const CLine &line) const noexcept
    {
        std::size_t h1 = std::hash<int> {} (line.ln);
        std::size_t h2 = std::hash<CAsm::FileUID> {} (line.file);

        return (h1 << 4) ^ (h2 << 8);
    }
};

/*=======================================================================*/

// Info for the simulator with one line of program source
struct CDebugLine
{
    uint8_t flags; // Flags describing the line (Dbg Flag)
    sim_addr_t addr; // Program address 6502
    CLine line;

    CDebugLine()
        : flags(CAsm::DBG_EMPTY)
        , addr(0)
    {
    }

    CDebugLine(int ln, CAsm::FileUID uid, sim_addr_t addr, int flg)
        : flags((uint8_t)flg)
        , addr(addr)
        , line(ln, uid)
    {
    }

    CDebugLine(const CDebugLine &src)
        : flags(src.flags)
        , addr(src.addr)
        , line(src.line)
    {
    }

    CDebugLine(CDebugLine &&src)
        : flags(std::move(src.flags))
        , addr(std::move(src.addr))
        , line(std::move(src.line))
    {
    }

    const CDebugLine &operator =(const CDebugLine &src)
    {
        flags = src.flags;
        addr = src.addr;
        line = src.line;

        return *this;
    }

    const CDebugLine &operator =(CDebugLine &&src)
    {
        flags = std::move(src.flags);
        addr = std::move(src.addr);
        line = std::move(src.line);

        return *this;
    }
};

/*=======================================================================*/

class CDebugLines
{
private:
    // Associative arrays for quick finding an address or line
    std::vector<CDebugLine> m_lines;
    std::unordered_map<uint32_t, int> addr_to_idx;
    std::unordered_map<CLine, int> line_to_idx;

public:
    CDebugLines() 
        : m_lines(50)
        , addr_to_idx(50)
        , line_to_idx(50)
    {
    }

    // Finding the line corresponding to the address
    CDebugLine GetLine(sim_addr_t addr)
    {
        static const CDebugLine empty; // empty object -to mark "line not found"

        auto search = addr_to_idx.find(addr);

        if (search != addr_to_idx.end())
            return m_lines.at(search->second);

        return empty;
    }

    // Finding the address corresponding to the line
    CDebugLine GetAddress(int ln, CAsm::FileUID file)
    {
        static const CDebugLine empty; // empty object -to mark "address not found"

        auto search = line_to_idx.find(CLine(ln, file));

        if (search != line_to_idx.end())
            return m_lines.at(search->second);

        return empty;
    }

    void AddLine(CDebugLine &dl)
    {
        ASSERT(dl.flags != CAsm::DBG_EMPTY); // Unfilled line description

        int idx = m_lines.size();
        m_lines.push_back(dl); // Adding information about the line, remembering the index

        addr_to_idx[dl.addr] = idx; // Saving the index
        line_to_idx[dl.line] = idx; // As above
    }

    void Empty()
    {
        m_lines.clear();

        addr_to_idx.clear();
        line_to_idx.clear();
    }
};

/*=======================================================================*/

// Tracks locations of breakpoints
class CDebugBreakpoints
{
private:
    const size_t NUM_ELEMENTS = 0x1000000;

    uint8_t *m_data;

    uint32_t temp_bp_index;

public:
    CDebugBreakpoints()
        : m_data(nullptr)
        , temp_bp_index(0)
    {
        m_data = new uint8_t[NUM_ELEMENTS];
        memset(m_data, 0, NUM_ELEMENTS * sizeof(uint8_t));
    }

    virtual ~CDebugBreakpoints()
    {
        delete[] m_data;
    }

    uint8_t operator[](uint32_t addr) const 
    {
        ASSERT(addr < NUM_ELEMENTS);
        return m_data[addr]; 
    }

    uint8_t &operator[](uint32_t addr) 
    { 
        ASSERT(addr < NUM_ELEMENTS);
        return m_data[addr];
    }

    CAsm::Breakpoint Set(uint32_t addr, int bp = CAsm::BPT_EXECUTE) // Set a breakpoint
    {
        ASSERT((bp & ~CAsm::BPT_MASK) == 0); // Illegal combination of breakpoint bits
        return static_cast<CAsm::Breakpoint>((*this)[addr] |= bp);
    }

    CAsm::Breakpoint Clr(uint32_t addr, int bp = CAsm::BPT_MASK) // Clear a breakpoint
    {
        ASSERT((bp & ~CAsm::BPT_MASK) == 0); // Illegal combination of breakpoint bits
        return static_cast<CAsm::Breakpoint>((*this)[addr] &= ~bp);
    }

    CAsm::Breakpoint Toggle(uint32_t addr, int bp)
    {
        ASSERT((bp & ~CAsm::BPT_MASK) == 0); // Illegal combination of breakpoint bits
        return static_cast<CAsm::Breakpoint>((*this)[addr] &= ~bp);
    }

    CAsm::Breakpoint Get(uint32_t addr)
    {
        return static_cast<CAsm::Breakpoint>((*this)[addr]);
    }

    void Enable(uint32_t addr, bool enable = true)
    {
        ASSERT((*this)[addr] & CAsm::BPT_MASK); // There is no breakpoint at the given address

        if (enable)
            (*this)[addr] &= ~CAsm::BPT_DISABLED;
        else
            (*this)[addr] |= CAsm::BPT_DISABLED;
    }

    void ClrBrkp(uint32_t addr) // clear the breakpoint
    {
        (*this)[addr] = CAsm::BPT_NONE;
    }

    void SetTemporaryExec(uint32_t addr)
    {
        temp_bp_index = addr;
        (*this)[addr] |= CAsm::BPT_TEMP_EXEC;
    }

    void RemoveTemporaryExec()
    {
        (*this)[temp_bp_index] &= ~CAsm::BPT_TEMP_EXEC;
    }

    void ClearAll() // Remove all breakpoints
    {
        memset(m_data, CAsm::BPT_NONE, NUM_ELEMENTS);
    }
};

/*=======================================================================*/

// Information about identifiers
class CDebugIdents
{
    std::vector<std::string> m_name;
    std::vector<CIdent> m_info;

    //CArray<CIdent, const CIdent&> m_info;

public:
    void SetArrSize(int size)
    {
        m_name.clear();
        m_info.clear();

        m_name.reserve(size);
        m_info.reserve(size);
    }
    
    void SetIdent(int index, const std::string &name, const CIdent &info)
    {
        m_name[index] = name;
        m_info[index] = info;
    }

    void GetIdent(int index, std::string &name, CIdent &info)
    {
        name = m_name[index];
        info = m_info[index];
    }

    int GetCount()
    {
        ASSERT(m_name.size() == m_info.size());
        return m_name.size();
    }

    void Empty()
    {
        m_name.clear();
        m_info.clear();
    }
};

/*=======================================================================*/

class CDebugInfo
{
    CDebugLines m_lines;             // Information about lines
    CDebugIdents m_idents;           // Information about identifiers
    CDebugBreakpoints m_breakpoints; // Information about places of interruptions
    CMapFile m_map_file;             // Mapping the phase of the source file to 'fuid' and vice versa

public:

    void Empty()
    {
        m_lines.Empty();
        m_idents.Empty();
    }

    void AddLine(CDebugLine &dl)
    {
        m_lines.AddLine(dl);
    }

    // Finding info that corresponds to the document and line number.
    CDebugLine GetLineInfo(CAsm::FileUID fileId, int lineNumber)
    {
        return m_lines.GetAddress(lineNumber, fileId);;
    }

    // Finding the line corresponding to the address
    CDebugLine GetLineInfo(sim_addr_t address)
    {
        return m_lines.GetLine(address);
    }

    CAsm::Breakpoint SetBreakpoint(int line, CAsm::FileUID file, int bp= CAsm::BPT_NONE); // Interrupt setting
    CAsm::Breakpoint ToggleBreakpoint(int line, CAsm::FileUID file);
    CAsm::Breakpoint GetBreakpoint(int line, CAsm::FileUID file);
    CAsm::Breakpoint ModifyBreakpoint(int line, CAsm::FileUID file, int bp);
    void ClrBreakpoint(int line, CAsm::FileUID file);

    CAsm::Breakpoint GetBreakpoint(uint32_t addr)
    {
        return m_breakpoints.Get(addr);
    }

    void SetTemporaryExecBreakpoint(uint32_t addr)
    {
        m_breakpoints.SetTemporaryExec(addr);
    }

    void RemoveTemporaryExecBreakpoint()
    {
        m_breakpoints.RemoveTemporaryExec();
    }

    CAsm::FileUID GetFileUID(const std::string &doc_title)
    {
        return m_map_file.GetFileUID(doc_title); // File ID
    }

    const std::string GetFilePath(CAsm::FileUID fuid)
    {
        return fuid ? m_map_file.GetPath(fuid) : ""; // name (path to) file
    }

    void ResetFileMap()
    {
        m_map_file.Reset();
    }

    void SetIdentArrSize(int size)
    {
        m_idents.SetArrSize(size);
    }

    void SetIdent(int index, const std::string &name, const CIdent &info)
    {
        m_idents.SetIdent(index, name, info);
    }

    void GetIdent(int index, std::string &name, CIdent &info)
    {
        m_idents.GetIdent(index, name, info);
    }

    int GetIdentCount()
    {
        return m_idents.GetCount();
    }
};

typedef std::shared_ptr<CDebugInfo> PDebugInfo;

/*=======================================================================*/

#endif /* DEBUG_INFO_6502_H__ */

/*=======================================================================*/
