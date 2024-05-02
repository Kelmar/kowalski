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

#include "StdAfx.h"
#include "resource.h"
//#include "6502.h"
#include "MainFrm.h"
#include "Deasm6502Doc.h"
#include "6502View.h"
#include "6502Doc.h"
#include "Deasm.h"

//-----------------------------------------------------------------------------

uint16_t CSym6502::io_addr = 0xE000; // Beginning of the simulator I/O area
bool CSym6502::io_enabled = true;
int CSym6502::bus_width = 16;
//static const int SIM_THREAD_PRIORITY = THREAD_PRIORITY_BELOW_NORMAL; // Priority (except animate)
bool CSym6502::s_bWriteProtectArea = false;
uint16_t CSym6502::s_uProtectFromAddr = 0xc000;
uint16_t CSym6502::s_uProtectToAddr = 0xcfff;
bool extracycle = false; //% Bug Fix 1.2.12.1 - fix cycle counts, used to signal page crossings
ULONG saveCycles = 0; //% Bug Fix 1.2.12.18 - fix command log display

//-----------------------------------------------------------------------------

uint8_t CContext::get_status_reg() const
{
    ASSERT(negative == false || negative == true);
    ASSERT(overflow == false || overflow == true);
    ASSERT(zero == false || zero == true);
    ASSERT(carry == false || carry == true);
    ASSERT(reserved == false || reserved == true);
    ASSERT(break_bit == false || break_bit == true);
    ASSERT(decimal == false || decimal == true);
    ASSERT(interrupt == false || interrupt == true);

    return negative<<N_NEGATIVE | overflow<<N_OVERFLOW | zero<<N_ZERO | carry<<N_CARRY |
           true<<N_RESERVED | break_bit<<N_BREAK | decimal<<N_DECIMAL | interrupt<<N_INTERRUPT; //% Bug fix 1.2.12.3&10 - S reg status bits wrong
}

void CContext::set_status_reg_bits(uint8_t reg)
{
    negative = !!(reg & NEGATIVE);
    overflow = !!(reg & OVERFLOW);
    zero = !!(reg & ZERO);
    carry = !!(reg & CARRY);
    reserved = 1; //% Bug fix 1.2.12.3 BRK bit trouble
    break_bit = 1; //% Bug fix 1.2.12.3 BRK bit trouble
    decimal = !!(reg & DECIMAL);
    interrupt = !!(reg & INTERRUPT);
}

//=============================================================================

uint16_t CSym6502::get_argument_address(bool bWrite)
{
    uint8_t arg;
//	uint16_t addr;
    uint32_t addr;

    uint8_t mode= m_vCodeToMode[ctx.mem[ctx.pc]];
//	uint16_t pc= ctx.pc;
    uint32_t pc= ctx.pc;
    inc_prog_counter(); // Bypass the command

    extracycle = false; //% bug Fix 1.2.12.1 - fix cycle timing

    switch (mode)
    {
    case A_ZPG:
    case A_ZPG2:
        addr = ctx.mem[ctx.pc]; // address on zero page
        inc_prog_counter();
        break;

    case A_ZPG_X:
        addr = uint8_t(ctx.mem[ctx.pc] + ctx.x);
        inc_prog_counter();
        break;

    case A_ZPG_Y:
        addr = uint8_t(ctx.mem[ctx.pc] + ctx.y);
        inc_prog_counter();
        break;

    case A_ZPGI:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg);
        inc_prog_counter();
        break;

    case A_ABS:
        addr = ctx.mem.GetWord(ctx.pc);
        inc_prog_counter(2);
        break;

    case A_ABS_X:
        addr = ctx.mem.GetWord(ctx.pc) + ctx.x;
        if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        inc_prog_counter(2);
        break;

    case A_ABS_Y:
        addr = ctx.mem.GetWord(ctx.pc) + ctx.y;
        if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        inc_prog_counter(2);
        break;

    case A_ZPGI_X:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg + ctx.x);
        inc_prog_counter();
        break;

    case A_ZPGI_Y:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg) + ctx.y;
        if ((addr>>8) != (ctx.mem.GetWordInd(arg)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        inc_prog_counter();
        break;

    case A_ABSI:						// only JMP(xxxx) supports this addr mode
        addr = ctx.mem.GetWord(ctx.pc);
        if (wxGetApp().m_global.GetProcType() && (addr & 0xFF) == 0xFF)	// LSB == 0xFF?
            addr = ctx.mem.GetWord(addr, addr - 0xFF);	// erroneously just as 6502 would do
        else
            addr = ctx.mem.GetWord(addr);
        inc_prog_counter(2);
        break;

    case A_ABSI_X:
        addr = ctx.mem.GetWord(ctx.pc) + ctx.x;
        addr = ctx.mem.GetWord(addr);
        inc_prog_counter(2);
        break;

    case A_ZREL: // exceptionally here: addr = zpg (lo) + relative (hi)
        //the cell number from page zero is returned in the lower byte
        //in the top byte the relative offset
        addr = ctx.mem.GetWord(ctx.pc);
        //addr = ctx.mem[ctx.pc]; // cell address on zero page
        //addr += uint16_t( ctx.mem[ctx.pc + 1] ) << 8; // offset
        inc_prog_counter(2);
        break;

    case A_ABSL:
        addr = ctx.mem.GetWord(ctx.pc);
        inc_prog_counter(3);
        break;

    case A_ABSL_X:
        addr = ctx.mem.GetWord(ctx.pc) + ctx.x;
        inc_prog_counter(3);
        break;

    case A_ZPIL:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg);
        inc_prog_counter();
        break;

    case A_ZPIL_Y:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg) + ctx.y;
        if ((addr>>8) != (ctx.mem.GetWordInd(arg)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        inc_prog_counter();
        break;

    case A_SR:
        addr = ctx.mem[ctx.pc]; // address on zero page
        inc_prog_counter();
        break;

    case A_SRI_Y:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg) + ctx.y;
        inc_prog_counter();
        break;

    case A_IMP:
    case A_IMP2:
    case A_ACC:
    case A_IMM:
    default:
        ASSERT(false);
        return 0;
    }

    if (bWrite && addr >= s_uProtectFromAddr && addr <= s_uProtectToAddr)
    {
        ctx.pc = pc; // restore original value
        throw SYM_ILL_WRITE;
    }

    return addr;
}

uint8_t CSym6502::get_argument_value()
{
    uint8_t arg;
//	uint16_t addr;
    uint32_t addr;

    uint8_t mode= m_vCodeToMode[ctx.mem[ctx.pc]];
    inc_prog_counter(); // bypass the command

    extracycle = false; //% bug Fix 1.2.12.1 - fix cycle timing

    switch (mode)
    {
    case A_IMP:
    case A_ACC:
        return 0;

    case A_IMP2:
    case A_IMM:
    case A_REL:
        arg = ctx.mem[ctx.pc];
        inc_prog_counter();
        return arg;

    case A_ZPGI:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg);
        inc_prog_counter();
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ZPG:
        arg = ctx.mem[ctx.mem[ctx.pc]]; // number at address
        inc_prog_counter();
        return arg;

    case A_ZPG_X:
        addr = (ctx.mem[ctx.pc] + ctx.x) & 0xFF; // address
        arg = ctx.mem[addr]; // number at address
        inc_prog_counter();
        return arg;

    case A_ZPG_Y:
        arg = ctx.mem[ (ctx.mem[ctx.pc] + ctx.y) & 0xFF ]; // number at address
        inc_prog_counter();
        return arg;

    case A_ABS:
        addr = ctx.mem.GetWord(ctx.pc);
        inc_prog_counter(2);
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

//	case A_ABSI:

    case A_ABS_X:
        addr = ctx.mem.GetWord(ctx.pc) + ctx.x;
        if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        inc_prog_counter(2);
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ABS_Y:
        addr = ctx.mem.GetWord(ctx.pc) + ctx.y;
        if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        inc_prog_counter(2);
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ZPGI_X:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg + ctx.x);
        inc_prog_counter();
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ZPGI_Y:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg) + ctx.y;
        if ((addr>>8) != (ctx.mem.GetWordInd(arg)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        inc_prog_counter();
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ABSL:
        addr = ctx.mem.GetWord(ctx.pc);
        inc_prog_counter(3);
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ABSL_X:
        addr = ctx.mem.GetWord(ctx.pc)+ ctx.x;
        inc_prog_counter(3);
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ZPIL:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg);
        inc_prog_counter();
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_ZPIL_Y:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg) + ctx.y;
        inc_prog_counter();
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_SR:
        arg = ctx.mem[ctx.mem[ctx.pc]]; // number at address
        inc_prog_counter();
        return arg;

    case A_SRI_Y:
        arg = ctx.mem[ctx.pc]; // cell address on zero page
        addr = ctx.mem.GetWordInd(arg) + ctx.y;
        inc_prog_counter();
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case A_RELL:
    case A_ABSI_X:
    case A_ABSI: // These modes are only supported through get_argument_address()
    case A_ZREL:
    case A_ZPG2:
    default:
        ASSERT(false);
        return 0;
    }
}

// The function executes the instruction indicated by ctx.pc, changing the state accordingly
// registers and memory (ctx.mem)
CAsm::SymStat CSym6502::perform_cmd()
{
    try
    {
        return perform_command();
    }
    catch (SymStat s)
    {
        return s;
    }

    return SYM_OK; // !? -- Not possible to get here. -- B.Simonds (May 1, 2024)
}

CAsm::SymStat CSym6502::perform_command()
{
    uint8_t cmd = ctx.mem[ctx.pc];
    uint8_t arg, acc;
    //uint16_t addr;
    uint32_t addr;
    uint8_t zero, overflow, carry, negative;
    uint8_t zeroc, negativec;

//% Bug Fix 1.2.12.18 - Command Log assembly not lined up witgh registers
#define TOBCD(a) (((((a) / 10) % 10) << 4) | ((a) % 10))
#define TOBIN(a) (((a) >> 4) * 10 + ((a) & 0x0F))

    //CmdInfo ci(ctx);
    //m_Log.Record(ci);

    if (m_nInterruptTrigger != NONE) //% Bug fix 1.2.12.19 - RST,IRQ,NMI cause Sim to run.
    {
        interrupt(m_nInterruptTrigger);
        cmd = ctx.mem[ctx.pc];
    }

    pre = ctx;
    pre.intFlag =false;

    if (pre.uCycles > saveCycles)
        pre.intFlag = true;

    pre.uCycles = saveCycles;
//% End Bug Fixs

    switch (m_vCodeToCommand[cmd])
    {
    case C_ADC:
        arg = get_argument_value();
        acc = ctx.a;
        carry = ctx.carry;
        if (ctx.decimal)
        {
#if REWRITE_ASM
            __asm // BCD add
            {
                clc
                test carry, 0xFF
                jz $+7
                stc
                mov al, acc
                adc al, arg
                seto overflow
                setz zero
                daa
                mov acc, al
                setc carry
                sets negative
                setz zeroc
            }
#endif

            ctx.zero = ((wxGetApp().m_global.m_bProc6502 == 0) ? !!zero : !!zeroc);
            ctx.carry = !!carry;
            ctx.negative = !!negative;
            ctx.overflow = !!overflow;

            if (!(wxGetApp().m_global.m_bProc6502 == 0)) //% bug Fix 1.2.12.1 - fix cycle timing
                ctx.uCycles++; // Add a cycle in BCD mode
        }
        else
        {
#if REWRITE_ASM
            __asm // Binary addition
            {
                clc
                test carry, 0xFF
                jz $+7
                stc
                mov al, acc
                adc al, arg
                mov acc, al
                seto overflow
                setz zero
                sets negative
                setc carry
            }
#endif

            ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
        }
        ctx.a = acc;
        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;


    case C_SBC:
        arg = get_argument_value();
        acc = ctx.a;
        carry = ctx.carry;
        if (ctx.decimal)
        {
#if REWRITE_ASM
            __asm // BCD subtraction
            {
                clc
                test carry, 0xFF
                jnz $+7
                stc
                mov al, acc
                sbb al, arg
                seto overflow
                setc carry
                setz zero
                sets negative
                das
                mov acc, al
                sets negativec
                setz zeroc
            }
#endif

            ctx.carry = !carry; // Carray negation in accordance with convention 6502
            ctx.overflow = !!overflow;
            ctx.negative = ((wxGetApp().m_global.m_bProc6502 == 0) ? !!negative : !!negativec);
            ctx.zero = ((wxGetApp().m_global.m_bProc6502 == 0) ? !!zero : !!zeroc);

            if (!(wxGetApp().m_global.m_bProc6502 == 0)) //% bug Fix 1.2.12.1 - fix cycle timing
                ctx.uCycles++; // Add a cycle in BCD mode
        }
        else
        {
#if REWRITE_ASM
            __asm // Binary subtraction
            {
                clc
                test carry, 0xFF
                jnz $+7
                stc
                mov al, acc
                sbb al, arg
                mov acc, al
                seto overflow
                setz zero
                sets negative
                setc carry
            }
#endif

            ctx.set_status_reg_VZNC(overflow, zero, negative, !carry);
        }
        ctx.a = acc;
        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;


    case C_CMP:
        arg = get_argument_value();
        acc = ctx.a;
#if REWRITE_ASM
        __asm // Comparison (always binary, no BCD)
        {
            mov al, acc
            cmp al, arg
            setz zero
            sets negative
            setc carry
        }
#endif
        ctx.set_status_reg_ZNC(zero, negative, !carry);
        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;


    case C_CPX:
        arg = get_argument_value();
        acc = ctx.x;
#if REWRITE_ASM
        __asm // Comparison (always binary, no BCD)
        {
            mov al,acc
            cmp al,arg
            setz zero
            sets negative
            setc carry
        }
#endif
        ctx.set_status_reg_ZNC(zero, negative, !carry);
        break;


    case C_CPY:
        arg = get_argument_value();
        acc = ctx.y;
#if REWRITE_ASM
        __asm // Comparison (always binary, no BCD)
        {
            mov al,acc
            cmp al,arg
            setz zero
            sets negative
            setc carry
        }
#endif
        ctx.set_status_reg_ZNC(zero, negative, !carry);
        break;


    case C_ASL:
        if (m_vCodeToMode[cmd] == A_ACC) // Accumulator operation
        {
            inc_prog_counter(); // bypass the command
            acc = ctx.a;
#if REWRITE_ASM
            __asm
            {
                mov al, acc
                shl al, 1
                mov acc, al
                setz zero
                sets negative
                setc carry
            }
#endif
            ctx.a = acc;
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);
            arg = ctx.mem[addr];
#if REWRITE_ASM
            __asm
            {
                mov al, arg
                shl al, 1
                mov acc, al
                setz zero
                sets negative
                setc carry
            }
#endif
            ctx.mem[addr] = acc;
            if (!(wxGetApp().m_global.m_bProc6502 == 0) && extracycle)
                ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;


    case C_LSR:
        if (m_vCodeToMode[cmd] == A_ACC) // Accumulator operation
        {
            inc_prog_counter(); // bypass the command
            acc = ctx.a;
#if REWRITE_ASM
            __asm
            {
                mov al, acc
                shr al, 1
                mov acc, al
                setz zero
                sets negative
                setc carry
            }
#endif
            ctx.a = acc;
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);
            arg = ctx.mem[addr];
#if REWRITE_ASM
            __asm
            {
                mov al, arg
                shr al, 1
                mov acc, al
                setz zero
                sets negative
                setc carry
            }
#endif
            ctx.mem[addr] = acc;
            if (!(wxGetApp().m_global.m_bProc6502 == 0) && extracycle)
                ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;


    case C_ROL:
        carry = ctx.carry;
        if (m_vCodeToMode[cmd] == A_ACC) // Accumulator operation
        {
            inc_prog_counter(); // bypass the command
            acc = ctx.a;
#if REWRITE_ASM
            __asm
            {
                clc
                test carry, 0xFF
                jz $+7
                stc
                mov al, acc
                rcl al, 1
                mov acc, al
                setc carry
                test al, al
                setz zero
                sets negative
            }
#endif
            ctx.a = acc;
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);
            arg = ctx.mem[addr];
#if REWRITE_ASM
            __asm
            {
                clc
                test carry, 0xFF
                jz $+7
                stc
                mov al, arg
                rcl al, 1
                mov acc, al
                setc carry
                test al, al
                setz zero
                sets negative
            }
#endif
            ctx.mem[addr] = acc;
            if (!(wxGetApp().m_global.m_bProc6502 == 0) && extracycle)
                ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;


    case C_ROR:
        carry = ctx.carry;
        if (m_vCodeToMode[cmd] == A_ACC) // Accumulator operation
        {
            inc_prog_counter(); // bypass the command
            acc = ctx.a;
#if REWRITE_ASM
            __asm
            {
                clc
                test carry, 0xFF
                jz $+7
                stc
                mov al, acc
                rcr al, 1
                mov acc, al
                setc carry
                test al, al
                setz zero
                sets negative
            }
#endif
            ctx.a = acc;
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);
            arg = ctx.mem[addr];
#if REWRITE_ASM
            __asm
            {
                clc
                test carry, 0xFF
                jz $+7
                stc
                mov al, arg
                rcr al, 1
                mov acc, al
                setc carry
                test al, al
                setz zero
                sets negative
            }
#endif
            ctx.mem[addr] = acc;
            if (!(wxGetApp().m_global.m_bProc6502 == 0) && extracycle)
                ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;


    case C_AND:
        arg = get_argument_value();
        ctx.a &= arg;
        ctx.set_status_reg(ctx.a);
        if (extracycle) ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case C_ORA:
        arg = get_argument_value();
        ctx.a |= arg;
        ctx.set_status_reg(ctx.a);
        if (extracycle) ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case C_EOR:
        arg = get_argument_value();
        ctx.a ^= arg;
        ctx.set_status_reg(ctx.a);
        if (extracycle) ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case C_INC:
        if (m_vCodeToMode[cmd] == A_ACC)
        {
            inc_prog_counter();
            ctx.a++;
            ctx.set_status_reg(ctx.a);
        }
        else
        {

            addr = get_argument_address(s_bWriteProtectArea);
            ctx.mem[addr]++;
            ctx.set_status_reg(ctx.mem[addr]);
        }
        break;

    case C_DEC:
        if (m_vCodeToMode[cmd] == A_ACC)
        {
            inc_prog_counter();
            ctx.a--;
            ctx.set_status_reg(ctx.a);
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);
            ctx.mem[addr]--;
            ctx.set_status_reg(ctx.mem[addr]);
        }
        break;

    case C_INX:
        inc_prog_counter();
        ctx.x++;
        ctx.set_status_reg(ctx.x);
        break;

    case C_DEX:
        inc_prog_counter();
        ctx.x--;
        ctx.set_status_reg(ctx.x);
        break;

    case C_INY:
        inc_prog_counter();
        ctx.y++;
        ctx.set_status_reg(ctx.y);
        break;

    case C_DEY:
        inc_prog_counter();
        ctx.y--;
        ctx.set_status_reg(ctx.y);
        break;

    case C_TAX:
        inc_prog_counter();
        ctx.x = ctx.a;
        ctx.set_status_reg(ctx.x);
        break;

    case C_TXA:
        inc_prog_counter();
        ctx.a = ctx.x;
        ctx.set_status_reg(ctx.a);
        break;

    case C_TAY:
        inc_prog_counter();
        ctx.y = ctx.a;
        ctx.set_status_reg(ctx.y);
        break;

    case C_TYA:
        inc_prog_counter();
        ctx.a = ctx.y;
        ctx.set_status_reg(ctx.a);
        break;

    case C_TSX:
        inc_prog_counter();
        ctx.x = ctx.s;
        ctx.set_status_reg(ctx.x);
        break;

    case C_TXS:
        inc_prog_counter();
        ctx.s = ctx.x;
        break;

    case C_STA:
        addr = get_argument_address(s_bWriteProtectArea);
        if (check_io_write(addr))
            io_function(ctx.a);
        else
            ctx.mem[addr] = ctx.a;
        break;

    case C_STX:
        addr = get_argument_address(s_bWriteProtectArea);
        if (check_io_write(addr))
            io_function(ctx.x);
        else
            ctx.mem[addr] = ctx.x;
        break;

    case C_STY:
        addr = get_argument_address(s_bWriteProtectArea);
        if (check_io_write(addr))
            io_function(ctx.y);
        else
            ctx.mem[addr] = ctx.y;
        break;

    case C_LDA:
        ctx.set_status_reg( ctx.a = get_argument_value() );
        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case C_LDX:
        ctx.set_status_reg( ctx.x = get_argument_value() );
        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case C_LDY:
        ctx.set_status_reg( ctx.y = get_argument_value() );
        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case C_BIT:
        arg = get_argument_value();
        ctx.zero = (ctx.a & arg) == 0;
        //% bug Fix 1.2.13.5 - 65C02 BIT # only updates Z flag
        if (cmd != 0x89)
        {
            ctx.negative = (arg & 0x80) != 0;
            ctx.overflow = (arg & 0x40) != 0;
        }
        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.13.1 - fix cycle timing
        break;

    case C_PHA:
        inc_prog_counter();
        push_on_stack(ctx.a);
        break;

    case C_PLA:
        inc_prog_counter();
        ctx.a = pull_from_stack();
        ctx.set_status_reg(ctx.a);
        break;

    case C_PHP:
        inc_prog_counter();
        //% Bug Fix 1.2.12.10 - PHP not pushing flags correctly
        push_on_stack( ctx.get_status_reg() | CContext::BREAK | CContext::RESERVED );
        break;

    case C_PLP:
        inc_prog_counter();
        ctx.set_status_reg_bits(pull_from_stack());
        if (!wxGetApp().m_global.GetProcType()) // 65c02?
        {
            ctx.reserved = true; // 'reserved' bit always set
            ctx.break_bit = true;
        }
        break;

    case C_JSR:
        addr = get_argument_address(false);
        push_addr_on_stack(ctx.pc - 1 & 0xFFFF);
        //	push_addr_on_stack(ctx.pc - 1 & ctx.mem_mask);
        ctx.pc = addr;
        break;

    case C_JMP:
        addr = get_argument_address(false);
        ctx.pc = addr;
        break;

    case C_RTS:
        if (finish == FIN_BY_RTS && ctx.s == 0xFF) // RTS on empty stack?
            return SYM_FIN;
        ctx.pc = pull_addr_from_stack() + 1 & 0xFFFF; // & ctx.mem_mask;
        break;

    case C_RTI:
        ctx.set_status_reg_bits(pull_from_stack());
        ctx.pc = pull_addr_from_stack(); // & ctx.mem_mask;
        break;

    case C_BCC:
        arg = get_argument_value();
        if (!ctx.carry)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump forward
                ctx.pc += arg;
        }
        break;

    case C_BCS:
        arg = get_argument_value();
        if (ctx.carry)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_BVC:
        arg = get_argument_value();
        if (!ctx.overflow)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_BVS:
        arg = get_argument_value();
        if (ctx.overflow)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_BNE:
        arg = get_argument_value();
        if (!ctx.zero)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_BEQ:
        arg = get_argument_value();
        if (ctx.zero)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_BPL:
        arg = get_argument_value();
        if (!ctx.negative)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_BMI:
        arg = get_argument_value();
        if (ctx.negative)
        {
            AddBranchCycles(arg);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_NOP:
        inc_prog_counter();
        break;

    case C_CLI:
        inc_prog_counter();
        ctx.interrupt = false;
        break;

    case C_SEI:
        inc_prog_counter();
        ctx.interrupt = true;
        break;

    case C_CLD:
        inc_prog_counter();
        ctx.decimal = false;
        break;

    case C_SED:
        inc_prog_counter();
        ctx.decimal = true;
        break;

    case C_CLC:
        inc_prog_counter();
        ctx.carry = false;
        break;

    case C_SEC:
        inc_prog_counter();
        ctx.carry = true;
        break;

    case C_CLV:
        inc_prog_counter();
        ctx.overflow = false;
        break;

    case C_BRK:
        if (finish == FIN_BY_BRK)// instrukcja BRK ko�czy dzia�anie programu?
            return SYM_FIN;
        inc_prog_counter(2);
        //% Bug Fix 1.2.12.8 - BRK not executing when IRQ bit set
        //if (ctx.interrupt)
        //    break; // Interrupts disabled
        push_addr_on_stack(ctx.pc);
        ctx.break_bit = true;
        push_on_stack(ctx.get_status_reg() | CContext::RESERVED); //% Bug fix 1.2.12.3 - BRK status bits not correct
        //ctx.break_bit = false; // there's really no break bit in the flags register!
        ctx.interrupt = true;
        if (!(wxGetApp().m_global.m_bProc6502 == 0))
            ctx.decimal = false; //% Bug fix 1.2.12.9 - 65C02 clears D Flag in BRK
        ctx.pc = get_irq_addr();
        break;

    //---------- 65c02 --------------------------------------------------------

    case C_PLY:
        inc_prog_counter();
        ctx.y = pull_from_stack();
        ctx.set_status_reg(ctx.y);
        break;

    case C_PLX:
        inc_prog_counter();
        ctx.x = pull_from_stack();
        ctx.set_status_reg(ctx.x);
        break;

    case C_PHY:
        inc_prog_counter();
        push_on_stack(ctx.y);
        break;

    case C_PHX:
        inc_prog_counter();
        push_on_stack(ctx.x);
        break;

    case C_BRA:
        arg = get_argument_value();
        AddBranchCycles(arg);
        if (arg & 0x80) // jump back
            ctx.pc -= 0x100 - arg;
        else // jump foward
            ctx.pc += arg;
        break;

    case C_INA:
        inc_prog_counter();
        ctx.a++;
        ctx.set_status_reg( ctx.a );
        break;

    case C_DEA:
        inc_prog_counter();
        ctx.a--;
        ctx.set_status_reg( ctx.a );
        break;

    case C_STZ:
        addr = get_argument_address(s_bWriteProtectArea);
        if (check_io_write(addr))
        {
            uint8_t a = 0;
            io_function(a);
        }
        else
            ctx.mem[addr] = 0;
        break;

    case C_TRB:
        addr = get_argument_address(s_bWriteProtectArea);
        arg = ctx.mem[addr];
        //% bug Fix 1.2.12.6 - Only set the Z bit
        //ctx.negative = (arg & 0x80) != 0;
        //ctx.overflow = (arg & 0x40) != 0;
        ctx.mem[addr] = arg & ~ctx.a;
        ctx.zero = (arg & ctx.a) == 0;
        break;

    case C_TSB:
        addr = get_argument_address(s_bWriteProtectArea);
        arg = ctx.mem[addr];
        //% bug Fix 1.2.12.6 - Only set the Z bit
        //ctx.negative = (arg & 0x80) != 0;
        //ctx.overflow = (arg & 0x40) != 0;
        ctx.mem[addr] = arg | ctx.a;
        ctx.zero = (arg & ctx.a) == 0;
        break;

    case C_BBR:
        addr = get_argument_address(false); // zpg (lo), rel (hi)
        if (!( ctx.mem[addr & 0xFF] & uint8_t(1 << ((cmd >> 4) & 0x07)) ))
        {
            arg = addr >> 8;
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case C_BBS:
        addr = get_argument_address(false); // zpg (lo), rel (hi)
        if ( ctx.mem[addr & 0xFF] & uint8_t(1 << ((cmd >> 4) & 0x07)) )
        {
            arg = addr >> 8;
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            ctx.pc += arg;
        }
        break;

    case C_RMB:
        addr = get_argument_address(s_bWriteProtectArea);
        ctx.mem[addr] &= uint8_t(~(1 << ((cmd >> 4) & 0x07)));
        break;

    case C_SMB:
        addr = get_argument_address(s_bWriteProtectArea);
        ctx.mem[addr] |= uint8_t(1 << ((cmd >> 4) & 0x07));
        break;

    //------------65816--------------------------------------------------------
    case C_TXY:
        inc_prog_counter();
        ctx.y = ctx.x;
        ctx.set_status_reg(ctx.y);
        break;

    case C_TYX:
        inc_prog_counter();
        ctx.x = ctx.y;
        ctx.set_status_reg(ctx.x);
        break;

    case C_STP:
        return SYM_FIN;

    case C_BRL:
        addr = ctx.mem.GetWord(ctx.pc);
        inc_prog_counter(2);
        if (addr & 0x8000) // jump back
            ctx.pc -= 0x10000 - addr;
        else // jump foward
            ctx.pc += addr;
        break;

    //-------------------------------------------------------------------------

    case C_ILL:
        if (finish == FIN_BY_DB && cmd == 0xDB) // DB is invalid for 6502, 65C02 - STP for 65816
            return SYM_FIN;

        //% Bug Fix 1.2.12.2 - allow unused opcode to execute NOP's on 65C02
        if (!(wxGetApp().m_global.m_bProc6502 == 0)) //65C02 mode
        {
            arg = get_argument_value();
            extracycle = false;
            break;
        }

        return SYM_ILLEGAL_CODE;

    default:
        ASSERT(false);
        break;
    }

    ctx.uCycles += m_vCodeToCycles[cmd];

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
    CmdInfo ci(pre);
    CmdInfo cj(ctx);

    ci.a = cj.a;
    ci.x = cj.x;
    ci.y = cj.y;
    ci.s = cj.s;
    ci.flags = cj.flags;
    ci.uCycles = cj.uCycles - ci.uCycles; // this provides cycles used per instruction

    m_Log.Record(ci);
    saveCycles = ctx.uCycles;
    // end bug fix

    return SYM_OK;
}

CAsm::SymStat CSym6502::skip_cmd() // Skip the current statement
{
    inc_prog_counter( mode_to_len[m_vCodeToMode[ctx.mem[ctx.pc]]] );
    return SYM_OK;
}

uint16_t CSym6502::get_irq_addr()
{
    return ((uint16_t)ctx.mem[0xFFFE] | ((uint16_t)ctx.mem[0xFFFF] << 8)); // & ctx.mem_mask;
}

uint16_t CSym6502::get_nmi_addr()
{
    return ((uint16_t)ctx.mem[0xFFFA] | ((uint16_t)ctx.mem[0xFFFB] << 8)); // & ctx.mem_mask;
}

uint16_t CSym6502::get_rst_addr()
{
    return ((uint16_t)ctx.mem[0xFFFC] | ((uint16_t)ctx.mem[0xFFFD] << 8)); // & ctx.mem_mask;
}

//=============================================================================

CAsm::SymStat CSym6502::StepInto()
{
    ASSERT(fin_stat != SYM_FIN); // the program has already finished running

    if (running)
    {
        ASSERT(false);
        return SYM_OK;
    }

    set_translation_tables();
    stop_prog = false;
    running = true;
    old = ctx; // remembering the state to find differences
    fin_stat = perform_cmd();
    running = false;

    Update(fin_stat);

    return fin_stat;
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::StepOver()
{
    ASSERT(fin_stat != SYM_FIN); // the program has already finished running

    if (running)
    {
        ASSERT(false);
        return SYM_OK;
    }

    Update(SYM_RUN);
    old = ctx; // remembering the state to find differences
    stop_prog = false;
    running = true;

#if REWRITE_TO_WX_WIDGET
    CWinThread *thread = AfxBeginThread(CSym6502::start_step_over_thread, this, SIM_THREAD_PRIORITY, 0, CREATE_SUSPENDED);
    if (thread == NULL)
    {
        running = false;
        AfxMessageBox(IDS_ERR_SYM_THREAD);
    }
    else
    {
        hThread = (HANDLE)*thread;
        ResetPointer(); // hide the arrow
        thread->ResumeThread();
    }
#endif

    return SYM_OK;
}

UINT CSym6502::start_step_over_thread(void *ptr)
{
#if REWRITE_TO_WX_WIDGET
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->step_over();
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER+9998, pSym->fin_stat, 0); // Notify stopped
#endif

    return 0;
}

CAsm::SymStat CSym6502::step_over() // wykonanie instrukcji bez wchodzenia do podprogramu
{
    //uint16_t addr= uint16_t(ctx.pc & ctx.mem_mask);
    uint32_t addr = ctx.pc;
    bool jsr = false;
    uint8_t stack;

    set_translation_tables();

    switch (m_vCodeToCommand[ctx.mem[addr]])
    {
    case C_JSR:
        stack = ctx.s;
        jsr = true;
        
    case C_BRK:
        if (debug && !jsr)
            debug->SetTemporaryExecBreakpoint((addr+2) & ctx.mem_mask); // Break after instruction

        for (;;)
        {
            SymStat stat = perform_step(false);
            if (stat != SYM_OK)
                return stat;

            if (jsr && ctx.s == stack) // Return address removed after JSR command?
                return SYM_BPT_TEMP; // so stop
        }
        break;

    default:
        return perform_cmd();
    }
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::RunTillRet()
{
    ASSERT(fin_stat != SYM_FIN); // the program has already finished running

    if (running)
    {
        ASSERT(false);
        return SYM_OK;
    }

    Update(SYM_RUN);
    old = ctx; // remembering the state to find differences
    stop_prog = false;
    running = true;

#if REWRITE_TO_WX_WIDGET
    CWinThread *thread = AfxBeginThread(CSym6502::start_run_till_ret_thread, this, SIM_THREAD_PRIORITY, 0, CREATE_SUSPENDED);
    if (thread == NULL)
    {
        running = false;
        AfxMessageBox(IDS_ERR_SYM_THREAD);
    }
    else
    {
        hThread = (HANDLE)*thread;
        ResetPointer(); // hide the arrow
        thread->ResumeThread();
    }
#endif

    return SYM_OK;
}

UINT CSym6502::start_run_till_ret_thread(void *ptr)
{
#if REWRITE_TO_WX_WIDGET
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->run_till_ret();
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, pSym->fin_stat, 0); // Notify stopped
#endif

    return 0;
}

CAsm::SymStat CSym6502::run_till_ret() // Run to return from the subroutine
{
    set_translation_tables();

    uint8_t stack = ctx.s + 2;
    for (;;)
    {
        SymStat stat = perform_step(false);
        if (stat != SYM_OK)
            return stat;

        if (ctx.s == stack) // Return address removed from the stack?
            return SYM_BPT_TEMP; // so stop
    }
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::Run()
{
    ASSERT(fin_stat != SYM_FIN); // the program has already finished running

    if (running)
    {
        ASSERT(false);
        return SYM_OK;
    }

    Update(SYM_RUN);
    old = ctx; // remembering the state to find differences
    stop_prog = false;
    running = true;

#if REWRITE_TO_WX_WIDGET
    CWinThread *thread = AfxBeginThread(CSym6502::start_run_thread, this, SIM_THREAD_PRIORITY, 0, CREATE_SUSPENDED);
    if (thread == NULL)
    {
        running = false;
        AfxMessageBox(IDS_ERR_SYM_THREAD);
    }
    else
    {
        hThread = (HANDLE)*thread;
        ResetPointer(); // hide the arrow
        thread->ResumeThread();
    }
#endif

    return SYM_OK;
}


UINT CSym6502::start_run_thread(void *ptr)
{
#if REWRITE_TO_WX_WIDGET
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->run();
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, pSym->fin_stat, 0); // Notify stopped
#endif

    return 0;
}

CAsm::SymStat CSym6502::perform_step(bool animate)
{
    if (stop_prog) // stop executing?
        return SYM_STOP;

    if (m_nInterruptTrigger != NONE) // interrupt requested?
        interrupt(m_nInterruptTrigger);

    SymStat stat= perform_cmd();
    if (stat != SYM_OK)
        return stat;

    Breakpoint bp;
    if (debug && (bp=debug->GetBreakpoint(ctx.pc)) != BPT_NONE)
    {
        if (bp & BPT_EXECUTE)
            return SYM_BPT_EXECUTE;
        if (bp & BPT_TEMP_EXEC)
            return SYM_BPT_TEMP;
    }

#if REWRITE_TO_WX_WIDGET
    if (animate)
    {
        eventRedraw.ResetEvent(); // Waiting state
        wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, SYM_RUN, 1); // Refresh window
        eventRedraw.Lock(); // waiting for the window to be refreshed
    }
#endif

    return SYM_OK;
}

CAsm::SymStat CSym6502::run(bool animate /*= false*/)
{
    set_translation_tables();

    for (;;)
    {
        SymStat stat = perform_step(animate);
        if (stat != SYM_OK)
            return stat;
    }
}

void CSym6502::interrupt(int& nInterrupt) // interrupt requested: load pc ***
{
    ASSERT(running);

    if (nInterrupt & RST)
    {
        ctx.interrupt = true;
        if (!(wxGetApp().m_global.m_bProc6502==0))
            ctx.decimal = false; //% bug Fix 1.2.12.9 - 65C02 clears this bit
        ctx.pc = get_rst_addr();
        nInterrupt = NONE;
        ctx.uCycles += 7; //% bug Fix 1.2.12.1 - cycle counting not correct
    }
    else if (nInterrupt & NMI)
    {
        push_addr_on_stack(ctx.pc);
        ctx.break_bit = false;
        push_on_stack(ctx.get_status_reg());
        //ctx.interrupt = ???; // TODO: not sure...
        if (!(wxGetApp().m_global.m_bProc6502 == 0))
            ctx.decimal = false; //% bug Fix 1.2.12.9 - 65C02 clears this bit
        ctx.break_bit = true; //% bug Fix 1.2.12.4 - status bits not right
        ctx.pc = get_nmi_addr();
        nInterrupt &= ~NMI;
        ctx.uCycles += 7; //% bug Fix 1.2.12.1 - cycle counting not correct
    }
    else if (nInterrupt & IRQ)
    {
        nInterrupt &= ~IRQ;
        if (ctx.interrupt)
            return; // masked
        push_addr_on_stack(ctx.pc);
        ctx.break_bit = false;
        push_on_stack(ctx.get_status_reg());
        ctx.break_bit = true; //% bug Fix 1.2.12.4 - status bits not right
        ctx.interrupt = true;
        if (!(wxGetApp().m_global.m_bProc6502==0))
            ctx.decimal = false; //% bug Fix 1.2.12.9 - 65C02 clears this bit
        ctx.pc = get_irq_addr();

        ctx.uCycles += 7; //% bug Fix 1.2.12.1 - cycle counting not correct
    }
    else
    {
        ASSERT(false);
    }
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::Animate()
{
    ASSERT(fin_stat != SYM_FIN); // the program has already finished running

    if (running)
    {
        ASSERT(false);
        return SYM_OK;
    }

    Update(SYM_RUN);
    old = ctx; // remembering the state to find differences
    stop_prog = false;
    running = true;

#if REWRITE_TO_WX_WIDGET
    CWinThread *thread = AfxBeginThread(CSym6502::start_animate_thread, this, THREAD_PRIORITY_IDLE, 0, CREATE_SUSPENDED);
    if (thread == NULL)
    {
        running = false;
        AfxMessageBox(IDS_ERR_SYM_THREAD);
    }
    else
    {
        hThread = (HANDLE)*thread;
        thread->ResumeThread();
    }
#endif

    return SYM_OK;
}


UINT CSym6502::start_animate_thread(void *ptr)
{
#if REWRITE_TO_WX_WIDGET
    //WinThread *pThread = AfxGetThread();
    //pThread->SetThreadPriority(THREAD_PRIORITY_IDLE); // Enable refreshing
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->run(true);
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, pSym->fin_stat, 0); // Notify stopped
#endif

    return 0;
}

//-----------------------------------------------------------------------------

void CSym6502::SkipToAddr(uint16_t addr)
{
    ASSERT(fin_stat != SYM_FIN); // the program has already finished running

    if (running)
        return; // The program is now running

    ctx.pc = addr; // & ctx.mem_mask;
    Update(SYM_OK);
}

CAsm::SymStat CSym6502::SkipInstr()
{
    ASSERT(fin_stat != SYM_FIN); // the program has already finished running

    if (running)
        return SYM_OK; // The program is now running

    fin_stat = skip_cmd();

    Update(fin_stat);

    return fin_stat;
}

//-----------------------------------------------------------------------------

void CSym6502::AbortProg()
{
    /*
     * Not sure about this, it looks like they were attempting to override
     * the main application loop with one that runs async (like for games)
     * but ::GetMessage() blocks until it receives a message.  If they
     * had intended for it to run as an animation they probably wanted 
     * ::PeekMessage() instead.
     * 
     *                  -- B.Simonds (May 1, 2024)
     */

#if REWRITE_TO_WX_WIDGET
    stop_prog = true;
    //::WaitForSingleObject(hThread, INFINITE); // synchronization
    while (running)
    {
        MSG msg; // need to handle refresh messages
        // screen ('Animation' command) while waiting for completion
        if (!::GetMessage(&msg, NULL, NULL, NULL))
            break;
        // process this message
        //if (msg.message != WM_KICKIDLE && !PreTranslateMessage(&m_msgCur))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
    ASSERT(!running);
#endif
}

void CSym6502::ExitSym()
{
    ASSERT(running == false);
    ResetPointer(); // hide the arrow

    //wxWindow* pMain = (wxWindow*)wxGetApp()->m_pMainWnd;
    //pMain->ShowRegisterBar(false);
}

//-----------------------------------------------------------------------------

std::string CSym6502::GetLastStatMsg()
{
    return GetStatMsg(fin_stat);
}

std::string CSym6502::GetStatMsg(SymStat stat)
{
    std::string msg = "";

#if REWRITE_TO_WX_WIDGET
    switch (stat)
    {
    case SYM_OK:
    case SYM_BPT_TEMP:
        msg.LoadString(IDS_SYM_STAT_OK);
        break;

    case SYM_BPT_EXECUTE:
        msg.LoadString(IDS_SYM_STAT_BPX);
        break;

    case SYM_BPT_READ:
        msg.LoadString(IDS_SYM_STAT_BPR);
        break;

    case SYM_BPT_WRITE:
        msg.LoadString(IDS_SYM_STAT_BPW);
        break;

    case SYM_ILLEGAL_CODE:
        msg.LoadString(IDS_SYM_STAT_ILL);
        break;

    case SYM_STOP:
        msg.LoadString(IDS_SYM_STAT_STOP);
        break;

    case SYM_FIN:
        msg.LoadString(IDS_SYM_STAT_FIN);
        break;

    case SYM_RUN:
        msg.LoadString(IDS_SYM_STAT_RUN);
        break;

    case SYM_INP_WAIT:
        msg.LoadString(IDS_SYM_STAT_INP_WAIT);
        break;

    case SYM_ILL_WRITE:
        msg.LoadString(IDS_SYM_ILL_WRITE);
        break;

    default:
        ASSERT(false);
    }
#endif

    return msg;
}

//-----------------------------------------------------------------------------

void CSym6502::Update(SymStat stat, bool no_ok /*=false*/)
{
#if REWRITE_TO_WX_WIDGET
    wxWindow *pMain = (wxWindow *)wxGetApp()->m_pMainWnd;
    //pMain->m_wndRegisterBar.Update(&ctx,GetStatMsg(stat),&old);

    std::string reg = GetStatMsg(stat);
    /*
    reg.LoadString(CRegisterBar::IDD);
    CWnd *pWnd = CWnd::FindWindow(NULL,reg);
    if (pWnd)
    pWnd = pWnd->GetWindow(GW_CHILD);
    if (pWnd)
    pWnd = pWnd->GetWindow(GW_CHILD);
    if (pWnd)
    {
    reg = GetStatMsg(stat);
    pWnd->SendMessage(CBroadcast::WM_USER_UPDATE_REG_WND,(WPARAM)&reg,(LPARAM)&ctx);
    }
    */
    if (debug)
    {
        if (fin_stat == SYM_BPT_TEMP)
            debug->RemoveTemporaryExecBreakpoint();
        CDebugLine dl;
        debug->GetLine(dl, ctx.pc);
        if (/*dl.flags == CAsm::DBG_EMPTY ||*/ fin_stat == SYM_FIN)
            ResetPointer(); // hide the arrow
        else
            SetPointer(dl.line, ctx.pc); // position the arrow (->) in front of the current line
    }

    pMain->m_wndRegisterBar.SendMessage(CBroadcast::WM_USER_UPDATE_REG_WND, (WPARAM)&reg, (LPARAM)&ctx);

    if (stat == SYM_OK && !no_ok)
        pMain->m_wndStatusBar.SetPaneText(0, reg);

    if (running)
        eventRedraw.SetEvent(); // already updated -execute the next animated instruction
#endif
}

//-----------------------------------------------------------------------------

void CSym6502::Restart(const COutputMem &mem)
{
    ctx.Reset(mem);
    old = ctx;
    fin_stat = SYM_OK;
    m_Log.Clear();
    saveCycles = 0;
    ctx.set_status_reg_bits(0); //% Bug fix 1.2.12.3&10 - S reg bits not correct
}

void CSym6502::SymStart(uint32_t org)
{
    ctx.pc = org;
    ctx.s = 0xFF;
    saveCycles = 0;
    ctx.set_status_reg_bits(0); //% Bug fix 1.2.12.3&10 - S reg bits not correct

    if (debug)
    {
        CDebugLine dl;
        debug->GetLine(dl, org);
        // missing line corresponding to the beginning of the program
        //ASSERT(dl.flags != CAsm::DBG_EMPTY); 
        SetPointer(dl.line, org); // position the arrow (->) in front of the current line
    }
}

void CSym6502::SetPointer(const CLine &line, uint32_t addr) // position the arrow (->) in front of the current line
{
#if REWRITE_TO_WX_WIDGET
    POSITION posDoc = wxGetApp().m_pDocDeasmTemplate->GetFirstDocPosition();
    while (posDoc != NULL) // Are the windows from the disassembler?
    {
        CDocument *pDoc = wxGetApp().m_pDocDeasmTemplate->GetNextDoc(posDoc);
        ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDeasm6502Doc)));
        ((CDeasm6502Doc*)pDoc)->SetPointer(addr, true);
    }

    CSrc6502View* pView = FindDocView(line.file); // Get the document window
    if (m_fuidLastView != line.file && ::IsWindow(m_hwndLastView)) // Window changed?
    {
        if (CSrc6502View* pView = dynamic_cast<CSrc6502View*>(CWnd::FromHandlePermanent(m_hwndLastView)))
            SetPointer(pView, -1, false); // Hide arrow
        m_hwndLastView = 0;
    }
    if (!pView && debug)
    {
        if (const char* path = debug->GetFilePath(line.file))
        {
            // Try to open a document....
            C6502App* pApp = static_cast<C6502App*>(wxGetApp());
            pApp->m_bDoNotAddToRecentFileList = true;
            CDocument* pDoc = pApp->OpenDocumentFile(path);
            pApp->m_bDoNotAddToRecentFileList = false;
            if (CSrc6502Doc* pSrcDoc = dynamic_cast<CSrc6502Doc*>(pDoc))
            {
                POSITION pos = pSrcDoc->GetFirstViewPosition();
                if (pos != NULL)
                    pView = dynamic_cast<CSrc6502View*>(pSrcDoc->GetNextView(pos));
            }
        }
    }
    if (!pView)
    {
        //ResetPointer(); // hide the arrow, if there is
        return; // no document window containing the current line.
    }

    SetPointer(pView, line.ln, true); // force the window contents to move if necessary
    m_fuidLastView = line.file;
    m_hwndLastView = pView->m_hWnd;
#endif
}

void CSym6502::SetPointer(CSrc6502View* pView, int nLine, bool bScroll)
{
    if (!pView)
        return;

#if REWRITE_TO_WX_WIDGET
    CDocument* pDoc = pView->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();

    while (pos != NULL)
    {
        if (CSrc6502View* pSrcView = dynamic_cast<CSrc6502View*>(pDoc->GetNextView(pos)))
            pSrcView->SetPointer(nLine, bScroll && pSrcView == pView);
    }
#endif
}

void CSym6502::ResetPointer() // hide the arrow
{
#if REWRITE_TO_WX_WIDGET
    POSITION posDoc = wxGetApp().m_pDocDeasmTemplate->GetFirstDocPosition();

    while (posDoc != NULL) // Are there disassembler windows?
    {
        if (CDeasm6502Doc* pDoc = dynamic_cast<CDeasm6502Doc*>(wxGetApp().m_pDocDeasmTemplate->GetNextDoc(posDoc)))
            pDoc->SetPointer(-1, true);
    }

    if (m_fuidLastView)
    {
        if (CSrc6502View* pView = FindDocView(m_fuidLastView))
            SetPointer(pView, -1, false); // blurring of the arrow
    }
#endif

    m_fuidLastView = 0;
}

CSrc6502View *CSym6502::FindDocView(FileUID fuid)
{
#if REWRITE_TO_WX_WIDGET
    if (debug == NULL)
        return NULL;

    if (CFrameWnd* pFrame = dynamic_cast<CFrameWnd*>(AfxGetMainWnd()))
    {
        if (CFrameWnd* pActive = pFrame->GetActiveFrame())
        {
            if (CSrc6502View* pView = dynamic_cast<CSrc6502View*>(pActive->GetActiveView()))
            {
                if (debug->GetFileUID(pView->GetDocument()->GetPathName()) == fuid)
                    return pView;
            }
        }
    }
#endif

    return nullptr;
}

//=============================================================================

// Finding the terminal window
wxWindow *CSym6502::io_window()
{
    return wxGetApp().ioWindow();
}

uint8_t CSym6502::io_function()
{
    wxWindow *terminal = io_window();

    if (!terminal) // == 0 || !::IsWindow(terminal->m_hWnd))
    {
        io_func = IO_NONE;
        return 0;
    }

    int arg = 0;

    if (io_func == TERMINAL_IN)
    {
        //arg = terminal->SendMessage(CIOWindow::CMD_IN);
    }
    else
    {
        if (io_func == TERMINAL_GET_X_POS || io_func == TERMINAL_GET_Y_POS)
        {
            //arg = terminal->SendMessage(CIOWindow::CMD_POSITION, io_func == TERMINAL_GET_X_POS ? 0x3 : 0x2);
        }
    }

    io_func = IO_NONE;

    if (arg == -1) // break?
    {
        Break();
        return 0;
    }

    return uint8_t(arg);
}

CAsm::SymStat CSym6502::io_function(uint8_t arg)
{
    wxWindow *terminal = io_window();

    switch (io_func)
    {
    case TERMINAL_OUT:
        if (terminal)
        {
            //terminal->SendMessage(CIOWindow::CMD_PUTC, arg, 0);
        }
        break;

    case TERMINAL_OUT_CHR:
        if (terminal)
        {
            //terminal->SendMessage(CIOWindow::CMD_PUTC, arg, 1);
        }
        break;

    case TERMINAL_OUT_HEX:
        if (terminal)
        {
            //terminal->SendMessage(CIOWindow::CMD_PUTC, arg, 2);
        }
        break;

    case TERMINAL_CLS:
        if (terminal)
        {
            //terminal->SendMessage(CIOWindow::CMD_CLS);
        }
        break;

    case TERMINAL_IN:
        ASSERT(false);
        /*		if (terminal)
        arg = uint8_t(terminal->SendMessage(CIOWindow::CMD_IN));
        else
        arg = 0; */
        break;

    case TERMINAL_SET_X_POS:
    case TERMINAL_SET_Y_POS:
        if (terminal)
        {
            //terminal->SendMessage(CIOWindow::CMD_POSITION, io_func == TERMINAL_SET_X_POS ? 0x1 : 0x0, arg);
        }
        break;

    default:
        ASSERT(false); // unrecognized function
        break;
    }

    io_func = IO_NONE;
    return SYM_OK;
}

void CSym6502::ClearCyclesCounter()
{
    ASSERT(running == false);
    ctx.uCycles = 0;
}

void CSym6502::AddBranchCycles(uint8_t arg)
{
    ctx.uCycles++; // jump completed -> additional cycle

    if (arg & 0x80) // jump back
    {
        if (ctx.pc >> 8 != uint16_t(ctx.pc - (0x100 - arg)) >> 8)
            ctx.uCycles++; // changing memory page -> additional cycle
    }
    else // jump forward
    {
        if (ctx.pc >> 8 != uint16_t(ctx.pc + arg) >> 8)
            ctx.uCycles++; // changing memory page -> additional cycle
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CSym6502::check_io_write(uint16_t addr)
{
    if (io_enabled && addr >= io_addr && addr < io_addr + IO_LAST_FUNC)
    {
        io_func = IOFunc(addr - io_addr);
        if (io_func == TERMINAL_GET_X_POS)
            io_func = TERMINAL_SET_X_POS;
        else if (io_func == TERMINAL_GET_Y_POS)
            io_func = TERMINAL_SET_Y_POS;
        return true;
    }
    else
        return false;
}

bool CSym6502::check_io_read(uint16_t addr)
{
    if (io_enabled && addr >= io_addr && addr < io_addr + IO_LAST_FUNC)
    {
        io_func = IOFunc(addr - io_addr);
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

CSym6502::SymStat CSym6502::Interrupt(IntType eInt)
{
    m_nInterruptTrigger |= eInt;

    //% Bug fix 1.2.12.19 - RST, IRQ, NMI cause Sim to run.
    //if (!running)
    //    Run();

    return SYM_OK;
}

///////////////////////////////////////////////////////////////////////////////

void CSym6502::init()
{
    running = false;
    m_fuidLastView = 0;
    finish = FIN_BY_BRK;
    fin_stat = SYM_OK;
    //hThread = 0;
    //io_enabled = false;
    //io_addr = 0xE000;
    io_func = IO_NONE;
    m_nInterruptTrigger = NONE;
    m_vCodeToCommand = 0;
    m_vCodeToCycles = 0;
    m_vCodeToMode = 0;
    ctx.set_status_reg_bits(0); //% Bug fix 1.2.12.3&10 - S reg bits not correct
    ctx.mem_mask = uint32_t((1 << bus_width) - 1); // set mem mask to match bus width
}

void CSym6502::set_translation_tables()
{
    m_vCodeToCommand = CodeToCommand();
    m_vCodeToCycles = CodeToCycles();
    m_vCodeToMode = CodeToMode();
}

///////////////////////////////////////////////////////////////////////////////

std::string CmdInfo::Asm() const
{
    CDeasm deasm;
    CAsm::DeasmFmt fmt = CAsm::DeasmFmt(CAsm::DF_ADDRESS | CAsm::DF_CODE_BYTES | CAsm::DF_USE_BRK);  //% bug Fix 1.2.13.18 - show BRK vs. .DB $00

    wxString strLine = deasm.DeasmInstr(*this, fmt);

    wxString strBuf;

    //% bug Fix 1.2.13.18 - command log assembly not lined up with registers
    // * indicates RST, IRQ, or NMI have occurred
    if (intFlag)
        strBuf.Printf("%-30s A:%02x X:%02x Y:%02x F:%02x S:%03x  Cycles=%u *", strLine, int(a), int(x), int(y), int(flags), int(s + 0x100), uCycles);
    else
        strBuf.Printf("%-30s A:%02x X:%02x Y:%02x F:%02x S:%03x  Cycles=%u ", strLine, int(a), int(x), int(y), int(flags), int(s + 0x100), uCycles);

    return strBuf.ToStdString();
}
