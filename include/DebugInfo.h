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

#include "MapFile.h"
#include "Asm.h"
#include "Ident.h"

#ifndef _debug_info_h_
#define _debug_info_h_

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

    int operator ==(const CLine &arg) const
    {
        return ln == arg.ln && file == arg.file;
    }
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

// Info for the simulator with one line of program source
struct CDebugLine
{
    uint8_t flags; // Flags describing the line (Dbg Flag)
    uint32_t addr; // Program address 6502
    CLine line;

    CDebugLine()
        : flags(CAsm::DBG_EMPTY)
        , addr(0)
    {
    }

    CDebugLine(int ln, CAsm::FileUID uid, uint32_t addr, int flg)
        : flags((uint8_t)flg)
        , addr(addr)
        , line(ln, uid)
    {
    }

    CDebugLine(const CDebugLine &src)
    {
        memcpy(this, &src, sizeof(*this));
    }

    const CDebugLine &operator=(const CDebugLine &src)
    {
        memcpy(this, &src, sizeof(*this));
        return *this;
    }
};

class CDebugLines : std::vector<CDebugLine>
{
private:
    // Associative arrays for quick finding an address or line
    std::unordered_map<uint32_t, int> addr_to_idx;
    std::unordered_map<CLine, int> line_to_idx;

public:
    CDebugLines() 
        : addr_to_idx(50)
        , line_to_idx(50)
    {
        reserve(50);
    }

    // Finding the line corresponding to the address
    void GetLine(CDebugLine &ret, uint32_t addr)
    {
        static const CDebugLine empty; // empty object -to mark "line not found"

        auto search = addr_to_idx.find(addr);

        if (search != addr_to_idx.end())
            ret = at(search->second);

        ret = empty;
    }

    // Finding the address corresponding to the line
    void GetAddress(CDebugLine &ret, int ln, CAsm::FileUID file)
    {
        static const CDebugLine empty; // empty object -to mark "address not found"

        auto search = line_to_idx.find(CLine(ln, file));

        if (search != line_to_idx.end())
            ret = at(search->second);

        ret = empty;
    }

    void AddLine(CDebugLine &dl)
    {
        ASSERT(dl.flags != DBG_EMPTY); // Unfilled line description

        int idx = size();
        push_back(dl); // Adding information about the line, remembering the index

        addr_to_idx[dl.addr] = idx; // Saving the index
        line_to_idx[dl.line] = idx; // As above
    }

    void Empty()
    {
        clear();

        addr_to_idx.clear();
        line_to_idx.clear();
    }
};

// Tracks locations of breakpoints
class CDebugBreakpoints
{
private:
    const size_t NUM_ELEMENTS = 0x10000;

    uint8_t *m_data;

    uint32_t temp_bp_index;

public:
    CDebugBreakpoints()
        : m_data(nullptr)
        , temp_bp_index(0)
    {
        m_data = new uint8_t[NUM_ELEMENTS];
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

class CDebugInfo : CAsm
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

    void GetLine(CDebugLine &ret, uint32_t addr) // Finding the line corresponding to the address
    {
        m_lines.GetLine(ret,addr);
    }

    void GetAddress(CDebugLine &ret, int ln, FileUID file) // Finding the address corresponding to the line
    {
        m_lines.GetAddress(ret,ln,file);
    }

    Breakpoint SetBreakpoint(int line, FileUID file, int bp= BPT_NONE); // Interrupt setting
    Breakpoint ToggleBreakpoint(int line, FileUID file);
    Breakpoint GetBreakpoint(int line, FileUID file);
    Breakpoint ModifyBreakpoint(int line, FileUID file, int bp);
    void ClrBreakpoint(int line, FileUID file);

    Breakpoint GetBreakpoint(uint32_t addr)
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

    FileUID GetFileUID(const std::string &doc_title)
    {
        return m_map_file.GetFileUID(doc_title); // File ID
    }

    const std::string GetFilePath(FileUID fuid)
    {
        return fuid ? m_map_file.GetPath(fuid) : NULL; // name (path to) file
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


#endif
