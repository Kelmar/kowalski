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

/*=======================================================================*/

#include "DebugInfo.h"
#include "OutputMem.h"
#include "LogBuffer.h"

#include "sim.h"

/*=======================================================================*/

// TODO: Remove direct references to views.
class CSrc6502View;


class CSym6502
{
private:
    CContext m_ctx;
    class CDebugInfo *debug;

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
        m_ctx.PC(m_ctx.PC() + step);
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
        return m_ctx.GetWord(zp);
    }

    uint32_t get_Lword_indirect(uint16_t zp)
    {
        ASSERT(zp <= ((cpu16() && !m_ctx.emm) ? 0xFF : 0xFFFF));

        return m_ctx.GetLWord(zp);
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

    void SetStart(sim_addr_t address);
    void set_translation_tables();

    const uint8_t* m_vCodeToCommand;
    const uint8_t* m_vCodeToCycles;
    const uint8_t* m_vCodeToMode;

    const SimulatorConfig &Config() const { return m_ctx.Config(); }

public:
    CAsm::Finish finish; // Specifying how to end program execution

    uint16_t get_cop_addr() { return m_ctx.GetWord(0xFFF4); }
    uint16_t get_abort_addr() { return m_ctx.GetWord(0xFFF8); }
    uint16_t get_nmi_addr() { return m_ctx.GetWord(0xFFFA); }
    uint16_t get_rst_addr() { return m_ctx.GetWord(0xFFFC); }
    uint16_t get_irq_addr() { return m_ctx.GetWord(0xFFFE); }

    uint16_t get_cop_addr16() { return m_ctx.GetWord(0xFFE4); }
    uint16_t get_brk_addr16() { return m_ctx.GetWord(0xFFE6); }
    uint16_t get_abort_addr16() { return m_ctx.GetWord(0xFFE8); }
    uint16_t get_nmi_addr16() { return m_ctx.GetWord(0xFFEA); }
    uint16_t get_irq_addr16() { return m_ctx.GetWord(0xFFEE); }

    std::string GetStatMsg(Status stat) const;
    std::string GetLastStatMsg() const { return GetStatMsg(CurrentStatus); }

    void SkipInstr();
    void SkipToAddr(uint16_t addr);
    void set_addr_bus_width(UINT w) { UNUSED(w); }

    const CContext &GetContext() const { return m_ctx; }
    CContext &GetContext() { return m_ctx; }

    CSym6502(CContext &&context, sim_addr_t startAddress, CDebugInfo *debug);

    virtual ~CSym6502()
    {
        if (m_thread.joinable())
        {
            m_thread.get_stop_source().request_stop();
            m_thread.join();
        }
    }

    void Restart();

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

/*=======================================================================*/

#endif /* SIM_6502_H__ */

/*=======================================================================*/
