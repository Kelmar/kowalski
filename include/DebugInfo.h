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

struct CLine : CAsm
{
    int ln;       // Line number in the source file
    FileUID file; // File ID

    CLine(int ln, FileUID file)
        : ln(ln), file(file)
    { }

    CLine()
        : ln(0), file(0)
    { }

    int operator ==(const CLine &arg) const
    {
        return ln == arg.ln && file == arg.file;
    }

    operator DWORD() // Conversion for hash function (CMap<>::HashKey())
    {
        return DWORD((ln << 4) ^ (file << 8));
    }
};

// Info for the simulator with one line of program source
struct CDebugLine : CAsm
{
    uint8_t flags; // Flags describing the line (Dbg Flag)
    uint32_t addr; // Program address 6502
    CLine line;

    CDebugLine() : flags(CAsm::DBG_EMPTY), addr(0)
    { }

    CDebugLine(int ln, FileUID uid, uint32_t addr, int flg)
        : flags((uint8_t)flg)
        , addr(addr)
        , line(ln,uid)
    { }

    CDebugLine(const CDebugLine &src)
    {
        memcpy(this,&src,sizeof(*this));
    }

    const CDebugLine &operator=(const CDebugLine &src)
    {
        memcpy(this, &src, sizeof(*this));
        return *this;
    }
};

class CDebugLines : CArray<CDebugLine,CDebugLine&>, public CAsm
{
private:
    CMap<uint32_t, uint32_t, int, int> addr_to_idx;	// tablice asocjacyjne do szybkiego
    CMap<CLine, CLine&, int, int> line_to_idx;	// odszukiwania adresu lub wiersza

public:
    CDebugLines() : addr_to_idx(50), line_to_idx(50)
    {
        SetSize(50,50);
    }

    // znalezienie wiersza odpowiadaj�cego adresowi
    void GetLine(CDebugLine &ret, uint32_t addr)
    {
        static const CDebugLine empty;	// pusty obiekt - do oznaczenia "nie znaleziony wiersz"
        int idx;
        if (addr_to_idx.Lookup(addr,idx))
            ret = GetAt(idx);
        else
            ret = empty;
    }

    // znalezienie adresu odp. wierszowi
    void GetAddress(CDebugLine &ret, int ln, FileUID file)
    {
        static const CDebugLine empty;	// pusty obiekt - do oznaczenia "nie znaleziony adres"
        int idx;
        if (line_to_idx.Lookup(CLine(ln,file),idx))
            ret = GetAt(idx);
        else
            ret = empty;
    }

    void AddLine(CDebugLine &dl)
    {
        ASSERT(dl.flags != DBG_EMPTY);	// niewype�niony opis wiersza
        int idx = Add(dl);			// dopisanie info o wierszu, zapami�tanie indeksu
        addr_to_idx.SetAt(dl.addr,idx);	// zapisanie indeksu
        line_to_idx.SetAt(dl.line,idx);	// j.w.
    }

    void Empty()
    {
        RemoveAll();
        addr_to_idx.RemoveAll();
        line_to_idx.RemoveAll();
    }
};

// Tracks locations of breakpoints
class CDebugBreakpoints : CAsm, CByteArray
{
private:
    uint32_t temp_bp_index;

public:
    CDebugBreakpoints() : temp_bp_index(0)
    {
        SetSize(0x10000);
    }

    Breakpoint Set(uint32_t addr, int bp = BPT_EXECUTE) // Set a breakpoint
    {
        ASSERT((bp & ~BPT_MASK) == 0); // Illegal combination of breakpoint bits
        return Breakpoint((*this)[addr] |= bp);
    }

    Breakpoint Clr(uint32_t addr, int bp = BPT_MASK) // Clear a breakpoint
    {
        ASSERT((bp & ~BPT_MASK) == 0); // Illegal combination of breakpoint bits
        return Breakpoint((*this)[addr] &= ~bp);
    }

    Breakpoint Toggle(uint32_t addr, int bp)
    {
        ASSERT((bp & ~BPT_MASK) == 0); // Illegal combination of breakpoint bits
        return Breakpoint((*this)[addr] &= ~bp);
    }

    Breakpoint Get(uint32_t addr)
    {
        return Breakpoint((*this)[addr]);
    }

    void Enable(uint32_t addr, bool enable= true)
    {
        ASSERT((*this)[addr] & BPT_MASK); // There is no breakpoint at the given address

        if (enable)
            (*this)[addr] &= ~BPT_DISABLED;
        else
            (*this)[addr] |= BPT_DISABLED;
    }

    void ClrBrkp(uint32_t addr) // clear the breakpoint
    {
        (*this)[addr] = BPT_NONE;
    }

    void SetTemporaryExec(uint32_t addr)
    {
        temp_bp_index = addr;
        (*this)[addr] |= BPT_TEMP_EXEC;
    }

    void RemoveTemporaryExec()
    {
        (*this)[temp_bp_index] &= ~BPT_TEMP_EXEC;
    }

    void ClearAll() // Remove all breakpoints
    {
        memset(m_pData, BPT_NONE, m_nSize * sizeof(uint8_t));
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

        m_name.size(size);
        m_info.size(size);
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

    const std::string &GetFilePath(FileUID fuid)
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
        m_idents.SetIdent(index,name,info);
    }

    void GetIdent(int index, std::string &name, CIdent &info)
    {
        m_idents.GetIdent(index,name,info);
    }

    int GetIdentCount()
    {
        return m_idents.GetCount();
    }
};


#endif
