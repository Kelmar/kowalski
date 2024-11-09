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

// Deasm6502Doc.h : header file
//

#ifndef DEASM_6502_DOC_H__
#define DEASM_6502_DOC_H__

#include "Asm.h"

class CContext;

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502Doc document
class CDeasm6502Doc : public wxDocument //, CAsm
{
private:
    // Variables used when saving program disassembly
    uint32_t m_uStart;
    uint32_t m_uEnd; 
    uint32_t m_uLength;
    bool m_bSaveAsData;

public:
    virtual ~CDeasm6502Doc() { }

protected:
    /* constructor */ CDeasm6502Doc(); // protected constructor used by dynamic creation

public:
    uint32_t m_uStartAddr;
    uint32_t m_nPointerAddr; // pointer address by arrow (->) or CAsm::INVALID_ADDRESS

    bool GetSaveOptions();
    void DeassembleSave(std::ostream &stream, const CContext &ctx, uint32_t start, uint32_t end, int opt);
    
    void SetStart(uint32_t addr, bool draw = true);
    void SetPointer(uint32_t addr, bool scroll = false);

    bool OnNewDocument() override;
    std::ostream &SaveObject(std::ostream &stream) override;
};

#endif /* DEASM_6502_DOC_H__ */
