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

#ifndef SIM_6502_H__
#define SIM_6502_H__

/*************************************************************************/

#include "DebugInfo.h"
#include "OutputMem.h"
#include "LogBuffer.h"

#include "sim.h"

/*************************************************************************/

class CSrc6502View;

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
        cmd = ctx.bus.PeekByte(ctx.pc);
        arg1 = ctx.bus.PeekByte(ctx.pc + 1);
        arg2 = ctx.bus.PeekByte(ctx.pc + 2);
        arg3 = ctx.bus.PeekByte(ctx.pc + 3);
        uCycles = ctx.uCycles;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
        intFlag = ctx.intFlag;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
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
    {
    }

    CmdInfo() {}

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

/*************************************************************************/

class CSym6502
{
private:
    CContext ctx;
    CDebugInfo *debug;
    CommandLog m_log;

    ULONG m_saveCycles; //% Bug Fix 1.2.12.18 - fix command log display

public:
    /// @brief Simulator Status
    enum class Status
    {
        OK = 0,
        BPT_EXECUTE,        // Interrupt during execution
        BPT_READ,           // Interrupt on reading
        BPT_WRITE,          // Interrupt on writing
        BPT_TEMP,           // Interrupt during execution
        BPT_ILLEGAL_CODE,   // Illegal statement encountered
        STOP,               // Program stopped by user
        FINISH,             // Program finished
        RUN,                // Program started
        INP_WAIT,           // Waiting for data input
        ILL_WRITE           // Protected area writing attempt detected
    };

    enum class Vector
    {
        IRQ   = 0,
        BRK   = 1,
        RESET = 2,
        NMI   = 3,
        ABORT = 4,
        COP   = 5
    };

    /**
     * @brief Get the address for the supplied vector.
     * @param v The vector to get the address of.
     * @return Returns a location in memory that will hold the location of the requested vector.
     */
    virtual uint32_t getVectorAddress(Vector v);

public:
    static int bus_width;
    static bool s_bWriteProtectArea;
    static uint32_t s_uProtectFromAddr;
    static uint32_t s_uProtectToAddr;

    // interrupt types
    enum IntType { NONE = 0, IRQ = 1, NMI = 2, RST = 4 };

    bool cpu16() { return ctx.getProcessorType() == ProcessorType::WDC65816; }

private:
    bool waiFlag;

    void inc_prog_counter(int step = 1)
    {
        ctx.pc = uint32_t(ctx.pc + step);
    }

    bool running; // TODO: Make atomic?
    bool stop_prog; // TODO: Make atomic?

    int m_nInterruptTrigger;

    void skip_cmd(); // Skip the current statement
    void step_over();
    void run_till_ret();

    void run(std::stop_token stopToken);
    void perform_step();

    void interrupt(int& nInterrupt);    // interrupt requested: load pc

    void PerformCommand();
    void PerformCommandInner();

    uint32_t get_argument_address(bool bWrite); // get current cmd argument address
    uint16_t get_argument_value(bool rmask); // get current cmd argument value

    uint16_t get_word_indirect(uint16_t zp)
    {
        ASSERT(zp <= ((cpu16() && !ctx.emm) ? 0xFF : 0xFFFF));
        return ctx.getWord(zp);
    }

    uint16_t get_word(uint32_t addr)
    {
        return ctx.getWord(addr);
    }

    uint32_t get_Lword_indirect(uint16_t zp)
    {
        ASSERT(zp <= ((cpu16() && !ctx.emm) ? 0xFF : 0xFFFF));

        return ctx.getLWord(zp);
    }

    uint32_t get_Lword(uint32_t addr)
    {
        return ctx.getLWord(addr);
    }

    void push_on_stack(uint8_t arg)
    {
        if (cpu16() && !ctx.emm)
        {
            if (s_bWriteProtectArea && ctx.s >= s_uProtectFromAddr && ctx.s <= s_uProtectToAddr)
                throw CSym6502::Status::ILL_WRITE;

            ctx.setByte(ctx.s, arg);
            --ctx.s;
            ctx.s &= 0xFFFF;
        }
        else
        {
            ctx.setByte(0x100 + ctx.s--, arg);
            --ctx.s;
            ctx.s = (ctx.s & 0xFF) + 0x100;
        }        
    }

    void push_addr_on_stack(uint16_t arg)
    {
        if (cpu16() && !ctx.emm)
        {
            if (s_bWriteProtectArea && ctx.s >= s_uProtectFromAddr && ctx.s <= s_uProtectToAddr)
                throw CSym6502::Status::ILL_WRITE;

            ctx.setByte(ctx.s, (arg >> 8) & 0xFF);
            --ctx.s;
            ctx.s &= 0xFFFF;

            if (s_bWriteProtectArea && ctx.s >= s_uProtectFromAddr && ctx.s <= s_uProtectToAddr)
                throw CSym6502::Status::ILL_WRITE;

            ctx.setByte(ctx.s, arg & 0xFF);
            --ctx.s;
            ctx.s &= 0xFFFF;
        }
        else
        {
            ctx.setByte(0x100 + (ctx.s & 0xFF), (arg >> 8) & 0xFF);
            --ctx.s;
            ctx.s = (ctx.s & 0xFF) + 0x100;
            ctx.setByte(0x100 + (ctx.s & 0xFF), arg & 0xFF);
            --ctx.s;
            ctx.s = (ctx.s & 0xFF) + 0x100;
        }
    }

    uint8_t pull_from_stack()
    {
        if (cpu16())
        {
            if (ctx.emm && ((ctx.s & 0xFF) == 0xFF))
            {
                ctx.s = 0x100;
                return ctx.bus.GetByte(ctx.s);
            }
            else
                return ctx.bus.GetByte(++ctx.s);
        }
        else
        {
            ++ctx.s;
            return ctx.bus.GetByte(0x100 + (ctx.s & 0xFF));
        }
    }

    uint16_t pull_addr_from_stack()
    {
        if (cpu16())
        {
            if (ctx.emm && ((ctx.s & 0xFF) == 0xFF))
            {
                uint16_t tmp = 0x100;
                ctx.s = 0x101;
                return get_word(tmp);
            }
            else
            {
                uint16_t tmp = ++ctx.s;
                ++ctx.s;
                return get_word(tmp);
            }
        }
        else
        {
            uint8_t tmp = ++ctx.s & 0xFF;
            ++ctx.s;
            uint16_t tmp2 = ctx.bus.GetByte(0x100 + tmp);
            tmp2 |= uint16_t(ctx.bus.GetByte(0x100 + (ctx.s & 0xFF))) << uint16_t(8);
            return tmp2;
        }
    }

    static UINT start_step_over_thread(void *ptr);

    void RunThread(std::stop_token stopToken);

    static UINT start_run_till_ret_thread(void *ptr);

    std::jthread m_thread;

    /*
     * Looks like drawing functions in here; thinking these need to be moved 
     * into whatever view that is actually showing the source code. 
     * 
     *          -- B.Simonds (April 25, 2024)
     */

    void SetPointer(const CLine &line, uint32_t addr); // Placing an arrow (->) in front of the current line
    void SetPointer(CSrc6502View* pView, int nLine, bool bScroll); // helper fn
    void ResetPointer(); // Hiding the arrow

    CSrc6502View *FindDocView(CAsm::FileUID fuid);	// Find the document window
    CAsm::FileUID m_fuidLastView;			// Remembering the window in which the arrow is drawn
    //HWND m_hwndLastView;				// j.w.
    void AddBranchCycles(uint8_t arg);

    // This was problematic for me -- B.Simonds (April 25, 2024)
    //CEvent eventRedraw;			// Window refresh synchronization during animation

    void init();
    void set_translation_tables();

    const uint8_t* m_vCodeToCommand;
    const uint8_t* m_vCodeToCycles;
    const uint8_t* m_vCodeToMode;

public:
    CAsm::Finish finish; // Specifying how to end program execution

    uint16_t get_cop_addr() { return ctx.getWord(0xFFF4); }
    uint16_t get_abort_addr() { return ctx.getWord(0xFFF8); }
    uint16_t get_nmi_addr() { return ctx.getWord(0xFFFA); }
    uint16_t get_rst_addr() { return ctx.getWord(0xFFFC); }
    uint16_t get_irq_addr() { return ctx.getWord(0xFFFE); }

    uint16_t get_cop_addr16() { return ctx.getWord(0xFFE4); }
    uint16_t get_brk_addr16() { return ctx.getWord(0xFFE6); }
    uint16_t get_abort_addr16() { return ctx.getWord(0xFFE8); }
    uint16_t get_nmi_addr16() { return ctx.getWord(0xFFEA); }
    uint16_t get_irq_addr16() { return ctx.getWord(0xFFEE); }

    std::string GetStatMsg(Status stat) const;
    std::string GetLastStatMsg() const { return GetStatMsg(CurrentStatus); }

    void SkipInstr();
    void SkipToAddr(uint16_t addr);
    void set_addr_bus_width(UINT w) { UNUSED(w); }

    uint32_t get_pc() const { return ctx.pc; }
    void set_pc(uint32_t pc) { ctx.pc = pc; }

    const CContext &GetContext() const { return ctx; }
    CContext &GetContext() { return ctx; }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers - added pre
    CSym6502(ProcessorType processor)
        : ctx(processor)
        //, eventRedraw(true, true)
    {
        init();
    }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers - added pre
    CSym6502(ProcessorType processor, CDebugInfo *debug) 
        : ctx(processor)
        //, eventRedraw(true)
        , debug(debug)
    {
        init();
    }

    virtual ~CSym6502()
    {
        if (m_thread.joinable())
        {
            m_thread.get_stop_source().request_stop();
            m_thread.join();
        }
    }

    void Restart();

    void SetStart(sim_addr_t address);

    void StepInto();
    void StepOver();
    void RunTillRet();
    void Run();
    void Interrupt(IntType eInt);

    property<Status> CurrentStatus;

    inline
    bool CanContinue() const { return (CurrentStatus == Status::OK) || (CurrentStatus == Status::RUN); }

    inline
    bool IsFinished() const { return (CurrentStatus == Status::FINISH); }

    bool IsRunning() const { return running; }

    void Break() { stop_prog = true; }

    bool IsBroken() const { return stop_prog; }

    void AbortProg();
    void ExitSym();

    void ClearCyclesCounter();

    const CommandLog& GetLog() const
    {
        return m_log;
    }
};

typedef std::shared_ptr<CSym6502> PSym6502;

/*************************************************************************/

#endif /* SIM_6502_H__ */

/*************************************************************************/
