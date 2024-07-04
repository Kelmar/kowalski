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

#ifndef _sym_6502_h_
#define _sym_6502_h_

#include "DebugInfo.h"
#include "OutputMem.h"
#include "LogBuffer.h"

class CSrc6502View;
#undef OVERFLOW

struct ContextBase
{
    uint8_t a, x, y, s;
    uint32_t pc;

    ULONG uCycles;
    bool intFlag;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers

    bool negative, overflow, zero, carry;
    bool reserved, break_bit, decimal, interrupt;

    uint32_t mem_mask; // memory mask - depends on the width of the address bus,
    // contains ones in place of valid address bits
    bool io;
    uint16_t io_from, io_to;

    ContextBase()
    {
        memset(this, 0, sizeof(ContextBase));
    }

    ContextBase(const ContextBase &rhs)
    {
        memcpy(this, &rhs, sizeof(ContextBase));
    }

    const ContextBase &operator =(const ContextBase &rhs)
    {
        memcpy(this, &rhs, sizeof(ContextBase));
        return *this;
    }
};

class CContext : public ContextBase
{
    void reset()
    {
        pc = 0;
        a = x = y = s = 0;
        io = false;
        negative = overflow = zero = carry = false;
        break_bit = decimal = interrupt = false;
        reserved = true;
        uCycles = 0;
    }

public:
    
    enum Flags
    {
        NEGATIVE  = 0x80,
        OVERFLOW  = 0x40,
        RESERVED  = 0x20,
        BREAK     = 0x10,
        DECIMAL   = 0x08,
        INTERRUPT = 0x04,
        ZERO      = 0x02,
        CARRY     = 0x01,
        NONE      = 0x00,
        ALL       = 0xFF,
        // bit numbers
        N_NEGATIVE  = 7,
        N_OVERFLOW  = 6,
        N_RESERVED  = 5,
        N_BREAK     = 4,
        N_DECIMAL   = 3,
        N_INTERRUPT = 2,
        N_ZERO      = 1,
        N_CARRY     = 0
    };

    COutputMem &mem;

    // functions that change the contents of the flag register
    void set_status_reg_VZNC(bool v, bool z, bool n, bool c)
    {
        negative = !!n;
        overflow = !!v;
        zero = !!z;
        carry = !!c;
    }
    
    void set_status_reg_ZNC(bool z, bool n, bool c)
    {
        negative = !!n;
        zero = !!z;
        carry = !!c;
    }

    void set_status_reg_VZN(bool v, bool z, bool n)
    {
        negative = !!n;
        overflow = !!v;
        zero = !!z;
    }

    void set_status_reg_ZN(bool z, bool n)
    {
        negative = !!n;
        zero = !!z;
    }

    void set_status_reg(uint8_t val)
    {
        zero = val == 0;
        negative = !!(val & 0x80);
    }

    uint8_t get_status_reg() const;
    void set_status_reg_bits(uint8_t reg);
    void set_addr_bus_width(uint32_t w)
    {
        mem_mask = uint32_t((1 << w) - 1);
        ASSERT(w >= 10 && w <= 16);
    }

    CContext(COutputMem &mem, int addr_bus_width) : mem(mem)
    {
        set_addr_bus_width(addr_bus_width);
        reset();
    }

    CContext(const CContext &src) 
        : ContextBase(src)
        , mem(src.mem)
    {
    }

    CContext &operator= (const CContext &src)
    {
        ContextBase::operator =(src);
        mem = src.mem;
        return *this;
    }

    void Reset(const COutputMem &newMem)
    {
        this->mem = newMem;
        reset();
    }
};


struct CmdInfo	// single command info (for logging)
{
    CmdInfo(const CContext& ctx)
    {
        a = ctx.a;
        x = ctx.x;
        y = ctx.y;
        s = ctx.s;
        flags = ctx.get_status_reg();
        pc = ctx.pc;
        cmd = ctx.mem[ctx.pc];
        arg1 = ctx.mem[ctx.pc + 1];
        arg2 = ctx.mem[ctx.pc + 2];
        arg3 = ctx.mem[ctx.pc + 3];
        uCycles = ctx.uCycles;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
        intFlag = ctx.intFlag;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
        argVal = 0;
    }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
    CmdInfo(uint8_t a, uint8_t x, uint8_t y, uint8_t s, uint8_t flags, uint8_t cmd, uint8_t arg1, uint8_t arg2, uint32_t pc)
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
    {
    }

    CmdInfo() {}

    std::string Asm() const;

    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t s;
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

//=============================================================================

class CSym6502 //: public CAsm::
{
    CContext ctx, pre, old; //% bug Fix 1.2.13.18 - command log assembly not lined up with registers (added pre)
    CDebugInfo *debug;
    CommandLog m_Log;

public:
    static int bus_width;
    static uint16_t io_addr; // the beginning of the simulator I/O area
    static bool io_enabled;
    static bool s_bWriteProtectArea;
    static uint16_t s_uProtectFromAddr;
    static uint16_t s_uProtectToAddr;

    enum IOFunc // functions of subsequent bytes from the simulator's I/O area
    {
        IO_NONE      = -1,
        TERMINAL_CLS = 0,
        TERMINAL_OUT,
        TERMINAL_OUT_CHR,
        TERMINAL_OUT_HEX,
        TERMINAL_IN,
        TERMINAL_GET_X_POS,
        TERMINAL_GET_Y_POS,
        TERMINAL_SET_X_POS,
        TERMINAL_SET_Y_POS,
        IO_LAST_FUNC = TERMINAL_SET_X_POS
    };

    // interrupt types
    enum IntType { NONE = 0, IRQ = 1, NMI = 2, RST = 4 };

private:
    IOFunc io_func;

    wxWindow *io_window(); // Finding the terminal window
    wxWindow *io_open_window(); // Opening a terminal window

    void inc_prog_counter(int step = 1)
    {
        ctx.pc = uint32_t(ctx.pc + step);
    }

    bool running;
    bool stop_prog;
    CAsm::SymStat fin_stat;
    int m_nInterruptTrigger;

    CAsm::SymStat perform_cmd();
    CAsm::SymStat skip_cmd(); // Skip the current statement
    CAsm::SymStat step_over();
    CAsm::SymStat run_till_ret();
    CAsm::SymStat run(bool animate= false);
    void interrupt(int& nInterrupt);    // interrupt requested: load pc
    CAsm::SymStat perform_step(bool animate);
    CAsm::SymStat perform_command();

    uint16_t get_argument_address(bool bWrite); // get current cmd argument address
    uint8_t get_argument_value(); // get current cmd argument value

    uint16_t get_word_indirect(uint32_t zp)
    {
        ASSERT(zp < 0xFF);

        uint16_t lo = ctx.mem[zp];
        uint16_t hi = ctx.mem[zp + 1];

        return lo | (hi << 8);
    }

    uint16_t get_word(uint32_t addr)
    {
        return ctx.mem.getWord(addr);
    }

    void push_on_stack(uint8_t arg)
    {
        ctx.mem[0x100 + ctx.s--] = arg;
    }

    void push_addr_on_stack(uint16_t arg)
    {
        ctx.mem[0x100 + ctx.s--] = (arg >> 8) & 0xFF;
        ctx.mem[0x100 + ctx.s--] = arg & 0xFF;
    }

    uint8_t pull_from_stack()
    {
        return ctx.mem[0x100 + ++ctx.s];
    }

    uint16_t pull_addr_from_stack()
    {
        uint16_t tmp = ctx.mem[0x100 + ++ctx.s];
        tmp |= uint16_t(ctx.mem[0x100 + ++ctx.s]) << uint16_t(8);
        return tmp;
    }

    uint16_t get_irq_addr();

    static UINT start_step_over_thread(void *ptr);
    static UINT start_run_thread(void *ptr);
    static UINT start_animate_thread(void *ptr);
    static UINT start_run_till_ret_thread(void *ptr);

    //HANDLE hThread;

    /*
     * Looks like drawing functions in here; thinking these need to be moved 
     * into whatever view that is actually showing the source code. 
     * 
     *          -- B.Simonds (April 25, 2024)
     */

    void SetPointer(const CLine &line, uint32_t addr);	// Placing an arrow (->) in front of the current line
    void SetPointer(CSrc6502View* pView, int nLine, bool bScroll); // helper fn
    void ResetPointer();			// Hiding the arrow
    CSrc6502View *FindDocView(CAsm::FileUID fuid);	// Find the document window
    CAsm::FileUID m_fuidLastView;			// Remembering the window in which the arrow is drawn
    //HWND m_hwndLastView;				// j.w.
    void AddBranchCycles(uint8_t arg);

    // This was problematic for me -- B.Simonds (April 25, 2024)
    //CEvent eventRedraw;			// Window refresh synchronization during animation

    void init();
    void set_translation_tables();

    bool check_io_write(uint16_t addr);
    bool check_io_read(uint16_t addr);

    CAsm::SymStat io_function(uint8_t arg);
    uint8_t io_function();

    const uint8_t* m_vCodeToCommand;
    const uint8_t* m_vCodeToCycles;
    const uint8_t* m_vCodeToMode;

public:
    CAsm::Finish finish;		// Specifying how to end program execution

    uint16_t get_rst_addr();
    uint16_t get_nmi_addr();

    void Update(CAsm::SymStat stat, bool no_ok = false);

    std::string GetStatMsg(CAsm::SymStat stat);
    std::string GetLastStatMsg();

    CAsm::SymStat SkipInstr();
    void SkipToAddr(uint16_t addr);
    void set_addr_bus_width(UINT w) { UNUSED(w); }

    uint32_t get_pc() const { return ctx.pc; }
    void set_pc(uint32_t pc) { ctx.pc = pc; }

    const CContext* GetContext() { return &ctx; }
    void SetContext(CContext &context) { ctx = context; }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers - added pre
    CSym6502(COutputMem &mem, int addr_bus_width)
        : ctx(mem, addr_bus_width)
        , pre(mem, addr_bus_width)
        , old(mem, addr_bus_width)
        //, eventRedraw(true, true)
    {
        init();
    }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers - added pre
    CSym6502(COutputMem &mem, CDebugInfo *debug, int addr_bus_width) 
        : ctx(mem, addr_bus_width)
        , pre(mem, addr_bus_width)
        , old(mem, addr_bus_width)
        //, eventRedraw(true)
        , debug(debug)
    {
        init();
    }

    void Restart(const COutputMem &mem);
    void SymStart(uint32_t org);

    CAsm::SymStat StepInto();
    CAsm::SymStat StepOver();
    CAsm::SymStat RunTillRet();
    CAsm::SymStat Run();
    CAsm::SymStat Animate();
    CAsm::SymStat Interrupt(IntType eInt);

    bool IsFinished() const
    {
        return fin_stat == CAsm::SYM_FIN;
    }

    bool IsRunning() const { return running; }

    void Break() { stop_prog = true; }

    bool IsBroken() const { return stop_prog; }

    void AbortProg();
    void ExitSym();

    void ClearCyclesCounter();

    const CommandLog& GetLog() const
    {
        return m_Log;
    }
};

#endif
