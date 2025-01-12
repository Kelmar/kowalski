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

// TODO: Remove direct references to views.
class CSrc6502View;


class CSym6502
{
private:
    CContext m_ctx;
    CDebugInfo *debug;
    //CommandLog m_log;

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

    ProcessorType Processor() const { return m_ctx.Processor(); }

    bool cpu16() { return Processor() == ProcessorType::WDC65816; }

private:
    bool waiFlag;

    void inc_prog_counter(int step = 1)
    {
        m_ctx.pc = uint32_t(m_ctx.pc + step);
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
        ASSERT(zp <= ((cpu16() && !m_ctx.emm) ? 0xFF : 0xFFFF));
        return m_ctx.getWord(zp);
    }

    uint16_t get_word(uint32_t addr)
    {
        return m_ctx.getWord(addr);
    }

    uint32_t get_Lword_indirect(uint16_t zp)
    {
        ASSERT(zp <= ((cpu16() && !m_ctx.emm) ? 0xFF : 0xFFFF));

        return m_ctx.getLWord(zp);
    }

    uint32_t get_Lword(uint32_t addr)
    {
        return m_ctx.getLWord(addr);
    }

    void push_on_stack(uint8_t arg)
    {
        if (cpu16() && !m_ctx.emm)
        {
            if (s_bWriteProtectArea && m_ctx.s >= s_uProtectFromAddr && m_ctx.s <= s_uProtectToAddr)
                throw CSym6502::Status::ILL_WRITE;

            m_ctx.setByte(m_ctx.s, arg);
            --m_ctx.s;
            m_ctx.s &= 0xFFFF;
        }
        else
        {
            m_ctx.setByte(0x100 + m_ctx.s--, arg);
            --m_ctx.s;
            m_ctx.s = (m_ctx.s & 0xFF) + 0x100;
        }        
    }

    void push_addr_on_stack(uint16_t arg)
    {
        if (cpu16() && !m_ctx.emm)
        {
            if (s_bWriteProtectArea && m_ctx.s >= s_uProtectFromAddr && m_ctx.s <= s_uProtectToAddr)
                throw CSym6502::Status::ILL_WRITE;

            m_ctx.setByte(m_ctx.s, (arg >> 8) & 0xFF);
            --m_ctx.s;
            m_ctx.s &= 0xFFFF;

            if (s_bWriteProtectArea && m_ctx.s >= s_uProtectFromAddr && m_ctx.s <= s_uProtectToAddr)
                throw CSym6502::Status::ILL_WRITE;

            m_ctx.setByte(m_ctx.s, arg & 0xFF);
            --m_ctx.s;
            m_ctx.s &= 0xFFFF;
        }
        else
        {
            m_ctx.setByte(0x100 + (m_ctx.s & 0xFF), (arg >> 8) & 0xFF);
            --m_ctx.s;
            m_ctx.s = (m_ctx.s & 0xFF) + 0x100;
            m_ctx.setByte(0x100 + (m_ctx.s & 0xFF), arg & 0xFF);
            --m_ctx.s;
            m_ctx.s = (m_ctx.s & 0xFF) + 0x100;
        }
    }

    uint8_t pull_from_stack()
    {
        if (cpu16())
        {
            if (m_ctx.emm && ((m_ctx.s & 0xFF) == 0xFF))
            {
                m_ctx.s = 0x100;
                return m_ctx.bus.GetByte(m_ctx.s);
            }
            else
                return m_ctx.bus.GetByte(++m_ctx.s);
        }
        else
        {
            ++m_ctx.s;
            return m_ctx.bus.GetByte(0x100 + (m_ctx.s & 0xFF));
        }
    }

    uint16_t pull_addr_from_stack()
    {
        if (cpu16())
        {
            if (m_ctx.emm && ((m_ctx.s & 0xFF) == 0xFF))
            {
                uint16_t tmp = 0x100;
                m_ctx.s = 0x101;
                return get_word(tmp);
            }
            else
            {
                uint16_t tmp = ++m_ctx.s;
                ++m_ctx.s;
                return get_word(tmp);
            }
        }
        else
        {
            uint8_t tmp = ++m_ctx.s & 0xFF;
            ++m_ctx.s;
            uint16_t tmp2 = m_ctx.bus.GetByte(0x100 + tmp);
            tmp2 |= uint16_t(m_ctx.bus.GetByte(0x100 + (m_ctx.s & 0xFF))) << uint16_t(8);
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

    CSrc6502View *FindDocView(CAsm::FileUID fuid); // Find the document window
    CAsm::FileUID m_fuidLastView; // Remembering the window in which the arrow is drawn
    //HWND m_hwndLastView; // j.w.
    void AddBranchCycles(uint8_t arg);

    void init();
    void set_translation_tables();

    const uint8_t* m_vCodeToCommand;
    const uint8_t* m_vCodeToCycles;
    const uint8_t* m_vCodeToMode;

    const SimulatorConfig &Config() const { return m_ctx.Config(); }

public:
    CAsm::Finish finish; // Specifying how to end program execution

    uint16_t get_cop_addr() { return m_ctx.getWord(0xFFF4); }
    uint16_t get_abort_addr() { return m_ctx.getWord(0xFFF8); }
    uint16_t get_nmi_addr() { return m_ctx.getWord(0xFFFA); }
    uint16_t get_rst_addr() { return m_ctx.getWord(0xFFFC); }
    uint16_t get_irq_addr() { return m_ctx.getWord(0xFFFE); }

    uint16_t get_cop_addr16() { return m_ctx.getWord(0xFFE4); }
    uint16_t get_brk_addr16() { return m_ctx.getWord(0xFFE6); }
    uint16_t get_abort_addr16() { return m_ctx.getWord(0xFFE8); }
    uint16_t get_nmi_addr16() { return m_ctx.getWord(0xFFEA); }
    uint16_t get_irq_addr16() { return m_ctx.getWord(0xFFEE); }

    std::string GetStatMsg(Status stat) const;
    std::string GetLastStatMsg() const { return GetStatMsg(CurrentStatus); }

    void SkipInstr();
    void SkipToAddr(uint16_t addr);
    void set_addr_bus_width(UINT w) { UNUSED(w); }

    uint32_t get_pc() const { return m_ctx.pc; }
    void set_pc(uint32_t pc) { m_ctx.pc = pc; }

    const CContext &GetContext() const { return m_ctx; }
    CContext &GetContext() { return m_ctx; }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers - added pre
    CSym6502(const SimulatorConfig &config)
        : m_ctx(config)
    {
        init();
    }

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers - added pre
    CSym6502(const SimulatorConfig &config, CDebugInfo *debug) 
        : m_ctx(config)
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
};

typedef std::shared_ptr<CSym6502> PSym6502;

/*************************************************************************/

#endif /* SIM_6502_H__ */

/*************************************************************************/
