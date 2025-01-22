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

#ifndef DEASM_6502_H__
#define DEASM_6502_H__

/*=======================================================================*/

#include "sim.h"

/*=======================================================================*/

class CSrc6502View;

struct CmdInfo	// single command info (for logging)
{
    CmdInfo(const CContext &m_ctx)
    {
        a = m_ctx.a;
        x = m_ctx.x;
        y = m_ctx.y;
        s = m_ctx.StackPointer();
        flags = m_ctx.GetStatus();

        pc = m_ctx.PC();

        cmd = m_ctx.bus.PeekByte(pc);
        arg1 = m_ctx.bus.PeekByte(pc + 1);
        arg2 = m_ctx.bus.PeekByte(pc + 2);
        arg3 = m_ctx.bus.PeekByte(pc + 3);

        uCycles = m_ctx.uCycles;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
        intFlag = m_ctx.intFlag;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
        argVal = 0;
    }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
    CmdInfo(uint16_t a, uint16_t x, uint16_t y, uint8_t s, uint8_t flags, uint8_t cmd, uint8_t arg1, uint8_t arg2, uint32_t pc)
        : a(a)
        , x(x)
        , y(y)
        , s(s)
        , flags(flags)
        , cmd(cmd)
        , arg1(arg1)
        , arg2(arg2)
        , arg3(0)
        , pc(pc)
        , uCycles(0)
        , intFlag(0)
        , argVal(0)
    { }

    CmdInfo() { }

    std::string Asm() const;

    uint16_t a;
    uint8_t b;
    uint16_t x;
    uint16_t y;
    uint16_t s;
    uint8_t flags;
    uint8_t cmd;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t arg3; //% 65816
    uint32_t pc;
    ULONG uCycles; //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
    bool intFlag;
    uint16_t argVal;
};

typedef CLogBuffer<CmdInfo> CommandLog;

/*=======================================================================*/

class CDeasm
{
private:
    static const char mnemonics[];

    std::string SetMemZPGInfo(uint8_t addr, uint8_t val);   // Cell description of page zero of memory
    std::string SetMemInfo(uint32_t addr, uint8_t val);     // Memory cell description
    std::string SetValInfo(uint8_t val);                    // Value description 'val'
    std::string SetWordInfo(uint16_t val);                  // Value description 'val'

    std::shared_ptr<CSym6502> m_sim;

public:
    /* constructor */ CDeasm(std::shared_ptr<CSym6502> sim)
        : m_sim(sim)
    {
    }

    virtual ~CDeasm()
    {
    }

    std::string DeasmInstr(CAsm::DeasmFmt flags, int32_t &ptr);
    std::string DeasmInstr(const CmdInfo &ci, CAsm::DeasmFmt flags);
    std::string ArgumentValue(uint32_t ptr = CAsm::INVALID_ADDRESS);

    std::string Mnemonic(uint8_t code, ProcessorType procType, bool bUseBrk = false) const;
    std::string Argument(uint8_t cmd, CAsm::CodeAdr mode, uint32_t addr, uint8_t arg1, uint8_t arg2, uint8_t arg3, bool bLabel = false, bool bHelp = false) const;

    int FindPrevAddr(uint32_t &addr, int cnt = 1);
    int FindNextAddr(uint32_t &addr, int cnt = 1);
    int FindDelta(uint32_t &addr, uint32_t dest, int max_lines);
};

/*=======================================================================*/

#endif /* DEASM_6502_H__ */

/*=======================================================================*/
