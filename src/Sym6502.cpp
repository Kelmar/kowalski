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

bool cpu16 = wxGetApp().m_global.GetProcType() == ProcessorType::WDC65816;
bool waiFlag = false;

uint32_t CSym6502::io_addr = 0xE000; // Beginning of the simulator I/O area
bool CSym6502::io_enabled = true;
int CSym6502::bus_width = 16;
//static const int SIM_THREAD_PRIORITY = THREAD_PRIORITY_BELOW_NORMAL; // Priority (except animate)
bool CSym6502::s_bWriteProtectArea = false;
uint32_t CSym6502::s_uProtectFromAddr = 0xc000;
uint32_t CSym6502::s_uProtectToAddr = 0xcfff;
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

    if (wxGetApp().m_global.GetProcType() == ProcessorType::WDC65816)
    {
        return
            negative << N_NEGATIVE |
            overflow << N_OVERFLOW |
            zero << N_ZERO |
            carry << N_CARRY |
            mem16 << N_MEMORY |
            xy16 << N_INDEX |
            decimal << N_DECIMAL |
            interrupt << N_INTERRUPT; //% Bug fix 1.2.12.3&10 - S reg status bits wrong
    }
    else
    {
        return
            negative << N_NEGATIVE |
            overflow << N_OVERFLOW |
            zero << N_ZERO |
            carry << N_CARRY |
            1 << N_RESERVED |
            break_bit << N_BREAK |
            decimal << N_DECIMAL |
            interrupt << N_INTERRUPT; //% Bug fix 1.2.12.3&10 - S reg status bits wrong
    }
}

void CContext::set_status_reg_bits(uint8_t reg)
{
    negative = !!(reg & NEGATIVE);
    overflow = !!(reg & OVERFLOW);
    zero = !!(reg & ZERO);
    carry = !!(reg & CARRY);
    decimal = !!(reg & DECIMAL);
    interrupt = !!(reg & INTERRUPT);

    if (wxGetApp().m_global.GetProcType() == ProcessorType::WDC65816)
    {
        mem16 = !!(reg & MEMORY);
        xy16 = !!(reg & INDEX);
    }
    else
    {
        reserved = 1; //% Bug fix 1.2.12.3 BRK bit trouble
        break_bit = 1; //% Bug fix 1.2.12.3 BRK bit trouble
    }
}

uint32_t CSym6502::getVectorAddress(Vector v)
{
    switch (v)
    {
    case Vector::IRQ:
    case Vector::BRK:
        return 0x0000'FFFE;

    case Vector::RESET:
        return 0x0000'FFFC;

    case Vector::NMI:
        return 0x0000'FFFA;

    case Vector::ABORT:
        return 0x0000'FFF8;

    case Vector::COP:
        return 0x0000'FFF4;

    default:
        ASSERT(false);
        wxLogWarning(_("Invalid vector request"));
        return CAsm::INVALID_ADDRESS;
    }
}

//=============================================================================

uint32_t CSym6502::get_argument_address(bool bWrite)
{
    uint16_t arg;
    uint32_t addr;
    uint32_t pc = ctx.pc; // Save original PC
    uint8_t mode;

    if (cpu16)
        mode = m_vCodeToMode[ctx.mem[ctx.pc + (ctx.pbr << 16)]];
    else
        mode = m_vCodeToMode[ctx.mem[ctx.pc]];

    inc_prog_counter(); // Bypass the command

    extracycle = false; //% bug Fix 1.2.12.1 - fix cycle timing

    switch (mode)
    {
    case CAsm::A_ZPG:
    case CAsm::A_ZPG2:
        if (cpu16)
        {
            addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir) & 0xFFFF;

            if ((ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
            addr = ctx.mem[ctx.pc]; // address on zero page

        inc_prog_counter();
        break;

    case CAsm::A_ZPG_X:
        if (cpu16)
        {
            if (ctx.emm && ((ctx.dir & 0xFF) == 0))
                addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr << 16)] + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xff);
            else
                addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xffff;

            if ((ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
            addr = (ctx.mem[ctx.pc] + (ctx.x & 0xff)) & 0xff;

        inc_prog_counter();
        break;

    case CAsm::A_ZPG_Y:
        if (cpu16)
        {
            if (ctx.emm && (ctx.dir & 0xff) == 0)
            {
                addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr << 16)] + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y)) & 0xff); //**
            }
            else
                addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y)) & 0xffff;

            if ((ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
            addr = (ctx.mem[ctx.pc] + (ctx.y & 0xff)) & 0xff;

        inc_prog_counter();
        break;

    case CAsm::A_ZPGI: // 65c02 65816 only
        if (cpu16)
        {
            arg = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir) & 0xFFFF;
            addr = get_word_indirect(arg) + (ctx.dbr << 16);

            if ((ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
        {
            arg = ctx.mem[ctx.pc];
            addr = get_word_indirect(arg);
        }

        inc_prog_counter();
        break;

    case CAsm::A_ABS:
        if (cpu16)
            addr = get_word(ctx.pc + (ctx.pbr << 16)) + (ctx.dbr << 16);
        else
            addr = get_word(ctx.pc);

        inc_prog_counter(2);
        break;

    case CAsm::A_ABS_X:
        if (cpu16)
            addr = (get_word(ctx.pc + (ctx.pbr << 16)) + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) + (ctx.dbr << 16);
        else
            addr = get_word(ctx.pc) + (ctx.x & 0xff);

        if ((addr >> 8) != static_cast<uint32_t>(get_word(ctx.pc) >> 8))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter(2);
        break;

    case CAsm::A_ABS_Y:
        if (cpu16)
            addr = (get_word(ctx.pc + (ctx.pbr << 16)) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y)) + (ctx.dbr << 16);
        else
            addr = get_word(ctx.pc) + (ctx.y & 0xff);

        if ((addr >> 8) != static_cast<uint32_t>(get_word(ctx.pc) >> 8))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter(2);
        break;

    case CAsm::A_ZPGI_X:
        if (cpu16)
        {
            if (ctx.emm)
            {
                if ((ctx.dir & 0xFF) == 0x00)
                {
                    arg = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr << 16)] + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xFF);
                    addr = ctx.mem[arg] + (ctx.dbr << 16);
                    arg = ((arg + 1) & 0xFF) + (arg & 0xFF00);
                    addr += (ctx.mem[arg] << 8);
                }
                else
                {
                    arg = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr << 16)] + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xFFFF);
                    addr = ctx.mem[arg] + (ctx.dbr << 16);
                    arg = ((arg + 1) & 0xFF) + (arg & 0xFF00);
                    addr += (ctx.mem[arg] << 8);
                }
            }
            else
            {
                arg = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xFFFF;
                addr = get_word_indirect(arg) + (ctx.dbr << 16);
            }

            if ((ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
        {
            arg = ctx.mem[ctx.pc]; // cell address on zero page
            addr = get_word_indirect((arg + (ctx.x & 0xff)) & 0xff);
        }

        inc_prog_counter();
        break;

    case CAsm::A_ZPGI_Y:

        if (cpu16)
        {
            arg = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir) & 0xffff;
            addr = get_word_indirect(arg) + (ctx.dbr << 16) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
            if ((ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
        {
            arg = ctx.mem[ctx.pc]; // cell address on zero page
            addr = get_word_indirect(arg) + (ctx.y & 0xff);
        }

        if (uint16_t(addr >> 8u) != (get_word_indirect(arg) >> 8u))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter();
        break;

    case CAsm::A_ABSI: // only JMP(xxxx) supports this addr mode
        addr = (cpu16 && !ctx.emm) ? get_word(ctx.pc + (ctx.pbr << 16)) : get_word(ctx.pc);

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502 && (addr & 0xFF) == 0xFF) // LSB == 0xFF?
        {
            // Recreate 6502 bug
            uint16_t hAddr = addr - 0xFF;
            uint16_t lo = ctx.mem[addr];
            uint16_t hi = ctx.mem[hAddr];

            addr = lo | (hi << 8); // erroneously just as 6502 would do
        }
        else
            addr = get_word(addr);

        if (cpu16 && !ctx.emm)
            addr += (ctx.pbr << 16);

        inc_prog_counter(2);
        break;

    case CAsm::A_ABSI_X:
        if (cpu16)
        {
            addr = (get_word(ctx.pc + (ctx.pbr << 16)) + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xFFFF;
            addr = get_word(addr + (ctx.pbr << 16));
        }
        else
        {
            addr = get_word(ctx.pc) + (ctx.x & 0xFF);
            addr = get_word(addr);
        }

        inc_prog_counter(2);
        break;

    case CAsm::A_ZREL: // 65816 only
        addr = get_word(ctx.pc);
        inc_prog_counter(2);
        break;

    case CAsm::A_ABSL: // 65816 only
        addr = get_Lword(ctx.pc + (ctx.pbr << 16));
        inc_prog_counter(3);
        break;

    case CAsm::A_ABSL_X: // 65816 only
        addr = (get_Lword(ctx.pc + (ctx.pbr << 16)) + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0x00FFFFFF;
        inc_prog_counter(3);
        break;

    case CAsm::A_ZPIL: // 65816 only
        arg = ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir; // cell address on zero page
        addr = get_Lword_indirect(arg);
        inc_prog_counter();

        if ((ctx.dir & 0xFF) != 0)
            extracycle = true;

        break;

    case CAsm::A_ZPIL_Y: // 65816 only
        arg = ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir; // cell address on zero page
        addr = get_Lword_indirect(arg) + (ctx.dbr << 16) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y);

        inc_prog_counter();

        if ((ctx.dir & 0xFF) != 0)
            extracycle = true;

        break;

    case CAsm::A_SR: // 65816 only
        addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.s) & 0xFFFF; // address on zero page
        inc_prog_counter();
        break;

    case CAsm::A_SRI_Y:
        arg = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.s) & 0xFFFF; // address on zero page
        addr = get_word_indirect(arg) + (ctx.dbr << 16) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
        inc_prog_counter();
        break;

    case CAsm::A_INDL: // 65816 only
        arg = get_word(ctx.pc + (ctx.pbr << 16));
        addr = get_Lword_indirect(arg);
        inc_prog_counter(2);
        break;

    case CAsm::A_IMP:
    case CAsm::A_IMP2:
    case CAsm::A_ACC:
    case CAsm::A_IMM:
    default:
        ASSERT(false);
        return 0;
    }

    if (bWrite && addr >= s_uProtectFromAddr && addr <= s_uProtectToAddr)
    {
        ctx.pc = pc; // restore original value
        throw CAsm::SYM_ILL_WRITE;
    }

    return addr;
}

uint16_t CSym6502::get_argument_value(bool rmask)
{
    uint8_t arg;
    uint32_t addr;
    uint16_t val;
    uint8_t mode;

    if (cpu16)
        mode = m_vCodeToMode[ctx.mem[ctx.pc + (ctx.pbr << 16)]];
    else
        mode = m_vCodeToMode[ctx.mem[ctx.pc]];

    inc_prog_counter(); // bypass the command

    extracycle = false; //% bug Fix 1.2.12.1 - fix cycle timing

    switch (mode)
    {
    case CAsm::A_IMP:
    case CAsm::A_ACC:
        return 0;

    case CAsm::A_IMM:
        if (cpu16 && !ctx.emm)
        {
            val = ctx.mem[ctx.pc + (ctx.pbr << 16)];

            if (rmask)
            {
                inc_prog_counter();
                val += ctx.mem[ctx.pc + (ctx.pbr << 16)] << 8;
            }
        }
        else
            val = ctx.mem[ctx.pc];

        inc_prog_counter();
        return val;

    case CAsm::A_IMP2:
    case CAsm::A_REL:
        if (cpu16)
            arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];
        else
            arg = ctx.mem[ctx.pc];

        inc_prog_counter();
        return arg;

    case CAsm::A_ZPGI:
        if (cpu16)
        {
            arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];

            if (!ctx.emm)
            {
                addr = get_word_indirect((arg + ctx.dir) & 0xFFFF) + (ctx.dbr << 16);
                val = check_io_read(addr) ? io_function() : ctx.mem[addr];

                if (rmask)
                    val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

                if ((ctx.dir & 0xFF) != 0)
                    extracycle = true;

                inc_prog_counter();
                return val;
            }
            else
            {
                uint32_t adr1, adr2;
                adr1 = arg + ctx.dir;

                if ((ctx.dir & 0xFF) == 0)
                    adr2 = ((arg + 1) & 0xFF) + ctx.dir;
                else
                    adr2 = arg + 1 + ctx.dir;

                addr = ctx.mem[adr1] + (ctx.dbr << 16);
                addr += ctx.mem[adr2] << 8;

                val = check_io_read(addr) ? io_function() : ctx.mem[addr];

                if (rmask)
                    val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

                if ((ctx.dir & 0xff) != 0)
                    extracycle = true;

                inc_prog_counter();
                return val;
            }
        }
        else
        {
            arg = ctx.mem[ctx.pc]; // cell address on zero page
            addr = get_word_indirect(arg);
            inc_prog_counter();
            return check_io_read(addr) ? io_function() : ctx.mem[addr];
        }

    case CAsm::A_ZPG:
        if (cpu16)
        {
            addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir) & 0xFFFF;
            val = ctx.mem[addr];

            if (rmask)
                val += (ctx.mem[addr + 1]) << 8;

            if ((ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
            val = ctx.mem[ctx.mem[ctx.pc]] & 0xFF; // number at address

        inc_prog_counter();
        return val;

    case CAsm::A_ZPG_X:
        if (cpu16)
        {
            if (ctx.emm && ((ctx.dir & 0xFF) == 0))
                addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr << 16)] + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xFF); // adres
            else
                addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xffff; // adres

            val = ctx.mem[addr];

            if (rmask)
                val += (ctx.mem[addr + 1]) << 8;

            if ((ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
        {
            addr = (ctx.mem[ctx.pc] + (ctx.x & 0xFF)) & 0xFF;
            val = ctx.mem[addr]; // number at address
        }

        inc_prog_counter();
        return val;

    case CAsm::A_ZPG_Y:
        if (cpu16)
        {
            if (ctx.emm && ((ctx.dir & 0xFF) == 0))
                addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr << 16)] + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y)) & 0xFF);
            else
                addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.dir + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y)) & 0xFFFF;

            val = ctx.mem[addr];

            if (rmask)
                val += (ctx.mem[addr + 1]) << 8;

            if ((ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
            val = ctx.mem[(ctx.mem[ctx.pc] + (ctx.y & 0xFF)) & 0xFF];

        inc_prog_counter();
        return val;

    case CAsm::A_ABS:
        if (cpu16)
        {
            addr = (get_word(ctx.pc + (ctx.pbr << 16)) & 0xFFFF) + (ctx.dbr << 16);
            inc_prog_counter(2);

            val = check_io_read(addr) ? io_function() : ctx.mem[addr];

            if (rmask)
                val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

            return val;
        }
        else
        {
            addr = get_word(ctx.pc);
            inc_prog_counter(2);
            return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address
        }

    case CAsm::A_ABS_X:
        if (cpu16)
        {
            addr = get_word(ctx.pc + (ctx.pbr << 16)) + (ctx.dbr << 16) + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x);
            inc_prog_counter(2);
            val = check_io_read(addr) ? io_function() : ctx.mem[addr];

            if (rmask)
                val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

            if (!ctx.xy16)
                ctx.uCycles++;
            else
            {
                if (uint16_t(addr >> 8u) != (get_word(ctx.pc + (ctx.pbr << 16)) >> 8u))
                    extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
            }

            return val;
        }
        else
        {
            addr = get_word(ctx.pc) + (ctx.x & 0xFF);

            if (uint16_t(addr >> 8u) != (get_word(ctx.pc) >> 8u))
                extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        inc_prog_counter(2);
        return check_io_read(addr) ? io_function() : ctx.mem[addr];

    case CAsm::A_ABS_Y:
        if (cpu16)
        {
            addr = get_word(ctx.pc + (ctx.pbr << 16)) + (ctx.dbr << 16) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y);
            inc_prog_counter(2);
            val = check_io_read(addr) ? io_function() : ctx.mem[addr];

            if (rmask)
                val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

            if (!ctx.xy16)
                ctx.uCycles++;
            else
            {
                if (uint16_t(addr >> 8u) != (get_word(ctx.pc + (ctx.pbr << 16)) >> 8u))
                    extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
            }

            return val;
        }
        else
        {
            addr = get_word(ctx.pc) + (ctx.y & 0xff);
            if (uint16_t(addr >> 8u) != (get_word(ctx.pc) >> 8u))
                extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        inc_prog_counter(2);
        return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address

    case CAsm::A_ZPGI_X:
        uint32_t adr1, adr2;
        if (cpu16)
        {
            arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];

            if (ctx.emm)
            {
                if ((ctx.dir & 0xFF) == 0)
                {
                    adr1 = ((arg + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xFF) + ctx.dir;
                    adr2 = ((arg + 1 + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xFF) + ctx.dir;
                    addr = ctx.mem[adr1] + (ctx.dbr << 16);
                    addr += ctx.mem[adr2] << 8;
                }
                else
                {
                    adr1 = ((arg + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xFFFF) + ctx.dir;
                    adr2 = (adr1 & 0xFFFF00) + (((adr1 & 0xFF) + 1) & 0xFF);
                    addr = ctx.mem[adr1] + (ctx.dbr << 16);
                    addr += ctx.mem[adr2] << 8;
                }
            }
            else
                addr = get_word_indirect((arg + ctx.dir + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xFFFFF) + (ctx.dbr << 16);

            if ((ctx.dir & 0xFF) != 0)
                extracycle = true;

            inc_prog_counter();
            val = check_io_read(addr) ? io_function() : ctx.mem[addr];

            if (rmask)
                val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

            return val;
        }
        else
        {
            arg = ctx.mem[ctx.pc]; // cell address on zero page
            addr = get_word_indirect((arg + (ctx.x & 0xFF)) & 0xFF);
            inc_prog_counter();
            return check_io_read(addr) ? io_function() : ctx.mem[addr]; // number at address
        }

    case CAsm::A_ZPGI_Y:
        if (cpu16)
        {
            arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];

            if (!ctx.emm)
            {
                addr = ((get_word_indirect((arg + ctx.dir) & 0xFFFF) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y)) & 0xFFFFFF) + (ctx.dbr << 16);

                if ((ctx.dir & 0xFF) != 0)
                    ctx.uCycles++;

                inc_prog_counter();

                if (!ctx.xy16)
                {
                    ctx.uCycles++;

                    if (uint16_t(addr >> 8u) != (get_word_indirect(arg + ctx.dir) >> 8u))
                        extracycle = true;
                }

                val = check_io_read(addr) ? io_function() : ctx.mem[addr];

                if (rmask)
                    val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

                return val;
            }
            else
            {
                addr = ((get_word_indirect((arg + ctx.dir) & 0xFFFF) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y)) & 0xFFFF) + (ctx.dbr << 16);

                if ((ctx.dir & 0xFF) != 0)
                    extracycle = true;

                inc_prog_counter();

                if (!ctx.xy16)
                {
                    ctx.uCycles++;

                    if (uint16_t(addr >> 8u) != (get_word_indirect(arg + ctx.dir) >> 8u))
                        extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
                }

                val = check_io_read(addr) ? io_function() : ctx.mem[addr];

                if (rmask)
                    val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

                return val;
            }
        }
        else
        {
            arg = ctx.mem[ctx.pc]; // cell address on zero page
            addr = get_word_indirect(arg) + (ctx.y & 0xFF);
        }

        if (uint16_t(addr >> 8u) != (get_word_indirect(arg) >> 8u))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter();
        return check_io_read(addr) ? io_function() : ctx.mem[addr];

    case CAsm::A_ABSL: // 65816 only
        addr = get_Lword(ctx.pc + (ctx.pbr << 16));
        inc_prog_counter(3);
        val = check_io_read(addr) ? io_function() : ctx.mem[addr];

        if (rmask)
            val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

        return val;

    case CAsm::A_ABSL_X: // 65816 only
        addr = (get_Lword(ctx.pc + (ctx.pbr << 16)) + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xFFFFFF;
        inc_prog_counter(3);
        val = check_io_read(addr) ? io_function() : ctx.mem[addr];

        if (rmask)
            val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

        return val;

    case CAsm::A_ZPIL: // 65816 only
        arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];
        addr = get_Lword_indirect((arg + ctx.dir) & 0xFFFF);
        inc_prog_counter();

        if ((ctx.dir & 0xFF) != 0)
            extracycle = true;

        val = check_io_read(addr) ? io_function() : ctx.mem[addr];

        if (rmask)
            val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

        return val;

    case CAsm::A_ZPIL_Y: // 65816 only
        arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];
        addr = (get_Lword_indirect((arg + ctx.dir) & 0xFFFF) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y)) & 0xFFFFFF;
        inc_prog_counter();

        if ((ctx.dir & 0xFF) != 0)
            extracycle = true;

        val = check_io_read(addr) ? io_function() : ctx.mem[addr];

        if (rmask)
            val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

        return val;

    case CAsm::A_SR: // 65816 only
        addr = (ctx.mem[ctx.pc + (ctx.pbr << 16)] + ctx.s) & 0xFFFF;
        val = ctx.mem[addr];

        if (rmask)
            val += (ctx.mem[addr + 1]) << 8;

        inc_prog_counter();
        return val;

    case CAsm::A_SRI_Y: // 65816 only
        arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];
        addr = (get_word_indirect(((arg + ctx.s) & 0xFFFF) + (ctx.dbr << 16)) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y) + (ctx.dbr << 16)) & 0xFFFFFF;
        inc_prog_counter();
        val = check_io_read(addr) ? io_function() : ctx.mem[addr];

        if (rmask)
            val += (check_io_read(addr + 1) ? io_function() : ctx.mem[addr + 1]) << 8;

        return val;

    case CAsm::A_RELL:
    case CAsm::A_ABSI_X:
    case CAsm::A_ABSI: // These modes are only supported through get_argument_address()
    case CAsm::A_ZREL:
    case CAsm::A_ZPG2:
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
    catch (CAsm::SymStat s)
    {
        return s;
    }
}

CAsm::SymStat CSym6502::perform_command()
{
    uint8_t cmd;
    uint16_t arg, acc;
    uint32_t addr, acc32;
    uint8_t zero, overflow, carry, negative;
    uint8_t zeroc, negativec;
    uint8_t acc8, arg8;

    int tmp;

#define TOBCD(a) (((((a) / 10) % 10) << 4) | ((a) % 10))
#define TOBIN(a) (((a) >> 4) * 10 + ((a) & 0x0F))

    cpu16 = !!(wxGetApp().m_global.GetProcType() == ProcessorType::WDC65816);
    wxGetApp().m_global.m_bBank = cpu16;
    wxGetApp().m_global.m_bPBR = ctx.pbr;

    if (cpu16 && !ctx.emm && !ctx.mem16)
        ctx.a = (ctx.a & 0xFF) | (ctx.b << 8);

    if (m_nInterruptTrigger != NONE)
    {
        interrupt(m_nInterruptTrigger);
        cmd = ctx.mem[ctx.pc];
    }

    if (cpu16)
        cmd = ctx.mem[ctx.pc + (ctx.pbr << 16)];
    else
        cmd = ctx.mem[ctx.pc];

    pre = ctx;
    pre.intFlag = false;

    if (pre.uCycles > saveCycles)
        pre.intFlag = true;

    pre.uCycles = saveCycles;
    //% End Bug Fixs

    switch (m_vCodeToCommand[cmd])
    {
    case CAsm::C_ADC:
        carry = ctx.carry;

        if (cpu16 && !ctx.mem16 && !ctx.emm)
        {
            arg = get_argument_value(true);
            acc = ctx.a;

            if (ctx.decimal)
            {
                addr = (acc & 0x0f) + (arg & 0x0f) + (carry ? 1 : 0);
                if ((addr & 0xff) > 9) addr += 0x06;
                addr += (acc & 0xf0) + (arg & 0xf0);
                if ((addr & 0xff0) > 0x9F) addr += 0x60;
                addr += (acc & 0xf00) + (arg & 0xf00);
                if ((addr & 0xff00) > 0x9FF) addr += 0x0600;
                addr += (acc & 0xf000) + (arg & 0xf000);
                if ((addr & 0xff000) > 0x9FFF) addr += 0x06000;
                if (addr > 0xffff) carry = true; else carry = false;
                if ((addr & 0xffff) == 0) zero = true; else zero = false;
                if (addr & 0x8000) negative = true; else negative = false;

                overflow = ((acc & 0x8000u) == (arg & 0x8000u)) && (addr & 0x8000u) != (acc & 0x8000u);

                if (ctx.decimal && (addr & 0xff000) > 0x19000) overflow = false;
                ctx.a = (addr & 0xff);
                ctx.b = (addr >> 8) & 0xff;
            }
            else
            {
                addr = acc + arg + (carry ? 1 : 0);
                if (addr > 65535) carry = true; else carry = false;
                if ((addr & 0xffff) == 0) zero = true; else zero = false;
                if (addr & 0x8000) negative = true; else negative = false;

                overflow = ((acc & 0x8000u) == (arg & 0x8000u)) && (addr & 0x8000u) != (acc & 0x8000u);

                ctx.a = addr & 0xffff;
                ctx.b = (addr >> 8) & 0xff;
            }

            ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
            ctx.uCycles++;   // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);

            arg8 = static_cast<uint8_t>(arg & 0xFF);
            acc8 = static_cast<uint8_t>(ctx.a & 0xFF);

            if (ctx.decimal)
            {
                // Decimal addition
                tmp = acc8 + arg8 + (ctx.carry ? 1 : 0);
                zero = (tmp & 0xFF) == 0;

                bool af = ((acc8 ^ arg8) & 0x80) == 0;
                bool at = ((acc8 ^ tmp) & 0x80) != 0;

                ctx.overflow = af && at;

                int test = (acc8 & 0x0F) + (arg8 & 0x0F) + (ctx.carry ? 1 : 0);

                if (test > 9)
                    tmp += 6;

                if (tmp > 0x99)
                    tmp += 96;

                acc = (uint8_t)(tmp & 0xFF);
                zeroc = acc == 0;

                bool isM6502 = wxGetApp().m_global.m_procType == ProcessorType::M6502;

                ctx.zero = isM6502 ? !!zero : !!zeroc;
                ctx.carry = tmp > 0x99;
                ctx.negative = (acc & 0x80) != 0;

                if (!isM6502) //% bug Fix 1.2.12.1 - fix cycle timing
                    ctx.uCycles++; // Add a cycle in BCD mode
            }
            else
            {
                // Binary addition
                tmp = acc8 + arg8 + (ctx.carry ? 1 : 0);

                bool af = ((acc8 ^ arg8) & 0x80) == 0;
                bool at = ((acc8 ^ tmp) & 0x80) != 0;

                acc = (uint8_t)(tmp & 0xFF);

                overflow = af && at;
                zero = acc == 0;
                negative = (acc & 0x80) != 0;
                carry = tmp > 0xFF;

                ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
            }
        }

        ctx.a = acc;

        if (cpu16 && extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_SBC:
        carry = ctx.carry;

        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_argument_value(true);
            acc = ctx.a;

            if (ctx.decimal)
            {
                arg = ~arg;
                addr = (acc & 0x0F) + (arg & 0x0F) + (carry ? 1 : 0);
                if (addr <= 0x0F) addr = ((addr - 0x06) & 0x0f) + (addr & 0xfff0);
                addr = (acc & 0xF0) + (arg & 0xF0) + (addr > 0x0F ? 0x10 : 0) + (addr & 0x0F);
                if (addr <= 0xFF) addr = ((addr - 0x60) & 0xF0) + (addr & 0xff0f);
                addr = (acc & 0xF00) + (arg & 0xF00) + (addr > 0xFF ? 0x100 : 0) + (addr & 0xFF);
                if (addr <= 0xFFF) addr = ((addr - 0x0600) & 0x0f00) + (addr & 0xf0ff);
                addr = (acc & 0xF000) + (arg & 0xF000) + (addr > 0xFFF ? 0x1000 : 0) + (addr & 0xFFF);
                if (~(acc ^ arg) & (acc ^ addr) & 0x8000) overflow = true; else overflow = false;
                if (addr <= 0xFFFF) addr = ((addr - 0x6000) & 0xf000) + (addr & 0xf0fff);
                if (addr > 0xFFFF) carry = true; else carry = false;
                if ((addr & 0xffff) == 0) zero = true; else zero = false;
                if ((addr & 0x8000) != 0) negative = true; else negative = false;
                ctx.a = addr & 0xffff;
                ctx.b = (addr >> 8) & 0xff;
            }
            else
            {
                addr = acc - arg - (carry ? 0 : 1);
                if (addr > 65535) carry = false; else carry = true;
                if ((addr & 0xffff) == 0) zero = true; else zero = false;
                if (addr & 0x8000) negative = true; else negative = false;
                overflow = ((acc & 0x8000u) == (arg & 0x8000u)) && (addr & 0x8000u) != (acc & 0x8000u);
                ctx.a = addr & 0xffff;
                ctx.b = (addr >> 8) & 0xff;
            }
            ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
            ctx.uCycles++;   // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            acc = ctx.a;
            carry = ctx.carry;

            if (ctx.decimal)
            {
                // Decimal subtraction
                int lo = (acc & 0x0F) - (arg & 0x0F) - (carry ? 0 : 1);
                int hi = (acc & 0xF0) - (arg & 0xF0);

                zero = lo == 0 && hi == 0;

                bool af = lo & 0x10; // Half carry flag

                if ((lo & 0x0F) > 9 || af)
                {
                    lo -= 6;
                    --hi;
                }

                negative = hi & 0x80;
                overflow = ((acc ^ hi) & 0x80) && ((acc ^ arg) & 0x80);

                if (hi > 0x99 || (hi & 0x100))
                {
                    hi -= 0x60;
                    carry = true;
                }
                else
                    carry = false;

                acc = (uint8_t)(((hi & 0xF0) | (lo & 0x0F)) & 0xFF);

                negativec = (acc & 0x80) != 0;
                zeroc = acc == 0;

                bool isM6502 = wxGetApp().m_global.m_procType == ProcessorType::M6502;

                ctx.carry = !carry; // Carray negation in accordance with convention 6502
                ctx.overflow = !!overflow;
                ctx.negative = (isM6502 ? !!negative : !!negativec);
                ctx.zero = (isM6502 ? !!zero : !!zeroc);

                if (!isM6502) //% bug Fix 1.2.12.1 - fix cycle timing
                    ctx.uCycles++; // Add a cycle in BCD mode
            }
            else
            {
                tmp = acc - arg - (carry ? 0 : 1);

                overflow = ((acc ^ tmp) & 0x80) && ((acc ^ arg) & 0x80);
                carry = tmp < 0;
                acc = (uint8_t)(tmp & 0xFF);
                zero = acc == 0;
                negative = acc & 0x80;

                ctx.set_status_reg_VZNC(overflow, zero, negative, !carry);
            }

            ctx.a = acc;
        }

        if (cpu16 && extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case CAsm::C_CMP:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_argument_value(true);
            acc = ctx.a;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFFFF) == 0;
            negative = tmp & 0x8000;

            ctx.uCycles++;   // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false) & 0xFF;
            acc = ctx.a;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFF) == 0;
            negative = tmp & 0x80;
        }

        ctx.set_status_reg_ZNC(zero, negative, carry);

        if (cpu16 && extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_CPX:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            arg = get_argument_value(true);
            acc = ctx.x;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFFFF) == 0;
            negative = tmp & 0x8000;
        }
        else
        {
            arg = get_argument_value(false) & 0xFF;
            acc = ctx.x & 0xFF;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFF) == 0;
            negative = tmp & 0x80;
        }

        ctx.set_status_reg_ZNC(zero, negative, !carry);
        break;

    case CAsm::C_CPY:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            arg = get_argument_value(true);
            acc = ctx.y;

            // Compare always in binary, don't set acc
            tmp = acc - arg - (ctx.carry ? 0 : 1);

            carry = tmp < 0;
            zero = (tmp & 0xFFFF) == 0;
            negative = tmp & 0x8000;
        }
        else
        {
            arg = get_argument_value(false) & 0xFF;
            acc = ctx.y & 0xFF;

            // Compare always in binary, don't set acc
            tmp = acc - arg - (ctx.carry ? 0 : 1);

            carry = tmp < 0;
            zero = (tmp & 0xFF) == 0;
            negative = tmp & 0x80;
        }

        ctx.set_status_reg_ZNC(zero, negative, !carry);
        break;

    case CAsm::C_ASL:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();

                acc32 = ctx.a << 1;
                carry = !!(ctx.a & 0x8000);
                zero = !!((acc32 & 0xffff) == 0);
                negative = !!(acc32 & 0x8000);

                ctx.a = acc32 & 0xffff;
                ctx.b = (acc32 >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = get_word(addr);

                acc32 = acc << 1;
                carry = !!(acc & 0x8000);
                zero = !!((acc32 & 0xffff) == 0);
                negative = !!(acc32 & 0x8000);

                ctx.mem[addr] = acc32 & 0xff;
                ctx.mem[addr + 1] = (acc32 >> 8) & 0xff;
                ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = ctx.a;

                carry = acc & 0x80;
                acc <<= 1;
                zero = acc == 0;
                negative = acc & 0x80;

                ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = ctx.mem[addr];

                carry = acc & 0x80;
                acc <<= 1;
                zero = (acc & 0xFF) == 0;
                negative = acc & 0x80;

                ctx.mem[addr] = static_cast<uint8_t>(acc & 0xFF);
            }
        }

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502 && extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_LSR:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();
                acc = ctx.a >> 1;
                carry = !!(ctx.a & 0x01);
                zero = !!(acc == 0);
                negative = !!(acc & 0x8000);
                ctx.a = acc;
                ctx.b = (acc >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                arg = get_word(addr);
                acc = arg >> 1;
                carry = !!(arg & 0x01);
                zero = !!((acc & 0xffff) == 0);
                negative = !!(acc & 0x8000);
                ctx.mem[addr++] = acc & 0xff;
                ctx.mem[addr] = (acc >> 8) & 0xff;
                ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = ctx.a;

                carry = acc & 0x01;
                acc >>= 1;
                zero = acc == 0;
                negative = acc & 0x80;

                ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = ctx.mem[addr];

                carry = acc & 0x01;
                acc >>= 1;
                zero = acc == 0;
                negative = acc & 0x80;

                ctx.mem[addr] = acc;
            }
        }

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502 && extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_ROL:
        carry = ctx.carry;
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();
                acc = (ctx.a << 1) + (carry ? 1 : 0);
                carry = !!(ctx.a & 0x8000);
                zero = !!(acc == 0);
                negative = !!(acc & 0x8000);
                ctx.a = acc;
                ctx.b = (acc >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                arg = get_word(addr);
                acc = (arg << 1) + (carry ? 1 : 0);
                carry = !!(arg & 0x8000);
                zero = !!((acc & 0xffff) == 0);
                negative = !!(acc & 0x8000);
                ctx.mem[addr++] = acc & 0xff;
                ctx.mem[addr] = (acc >> 8) & 0xff;
                ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = ctx.a;

                carry = acc & 0x80;
                acc <<= 1;

                if (ctx.carry)
                    acc |= 1;
                else
                    acc &= ~1;

                carry = acc > 0xFF;
                zero = acc == 0;
                negative = acc & 0x80;

                ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = ctx.mem[addr];

                carry = acc & 0x80;
                acc <<= 1;

                if (ctx.carry)
                    acc |= 1;
                else
                    acc &= ~1;

                carry = acc > 0xFF;
                zero = acc == 0;
                negative = acc & 0x80;

                ctx.mem[addr] = acc;
            }
        }

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502 && extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_ROR:
        carry = ctx.carry;
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();
                acc = (ctx.a >> 1) + (carry ? 0x8000 : 0);;
                carry = !!(ctx.a & 0x01);
                zero = !!(acc == 0);
                negative = !!(acc & 0x8000);
                ctx.a = acc;
                ctx.b = (acc >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                arg = get_word(addr);
                acc = (arg >> 1) + (carry ? 0x8000 : 0);
                carry = !!(arg & 0x01);
                zero = !!((acc & 0xffff) == 0);
                negative = !!(acc & 0x8000);
                ctx.mem[addr++] = acc & 0xff;
                ctx.mem[addr] = (acc >> 8) & 0xff;
                ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = ctx.a;

                carry = acc & 0x01;
                acc >>= 1;

                if (ctx.carry)
                {
                    acc |= 0x80;
                    negative = true;
                }
                else
                {
                    acc &= ~0x80;
                    negative = false;
                }

                ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = ctx.mem[addr];

                carry = acc & 0x01;
                acc >>= 1;

                if (ctx.carry)
                {
                    acc |= 0x80;
                    negative = true;
                }
                else
                {
                    acc &= ~0x80;
                    negative = false;
                }

                ctx.mem[addr] = acc;
            }
            zero = acc == 0;
        }

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502 && extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_AND:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_argument_value(true);
            ctx.a &= arg;
            ctx.b = (ctx.a >> 8) & 0xFF;
            ctx.set_status_reg16(ctx.a);
            ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            ctx.a &= (arg & 0xFF);
            ctx.set_status_reg(ctx.a);
        }

        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_ORA:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_argument_value(true);
            ctx.a |= arg;
            ctx.b = (ctx.a >> 8) & 0xFF;
            ctx.set_status_reg16(ctx.a);
            ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            ctx.a |= (arg & 0xFF);
            ctx.set_status_reg(ctx.a);
        }

        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_EOR:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_argument_value(true);
            ctx.a ^= arg;
            ctx.b = (ctx.a >> 8) & 0xFF;
            ctx.set_status_reg16(ctx.a);
            ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            ctx.a = (ctx.a & 0xFF) ^ (arg & 0xFF);
            ctx.set_status_reg(ctx.a);
        }

        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_INC:
        if (m_vCodeToMode[cmd] == CAsm::A_ACC)
        {
            inc_prog_counter();
            ctx.a++;

            if (cpu16 && !ctx.emm && !ctx.mem16)
            {
                ctx.b = (ctx.a >> 8) & 0xFF;
                ctx.set_status_reg16(ctx.a);
            }
            else
                ctx.set_status_reg(ctx.a);
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);

            if (cpu16 && !ctx.emm && !ctx.mem16)
            {
                tmp = get_word(addr);
                ++tmp;
                ctx.set_status_reg16(tmp);
                ctx.mem[addr] = tmp & 0xFF;
                ctx.mem[addr + 1] = (tmp >> 8) & 0xFF;
                ctx.uCycles += 2; // 16 bit operation adds 2 cycles
            }
            else
            {
                ctx.mem[addr]++;
                ctx.set_status_reg(ctx.mem[addr]);
            }

            if (cpu16 && extracycle)
                ctx.uCycles++;
        }
        break;

    case CAsm::C_DEC:
        if (m_vCodeToMode[cmd] == CAsm::A_ACC)
        {
            inc_prog_counter();
            ctx.a--;

            if (cpu16 && !ctx.emm && ctx.mem16)
            {
                ctx.b = (ctx.a >> 8) & 0xFF;
                ctx.set_status_reg16(ctx.a);
            }
            else
                ctx.set_status_reg(ctx.a);
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);

            if (cpu16 && !ctx.emm && !ctx.mem16)
            {
                tmp = get_word(addr);
                --tmp;
                ctx.set_status_reg16(tmp);
                ctx.mem[addr] = tmp & 0xFF;
                ctx.mem[addr + 1] = (tmp >> 8) & 0xFF;
                ctx.uCycles += 2; // 16 bit operation adds 2 cycles;
            }
            else
            {
                ctx.mem[addr]--;
                ctx.set_status_reg(ctx.mem[addr]);
            }

            if (cpu16 && extracycle)
                ctx.uCycles++;
        }
        break;

    case CAsm::C_INX:
        inc_prog_counter();
        ctx.x++;

        if (cpu16 && !ctx.emm && !ctx.xy16)
            ctx.set_status_reg16(ctx.x);
        else
            ctx.set_status_reg(ctx.x & 0xFF);

        break;

    case CAsm::C_DEX:
        inc_prog_counter();
        ctx.x--;

        if (cpu16 && !ctx.emm && !ctx.xy16)
            ctx.set_status_reg16(ctx.x);
        else
            ctx.set_status_reg(ctx.x & 0xFF);

        break;

    case CAsm::C_INY:
        inc_prog_counter();
        ctx.y++;

        if (cpu16 && !ctx.emm && !ctx.xy16)
            ctx.set_status_reg16(ctx.y);
        else
            ctx.set_status_reg(ctx.y & 0xFF);

        break;

    case CAsm::C_DEY:
        inc_prog_counter();
        ctx.y--;

        if (cpu16 && !ctx.emm && !ctx.xy16)
            ctx.set_status_reg16(ctx.y);
        else
            ctx.set_status_reg(ctx.y & 0xFF);

        break;

    case CAsm::C_TAX:
        inc_prog_counter();
        ctx.x = ctx.a;

        if (cpu16 && !ctx.emm && !ctx.xy16)
            ctx.set_status_reg16(ctx.x);
        else
            ctx.set_status_reg(ctx.x & 0xFF);

        break;

    case CAsm::C_TXA:
        inc_prog_counter();
        ctx.a = ctx.x;

        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            ctx.b = (ctx.a >> 8) & 0xFF;
            ctx.set_status_reg16(ctx.a);
        }
        else
            ctx.set_status_reg(ctx.a & 0xFF);

        break;

    case CAsm::C_TAY:
        inc_prog_counter();
        ctx.y = ctx.a;

        if (cpu16 && !ctx.emm && !ctx.xy16)
            ctx.set_status_reg16(ctx.y);
        else
            ctx.set_status_reg(ctx.y & 0xFF);

        break;

    case CAsm::C_TYA:
        inc_prog_counter();
        ctx.a = ctx.y;

        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            ctx.set_status_reg16(ctx.a);
            ctx.b = (ctx.a >> 8) & 0xff;
        }
        else
            ctx.set_status_reg(ctx.a & 0xff);

        break;

    case CAsm::C_TSX:
        inc_prog_counter();

        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            ctx.x = ctx.s;
            ctx.set_status_reg16(ctx.x);
        }
        else
        {
            ctx.x = ctx.s & 0xFF;
            ctx.set_status_reg(ctx.x & 0xFF);
        }
        break;

    case CAsm::C_TXS:
        inc_prog_counter();
        if (cpu16 && !ctx.emm)
        {
            ctx.s = ctx.x;
            wxGetApp().m_global.m_bSRef = ctx.s;
        }
        else
        {
            ctx.s = ctx.x & 0xff;
            wxGetApp().m_global.m_bSRef = 0x1ff;
        }
        break;

    case CAsm::C_STA:
        addr = get_argument_address(s_bWriteProtectArea);
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            if (check_io_write(addr))
            {
                io_function(ctx.a & 0xFF);

                if (check_io_write(addr + 1))
                    io_function((ctx.a >> 8) & 0xFF);
                else
                    ctx.mem[addr + 1] = (ctx.a >> 8) & 0xFF;
            }
            else
            {
                ctx.mem[addr] = ctx.a & 0xFF;

                if (check_io_write(addr + 1))
                    io_function((ctx.a >> 8) & 0xFF);
                else
                    ctx.mem[addr + 1] = (ctx.a >> 8) & 0xFF;
            }

            ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
        {
            if (check_io_write(addr))
                io_function(ctx.a & 0xFF);
            else
                ctx.mem[addr] = ctx.a & 0xFF;
        }

        if (cpu16 && extracycle)
            ctx.uCycles++;

        break;

    case CAsm::C_STX:
        addr = get_argument_address(s_bWriteProtectArea);

        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            if (check_io_write(addr))
            {
                io_function(ctx.x & 0xFF);

                if (check_io_write(addr + 1))
                    io_function((ctx.x >> 8) & 0xFF);
                else
                    ctx.mem[addr + 1] = (ctx.x >> 8) & 0xFF;
            }
            else
            {
                ctx.mem[addr] = ctx.x & 0xFF;

                if (check_io_write(addr + 1))
                    io_function((ctx.x >> 8) & 0xFF);
                else
                    ctx.mem[addr + 1] = (ctx.x >> 8) & 0xFF;
            }

            ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
        {
            if (check_io_write(addr))
                io_function(ctx.x & 0xFF);
            else
                ctx.mem[addr] = ctx.x & 0xFF;
        }

        if (cpu16 && extracycle)
            ctx.uCycles++;

        break;

    case CAsm::C_STY:
        addr = get_argument_address(s_bWriteProtectArea);
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            if (check_io_write(addr))
            {
                io_function(ctx.y & 0xFF);

                if (check_io_write(addr + 1))
                    io_function((ctx.y >> 8) & 0xFF);
                else
                    ctx.mem[addr + 1] = (ctx.y >> 8) & 0xFF;
            }
            else
            {
                ctx.mem[addr] = ctx.y & 0xFF;

                if (check_io_write(addr + 1))
                    io_function((ctx.y >> 8) & 0xFF);
                else
                    ctx.mem[addr + 1] = (ctx.y >> 8) & 0xFF;
            }

            ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
        {
            if (check_io_write(addr))
                io_function(ctx.y & 0xFF);
            else
                ctx.mem[addr] = ctx.y & 0xFF;
        }

        if (cpu16 && extracycle)
            ctx.uCycles++;

        break;

    case CAsm::C_LDA:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            ctx.set_status_reg16(ctx.a = get_argument_value(true));
            ctx.b = (ctx.a >> 8) & 0xFF;
            ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
            ctx.set_status_reg((ctx.a = (get_argument_value(false) & 0xFF)) & 0xFF);

        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_LDX:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            ctx.set_status_reg16(ctx.x = get_argument_value(true));
            ctx.uCycles++;  // add 1 cycle for 16 bit operation 

        }
        else
            ctx.set_status_reg((ctx.x = (get_argument_value(false) & 0xFF)) & 0xFF);

        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_LDY:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            ctx.set_status_reg16(ctx.y = get_argument_value(true));
            ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
            ctx.set_status_reg((ctx.y = (get_argument_value(false) & 0xFF)) & 0xFF);

        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case CAsm::C_BIT:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_argument_value(true);
            ctx.zero = ((ctx.a & arg) == 0);
            ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            ctx.zero = (((ctx.a & 0xFF) & (arg & 0xFF)) == 0);
        }

        if (cmd != 0x89) // 65C02/816 BIT # only updates Z flag
        {
            if (cpu16 && !ctx.emm && !ctx.mem16)
            {
                ctx.negative = (arg & 0x8000) != 0;
                ctx.overflow = (arg & 0x4000) != 0;
            }
            else
            {
                ctx.negative = (arg & 0x80) != 0;
                ctx.overflow = (arg & 0x40) != 0;
            }
        }

        if (extracycle)
            ctx.uCycles++; //% bug Fix 1.2.13.1 - fix cycle timing
        break;

    case CAsm::C_PHA:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            push_addr_on_stack(ctx.a);
            ctx.uCycles++;
        }
        else
            push_on_stack(ctx.a & 0xFF);

        inc_prog_counter();
        break;

    case CAsm::C_PLA:
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            ctx.a = pull_addr_from_stack();
            ctx.b = (ctx.a >> 8) & 0xFF;
            ctx.set_status_reg16(ctx.a);
            ctx.uCycles++;
        }
        else
        {
            ctx.a = pull_from_stack();
            ctx.set_status_reg(ctx.a & 0xFF);
        }

        inc_prog_counter();
        break;

    case CAsm::C_PHP:
        inc_prog_counter();
        if (cpu16 && !ctx.emm)
            push_on_stack(ctx.get_status_reg());
        else
            push_on_stack(ctx.get_status_reg() | CContext::BREAK | CContext::RESERVED);
        break;

    case CAsm::C_PLP:
        inc_prog_counter();
        ctx.set_status_reg_bits(pull_from_stack());
        if (wxGetApp().m_global.GetProcType() != ProcessorType::M6502)
        {
            // 65C02 mode
            ctx.reserved = true; // 'reserved' bit always set
            ctx.break_bit = true;
        }
        break;

    case CAsm::C_JSR:
        addr = get_argument_address(false);
        if (cpu16 && (cmd == 0xFC) && ctx.emm)
        {
            ctx.emm = false;
            push_addr_on_stack((ctx.pc - 1) & 0xFFFF);
            ctx.emm = true;
        }
        else
            push_addr_on_stack((ctx.pc - 1) & 0xFFFF);
        ctx.pc = addr;
        break;

    case CAsm::C_JMP:
        addr = get_argument_address(false) & 0xFFFF;
        ctx.pc = addr;
        break;

    case CAsm::C_RTS:
        if ((finish == CAsm::FIN_BY_RTS) && ((ctx.s & 0xFF) == 0xFF)) // RTS on empty stack?
            return CAsm::SYM_FIN;
        ctx.pc = (pull_addr_from_stack() + 1) & 0xFFFF;
        break;

    case CAsm::C_RTI:
        ctx.set_status_reg_bits(pull_from_stack());
        ctx.pc = pull_addr_from_stack();
        if (cpu16 && !ctx.emm)
        {
            ctx.pbr = pull_from_stack();
            wxGetApp().m_global.m_bPBR = ctx.pbr;
            ctx.uCycles++; // native mode takes 1 extra cycle
        }
        break;

    case CAsm::C_BCC:
        arg = get_argument_value(false);
        if (!ctx.carry)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump forward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BCS:
        arg = get_argument_value(false);
        if (ctx.carry)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BVC:
        arg = get_argument_value(false);
        if (!ctx.overflow)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BVS:
        arg = get_argument_value(false);
        if (ctx.overflow)
        {
            AddBranchCycles(arg && 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BNE:
        arg = get_argument_value(false);
        if (!ctx.zero)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BEQ:
        arg = get_argument_value(false);
        if (ctx.zero)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BPL:
        arg = get_argument_value(false);
        if (!ctx.negative)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BMI:
        arg = get_argument_value(false);
        if (ctx.negative)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump foward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_NOP:
        inc_prog_counter();
        break;

    case CAsm::C_CLI:
        inc_prog_counter();
        ctx.interrupt = false;
        break;

    case CAsm::C_SEI:
        inc_prog_counter();
        ctx.interrupt = true;
        break;

    case CAsm::C_CLD:
        inc_prog_counter();
        ctx.decimal = false;
        break;

    case CAsm::C_SED:
        inc_prog_counter();
        ctx.decimal = true;
        break;

    case CAsm::C_CLC:
        inc_prog_counter();
        ctx.carry = false;
        break;

    case CAsm::C_SEC:
        inc_prog_counter();
        ctx.carry = true;
        break;

    case CAsm::C_CLV:
        inc_prog_counter();
        ctx.overflow = false;
        break;

    case CAsm::C_BRK:
        if (finish == CAsm::FIN_BY_BRK) // BRK instruction terminates the program?
            return CAsm::SYM_FIN;

        if (cpu16 && !ctx.emm)
        {
            push_on_stack(ctx.pbr);
            push_addr_on_stack(ctx.pc);
            push_on_stack(ctx.get_status_reg());
            ctx.decimal = false;
            ctx.pc = get_brk_addr16();
            ctx.uCycles++; // native mode takes 1 extra cycle
        }
        else
        {
            push_addr_on_stack(ctx.pc);
            ctx.break_bit = true;
            push_on_stack(ctx.get_status_reg() | CContext::RESERVED);
            if (wxGetApp().m_global.GetProcType() != ProcessorType::M6502)
                ctx.decimal = false;
            ctx.pc = get_irq_addr();
        }
        ctx.interrupt = true; // after pushing status
        ctx.pbr = 0;
        wxGetApp().m_global.m_bPBR = ctx.pbr;
        break;

        //---------- 65c02 --------------------------------------------------------

    case CAsm::C_PHX:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            push_addr_on_stack(ctx.x);
            ctx.uCycles++;
        }
        else
            push_on_stack(ctx.x & 0xFF);
        inc_prog_counter();
        break;

    case CAsm::C_PLX:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            ctx.x = pull_addr_from_stack();
            ctx.set_status_reg16(ctx.x);
            ctx.uCycles++;
        }
        else
        {
            ctx.x = pull_from_stack();
            ctx.set_status_reg(ctx.x & 0xFF);
        }
        inc_prog_counter();
        break;

    case CAsm::C_PHY:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            push_addr_on_stack(ctx.y);
            ctx.uCycles++;
        }
        else
            push_on_stack(ctx.y & 0xFF);
        inc_prog_counter();
        break;

    case CAsm::C_PLY:
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            ctx.y = pull_addr_from_stack();
            ctx.set_status_reg16(ctx.y);
            ctx.uCycles++;
        }
        else
        {
            ctx.y = pull_from_stack();
            ctx.set_status_reg(ctx.y & 0xFF);
        }
        inc_prog_counter();
        break;

    case CAsm::C_BRA:
        arg = get_argument_value(false);
        AddBranchCycles(arg & 0xFF);
        if (arg & 0x80) // jump back
            ctx.pc -= 0x100 - arg;
        else // jump forward
            ctx.pc += arg;
        break;

    case CAsm::C_INA:
        inc_prog_counter();
        ctx.a++;
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            ctx.set_status_reg16(ctx.a);
            ctx.b = (ctx.a >> 8) & 0xFF;
        }
        else
            ctx.set_status_reg(ctx.a & 0xFF);
        break;

    case CAsm::C_DEA:
        inc_prog_counter();
        ctx.a--;
        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            ctx.set_status_reg16(ctx.a);
            ctx.b = (ctx.a >> 8) & 0xFF;
        }
        else
            ctx.set_status_reg(ctx.a & 0xFF);

        break;

    case CAsm::C_STZ:
        addr = get_argument_address(s_bWriteProtectArea);
        if (check_io_write(addr))
        {
            uint8_t a = 0;
            io_function(a);
        }
        else
            ctx.mem[addr] = 0;

        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            addr++;
            if (check_io_write(addr))
                io_function(0);
            else
                ctx.mem[addr] = 0;
        }

        break;

    case CAsm::C_TRB:
        addr = get_argument_address(s_bWriteProtectArea);

        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_word(addr);
            ctx.mem[addr] = (arg & ~ctx.a) & 0xFF;
            ctx.mem[addr + 1] = ((arg & ~ctx.a) >> 8) & 0xFF;
            ctx.zero = (arg & ctx.a) == 0;
            ctx.uCycles += 2;   // 16 bit operation adds 2 cycle
        }
        else
        {
            arg = ctx.mem[addr];
            ctx.mem[addr] = (arg & 0xFF) & ~(ctx.a & 0xFF);
            ctx.zero = ((arg & 0xFF) & (ctx.a & 0xFF)) == 0;
        }

        if (extracycle)
            ctx.uCycles++;

        break;

    case CAsm::C_TSB:
        addr = get_argument_address(s_bWriteProtectArea);

        if (cpu16 && !ctx.emm && !ctx.mem16)
        {
            arg = get_word(addr);
            ctx.mem[addr] = (arg | ctx.a) & 0xFF;
            ctx.mem[addr + 1] = ((arg | ctx.a) >> 8) & 0xFF;
            ctx.zero = (arg & ctx.a) == 0;
            ctx.uCycles += 2;   // 16 bit operation adds 2 cycle
        }
        else
        {
            arg = ctx.mem[addr];
            ctx.mem[addr] = (arg & 0xFF) | (ctx.a & 0xFF);
            ctx.zero = ((arg & 0xFF) & (ctx.a & 0xFF)) == 0;
        }

        if (extracycle)
            ctx.uCycles++;

        break;

    case CAsm::C_BBR:
        addr = get_argument_address(false);
        if (!(ctx.mem[addr & 0xFF] & uint8_t(1 << ((cmd >> 4) & 0x07))))
        {
            arg = addr >> 8;
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            else // jump forward
                ctx.pc += arg;
        }
        break;

    case CAsm::C_BBS:
        addr = get_argument_address(false);
        if (ctx.mem[addr & 0xFF] & uint8_t(1 << ((cmd >> 4) & 0x07)))
        {
            arg = addr >> 8;
            if (arg & 0x80) // jump back
                ctx.pc -= 0x100 - arg;
            ctx.pc += arg;
        }
        break;

    case CAsm::C_RMB:
        addr = get_argument_address(s_bWriteProtectArea);
        ctx.mem[addr] &= uint8_t(~(1 << ((cmd >> 4) & 0x07)));
        break;

    case CAsm::C_SMB:
        addr = get_argument_address(s_bWriteProtectArea);
        ctx.mem[addr] |= uint8_t(1 << ((cmd >> 4) & 0x07));
        break;

        //------------65816--------------------------------------------------------
    case CAsm::C_TXY:
        inc_prog_counter();
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            ctx.y = ctx.x;
            ctx.set_status_reg16(ctx.y);
        }
        else
        {
            ctx.y = ctx.x & 0xFF;
            ctx.set_status_reg(ctx.y & 0xFF);
        }
        break;

    case CAsm::C_TYX:
        inc_prog_counter();
        if (cpu16 && !ctx.emm && !ctx.xy16)
        {
            ctx.x = ctx.y;
            ctx.set_status_reg16(ctx.x);
        }
        else
        {
            ctx.x = ctx.y & 0xFF;
            ctx.set_status_reg(ctx.x & 0xFF);
        }
        break;

    case CAsm::C_STP:
        inc_prog_counter();
        return CAsm::SYM_STOP;

    case CAsm::C_BRL:
        inc_prog_counter();
        addr = get_word(ctx.pc + (ctx.pbr << 16));
        inc_prog_counter(2);

        if (addr & 0x8000)
            ctx.pc = (ctx.pc - (0x10000 - addr)) & 0xFFFF;
        else
            ctx.pc = (ctx.pc + addr) & 0xFFFF;

        break;

    case CAsm::C_JSL:
        inc_prog_counter();
        addr = get_word(ctx.pc + (ctx.pbr << 16));
        inc_prog_counter(2);

        if (ctx.emm && (ctx.s == 0x0100))
        {
            ctx.emm = false;
            push_on_stack(ctx.pbr);
            push_addr_on_stack(ctx.pc & 0xFFFF);
            ctx.s = 0x01FD;
            ctx.emm = true;
        }
        else
        {
            push_on_stack(ctx.pbr);
            push_addr_on_stack(ctx.pc & 0xFFFF);
        }

        ctx.pbr = ctx.mem[ctx.pc + (ctx.pbr << 16)];
        wxGetApp().m_global.m_bPBR = ctx.pbr;
        ctx.pc = addr & 0xFFFF;
        break;

    case CAsm::C_JML:
        addr = get_argument_address(false);
        ctx.pc = addr & 0xFFFF;
        ctx.pbr = (addr >> 16) & 0xFF;
        wxGetApp().m_global.m_bPBR = ctx.pbr;
        break;

    case CAsm::C_COP:
        inc_prog_counter(2);

        if (!ctx.emm)
        {
            push_on_stack(ctx.pbr);
            push_addr_on_stack(ctx.pc);
            push_on_stack(ctx.get_status_reg());
            ctx.pc = get_cop_addr16();
            ctx.uCycles++; // native mode takes 1 extra cycle
        }
        else
        {
            push_addr_on_stack(ctx.pc);
            push_on_stack(ctx.get_status_reg());
            ctx.pc = get_cop_addr();
        }

        ctx.interrupt = true; // after pushing status
        ctx.decimal = false;
        ctx.pbr = 0;
        break;

    case CAsm::C_MVN:
    {
        ctx.dbr = ctx.mem[ctx.pc + 1 + (ctx.pbr << 16)];
        arg = ctx.mem[ctx.pc + 2 + (ctx.pbr << 16)];
        //ctx.a++;
        //while (ctx.a--){
        // check for write protect error
        uint32_t ptr = (ctx.dbr << 16) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y);

        if (s_bWriteProtectArea && ptr >= s_uProtectFromAddr && ptr <= s_uProtectToAddr)
            throw CAsm::SYM_ILL_WRITE;

        ctx.mem[(ctx.dbr << 16) + (ctx.xy16 ? (ctx.y++ & 0xFF) : ctx.y++)] =
            ctx.mem[(arg << 16) + (ctx.xy16 ? (ctx.x++ & 0xFF) : ctx.x++)];
        //}
        if (((--ctx.a) & 0xFFFF) == 0xFFFF)
            inc_prog_counter(3);  // repeat opcode until A = 0xFFFF
        ctx.b = (ctx.a >> 8) & 0xFF;
        break;
    }
    case CAsm::C_MVP:
    {
        ctx.dbr = ctx.mem[ctx.pc + 1 + (ctx.pbr << 16)];
        arg = ctx.mem[ctx.pc + 2 + (ctx.pbr << 16)];
        //ctx.a++;
        //while (ctx.a--){
        // check for write protect error
        uint32_t ptr = (ctx.dbr << 16) + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y);

        if (s_bWriteProtectArea && ptr >= s_uProtectFromAddr && ptr <= s_uProtectToAddr)
            throw CAsm::SYM_ILL_WRITE;

        ctx.mem[(ctx.dbr << 16) + (ctx.xy16 ? (ctx.y-- & 0xFF) : ctx.y--)] =
            ctx.mem[(arg << 16) + (ctx.xy16 ? (ctx.x-- & 0xFF) : ctx.x--)];
        //}

        if (((--ctx.a) & 0xFFFF) == 0xFFFF)
            inc_prog_counter(3);   // repeat opcode until A = 0xFFFF
        ctx.b = (ctx.a >> 8) & 0xFF;
        break;
    }

    case CAsm::C_PEA:
        inc_prog_counter();
        addr = get_word(ctx.pc + (ctx.pbr << 16));

        if (ctx.emm && (ctx.s == 0x0100))
        {
            ctx.emm = false;
            push_addr_on_stack(addr);
            ctx.s = 0x01FE;
            ctx.emm = true;
        }
        else
            push_addr_on_stack(addr);

        inc_prog_counter(2);
        break;

    case CAsm::C_PEI:
        inc_prog_counter();
        arg = ctx.mem[ctx.pc + (ctx.pbr << 16)];
        addr = (arg + ctx.dir) & 0xFFFF;
        arg = get_word(addr);

        if (ctx.emm && (ctx.s == 0x0100))
        {
            ctx.emm = false;
            push_addr_on_stack(arg);
            ctx.s = 0x01FE;
            ctx.emm = true;
        }
        else
            push_addr_on_stack(arg);

        if ((ctx.dir & 0xFF) != 0)
            ctx.uCycles++;

        inc_prog_counter();
        break;

    case CAsm::C_PER:
        inc_prog_counter();
        arg = get_word(ctx.pc + (ctx.pbr << 16));
        inc_prog_counter(2);
        if (arg & 0x8000)
            addr = (ctx.pc - (0x10000 - arg)) & 0xFFFF;
        else
            addr = (ctx.pc + arg) & 0xFFFF;

        if (ctx.emm && (ctx.s == 0x0100))
        {
            ctx.emm = false;
            push_addr_on_stack(addr);
            ctx.s = 0x01FE;
            ctx.emm = true;
        }
        else
            push_addr_on_stack(addr);
        break;

    case CAsm::C_PHB:
        push_on_stack(ctx.dbr);
        inc_prog_counter();
        break;

    case CAsm::C_PHD:
        if (ctx.emm && (ctx.s == 0x0100))
        {
            ctx.mem[0xFF] = ctx.dir & 0xFF;
            ctx.mem[0x100] = (ctx.dir >> 8) & 0xFF;
            ctx.s = 0x01FE;
        }
        else
            push_addr_on_stack(ctx.dir);

        inc_prog_counter();
        break;

    case CAsm::C_PHK:
        push_on_stack(ctx.pbr);
        inc_prog_counter();
        break;

    case CAsm::C_PLB:
        inc_prog_counter();
        if (ctx.emm && (ctx.s == 0x01FF)) // special case
        {
            ctx.dbr = ctx.mem[0x200];
            ctx.s = 0x100;
        }
        else
            ctx.dbr = pull_from_stack();

        ctx.set_status_reg(ctx.dbr);
        break;

    case CAsm::C_PLD:
        inc_prog_counter();
        if (ctx.emm && (ctx.s == 0x01FF)) // special case
        {
            ctx.dir = get_word(0x200);
            ctx.s = 0x0101;
        }
        else
            ctx.dir = pull_addr_from_stack();

        ctx.set_status_reg16(ctx.dir);
        break;

    case CAsm::C_REP:
        inc_prog_counter();
        arg8 = (ctx.mem[ctx.pc + (ctx.pbr << 16)]) ^ (0xFF); // EOR
        ctx.set_status_reg_bits(ctx.get_status_reg() & arg8);
        inc_prog_counter();
        break;

    case CAsm::C_SEP:
        inc_prog_counter();
        arg8 = ctx.mem[ctx.pc + (ctx.pbr << 16)];
        ctx.set_status_reg_bits(ctx.get_status_reg() | arg8);
        if (ctx.xy16)
        {
            ctx.x = ctx.x & 0xFF;
            ctx.y = ctx.y & 0xFF;
        }
        inc_prog_counter();
        break;

    case CAsm::C_RTL:
        if (ctx.emm && (ctx.s == 0x01FF))
        {
            ctx.pc = get_word(0x200) + 1;
            ctx.pbr = ctx.mem[0x202];
            ctx.s = 0x0102;
        }
        else
        {
            ctx.pc = ((pull_addr_from_stack() + 1) & 0xFFFF);
            ctx.pbr = (pull_from_stack() & 0xFF);
        }

        wxGetApp().m_global.m_bPBR = ctx.pbr;
        break;

    case CAsm::C_TCD:
        inc_prog_counter();

        if (ctx.mem16)
            ctx.dir = (ctx.a & 0xFF) + (ctx.b << 8);
        else
            ctx.dir = ctx.a;

        ctx.set_status_reg16(ctx.dir);
        break;

    case CAsm::C_TCS:
        inc_prog_counter();

        if (ctx.mem16)
        {
            ctx.s = (ctx.a & 0xFF) + (ctx.b << 8);
            wxGetApp().m_global.m_bSRef = ctx.s;
        }
        else
        {
            ctx.s = ctx.a;
            wxGetApp().m_global.m_bSRef = ctx.s + 0x100;
        }
        break;

    case CAsm::C_TDC:
        inc_prog_counter();
        ctx.a = ctx.dir;
        ctx.b = (ctx.a >> 8) & 0xFF;
        ctx.set_status_reg16(ctx.a);
        break;

    case CAsm::C_TSC:
        inc_prog_counter();
        ctx.a = ctx.s;
        ctx.b = (ctx.a >> 8) & 0xFF;
        ctx.set_status_reg16(ctx.a);
        break;

    case CAsm::C_WAI:
        // don't inc PC so this instruction just repeats until Interrupt or Reset changes it
        // need to modify return addr so after IRQ continues to next instruction
        // this is done in the NMI and IRQ handler routine (see further down)
        waiFlag = true;
        break;

    case CAsm::C_WDM: // NOP plus sig byte
        inc_prog_counter(2);
        break;

    case CAsm::C_XBA:
        inc_prog_counter();
        arg = ctx.b;
        ctx.b = ctx.a & 0xFF;
        ctx.a = arg;
        ctx.set_status_reg(ctx.a & 0xFF);
        break;

    case CAsm::C_XCE:
        inc_prog_counter();
        arg = ctx.emm;
        ctx.emm = ctx.carry;
        ctx.carry = arg;

        if (ctx.emm)
        {
            ctx.mem16 = true;
            ctx.xy16 = true;
        }
        break;

        //-------------------------------------------------------------------------

    case CAsm::C_ILL:
        if (finish == CAsm::FIN_BY_DB && cmd == 0xDB) // DB is invalid for 6502 and 65C02 - STP for 65816
            return CAsm::SYM_FIN;

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502)
        {
            // 65C02 mode
            arg = get_argument_value(false);
            extracycle = false;
            break;
        }

        return CAsm::SYM_ILLEGAL_CODE;

    default:
        ASSERT(false);
        break;
    }

    ctx.uCycles += m_vCodeToCycles[cmd];

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

    return CAsm::SYM_OK;
}

CAsm::SymStat CSym6502::skip_cmd() // Skip the current statement
{
    inc_prog_counter(CAsm::mode_to_len[m_vCodeToMode[ctx.mem[ctx.pc]]]);
    return CAsm::SYM_OK;
}

uint16_t CSym6502::get_irq_addr()
{
    return ((uint16_t)ctx.mem[0xFFFE] | ((uint16_t)ctx.mem[0xFFFF] << 8));
}

uint16_t CSym6502::get_irq_addr16()
{
    return ((uint16_t)ctx.mem[0xFFEE] | ((uint16_t)ctx.mem[0xFFEF] << 8));
}

uint16_t CSym6502::get_nmi_addr()
{
    return ((uint16_t)ctx.mem[0xFFFA] | ((uint16_t)ctx.mem[0xFFFB] << 8));
}

uint16_t CSym6502::get_nmi_addr16()
{
    return ((uint16_t)ctx.mem[0xFFEA] | ((uint16_t)ctx.mem[0xFFEB] << 8));
}

uint16_t CSym6502::get_brk_addr16()
{
    return ((uint16_t)ctx.mem[0xFFE6] | ((uint16_t)ctx.mem[0xFFE7] << 8));
}

uint16_t CSym6502::get_cop_addr()
{
    return ((uint16_t)ctx.mem[0xFFF4] | ((uint16_t)ctx.mem[0xFFF5] << 8));
}

uint16_t CSym6502::get_cop_addr16()
{
    return ((uint16_t)ctx.mem[0xFFE4] | ((uint16_t)ctx.mem[0xFFE5] << 8));
}

uint16_t CSym6502::get_abort_addr()
{
    return ((uint16_t)ctx.mem[0xFFF8] | ((uint16_t)ctx.mem[0xFFF9] << 8));
}

uint16_t CSym6502::get_abort_addr16()
{
    return ((uint16_t)ctx.mem[0xFFE8] | ((uint16_t)ctx.mem[0xFFE9] << 8));
}

uint16_t CSym6502::get_rst_addr()
{
    return ((uint16_t)ctx.mem[0xFFFC] | ((uint16_t)ctx.mem[0xFFFD] << 8));
}

//=============================================================================

CAsm::SymStat CSym6502::StepInto()
{
    ASSERT(fin_stat != CAsm::SYM_FIN);

    if (running)
    {
        ASSERT(false);
        return CAsm::SYM_OK;
    }

    set_translation_tables();
    stop_prog = false;
    running = true;
    old = ctx;
    fin_stat = perform_cmd();
    running = false;

    Update(fin_stat);

    return fin_stat;
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::StepOver()
{
    ASSERT(fin_stat != CAsm::SYM_FIN);

    if (running)
    {
        ASSERT(false);
        return CAsm::SYM_OK;
    }

    Update(CAsm::SYM_RUN);
    old = ctx;
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
        ResetPointer();
        thread->ResumeThread();
}
#endif

    return CAsm::SYM_OK;
}

UINT CSym6502::start_step_over_thread(void *ptr)
{
    UNUSED(ptr);

#if REWRITE_TO_WX_WIDGET
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->step_over();
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, pSym->fin_stat, 0);
#endif

    return 0;
}

CAsm::SymStat CSym6502::step_over()
{
    uint32_t addr = ctx.pc;
    uint16_t stack = 0;
    bool jsr = false;

    if (cpu16)
        addr += (ctx.pbr << 16);

    set_translation_tables();

    switch (m_vCodeToCommand[ctx.mem[addr]])
    {
    case CAsm::C_JSR:
    case CAsm::C_JSL:
        stack = ctx.s;
        jsr = true;
        [[fallthrough]];

    case CAsm::C_BRK:
        if (debug && !jsr)
            debug->SetTemporaryExecBreakpoint((addr + 2) & ctx.mem_mask);

        for (;;)
        {
            CAsm::SymStat stat = perform_step(false);

            if (stat != CAsm::SYM_OK)
                return stat;

            if (jsr && ctx.s == stack)
                return CAsm::SYM_BPT_TEMP;
        }
        break;

    default:
        return perform_cmd();
    }
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::RunTillRet()
{
    ASSERT(fin_stat != CAsm::SYM_FIN);

    if (running)
    {
        ASSERT(false);
        return CAsm::SYM_OK;
    }

    Update(CAsm::SYM_RUN);
    old = ctx;
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
        ResetPointer();
        thread->ResumeThread();
}
#endif

    return CAsm::SYM_OK;
    }

UINT CSym6502::start_run_till_ret_thread(void *ptr)
{
    UNUSED(ptr);

#if REWRITE_TO_WX_WIDGET
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->run_till_ret();
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, pSym->fin_stat, 0);
#endif

    return 0;
}

CAsm::SymStat CSym6502::run_till_ret()
{
    set_translation_tables();

    uint16_t stack = ctx.s + 2;
    for (;;)
    {
        CAsm::SymStat stat = perform_step(false);
        if (stat != CAsm::SYM_OK)
            return stat;

        if (ctx.s == stack)
            return CAsm::SYM_BPT_TEMP;
    }
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::Run()
{
    ASSERT(fin_stat != CAsm::SYM_FIN);

    if (running)
    {
        ASSERT(false);
        return CAsm::SYM_OK;
    }

    Update(CAsm::SYM_RUN);
    old = ctx;
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
        ResetPointer();
        thread->ResumeThread();
}
#endif

    return CAsm::SYM_OK;
}


UINT CSym6502::start_run_thread(void *ptr)
{
    UNUSED(ptr);

#if REWRITE_TO_WX_WIDGET
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->run();
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, pSym->fin_stat, 0);
#endif

    return 0;
}

CAsm::SymStat CSym6502::perform_step(bool animate)
{
    UNUSED(animate);

    if (stop_prog) // stop executing?
        return CAsm::SYM_STOP;

    if (m_nInterruptTrigger != NONE) // interrupt requested?
        interrupt(m_nInterruptTrigger);

    CAsm::SymStat stat = perform_cmd();
    if (stat != CAsm::SYM_OK)
        return stat;

    CAsm::Breakpoint bp;
    uint32_t addr = ctx.pc;

    if (cpu16)
        addr += (ctx.pbr << 16);

    if (debug && (bp = debug->GetBreakpoint(addr)) != CAsm::BPT_NONE)
    {
        if (bp & CAsm::BPT_EXECUTE)
            return CAsm::SYM_BPT_EXECUTE;

        if (bp & CAsm::BPT_TEMP_EXEC)
            return CAsm::SYM_BPT_TEMP;
    }

#if REWRITE_TO_WX_WIDGET
    if (animate)
    {
        eventRedraw.ResetEvent();
        wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, SYM_RUN, 1);
        eventRedraw.Lock();
}
#endif

    return CAsm::SYM_OK;
}

CAsm::SymStat CSym6502::run(bool animate /*= false*/)
{
    set_translation_tables();

    for (;;)
    {
        CAsm::SymStat stat = perform_step(animate);
        if (stat != CAsm::SYM_OK)
            return stat;
    }
}

void CSym6502::interrupt(int &nInterrupt) // interrupt requested: load pc ***
{
    ASSERT(running);

    if (nInterrupt & RST)
    {
        ctx.interrupt = false;

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502)
            ctx.decimal = false; //% 65C02 clears this bit

        ctx.pc = get_rst_addr();
        nInterrupt = NONE;
        ctx.uCycles += 7;
    }
    else if (nInterrupt & NMI)
    {
        if (waiFlag)
        {
            inc_prog_counter();
            waiFlag = false;
        }

        if (cpu16 && !ctx.emm)
        {
            push_on_stack(ctx.pbr);
            push_addr_on_stack(ctx.pc);
            ctx.pc = get_nmi_addr16();
        }
        else
        {
            push_addr_on_stack(ctx.pc);
            ctx.pc = get_nmi_addr();
        }

        ctx.break_bit = false;
        push_on_stack(ctx.get_status_reg());

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502)
            ctx.decimal = false; // 65C02 clears this bit

        ctx.break_bit = true;
        nInterrupt &= ~NMI;
        ctx.uCycles += 7;
    }
    else if (nInterrupt & IRQ)
    {
        nInterrupt &= ~IRQ;

        if (waiFlag)
        {
            inc_prog_counter();
            waiFlag = false;
        }

        if (ctx.interrupt)
            return;

        if (cpu16 && !ctx.emm)
        {
            push_on_stack(ctx.pbr);
            push_addr_on_stack(ctx.pc);
            ctx.pc = get_irq_addr16();
        }
        else
        {
            push_addr_on_stack(ctx.pc);
            ctx.pc = get_irq_addr();
        }

        ctx.break_bit = false;
        push_on_stack(ctx.get_status_reg());
        ctx.break_bit = true;
        ctx.interrupt = true;

        if (wxGetApp().m_global.m_procType != ProcessorType::M6502)
            ctx.decimal = false; // 65C02 clears this bit

        ctx.uCycles += 7;
    }
    else
    {
        ASSERT(false);
    }
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::Animate()
{
    ASSERT(fin_stat != CAsm::SYM_FIN);

    if (running)
    {
        ASSERT(false);
        return CAsm::SYM_OK;
    }

    Update(CAsm::SYM_RUN);
    old = ctx;
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

    return CAsm::SYM_OK;
}


UINT CSym6502::start_animate_thread(void *ptr)
{
    UNUSED(ptr);

#if REWRITE_TO_WX_WIDGET
    CSym6502 *pSym = (CSym6502 *)ptr;
    pSym->fin_stat = pSym->run(true);
    pSym->running = false;
    wxGetApp()->GetMainWnd()->PostMessage(WM_USER + 9998, pSym->fin_stat, 0);
#endif

    return 0;
}

//-----------------------------------------------------------------------------

void CSym6502::SkipToAddr(uint16_t addr)
{
    ASSERT(fin_stat != CAsm::SYM_FIN);

    if (running)
        return;

    ctx.pc = addr;
    Update(CAsm::SYM_OK);
}

CAsm::SymStat CSym6502::SkipInstr()
{
    ASSERT(fin_stat != CAsm::SYM_FIN);

    if (running)
        return CAsm::SYM_OK;

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
        MSG msg;

        if (!::GetMessage(&msg, NULL, NULL, NULL))
            break;

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    ASSERT(!running);
#endif
}

void CSym6502::ExitSym()
{
    ASSERT(running == false);
    ResetPointer();
}

//-----------------------------------------------------------------------------

std::string CSym6502::GetLastStatMsg()
{
    return GetStatMsg(fin_stat);
}

std::string CSym6502::GetStatMsg(CAsm::SymStat stat)
{
    UNUSED(stat);

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

void CSym6502::Update(CAsm::SymStat stat, bool no_ok /*=false*/)
{
    UNUSED(stat);
    UNUSED(no_ok);

#if REWRITE_TO_WX_WIDGET
    wxWindow *pMain = (wxWindow *)wxGetApp()->m_pMainWnd;

    std::string reg = GetStatMsg(stat);

    if (debug)
    {
        if (fin_stat == SYM_BPT_TEMP)
            debug->RemoveTemporaryExecBreakpoint();

        CDebugLine dl;

        if (cpu16)
            debug->GetLine(dl, ctx.pc + (ctx.pbr << 16));
        else
            debug->GetLine(dl, ctx.pc);

        if (fin_stat == SYM_FIN)
            ResetPointer();
        else
        {
            if (cpu16)
                SetPointer(dl.line, ctx.pc + (ctx.pbr << 16));
            else
                SetPointer(dl.line, ctx.pc);
        }
    }

    pMain->m_wndRegisterBar.SendMessage(CBroadcast::WM_USER_UPDATE_REG_WND, (WPARAM)&reg, (LPARAM)&ctx);

    if (stat == SYM_OK && !no_ok)
        pMain->m_wndStatusBar.SetPaneText(0, reg);

    if (running)
        eventRedraw.SetEvent();
#endif
}

//-----------------------------------------------------------------------------

void CSym6502::Restart(const COutputMem &mem)
{
    ctx.Reset(mem);
    old = ctx;
    fin_stat = CAsm::SYM_OK;
    m_Log.Clear();
    saveCycles = 0;
    ctx.set_status_reg_bits(0);
}

void CSym6502::SymStart(uint32_t org)
{
    ctx.pc = org;
    ctx.s = 0x01FF;
    wxGetApp().m_global.m_bSRef = ctx.s;
    saveCycles = 0;
    ctx.set_status_reg_bits(0);

    if (debug)
    {
        CDebugLine dl;
        debug->GetLine(dl, org);
        SetPointer(dl.line, org);
    }
}

void CSym6502::SetPointer(const CLine &line, uint32_t addr)
{
    UNUSED(line);
    UNUSED(addr);

#if REWRITE_TO_WX_WIDGET
    POSITION posDoc = wxGetApp().m_pDocDeasmTemplate->GetFirstDocPosition();
    while (posDoc != NULL)
    {
        CDocument *pDoc = wxGetApp().m_pDocDeasmTemplate->GetNextDoc(posDoc);
        ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDeasm6502Doc)));
        ((CDeasm6502Doc *)pDoc)->SetPointer(addr, true);
    }

    CSrc6502View *pView = FindDocView(line.file);
    if (m_fuidLastView != line.file && ::IsWindow(m_hwndLastView))
    {
        if (CSrc6502View *pView = dynamic_cast<CSrc6502View *>(CWnd::FromHandlePermanent(m_hwndLastView)))
            SetPointer(pView, -1, false);
        m_hwndLastView = 0;
    }
    if (!pView && debug)
    {
        if (const char *path = debug->GetFilePath(line.file))
        {
            C6502App *pApp = static_cast<C6502App *>(wxGetApp());
            pApp->m_bDoNotAddToRecentFileList = true;
            CDocument *pDoc = pApp->OpenDocumentFile(path);
            pApp->m_bDoNotAddToRecentFileList = false;

            if (CSrc6502Doc *pSrcDoc = dynamic_cast<CSrc6502Doc *>(pDoc))
            {
                POSITION pos = pSrcDoc->GetFirstViewPosition();

                if (pos != nullptr)
                    pView = dynamic_cast<CSrc6502View *>(pSrcDoc->GetNextView(pos));
            }
        }
    }

    if (!pView)
        return; // no document window containing the current line.

    SetPointer(pView, line.ln, true); // force the window contents to move if necessary
    m_fuidLastView = line.file;
    m_hwndLastView = pView->m_hWnd;
#endif
}

void CSym6502::SetPointer(CSrc6502View *pView, int nLine, bool bScroll)
{
    UNUSED(nLine);
    UNUSED(bScroll);

    if (!pView)
        return;

#if REWRITE_TO_WX_WIDGET
    CDocument *pDoc = pView->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();

    while (pos != NULL)
    {
        if (CSrc6502View *pSrcView = dynamic_cast<CSrc6502View *>(pDoc->GetNextView(pos)))
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
        if (CDeasm6502Doc *pDoc = dynamic_cast<CDeasm6502Doc *>(wxGetApp().m_pDocDeasmTemplate->GetNextDoc(posDoc)))
            pDoc->SetPointer(-1, true);
    }

    if (m_fuidLastView)
    {
        if (CSrc6502View *pView = FindDocView(m_fuidLastView))
            SetPointer(pView, -1, false); // blurring of the arrow
    }
#endif

    m_fuidLastView = 0;
}

CSrc6502View *CSym6502::FindDocView(CAsm::FileUID fuid)
{
    UNUSED(fuid);

#if REWRITE_TO_WX_WIDGET
    if (debug == NULL)
        return NULL;

    if (CFrameWnd *pFrame = dynamic_cast<CFrameWnd *>(AfxGetMainWnd()))
    {
        if (CFrameWnd *pActive = pFrame->GetActiveFrame())
        {
            if (CSrc6502View *pView = dynamic_cast<CSrc6502View *>(pActive->GetActiveView()))
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

    if (!terminal)
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
    UNUSED(arg);

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
    return CAsm::SYM_OK;
}

void CSym6502::ClearCyclesCounter()
{
    ASSERT(running == false);
    ctx.uCycles = 0;
}

void CSym6502::AddBranchCycles(uint8_t arg)
{
    ctx.uCycles++; // Branch taken

    if (!cpu16 || (cpu16 && ctx.emm))
    {
        if (arg & 0x80) // jump back
        {
            if (ctx.pc >> 8 != static_cast<uint32_t>((ctx.pc - (0x100 - arg)) >> 8))
                ctx.uCycles++; // changing memory page -> additional cycle
        }
        else // jump forward
        {
            if (ctx.pc >> 8 != static_cast<uint32_t>((ctx.pc + arg) >> 8))
                ctx.uCycles++; // changing memory page -> additional cycle
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CSym6502::check_io_write(uint32_t addr)
{
    if (io_enabled && (addr >= io_addr) && (addr < (io_addr + IO_LAST_FUNC)))
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

bool CSym6502::check_io_read(uint32_t addr)
{
    if (io_enabled && (addr >= io_addr) && (addr < (io_addr + IO_LAST_FUNC)))
    {
        io_func = IOFunc(addr - io_addr);
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////

CAsm::SymStat CSym6502::Interrupt(IntType eInt)
{
    m_nInterruptTrigger |= eInt;

    return CAsm::SYM_OK;
}

///////////////////////////////////////////////////////////////////////////////

void CSym6502::init()
{
    running = false;
    m_fuidLastView = 0;
    finish = CAsm::FIN_BY_BRK;
    fin_stat = CAsm::SYM_OK;
    //hThread = 0;
    io_func = IO_NONE;
    m_nInterruptTrigger = NONE;
    m_vCodeToCommand = 0;
    m_vCodeToCycles = 0;
    m_vCodeToMode = 0;
    ctx.set_status_reg_bits(0);

    if (wxGetApp().m_global.GetProcType() == ProcessorType::WDC65816)
        ctx.mem_mask = uint32_t((1 << 24) - 1);
    else
        ctx.mem_mask = uint32_t((1 << 16) - 1);
}

void CSym6502::set_translation_tables()
{
    m_vCodeToCommand = CAsm::CodeToCommand();
    m_vCodeToCycles = CAsm::CodeToCycles();
    m_vCodeToMode = CAsm::CodeToMode();
}

///////////////////////////////////////////////////////////////////////////////

std::string CmdInfo::Asm() const
{
    CDeasm deasm;
    CAsm::DeasmFmt fmt = CAsm::DeasmFmt(CAsm::DF_ADDRESS | CAsm::DF_CODE_BYTES | CAsm::DF_USE_BRK);  //% bug Fix 1.2.13.18 - show BRK vs. .DB $00

    wxString strLine = deasm.DeasmInstr(*this, fmt);

    wxString strBuf;

    // * indicates RST, IRQ, or NMI have occurred
    if (wxGetApp().m_global.GetProcType() == ProcessorType::WDC65816)
    {
        if (intFlag)
            strBuf.Format("%-36s A:%04x X:%04x Y:%04x F:%02x S:%04x  Cycles=%u *", strLine, int(a), int(x), int(y), int(flags), int(s), uCycles);
        else
            strBuf.Format("%-36s A:%04x X:%04x Y:%04x F:%02x S:%04x  Cycles=%u ", strLine, int(a), int(x), int(y), int(flags), int(s), uCycles);
    }
    else
    {
        if (intFlag)
            strBuf.Printf("%-30s A:%02x X:%02x Y:%02x F:%02x S:%04x  Cycles=%u *", strLine, int(a), int(x), int(y), int(flags), int(s + 0x100), uCycles);
        else
            strBuf.Printf("%-30s A:%02x X:%02x Y:%02x F:%02x S:%04x  Cycles=%u ", strLine, int(a), int(x), int(y), int(flags), int(s + 0x100), uCycles);
    }

    return strBuf.ToStdString();
}
