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
#include "sim.h"

// TODO: Remove direct references to the simulator controller from here.
#include "SimulatorController.h"

// TODO: Remove references to views.
#include "MainFrm.h"
#include "6502View.h"
#include "6502Doc.h"

/*************************************************************************/

bool waiFlag = false;
int CSym6502::bus_width = 16;
//static const int SIM_THREAD_PRIORITY = THREAD_PRIORITY_BELOW_NORMAL; // Priority (except animate)
bool CSym6502::s_bWriteProtectArea = false;
uint32_t CSym6502::s_uProtectFromAddr = 0xc000;
uint32_t CSym6502::s_uProtectToAddr = 0xcfff;
bool extracycle = false; //% Bug Fix 1.2.12.1 - fix cycle counts, used to signal page crossings

/*************************************************************************/

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

/*************************************************************************/

uint32_t CSym6502::get_argument_address(bool bWrite)
{
    uint16_t arg;
    uint32_t addr;
    uint32_t pc = m_ctx.pc; // Save original PC
    uint8_t mode;

    if (cpu16())
        mode = m_vCodeToMode[m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16))];
    else
        mode = m_vCodeToMode[m_ctx.getByte(m_ctx.pc)];

    inc_prog_counter(); // Bypass the command

    extracycle = false; //% bug Fix 1.2.12.1 - fix cycle timing

    switch (mode)
    {
    case CAsm::A_ZPG:
    case CAsm::A_ZPG2:
        if (cpu16())
        {
            addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir) & 0xFFFF;

            if ((m_ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
            addr = m_ctx.getByte(m_ctx.pc); // address on zero page

        inc_prog_counter();
        break;

    case CAsm::A_ZPG_X:
        if (cpu16())
        {
            if (m_ctx.emm && ((m_ctx.dir & 0xFF) == 0))
                addr = m_ctx.dir + ((m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xff);
            else
                addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xffff;

            if ((m_ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
            addr = (m_ctx.getByte(m_ctx.pc) + (m_ctx.x & 0xff)) & 0xff;

        inc_prog_counter();
        break;

    case CAsm::A_ZPG_Y:
        if (cpu16())
        {
            if (m_ctx.emm && (m_ctx.dir & 0xff) == 0)
            {
                addr = m_ctx.dir + ((m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.y & 0xff) : m_ctx.y)) & 0xff); //**
            }
            else
                addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir + (m_ctx.xy16 ? (m_ctx.y & 0xff) : m_ctx.y)) & 0xffff;

            if ((m_ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
            addr = (m_ctx.getByte(m_ctx.pc) + (m_ctx.y & 0xff)) & 0xff;

        inc_prog_counter();
        break;

    case CAsm::A_ZPGI: // 65c02 65816 only
        if (cpu16())
        {
            arg = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir) & 0xFFFF;
            addr = get_word_indirect(arg) + (m_ctx.dbr << 16);

            if ((m_ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
        {
            arg = m_ctx.getByte(m_ctx.pc);
            addr = get_word_indirect(arg);
        }

        inc_prog_counter();
        break;

    case CAsm::A_ABS:
        if (cpu16())
            addr = get_word(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.dbr << 16);
        else
            addr = get_word(m_ctx.pc);

        inc_prog_counter(2);
        break;

    case CAsm::A_ABS_X:
        if (cpu16())
            addr = (get_word(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xff) : m_ctx.x)) + (m_ctx.dbr << 16);
        else
            addr = get_word(m_ctx.pc) + (m_ctx.x & 0xff);

        if ((addr >> 8) != static_cast<uint32_t>(get_word(m_ctx.pc) >> 8))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter(2);
        break;

    case CAsm::A_ABS_Y:
        if (cpu16())
            addr = (get_word(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.y & 0xff) : m_ctx.y)) + (m_ctx.dbr << 16);
        else
            addr = get_word(m_ctx.pc) + (m_ctx.y & 0xff);

        if ((addr >> 8) != static_cast<uint32_t>(get_word(m_ctx.pc) >> 8))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter(2);
        break;

    case CAsm::A_ZPGI_X:
        if (cpu16())
        {
            if (m_ctx.emm)
            {
                if ((m_ctx.dir & 0xFF) == 0x00)
                {
                    arg = m_ctx.dir + ((m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xFF);
                    addr = m_ctx.getByte(arg) + (m_ctx.dbr << 16);
                    arg = ((arg + 1) & 0xFF) + (arg & 0xFF00);
                    addr += (m_ctx.getByte(arg) << 8);
                }
                else
                {
                    arg = m_ctx.dir + ((m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xff) : m_ctx.x)) & 0xFFFF);
                    addr = m_ctx.getByte(arg) + (m_ctx.dbr << 16);
                    arg = ((arg + 1) & 0xFF) + (arg & 0xFF00);
                    addr += (m_ctx.getByte(arg) << 8);
                }
            }
            else
            {
                arg = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir + (m_ctx.xy16 ? (m_ctx.x & 0xff) : m_ctx.x)) & 0xFFFF;
                addr = get_word_indirect(arg) + (m_ctx.dbr << 16);
            }

            if ((m_ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
        {
            arg = m_ctx.getByte(m_ctx.pc); // cell address on zero page
            addr = get_word_indirect((arg + (m_ctx.x & 0xff)) & 0xff);
        }

        inc_prog_counter();
        break;

    case CAsm::A_ZPGI_Y:

        if (cpu16())
        {
            arg = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir) & 0xffff;
            addr = get_word_indirect(arg) + (m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y & 0xff) : m_ctx.y);
            if ((m_ctx.dir & 0xff) != 0)
                extracycle = true;
        }
        else
        {
            arg = m_ctx.getByte(m_ctx.pc); // cell address on zero page
            addr = get_word_indirect(arg) + (m_ctx.y & 0xff);
        }

        if (uint16_t(addr >> 8u) != (get_word_indirect(arg) >> 8u))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter();
        break;

    case CAsm::A_ABSI: // only JMP(xxxx) supports this addr mode
        addr = (cpu16() && !m_ctx.emm) ? get_word(m_ctx.pc + (m_ctx.pbr << 16)) : get_word(m_ctx.pc);

        if (Processor() != ProcessorType::M6502 && (addr & 0xFF) == 0xFF) // LSB == 0xFF?
        {
            // Recreate 6502 bug
            uint16_t hAddr = addr - 0xFF;
            uint16_t lo = m_ctx.getByte(addr);
            uint16_t hi = m_ctx.getByte(hAddr);

            addr = lo | (hi << 8); // erroneously just as 6502 would do
        }
        else
            addr = get_word(addr);

        if (cpu16() && !m_ctx.emm)
            addr += (m_ctx.pbr << 16);

        inc_prog_counter(2);
        break;

    case CAsm::A_ABSI_X:
        if (cpu16())
        {
            addr = (get_word(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xFFFF;
            addr = get_word(addr + (m_ctx.pbr << 16));
        }
        else
        {
            addr = get_word(m_ctx.pc) + (m_ctx.x & 0xFF);
            addr = get_word(addr);
        }

        inc_prog_counter(2);
        break;

    case CAsm::A_ZREL: // 65816 only
        addr = get_word(m_ctx.pc);
        inc_prog_counter(2);
        break;

    case CAsm::A_ABSL: // 65816 only
        addr = get_Lword(m_ctx.pc + (m_ctx.pbr << 16));
        inc_prog_counter(3);
        break;

    case CAsm::A_ABSL_X: // 65816 only
        addr = (get_Lword(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0x00FFFFFF;
        inc_prog_counter(3);
        break;

    case CAsm::A_ZPIL: // 65816 only
        arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir; // cell address on zero page
        addr = get_Lword_indirect(arg);
        inc_prog_counter();

        if ((m_ctx.dir & 0xFF) != 0)
            extracycle = true;

        break;

    case CAsm::A_ZPIL_Y: // 65816 only
        arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir; // cell address on zero page
        addr = get_Lword_indirect(arg) + (m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y);

        inc_prog_counter();

        if ((m_ctx.dir & 0xFF) != 0)
            extracycle = true;

        break;

    case CAsm::A_SR: // 65816 only
        addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.s) & 0xFFFF; // address on zero page
        inc_prog_counter();
        break;

    case CAsm::A_SRI_Y:
        arg = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.s) & 0xFFFF; // address on zero page
        addr = get_word_indirect(arg) + (m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y & 0xff) : m_ctx.y);
        inc_prog_counter();
        break;

    case CAsm::A_INDL: // 65816 only
        arg = get_word(m_ctx.pc + (m_ctx.pbr << 16));
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
        m_ctx.pc = pc; // restore original value
        CurrentStatus = CSym6502::Status::ILL_WRITE;
        throw 0; // TODO: Rework to not use exceptions
        //return INVALID_ADDR;
    }

    return addr;
}

uint16_t CSym6502::get_argument_value(bool rmask)
{
    uint8_t arg;
    uint32_t addr;
    uint16_t val;
    uint8_t mode;

    if (cpu16())
        mode = m_vCodeToMode[m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16))];
    else
        mode = m_vCodeToMode[m_ctx.getByte(m_ctx.pc)];

    inc_prog_counter(); // bypass the command

    extracycle = false; //% bug Fix 1.2.12.1 - fix cycle timing

    switch (mode)
    {
    case CAsm::A_IMP:
    case CAsm::A_ACC:
        return 0;

    case CAsm::A_IMM:
        if (cpu16() && !m_ctx.emm)
        {
            val = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));

            if (rmask)
            {
                inc_prog_counter();
                val += m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) << 8;
            }
        }
        else
            val = m_ctx.getByte(m_ctx.pc);

        inc_prog_counter();
        return val;

    case CAsm::A_IMP2:
    case CAsm::A_REL:
        if (cpu16())
            arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
        else
            arg = m_ctx.getByte(m_ctx.pc);

        inc_prog_counter();
        return arg;

    case CAsm::A_ZPGI:
        if (cpu16())
        {
            arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));

            if (!m_ctx.emm)
            {
                addr = get_word_indirect((arg + m_ctx.dir) & 0xFFFF) + (m_ctx.dbr << 16);
                val = m_ctx.getByte(addr);

                if (rmask)
                    val += m_ctx.getByte(addr + 1) << 8;

                if ((m_ctx.dir & 0xFF) != 0)
                    extracycle = true;

                inc_prog_counter();
                return val;
            }
            else
            {
                uint32_t adr1, adr2;
                adr1 = arg + m_ctx.dir;

                if ((m_ctx.dir & 0xFF) == 0)
                    adr2 = ((arg + 1) & 0xFF) + m_ctx.dir;
                else
                    adr2 = arg + 1 + m_ctx.dir;

                addr = m_ctx.getByte(adr1) + (m_ctx.dbr << 16);
                addr += m_ctx.getByte(adr2) << 8;

                val = m_ctx.getByte(addr);

                if (rmask)
                    val += m_ctx.getByte(addr + 1) << 8;

                if ((m_ctx.dir & 0xff) != 0)
                    extracycle = true;

                inc_prog_counter();
                return val;
            }
        }
        else
        {
            arg = m_ctx.getByte(m_ctx.pc); // cell address on zero page
            addr = get_word_indirect(arg);
            inc_prog_counter();
            return m_ctx.getByte(addr);
        }

    case CAsm::A_ZPG:
        if (cpu16())
        {
            addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir) & 0xFFFF;
            val = m_ctx.getByte(addr);

            if (rmask)
                val += (m_ctx.getByte(addr + 1)) << 8;

            if ((m_ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
            val = m_ctx.getByte(m_ctx.getByte(m_ctx.pc)) & 0xFF; // number at address

        inc_prog_counter();
        return val;

    case CAsm::A_ZPG_X:
        if (cpu16())
        {
            if (m_ctx.emm && ((m_ctx.dir & 0xFF) == 0))
                addr = m_ctx.dir + ((m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xFF); // adres
            else
                addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xffff; // adres

            val = m_ctx.getByte(addr);

            if (rmask)
                val += (m_ctx.getByte(addr + 1)) << 8;

            if ((m_ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
        {
            addr = (m_ctx.getByte(m_ctx.pc) + (m_ctx.x & 0xFF)) & 0xFF;
            val = m_ctx.getByte(addr); // number at address
        }

        inc_prog_counter();
        return val;

    case CAsm::A_ZPG_Y:
        if (cpu16())
        {
            if (m_ctx.emm && ((m_ctx.dir & 0xFF) == 0))
                addr = m_ctx.dir + ((m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y)) & 0xFF);
            else
                addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.dir + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y)) & 0xFFFF;

            val = m_ctx.getByte(addr);

            if (rmask)
                val += (m_ctx.getByte(addr + 1)) << 8;

            if ((m_ctx.dir & 0xFF) != 0)
                extracycle = true;
        }
        else
            val = m_ctx.getByte((m_ctx.getByte(m_ctx.pc) + (m_ctx.y & 0xFF)) & 0xFF);

        inc_prog_counter();
        return val;

    case CAsm::A_ABS:
        if (cpu16())
        {
            addr = (get_word(m_ctx.pc + (m_ctx.pbr << 16)) & 0xFFFF) + (m_ctx.dbr << 16);
            inc_prog_counter(2);

            val = m_ctx.getByte(addr);

            if (rmask)
                val += m_ctx.getByte(addr + 1) << 8;

            return val;
        }
        else
        {
            addr = get_word(m_ctx.pc);
            inc_prog_counter(2);
            return m_ctx.getByte(addr); // number at address
        }

    case CAsm::A_ABS_X:
        if (cpu16())
        {
            addr = get_word(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x);
            inc_prog_counter(2);
            val = m_ctx.getByte(addr);

            if (rmask)
                val += m_ctx.getByte(addr + 1) << 8;

            if (!m_ctx.xy16)
                m_ctx.uCycles++;
            else
            {
                if (uint16_t(addr >> 8u) != (get_word(m_ctx.pc + (m_ctx.pbr << 16)) >> 8u))
                    extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
            }

            return val;
        }
        else
        {
            addr = get_word(m_ctx.pc) + (m_ctx.x & 0xFF);

            if (uint16_t(addr >> 8u) != (get_word(m_ctx.pc) >> 8u))
                extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        inc_prog_counter(2);
        return m_ctx.getByte(addr);

    case CAsm::A_ABS_Y:
        if (cpu16())
        {
            addr = get_word(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y);
            inc_prog_counter(2);
            val = m_ctx.getByte(addr);

            if (rmask)
                val += m_ctx.getByte(addr + 1) << 8;

            if (!m_ctx.xy16)
                m_ctx.uCycles++;
            else
            {
                if (uint16_t(addr >> 8u) != (get_word(m_ctx.pc + (m_ctx.pbr << 16)) >> 8u))
                    extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
            }

            return val;
        }
        else
        {
            addr = get_word(m_ctx.pc) + (m_ctx.y & 0xff);
            if (uint16_t(addr >> 8u) != (get_word(m_ctx.pc) >> 8u))
                extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
        }
        inc_prog_counter(2);
        return m_ctx.getByte(addr); // number at address

    case CAsm::A_ZPGI_X:
        uint32_t adr1, adr2;
        if (cpu16())
        {
            arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));

            if (m_ctx.emm)
            {
                if ((m_ctx.dir & 0xFF) == 0)
                {
                    adr1 = ((arg + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xFF) + m_ctx.dir;
                    adr2 = ((arg + 1 + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xFF) + m_ctx.dir;
                    addr = m_ctx.getByte(adr1) + (m_ctx.dbr << 16);
                    addr += m_ctx.getByte(adr2) << 8;
                }
                else
                {
                    adr1 = ((arg + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xFFFF) + m_ctx.dir;
                    adr2 = (adr1 & 0xFFFF00) + (((adr1 & 0xFF) + 1) & 0xFF);
                    addr = m_ctx.getByte(adr1) + (m_ctx.dbr << 16);
                    addr += m_ctx.getByte(adr2) << 8;
                }
            }
            else
                addr = get_word_indirect((arg + m_ctx.dir + (m_ctx.xy16 ? (m_ctx.x & 0xFF) : m_ctx.x)) & 0xFFFFF) + (m_ctx.dbr << 16);

            if ((m_ctx.dir & 0xFF) != 0)
                extracycle = true;

            inc_prog_counter();
            val = m_ctx.getByte(addr);

            if (rmask)
                val += m_ctx.getByte(addr + 1) << 8;

            return val;
        }
        else
        {
            arg = m_ctx.getByte(m_ctx.pc); // cell address on zero page
            addr = get_word_indirect((arg + (m_ctx.x & 0xFF)) & 0xFF);
            inc_prog_counter();
            return m_ctx.getByte(addr); // number at address
        }

    case CAsm::A_ZPGI_Y:
        if (cpu16())
        {
            arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));

            if (!m_ctx.emm)
            {
                addr = ((get_word_indirect((arg + m_ctx.dir) & 0xFFFF) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y)) & 0xFFFFFF) + (m_ctx.dbr << 16);

                if ((m_ctx.dir & 0xFF) != 0)
                    m_ctx.uCycles++;

                inc_prog_counter();

                if (!m_ctx.xy16)
                {
                    m_ctx.uCycles++;

                    if (uint16_t(addr >> 8u) != (get_word_indirect(arg + m_ctx.dir) >> 8u))
                        extracycle = true;
                }

                val = m_ctx.getByte(addr);

                if (rmask)
                    val += m_ctx.getByte(addr + 1) << 8;

                return val;
            }
            else
            {
                addr = ((get_word_indirect((arg + m_ctx.dir) & 0xFFFF) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y)) & 0xFFFF) + (m_ctx.dbr << 16);

                if ((m_ctx.dir & 0xFF) != 0)
                    extracycle = true;

                inc_prog_counter();

                if (!m_ctx.xy16)
                {
                    m_ctx.uCycles++;

                    if (uint16_t(addr >> 8u) != (get_word_indirect(arg + m_ctx.dir) >> 8u))
                        extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
                }

                val = m_ctx.getByte(addr);

                if (rmask)
                    val += m_ctx.getByte(addr + 1) << 8;

                return val;
            }
        }
        else
        {
            arg = m_ctx.getByte(m_ctx.pc); // cell address on zero page
            addr = get_word_indirect(arg) + (m_ctx.y & 0xFF);
        }

        if (uint16_t(addr >> 8u) != (get_word_indirect(arg) >> 8u))
            extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing

        inc_prog_counter();
        return m_ctx.getByte(addr);

    case CAsm::A_ABSL: // 65816 only
        addr = get_Lword(m_ctx.pc + (m_ctx.pbr << 16));
        inc_prog_counter(3);
        val = m_ctx.getByte(addr);

        if (rmask)
            val += m_ctx.getByte(addr + 1) << 8;

        return val;

    case CAsm::A_ABSL_X: // 65816 only
        addr = (get_Lword(m_ctx.pc + (m_ctx.pbr << 16)) + (m_ctx.xy16 ? (m_ctx.x & 0xff) : m_ctx.x)) & 0xFFFFFF;
        inc_prog_counter(3);
        val = m_ctx.getByte(addr);

        if (rmask)
            val += m_ctx.getByte(addr + 1) << 8;

        return val;

    case CAsm::A_ZPIL: // 65816 only
        arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
        addr = get_Lword_indirect((arg + m_ctx.dir) & 0xFFFF);
        inc_prog_counter();

        if ((m_ctx.dir & 0xFF) != 0)
            extracycle = true;

        val = m_ctx.getByte(addr);

        if (rmask)
            val += m_ctx.getByte(addr + 1) << 8;

        return val;

    case CAsm::A_ZPIL_Y: // 65816 only
        arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
        addr = (get_Lword_indirect((arg + m_ctx.dir) & 0xFFFF) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y)) & 0xFFFFFF;
        inc_prog_counter();

        if ((m_ctx.dir & 0xFF) != 0)
            extracycle = true;

        val = m_ctx.getByte(addr);

        if (rmask)
            val += m_ctx.getByte(addr + 1) << 8;

        return val;

    case CAsm::A_SR: // 65816 only
        addr = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16)) + m_ctx.s) & 0xFFFF;
        val = m_ctx.getByte(addr);

        if (rmask)
            val += (m_ctx.getByte(addr + 1)) << 8;

        inc_prog_counter();
        return val;

    case CAsm::A_SRI_Y: // 65816 only
        arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
        addr = (get_word_indirect(((arg + m_ctx.s) & 0xFFFF) + (m_ctx.dbr << 16)) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y) + (m_ctx.dbr << 16)) & 0xFFFFFF;
        inc_prog_counter();
        val = m_ctx.getByte(addr);

        if (rmask)
            val += m_ctx.getByte(addr + 1) << 8;

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

// Temporary wrapper to catch/ignore exceptions on PerformCommandInner()
void CSym6502::PerformCommand()
{
    try
    {
        PerformCommandInner();
    }
    catch (...)
    {
    }
}

// The function executes the instruction indicated by m_ctx.pc, changing the state accordingly
// registers and memory (m_ctx.mem)
void CSym6502::PerformCommandInner()
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

    wxGetApp().m_global.m_bBank = cpu16();
    wxGetApp().m_global.m_bPBR = m_ctx.pbr;

    if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        m_ctx.a = (m_ctx.a & 0xFF) | (m_ctx.b << 8);

    if (m_nInterruptTrigger != NONE)
    {
        interrupt(m_nInterruptTrigger);
        cmd = m_ctx.getByte(m_ctx.pc);
    }

    if (cpu16())
        cmd = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
    else
        cmd = m_ctx.getByte(m_ctx.pc);

    //bool intFlag = m_ctx.uCycles > m_saveCycles; //% bug Fix 1.2.13.18 - command log assembly not lined up with registers (added pre)
    //ULONG oldCycles = m_saveCycles;

    switch (m_vCodeToCommand[cmd])
    {
    case CAsm::C_ADC:
        carry = m_ctx.carry;

        if (cpu16() && !m_ctx.mem16 && !m_ctx.emm)
        {
            arg = get_argument_value(true);
            acc = m_ctx.a;

            if (m_ctx.decimal)
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

                if (m_ctx.decimal && (addr & 0xff000) > 0x19000) overflow = false;
                m_ctx.a = (addr & 0xff);
                m_ctx.b = (addr >> 8) & 0xff;
            }
            else
            {
                addr = acc + arg + (carry ? 1 : 0);
                if (addr > 65535) carry = true; else carry = false;
                if ((addr & 0xffff) == 0) zero = true; else zero = false;
                if (addr & 0x8000) negative = true; else negative = false;

                overflow = ((acc & 0x8000u) == (arg & 0x8000u)) && (addr & 0x8000u) != (acc & 0x8000u);

                m_ctx.a = addr & 0xffff;
                m_ctx.b = (addr >> 8) & 0xff;
            }

            m_ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
            m_ctx.uCycles++;   // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);

            arg8 = static_cast<uint8_t>(arg & 0xFF);
            acc8 = static_cast<uint8_t>(m_ctx.a & 0xFF);

            if (m_ctx.decimal)
            {
                // Decimal addition
                tmp = acc8 + arg8 + (m_ctx.carry ? 1 : 0);
                zero = (tmp & 0xFF) == 0;

                bool af = ((acc8 ^ arg8) & 0x80) == 0;
                bool at = ((acc8 ^ tmp) & 0x80) != 0;

                m_ctx.overflow = af && at;

                int test = (acc8 & 0x0F) + (arg8 & 0x0F) + (m_ctx.carry ? 1 : 0);

                if (test > 9)
                    tmp += 6;

                if (tmp > 0x99)
                    tmp += 96;

                acc = (uint8_t)(tmp & 0xFF);
                zeroc = acc == 0;

                bool isM6502 = Processor() == ProcessorType::M6502;

                m_ctx.zero = isM6502 ? !!zero : !!zeroc;
                m_ctx.carry = tmp > 0x99;
                m_ctx.negative = (acc & 0x80) != 0;

                if (!isM6502) //% bug Fix 1.2.12.1 - fix cycle timing
                    m_ctx.uCycles++; // Add a cycle in BCD mode
            }
            else
            {
                // Binary addition
                tmp = acc8 + arg8 + (m_ctx.carry ? 1 : 0);

                bool af = ((acc8 ^ arg8) & 0x80) == 0;
                bool at = ((acc8 ^ tmp) & 0x80) != 0;

                acc = (uint8_t)(tmp & 0xFF);

                overflow = af && at;
                zero = acc == 0;
                negative = (acc & 0x80) != 0;
                carry = tmp > 0xFF;

                m_ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
            }
        }

        m_ctx.a = acc;

        if (cpu16() && extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_SBC:
        carry = m_ctx.carry;

        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_argument_value(true);
            acc = m_ctx.a;

            if (m_ctx.decimal)
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
                m_ctx.a = addr & 0xffff;
                m_ctx.b = (addr >> 8) & 0xff;
            }
            else
            {
                addr = acc - arg - (carry ? 0 : 1);
                if (addr > 65535) carry = false; else carry = true;
                if ((addr & 0xffff) == 0) zero = true; else zero = false;
                if (addr & 0x8000) negative = true; else negative = false;
                overflow = ((acc & 0x8000u) == (arg & 0x8000u)) && (addr & 0x8000u) != (acc & 0x8000u);
                m_ctx.a = addr & 0xffff;
                m_ctx.b = (addr >> 8) & 0xff;
            }
            m_ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
            m_ctx.uCycles++;   // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            acc = m_ctx.a;
            carry = m_ctx.carry;

            if (m_ctx.decimal)
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

                bool isM6502 = Processor() == ProcessorType::M6502;

                m_ctx.carry = !carry; // Carray negation in accordance with convention 6502
                m_ctx.overflow = !!overflow;
                m_ctx.negative = (isM6502 ? !!negative : !!negativec);
                m_ctx.zero = (isM6502 ? !!zero : !!zeroc);

                if (!isM6502) //% bug Fix 1.2.12.1 - fix cycle timing
                    m_ctx.uCycles++; // Add a cycle in BCD mode
            }
            else
            {
                tmp = acc - arg - (carry ? 0 : 1);

                overflow = ((acc ^ tmp) & 0x80) && ((acc ^ arg) & 0x80);
                carry = tmp < 0;
                acc = (uint8_t)(tmp & 0xFF);
                zero = acc == 0;
                negative = acc & 0x80;

                m_ctx.set_status_reg_VZNC(overflow, zero, negative, !carry);
            }

            m_ctx.a = acc;
        }

        if (cpu16() && extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case CAsm::C_CMP:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_argument_value(true);
            acc = m_ctx.a;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFFFF) == 0;
            negative = tmp & 0x8000;

            m_ctx.uCycles++;   // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false) & 0xFF;
            acc = m_ctx.a;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFF) == 0;
            negative = tmp & 0x80;
        }

        m_ctx.set_status_reg_ZNC(zero, negative, carry);

        if (cpu16() && extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_CPX:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            arg = get_argument_value(true);
            acc = m_ctx.x;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFFFF) == 0;
            negative = tmp & 0x8000;
        }
        else
        {
            arg = get_argument_value(false) & 0xFF;
            acc = m_ctx.x & 0xFF;

            // Compare always in binary, don't set acc
            tmp = acc - arg;

            carry = tmp < 0;
            zero = (tmp & 0xFF) == 0;
            negative = tmp & 0x80;
        }

        m_ctx.set_status_reg_ZNC(zero, negative, !carry);
        break;

    case CAsm::C_CPY:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            arg = get_argument_value(true);
            acc = m_ctx.y;

            // Compare always in binary, don't set acc
            tmp = acc - arg - (m_ctx.carry ? 0 : 1);

            carry = tmp < 0;
            zero = (tmp & 0xFFFF) == 0;
            negative = tmp & 0x8000;
        }
        else
        {
            arg = get_argument_value(false) & 0xFF;
            acc = m_ctx.y & 0xFF;

            // Compare always in binary, don't set acc
            tmp = acc - arg - (m_ctx.carry ? 0 : 1);

            carry = tmp < 0;
            zero = (tmp & 0xFF) == 0;
            negative = tmp & 0x80;
        }

        m_ctx.set_status_reg_ZNC(zero, negative, !carry);
        break;

    case CAsm::C_ASL:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();

                acc32 = m_ctx.a << 1;
                carry = !!(m_ctx.a & 0x8000);
                zero = !!((acc32 & 0xffff) == 0);
                negative = !!(acc32 & 0x8000);

                m_ctx.a = acc32 & 0xffff;
                m_ctx.b = (acc32 >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = get_word(addr);

                acc32 = acc << 1;
                carry = !!(acc & 0x8000);
                zero = !!((acc32 & 0xffff) == 0);
                negative = !!(acc32 & 0x8000);

                m_ctx.setWord(addr, acc32);
                m_ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = m_ctx.a;

                carry = acc & 0x80;
                acc <<= 1;
                zero = acc == 0;
                negative = acc & 0x80;

                m_ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = m_ctx.getByte(addr);

                carry = acc & 0x80;
                acc <<= 1;
                zero = (acc & 0xFF) == 0;
                negative = acc & 0x80;

                m_ctx.setByte(addr, acc & 0xFF);
            }
        }

        if (Processor() != ProcessorType::M6502 && extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        m_ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_LSR:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();
                acc = m_ctx.a >> 1;
                carry = !!(m_ctx.a & 0x01);
                zero = !!(acc == 0);
                negative = !!(acc & 0x8000);
                m_ctx.a = acc;
                m_ctx.b = (acc >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                arg = get_word(addr);
                acc = arg >> 1;
                carry = !!(arg & 0x01);
                zero = !!((acc & 0xffff) == 0);
                negative = !!(acc & 0x8000);
                m_ctx.setWord(addr, acc & 0xff);
                ++addr;
                m_ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = m_ctx.a;

                carry = acc & 0x01;
                acc >>= 1;
                zero = acc == 0;
                negative = acc & 0x80;

                m_ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = m_ctx.getByte(addr);

                carry = acc & 0x01;
                acc >>= 1;
                zero = acc == 0;
                negative = acc & 0x80;

                m_ctx.setByte(addr, acc);
            }
        }

        if (Processor() != ProcessorType::M6502 && extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        m_ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_ROL:
        carry = m_ctx.carry;
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();
                acc = (m_ctx.a << 1) + (carry ? 1 : 0);
                carry = !!(m_ctx.a & 0x8000);
                zero = !!(acc == 0);
                negative = !!(acc & 0x8000);
                m_ctx.a = acc;
                m_ctx.b = (acc >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                arg = get_word(addr);
                acc = (arg << 1) + (carry ? 1 : 0);
                carry = !!(arg & 0x8000);
                zero = !!((acc & 0xffff) == 0);
                negative = !!(acc & 0x8000);
                m_ctx.setWord(addr, acc);
                ++addr;
                m_ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = m_ctx.a;

                carry = acc & 0x80;
                acc <<= 1;

                if (m_ctx.carry)
                    acc |= 1;
                else
                    acc &= ~1;

                carry = acc > 0xFF;
                zero = acc == 0;
                negative = acc & 0x80;

                m_ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = m_ctx.getByte(addr);

                carry = acc & 0x80;
                acc <<= 1;

                if (m_ctx.carry)
                    acc |= 1;
                else
                    acc &= ~1;

                carry = acc > 0xFF;
                zero = acc == 0;
                negative = acc & 0x80;

                m_ctx.setByte(addr, acc);
            }
        }

        if (Processor() != ProcessorType::M6502 && extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        m_ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_ROR:
        carry = m_ctx.carry;
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC)
            {
                inc_prog_counter();
                acc = (m_ctx.a >> 1) + (carry ? 0x8000 : 0);;
                carry = !!(m_ctx.a & 0x01);
                zero = !!(acc == 0);
                negative = !!(acc & 0x8000);
                m_ctx.a = acc;
                m_ctx.b = (acc >> 8) & 0xff;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                arg = get_word(addr);
                acc = (arg >> 1) + (carry ? 0x8000 : 0);
                carry = !!(arg & 0x01);
                zero = !!((acc & 0xffff) == 0);
                negative = !!(acc & 0x8000);
                m_ctx.setWord(addr, acc);
                ++addr;
                m_ctx.uCycles += 2; // add 2 cycles for 16 bit operations
            }
        }
        else
        {
            if (m_vCodeToMode[cmd] == CAsm::A_ACC) // Accumulator operation
            {
                inc_prog_counter(); // bypass the command
                acc = m_ctx.a;

                carry = acc & 0x01;
                acc >>= 1;

                if (m_ctx.carry)
                {
                    acc |= 0x80;
                    negative = true;
                }
                else
                {
                    acc &= ~0x80;
                    negative = false;
                }

                m_ctx.a = acc;
            }
            else
            {
                addr = get_argument_address(s_bWriteProtectArea);
                acc = m_ctx.getByte(addr);

                carry = acc & 0x01;
                acc >>= 1;

                if (m_ctx.carry)
                {
                    acc |= 0x80;
                    negative = true;
                }
                else
                {
                    acc &= ~0x80;
                    negative = false;
                }

                m_ctx.setByte(addr, acc);
            }
            zero = acc == 0;
        }

        if (Processor() != ProcessorType::M6502 && extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        m_ctx.set_status_reg_ZNC(zero, negative, carry);
        break;

    case CAsm::C_AND:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_argument_value(true);
            m_ctx.a &= arg;
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
            m_ctx.set_status_reg16(m_ctx.a);
            m_ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            m_ctx.a &= (arg & 0xFF);
            m_ctx.set_status_reg(m_ctx.a);
        }

        if (extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_ORA:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_argument_value(true);
            m_ctx.a |= arg;
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
            m_ctx.set_status_reg16(m_ctx.a);
            m_ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            m_ctx.a |= (arg & 0xFF);
            m_ctx.set_status_reg(m_ctx.a);
        }

        if (extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_EOR:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_argument_value(true);
            m_ctx.a ^= arg;
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
            m_ctx.set_status_reg16(m_ctx.a);
            m_ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            m_ctx.a = (m_ctx.a & 0xFF) ^ (arg & 0xFF);
            m_ctx.set_status_reg(m_ctx.a);
        }

        if (extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_INC:
        if (m_vCodeToMode[cmd] == CAsm::A_ACC)
        {
            inc_prog_counter();
            m_ctx.a++;

            if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
            {
                m_ctx.b = (m_ctx.a >> 8) & 0xFF;
                m_ctx.set_status_reg16(m_ctx.a);
            }
            else
                m_ctx.set_status_reg(m_ctx.a);
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);

            if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
            {
                tmp = get_word(addr);
                ++tmp;
                m_ctx.set_status_reg16(tmp);
                m_ctx.setWord(addr, tmp);
                m_ctx.uCycles += 2; // 16 bit operation adds 2 cycles
            }
            else
            {
                uint8_t v = m_ctx.getByte(addr) + 1;
                m_ctx.setByte(addr, v);
                m_ctx.set_status_reg(v);
            }

            if (cpu16() && extracycle)
                m_ctx.uCycles++;
        }
        break;

    case CAsm::C_DEC:
        if (m_vCodeToMode[cmd] == CAsm::A_ACC)
        {
            inc_prog_counter();
            m_ctx.a--;

            if (cpu16() && !m_ctx.emm && m_ctx.mem16)
            {
                m_ctx.b = (m_ctx.a >> 8) & 0xFF;
                m_ctx.set_status_reg16(m_ctx.a);
            }
            else
                m_ctx.set_status_reg(m_ctx.a);
        }
        else
        {
            addr = get_argument_address(s_bWriteProtectArea);

            if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
            {
                tmp = get_word(addr);
                --tmp;
                m_ctx.set_status_reg16(tmp);
                m_ctx.setWord(addr, tmp);
                m_ctx.uCycles += 2; // 16 bit operation adds 2 cycles;
            }
            else
            {
                uint8_t v = m_ctx.getByte(addr) - 1;
                m_ctx.setByte(addr, v);
                m_ctx.set_status_reg(v);
            }

            if (cpu16() && extracycle)
                m_ctx.uCycles++;
        }
        break;

    case CAsm::C_INX:
        inc_prog_counter();
        m_ctx.x++;

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
            m_ctx.set_status_reg16(m_ctx.x);
        else
        {
            m_ctx.set_status_reg(m_ctx.x & 0xFF);
            m_ctx.x = m_ctx.x & 0xFF;
        }

        break;

    case CAsm::C_DEX:
        inc_prog_counter();
        m_ctx.x--;

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
            m_ctx.set_status_reg16(m_ctx.x);
        else
        {
            m_ctx.set_status_reg(m_ctx.x & 0xFF);
            m_ctx.x = m_ctx.x & 0xFF;
        }

        break;

    case CAsm::C_INY:
        inc_prog_counter();
        m_ctx.y++;

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
            m_ctx.set_status_reg16(m_ctx.y);
        else
        {
            m_ctx.set_status_reg(m_ctx.y & 0xFF);
            m_ctx.y = m_ctx.y & 0xFF;
        }

        break;

    case CAsm::C_DEY:
        inc_prog_counter();
        m_ctx.y--;

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
            m_ctx.set_status_reg16(m_ctx.y);
        else
        {
            m_ctx.set_status_reg(m_ctx.y & 0xFF);
            m_ctx.y = m_ctx.y & 0xFF;
        }

        break;

    case CAsm::C_TAX:
        inc_prog_counter();
        m_ctx.x = m_ctx.a;

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
            m_ctx.set_status_reg16(m_ctx.x);
        else
            m_ctx.set_status_reg(m_ctx.x & 0xFF);

        break;

    case CAsm::C_TXA:
        inc_prog_counter();
        m_ctx.a = m_ctx.x;

        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
            m_ctx.set_status_reg16(m_ctx.a);
        }
        else
            m_ctx.set_status_reg(m_ctx.a & 0xFF);

        break;

    case CAsm::C_TAY:
        inc_prog_counter();
        m_ctx.y = m_ctx.a;

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
            m_ctx.set_status_reg16(m_ctx.y);
        else
            m_ctx.set_status_reg(m_ctx.y & 0xFF);

        break;

    case CAsm::C_TYA:
        inc_prog_counter();
        m_ctx.a = m_ctx.y;

        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            m_ctx.set_status_reg16(m_ctx.a);
            m_ctx.b = (m_ctx.a >> 8) & 0xff;
        }
        else
            m_ctx.set_status_reg(m_ctx.a & 0xff);

        break;

    case CAsm::C_TSX:
        inc_prog_counter();

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.x = m_ctx.s;
            m_ctx.set_status_reg16(m_ctx.x);
        }
        else
        {
            m_ctx.x = m_ctx.s & 0xFF;
            m_ctx.set_status_reg(m_ctx.x & 0xFF);
        }
        break;

    case CAsm::C_TXS:
        inc_prog_counter();
        if (cpu16() && !m_ctx.emm)
        {
            m_ctx.s = m_ctx.x;
            wxGetApp().m_global.m_bSRef = m_ctx.s;
        }
        else
        {
            m_ctx.s = 0x100 + (m_ctx.x & 0xff); //****fix
            wxGetApp().m_global.m_bSRef = 0x1ff;
        }
        break;

    case CAsm::C_STA:
        addr = get_argument_address(s_bWriteProtectArea);
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            m_ctx.setWord(addr, m_ctx.a);
            m_ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
        {
            m_ctx.setByte(addr, m_ctx.a & 0xFF);
        }

        if (cpu16() && extracycle)
            m_ctx.uCycles++;

        break;

    case CAsm::C_STX:
        addr = get_argument_address(s_bWriteProtectArea);

        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.setWord(addr, m_ctx.x);
            m_ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
        {
            m_ctx.setByte(addr, m_ctx.x & 0xFF);
        }

        if (cpu16() && extracycle)
            m_ctx.uCycles++;

        break;

    case CAsm::C_STY:
        addr = get_argument_address(s_bWriteProtectArea);
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.setWord(addr, m_ctx.y);
            m_ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
        {
            m_ctx.setByte(addr, m_ctx.y & 0xFF);
        }

        if (cpu16() && extracycle)
            m_ctx.uCycles++;

        break;

    case CAsm::C_LDA:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            m_ctx.set_status_reg16(m_ctx.a = get_argument_value(true));
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
            m_ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
            m_ctx.set_status_reg((m_ctx.a = (get_argument_value(false) & 0xFF)) & 0xFF);

        if (extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_LDX:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.set_status_reg16(m_ctx.x = get_argument_value(true));
            m_ctx.uCycles++;  // add 1 cycle for 16 bit operation 

        }
        else
            m_ctx.set_status_reg((m_ctx.x = (get_argument_value(false) & 0xFF)) & 0xFF);

        if (extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing

        break;

    case CAsm::C_LDY:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.set_status_reg16(m_ctx.y = get_argument_value(true));
            m_ctx.uCycles++;  // add 1 cycle for 16 bit operation 
        }
        else
            m_ctx.set_status_reg((m_ctx.y = (get_argument_value(false) & 0xFF)) & 0xFF);

        if (extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.12.1 - fix cycle timing
        break;

    case CAsm::C_BIT:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_argument_value(true);
            m_ctx.zero = ((m_ctx.a & arg) == 0);
            m_ctx.uCycles++; // 16 bit operation adds 1 cycle
        }
        else
        {
            arg = get_argument_value(false);
            m_ctx.zero = (((m_ctx.a & 0xFF) & (arg & 0xFF)) == 0);
        }

        if (cmd != 0x89) // 65C02/816 BIT # only updates Z flag
        {
            if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
            {
                m_ctx.negative = (arg & 0x8000) != 0;
                m_ctx.overflow = (arg & 0x4000) != 0;
            }
            else
            {
                m_ctx.negative = (arg & 0x80) != 0;
                m_ctx.overflow = (arg & 0x40) != 0;
            }
        }

        if (extracycle)
            m_ctx.uCycles++; //% bug Fix 1.2.13.1 - fix cycle timing
        break;

    case CAsm::C_PHA:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            push_addr_on_stack(m_ctx.a);
            m_ctx.uCycles++;
        }
        else
            push_on_stack(m_ctx.a & 0xFF);

        inc_prog_counter();
        break;

    case CAsm::C_PLA:
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            m_ctx.a = pull_addr_from_stack();
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
            m_ctx.set_status_reg16(m_ctx.a);
            m_ctx.uCycles++;
        }
        else
        {
            m_ctx.a = pull_from_stack();
            m_ctx.set_status_reg(m_ctx.a & 0xFF);
        }

        inc_prog_counter();
        break;

    case CAsm::C_PHP:
        inc_prog_counter();
        if (cpu16() && !m_ctx.emm)
            push_on_stack(m_ctx.get_status_reg());
        else
            push_on_stack(m_ctx.get_status_reg() | CContext::BREAK | CContext::RESERVED);
        break;

    case CAsm::C_PLP:
        inc_prog_counter();
        m_ctx.set_status_reg_bits(pull_from_stack());
        if (Processor() != ProcessorType::M6502)
        {
            // 65C02 mode
            m_ctx.reserved = true; // 'reserved' bit always set
            m_ctx.break_bit = true;
        }
        break;

    case CAsm::C_JSR:
        addr = get_argument_address(false);
        if (cpu16() && (cmd == 0xFC) && m_ctx.emm)
        {
            m_ctx.emm = false;
            push_addr_on_stack((m_ctx.pc - 1) & 0xFFFF);
            m_ctx.emm = true;
        }
        else
            push_addr_on_stack((m_ctx.pc - 1) & 0xFFFF);
        m_ctx.pc = addr;
        break;

    case CAsm::C_JMP:
        addr = get_argument_address(false) & 0xFFFF;
        m_ctx.pc = addr;
        break;

    case CAsm::C_RTS:
        if ((finish == CAsm::FIN_BY_RTS) && ((m_ctx.s & 0xFF) == 0xFF)) // RTS on empty stack?
        {
            CurrentStatus = CSym6502::Status::FINISH;
            return;
        }

        m_ctx.pc = (pull_addr_from_stack() + 1) & 0xFFFF;
        break;

    case CAsm::C_RTI:
        m_ctx.set_status_reg_bits(pull_from_stack());
        m_ctx.pc = pull_addr_from_stack();
        if (cpu16() && !m_ctx.emm)
        {
            m_ctx.pbr = pull_from_stack();
            wxGetApp().m_global.m_bPBR = m_ctx.pbr;
            m_ctx.uCycles++; // native mode takes 1 extra cycle
        }
        break;

    case CAsm::C_BCC:
        arg = get_argument_value(false);
        if (!m_ctx.carry)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BCS:
        arg = get_argument_value(false);
        if (m_ctx.carry)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BVC:
        arg = get_argument_value(false);
        if (!m_ctx.overflow)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BVS:
        arg = get_argument_value(false);
        if (m_ctx.overflow)
        {
            AddBranchCycles(arg && 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BNE:
        arg = get_argument_value(false);
        if (!m_ctx.zero)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BEQ:
        arg = get_argument_value(false);
        if (m_ctx.zero)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BPL:
        arg = get_argument_value(false);
        if (!m_ctx.negative)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BMI:
        arg = get_argument_value(false);
        if (m_ctx.negative)
        {
            AddBranchCycles(arg & 0xFF);
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_NOP:
        inc_prog_counter();
        break;

    case CAsm::C_CLI:
        inc_prog_counter();
        m_ctx.interrupt = false;
        break;

    case CAsm::C_SEI:
        inc_prog_counter();
        m_ctx.interrupt = true;
        break;

    case CAsm::C_CLD:
        inc_prog_counter();
        m_ctx.decimal = false;
        break;

    case CAsm::C_SED:
        inc_prog_counter();
        m_ctx.decimal = true;
        break;

    case CAsm::C_CLC:
        inc_prog_counter();
        m_ctx.carry = false;
        break;

    case CAsm::C_SEC:
        inc_prog_counter();
        m_ctx.carry = true;
        break;

    case CAsm::C_CLV:
        inc_prog_counter();
        m_ctx.overflow = false;
        break;

    case CAsm::C_BRK:
        if (finish == CAsm::FIN_BY_BRK) // BRK instruction terminates the program?
        {
            CurrentStatus = CSym6502::Status::FINISH;
            return;
        }

        if (cpu16() && !m_ctx.emm)
        {
            push_on_stack(m_ctx.pbr);
            push_addr_on_stack(m_ctx.pc);
            push_on_stack(m_ctx.get_status_reg());
            m_ctx.decimal = false;
            m_ctx.pc = get_brk_addr16();
            m_ctx.uCycles++; // native mode takes 1 extra cycle
        }
        else
        {
            push_addr_on_stack(m_ctx.pc);
            m_ctx.break_bit = true;
            push_on_stack(m_ctx.get_status_reg() | CContext::RESERVED);
            if (Processor() != ProcessorType::M6502)
                m_ctx.decimal = false;
            m_ctx.pc = get_irq_addr();
        }
        m_ctx.interrupt = true; // after pushing status
        m_ctx.pbr = 0;
        wxGetApp().m_global.m_bPBR = m_ctx.pbr;
        break;

        //---------- 65c02 --------------------------------------------------------

    case CAsm::C_PHX:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            push_addr_on_stack(m_ctx.x);
            m_ctx.uCycles++;
        }
        else
            push_on_stack(m_ctx.x & 0xFF);
        inc_prog_counter();
        break;

    case CAsm::C_PLX:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.x = pull_addr_from_stack();
            m_ctx.set_status_reg16(m_ctx.x);
            m_ctx.uCycles++;
        }
        else
        {
            m_ctx.x = pull_from_stack();
            m_ctx.set_status_reg(m_ctx.x & 0xFF);
        }
        inc_prog_counter();
        break;

    case CAsm::C_PHY:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            push_addr_on_stack(m_ctx.y);
            m_ctx.uCycles++;
        }
        else
            push_on_stack(m_ctx.y & 0xFF);
        inc_prog_counter();
        break;

    case CAsm::C_PLY:
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.y = pull_addr_from_stack();
            m_ctx.set_status_reg16(m_ctx.y);
            m_ctx.uCycles++;
        }
        else
        {
            m_ctx.y = pull_from_stack();
            m_ctx.set_status_reg(m_ctx.y & 0xFF);
        }
        inc_prog_counter();
        break;

    case CAsm::C_BRA:
        arg = get_argument_value(false);
        AddBranchCycles(arg & 0xFF);
        if (arg & 0x80) // jump back
            m_ctx.pc -= 0x100 - arg;
        else // jump forward
            m_ctx.pc += arg;
        break;

    case CAsm::C_INA:
        inc_prog_counter();
        m_ctx.a++;
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            m_ctx.set_status_reg16(m_ctx.a);
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
        }
        else
            m_ctx.set_status_reg(m_ctx.a & 0xFF);
        break;

    case CAsm::C_DEA:
        inc_prog_counter();
        m_ctx.a--;
        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            m_ctx.set_status_reg16(m_ctx.a);
            m_ctx.b = (m_ctx.a >> 8) & 0xFF;
        }
        else
            m_ctx.set_status_reg(m_ctx.a & 0xFF);

        break;

    case CAsm::C_STZ:
        addr = get_argument_address(s_bWriteProtectArea);
        m_ctx.setByte(addr, 0);

        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            addr++;
            m_ctx.setByte(addr, 0);
        }

        break;

    case CAsm::C_TRB:
        addr = get_argument_address(s_bWriteProtectArea);

        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_word(addr);
            m_ctx.setWord(addr, (arg & ~m_ctx.a));
            m_ctx.zero = (arg & m_ctx.a) == 0;
            m_ctx.uCycles += 2;   // 16 bit operation adds 2 cycle
        }
        else
        {
            arg = m_ctx.getByte(addr);
            m_ctx.setByte(addr, (arg & 0xFF) & ~(m_ctx.a & 0xFF));
            m_ctx.zero = ((arg & 0xFF) & (m_ctx.a & 0xFF)) == 0;
        }

        if (extracycle)
            m_ctx.uCycles++;

        break;

    case CAsm::C_TSB:
        addr = get_argument_address(s_bWriteProtectArea);

        if (cpu16() && !m_ctx.emm && !m_ctx.mem16)
        {
            arg = get_word(addr);
            m_ctx.setWord(addr, arg | m_ctx.a);
            m_ctx.zero = (arg & m_ctx.a) == 0;
            m_ctx.uCycles += 2;   // 16 bit operation adds 2 cycle
        }
        else
        {
            arg = m_ctx.getByte(addr);
            m_ctx.setByte(addr, (arg & 0xFF) | (m_ctx.a & 0xFF));
            m_ctx.zero = ((arg & 0xFF) & (m_ctx.a & 0xFF)) == 0;
        }

        if (extracycle)
            m_ctx.uCycles++;

        break;

    case CAsm::C_BBR:
        addr = get_argument_address(false);
        if (!(m_ctx.getByte(addr & 0xFF) & uint8_t(1 << ((cmd >> 4) & 0x07))))
        {
            arg = addr >> 8;
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            else // jump forward
                m_ctx.pc += arg;
        }
        break;

    case CAsm::C_BBS:
        addr = get_argument_address(false);
        if (m_ctx.getByte(addr & 0xFF) & uint8_t(1 << ((cmd >> 4) & 0x07)))
        {
            arg = addr >> 8;
            if (arg & 0x80) // jump back
                m_ctx.pc -= 0x100 - arg;
            m_ctx.pc += arg;
        }
        break;

    case CAsm::C_RMB:
    {
        addr = get_argument_address(s_bWriteProtectArea);
        uint8_t v = m_ctx.getByte(addr) & uint8_t(~(1 << ((cmd >> 4) & 0x07)));
        m_ctx.setByte(addr, v);
        break;
    }

    case CAsm::C_SMB:
    {
        addr = get_argument_address(s_bWriteProtectArea);
        uint8_t v = m_ctx.getByte(addr) | uint8_t(1 << ((cmd >> 4) & 0x07));
        m_ctx.setByte(addr, v);
        break;
    }

        //------------65816--------------------------------------------------------
    case CAsm::C_TXY:
        inc_prog_counter();
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.y = m_ctx.x;
            m_ctx.set_status_reg16(m_ctx.y);
        }
        else
        {
            m_ctx.y = m_ctx.x & 0xFF;
            m_ctx.set_status_reg(m_ctx.y & 0xFF);
        }
        break;

    case CAsm::C_TYX:
        inc_prog_counter();
        if (cpu16() && !m_ctx.emm && !m_ctx.xy16)
        {
            m_ctx.x = m_ctx.y;
            m_ctx.set_status_reg16(m_ctx.x);
        }
        else
        {
            m_ctx.x = m_ctx.y & 0xFF;
            m_ctx.set_status_reg(m_ctx.x & 0xFF);
        }
        break;

    case CAsm::C_STP:
        inc_prog_counter();
        CurrentStatus = CSym6502::Status::STOP;
        return;

    case CAsm::C_BRL:
        inc_prog_counter();
        addr = get_word(m_ctx.pc + (m_ctx.pbr << 16));
        inc_prog_counter(2);

        if (addr & 0x8000)
            m_ctx.pc = (m_ctx.pc - (0x10000 - addr)) & 0xFFFF;
        else
            m_ctx.pc = (m_ctx.pc + addr) & 0xFFFF;

        break;

    case CAsm::C_JSL:
        inc_prog_counter();
        addr = get_word(m_ctx.pc + (m_ctx.pbr << 16));
        inc_prog_counter(2);

        if (m_ctx.emm && (m_ctx.s == 0x0100))
        {
            m_ctx.emm = false;
            push_on_stack(m_ctx.pbr);
            push_addr_on_stack(m_ctx.pc & 0xFFFF);
            m_ctx.s = 0x01FD;
            m_ctx.emm = true;
        }
        else
        {
            push_on_stack(m_ctx.pbr);
            push_addr_on_stack(m_ctx.pc & 0xFFFF);
        }

        m_ctx.pbr = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
        wxGetApp().m_global.m_bPBR = m_ctx.pbr;
        m_ctx.pc = addr & 0xFFFF;
        break;

    case CAsm::C_JML:
        addr = get_argument_address(false);
        m_ctx.pc = addr & 0xFFFF;
        m_ctx.pbr = (addr >> 16) & 0xFF;
        wxGetApp().m_global.m_bPBR = m_ctx.pbr;
        break;

    case CAsm::C_COP:
        inc_prog_counter(2);

        if (!m_ctx.emm)
        {
            push_on_stack(m_ctx.pbr);
            push_addr_on_stack(m_ctx.pc);
            push_on_stack(m_ctx.get_status_reg());
            m_ctx.pc = get_cop_addr16();
            m_ctx.uCycles++; // native mode takes 1 extra cycle
        }
        else
        {
            push_addr_on_stack(m_ctx.pc);
            push_on_stack(m_ctx.get_status_reg());
            m_ctx.pc = get_cop_addr();
        }

        m_ctx.interrupt = true; // after pushing status
        m_ctx.decimal = false;
        m_ctx.pbr = 0;
        break;

    case CAsm::C_MVN:
    {
        m_ctx.dbr = m_ctx.getByte(m_ctx.pc + 1 + (m_ctx.pbr << 16));
        arg = m_ctx.getByte(m_ctx.pc + 2 + (m_ctx.pbr << 16));
        //m_ctx.a++;
        //while (m_ctx.a--){
        // check for write protect error
        uint32_t ptr = (m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y);

        if (s_bWriteProtectArea && ptr >= s_uProtectFromAddr && ptr <= s_uProtectToAddr)
        {
            CurrentStatus = CSym6502::Status::ILL_WRITE;
            return;
        }

        uint8_t v = m_ctx.getByte((arg << 16) + (m_ctx.xy16 ? (m_ctx.x++ & 0xFF) : m_ctx.x++));
        m_ctx.setByte((m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y++ & 0xFF) : m_ctx.y++), v);
        //}
        if (((--m_ctx.a) & 0xFFFF) == 0xFFFF)
            inc_prog_counter(3);  // repeat opcode until A = 0xFFFF
        m_ctx.b = (m_ctx.a >> 8) & 0xFF;
        break;
    }
    case CAsm::C_MVP:
    {
        m_ctx.dbr = m_ctx.getByte(m_ctx.pc + 1 + (m_ctx.pbr << 16));
        arg = m_ctx.getByte(m_ctx.pc + 2 + (m_ctx.pbr << 16));
        //m_ctx.a++;
        //while (m_ctx.a--){
        // check for write protect error
        uint32_t ptr = (m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y & 0xFF) : m_ctx.y);

        if (s_bWriteProtectArea && ptr >= s_uProtectFromAddr && ptr <= s_uProtectToAddr)
        {
            CurrentStatus = CSym6502::Status::ILL_WRITE;
            return;
        }

        uint8_t v = m_ctx.getByte((arg << 16) + (m_ctx.xy16 ? (m_ctx.x-- & 0xFF) : m_ctx.x--));
        m_ctx.setByte((m_ctx.dbr << 16) + (m_ctx.xy16 ? (m_ctx.y-- & 0xFF) : m_ctx.y--), v);
        //}

        if (((--m_ctx.a) & 0xFFFF) == 0xFFFF)
            inc_prog_counter(3);   // repeat opcode until A = 0xFFFF
        m_ctx.b = (m_ctx.a >> 8) & 0xFF;
        break;
    }

    case CAsm::C_PEA:
        inc_prog_counter();
        addr = get_word(m_ctx.pc + (m_ctx.pbr << 16));

        if (m_ctx.emm && (m_ctx.s == 0x0100))
        {
            m_ctx.emm = false;
            push_addr_on_stack(addr);
            m_ctx.s = 0x01FE;
            m_ctx.emm = true;
        }
        else
            push_addr_on_stack(addr);

        inc_prog_counter(2);
        break;

    case CAsm::C_PEI:
        inc_prog_counter();
        arg = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
        addr = (arg + m_ctx.dir) & 0xFFFF;
        arg = get_word(addr);

        if (m_ctx.emm && (m_ctx.s == 0x0100))
        {
            m_ctx.emm = false;
            push_addr_on_stack(arg);
            m_ctx.s = 0x01FE;
            m_ctx.emm = true;
        }
        else
            push_addr_on_stack(arg);

        if ((m_ctx.dir & 0xFF) != 0)
            m_ctx.uCycles++;

        inc_prog_counter();
        break;

    case CAsm::C_PER:
        inc_prog_counter();
        arg = get_word(m_ctx.pc + (m_ctx.pbr << 16));
        inc_prog_counter(2);
        if (arg & 0x8000)
            addr = (m_ctx.pc - (0x10000 - arg)) & 0xFFFF;
        else
            addr = (m_ctx.pc + arg) & 0xFFFF;

        if (m_ctx.emm && (m_ctx.s == 0x0100))
        {
            m_ctx.emm = false;
            push_addr_on_stack(addr);
            m_ctx.s = 0x01FE;
            m_ctx.emm = true;
        }
        else
            push_addr_on_stack(addr);
        break;

    case CAsm::C_PHB:
        push_on_stack(m_ctx.dbr);
        inc_prog_counter();
        break;

    case CAsm::C_PHD:
        if (m_ctx.emm && (m_ctx.s == 0x0100))
        {
            m_ctx.setByte(0xFF, m_ctx.dir & 0xFF);
            m_ctx.setByte(0x100, (m_ctx.dir >> 8) & 0xFF);
            m_ctx.s = 0x01FE;
        }
        else
            push_addr_on_stack(m_ctx.dir);

        inc_prog_counter();
        break;

    case CAsm::C_PHK:
        push_on_stack(m_ctx.pbr);
        inc_prog_counter();
        break;

    case CAsm::C_PLB:
        inc_prog_counter();
        if (m_ctx.emm && (m_ctx.s == 0x01FF)) // special case
        {
            m_ctx.dbr = m_ctx.getByte(0x200);
            m_ctx.s = 0x100;
        }
        else
            m_ctx.dbr = pull_from_stack();

        m_ctx.set_status_reg(m_ctx.dbr);
        break;

    case CAsm::C_PLD:
        inc_prog_counter();
        if (m_ctx.emm && (m_ctx.s == 0x01FF)) // special case
        {
            m_ctx.dir = get_word(0x200);
            m_ctx.s = 0x0101;
        }
        else
            m_ctx.dir = pull_addr_from_stack();

        m_ctx.set_status_reg16(m_ctx.dir);
        break;

    case CAsm::C_REP:
        inc_prog_counter();
        arg8 = (m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16))) ^ (0xFF); // EOR
        m_ctx.set_status_reg_bits(m_ctx.get_status_reg() & arg8);
        inc_prog_counter();
        break;

    case CAsm::C_SEP:
        inc_prog_counter();
        arg8 = m_ctx.getByte(m_ctx.pc + (m_ctx.pbr << 16));
        m_ctx.set_status_reg_bits(m_ctx.get_status_reg() | arg8);
        if (m_ctx.xy16)
        {
            m_ctx.x = m_ctx.x & 0xFF;
            m_ctx.y = m_ctx.y & 0xFF;
        }
        inc_prog_counter();
        break;

    case CAsm::C_RTL:
        if (m_ctx.emm && (m_ctx.s == 0x01FF))
        {
            m_ctx.pc = get_word(0x200) + 1;
            m_ctx.pbr = m_ctx.getByte(0x202);
            m_ctx.s = 0x0102;
        }
        else
        {
            m_ctx.pc = ((pull_addr_from_stack() + 1) & 0xFFFF);
            m_ctx.pbr = (pull_from_stack() & 0xFF);
        }

        wxGetApp().m_global.m_bPBR = m_ctx.pbr;
        break;

    case CAsm::C_TCD:
        inc_prog_counter();

        if (m_ctx.mem16)
            m_ctx.dir = (m_ctx.a & 0xFF) + (m_ctx.b << 8);
        else
            m_ctx.dir = m_ctx.a;

        m_ctx.set_status_reg16(m_ctx.dir);
        break;

    case CAsm::C_TCS:
        inc_prog_counter();

        if (m_ctx.mem16)
        {
            m_ctx.s = (m_ctx.a & 0xFF) + (m_ctx.b << 8);
            wxGetApp().m_global.m_bSRef = m_ctx.s;
        }
        else
        {
            m_ctx.s = m_ctx.a;
            wxGetApp().m_global.m_bSRef = m_ctx.s + 0x100;
        }
        break;

    case CAsm::C_TDC:
        inc_prog_counter();
        m_ctx.a = m_ctx.dir;
        m_ctx.b = (m_ctx.a >> 8) & 0xFF;
        m_ctx.set_status_reg16(m_ctx.a);
        break;

    case CAsm::C_TSC:
        inc_prog_counter();
        m_ctx.a = m_ctx.s;
        m_ctx.b = (m_ctx.a >> 8) & 0xFF;
        m_ctx.set_status_reg16(m_ctx.a);
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
        arg = m_ctx.b;
        m_ctx.b = m_ctx.a & 0xFF;
        m_ctx.a = arg;
        m_ctx.set_status_reg(m_ctx.a & 0xFF);
        break;

    case CAsm::C_XCE:
        inc_prog_counter();
        arg = m_ctx.emm;
        m_ctx.emm = m_ctx.carry;
        m_ctx.carry = arg;

        if (m_ctx.emm)
        {
            m_ctx.mem16 = true;
            m_ctx.xy16 = true;
        }
        break;

        //-------------------------------------------------------------------------

    case CAsm::C_ILL:
        if (finish == CAsm::FIN_BY_DB && cmd == 0xDB) // DB is invalid for 6502 and 65C02 - STP for 65816
        {
            CurrentStatus = CSym6502::Status::FINISH;
            return;
        }

        if (Processor() != ProcessorType::M6502)
        {
            // 65C02 mode
            arg = get_argument_value(false);
            extracycle = false;
            break;
        }

        CurrentStatus = CSym6502::Status::BPT_ILLEGAL_CODE;
        return;

    default:
        ASSERT(false);
        break;
    }

    m_ctx.uCycles += m_vCodeToCycles[cmd];

#if 0
    // TODO: Remove reference to Deasm.h
    CmdInfo ci(m_ctx);

    ci.intFlag = intFlag;
    ci.uCycles -= oldCycles; // this provides cycles used per instruction

    m_log.Record(ci);
#endif

    m_saveCycles = m_ctx.uCycles;

    CurrentStatus = CSym6502::Status::OK;
}

void CSym6502::skip_cmd() // Skip the current statement
{
    inc_prog_counter(CAsm::mode_to_len[m_vCodeToMode[m_ctx.getByte(m_ctx.pc)]]);
}

//=============================================================================

void CSym6502::StepInto()
{
    ASSERT(CurrentStatus != CSym6502::Status::FINISH);

    if (running)
    {
        ASSERT(false);
        return;
    }

    set_translation_tables();

    stop_prog = false;
    running = true;
    PerformCommand();
    running = false;
}

//-----------------------------------------------------------------------------

void CSym6502::StepOver()
{
    ASSERT(CurrentStatus != CSym6502::Status::FINISH);

    if (running)
    {
        ASSERT(false);
        return;
    }

    stop_prog = false;
    running = true;
    CurrentStatus = CSym6502::Status::RUN;

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

void CSym6502::step_over()
{
    uint32_t addr = m_ctx.pc;
    uint16_t stack = 0;
    bool jsr = false;

    running = true;
    CurrentStatus = CSym6502::Status::RUN;

    if (cpu16())
        addr += (m_ctx.pbr << 16);

    set_translation_tables();

    switch (m_vCodeToCommand[m_ctx.getByte(addr)])
    {
    case CAsm::C_JSR:
    case CAsm::C_JSL:
        stack = m_ctx.s;
        jsr = true;
        [[fallthrough]];

    case CAsm::C_BRK:
        if (debug && !jsr)
            debug->SetTemporaryExecBreakpoint((addr + 2) & m_ctx.bus.maxAddress());

        for (;;)
        {
            perform_step();

            if (!CanContinue())
                return;

            if (jsr && m_ctx.s == stack)
            {
                CurrentStatus = CSym6502::Status::BPT_TEMP;
                return;
            }
        }
        break;

    default:
        PerformCommand();
        break;
    }
}

//-----------------------------------------------------------------------------

void CSym6502::RunTillRet()
{
    ASSERT(IsFinished());

    if (running)
    {
        ASSERT(false);
        return;
    }

    stop_prog = false;

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

void CSym6502::run_till_ret()
{
    set_translation_tables();

    uint16_t stack = m_ctx.s + 2;

    running = true;
    CurrentStatus = CSym6502::Status::RUN;

    for (;;)
    {
        perform_step();

        if (!CanContinue())
            return;

        if (m_ctx.s == stack)
        {
            CurrentStatus = CSym6502::Status::BPT_TEMP;
            return;
        }
    }
}

/*************************************************************************/

void CSym6502::Run()
{
    ASSERT(!IsFinished());

    if (running)
    {
        ASSERT(false);
        return;
    }

    ResetPointer(); // TODO: Move to UI/Thread?
    // More to the point, let the thread signal the start of the run, and let the UI update on that signal.

    try
    {
        m_thread = std::jthread([this] (std::stop_token stopToken) { RunThread(stopToken); });
    }
    catch (...)
    {
        running = false;
        throw; // Let UI figure out how to report this error.
    }
}

/*************************************************************************/

void CSym6502::RunThread(std::stop_token stopToken)
{
    // Ensure that the status gets updated even if thread crashes.
    deferred_action([this] ()
    {
        running = false; // TODO: Make atomic?
        CurrentStatus = CSym6502::Status::FINISH;
    });

    stop_prog = false; // TODO: Make atomic?
    running = true; // TODO: Make atomic?
    CurrentStatus = CSym6502::Status::RUN;

    run(stopToken);

    running = false;
}

/*************************************************************************/

void CSym6502::perform_step()
{
    if (stop_prog) // stop executing?
    {
        CurrentStatus = CSym6502::Status::STOP;
        return;
    }

    if (m_nInterruptTrigger != NONE) // interrupt requested?
        interrupt(m_nInterruptTrigger);

    PerformCommand();

    if (!CanContinue())
        return;

    uint32_t addr = m_ctx.pc;

    if (cpu16())
        addr += (m_ctx.pbr << 16);

    if (debug)
    {
        CAsm::Breakpoint bp = debug->GetBreakpoint(addr);

        if (bp != CAsm::BPT_NONE)
        {
            if (bp & CAsm::BPT_EXECUTE)
            {
                CurrentStatus = CSym6502::Status::BPT_EXECUTE;
                return;
            }

            if (bp & CAsm::BPT_TEMP_EXEC)
            {
                CurrentStatus = CSym6502::Status::BPT_TEMP;
                return;
            }
        }
    }

    CurrentStatus = CSym6502::Status::RUN;
}

/*************************************************************************/

void CSym6502::run(std::stop_token stopToken)
{
    set_translation_tables();

    for (;;)
    {
        if (stopToken.stop_requested())
        {
            CurrentStatus = CSym6502::Status::STOP;
            return;
        }

        perform_step();

        if (!CanContinue())
            return;
    }
}

/*************************************************************************/

void CSym6502::interrupt(int &nInterrupt) // interrupt requested: load pc ***
{
    ASSERT(running);

    if (nInterrupt & RST)
    {
        m_ctx.interrupt = false;

        if (Processor() != ProcessorType::M6502)
            m_ctx.decimal = false; //% 65C02 clears this bit

        m_ctx.pc = get_rst_addr();
        nInterrupt = NONE;
        m_ctx.uCycles += 7;
    }
    else if (nInterrupt & NMI)
    {
        if (waiFlag)
        {
            inc_prog_counter();
            waiFlag = false;
        }

        if (cpu16() && !m_ctx.emm)
        {
            push_on_stack(m_ctx.pbr);
            push_addr_on_stack(m_ctx.pc);
            m_ctx.pc = get_nmi_addr16();
        }
        else
        {
            push_addr_on_stack(m_ctx.pc);
            m_ctx.pc = get_nmi_addr();
        }

        m_ctx.break_bit = false;
        push_on_stack(m_ctx.get_status_reg());

        if (Processor() != ProcessorType::M6502)
            m_ctx.decimal = false; // 65C02 clears this bit

        m_ctx.break_bit = true;
        nInterrupt &= ~NMI;
        m_ctx.uCycles += 7;
    }
    else if (nInterrupt & IRQ)
    {
        nInterrupt &= ~IRQ;

        if (waiFlag)
        {
            inc_prog_counter();
            waiFlag = false;
        }

        if (m_ctx.interrupt)
            return;

        if (cpu16() && !m_ctx.emm)
        {
            push_on_stack(m_ctx.pbr);
            push_addr_on_stack(m_ctx.pc);
            m_ctx.pc = get_irq_addr16();
        }
        else
        {
            push_addr_on_stack(m_ctx.pc);
            m_ctx.pc = get_irq_addr();
        }

        m_ctx.break_bit = false;
        push_on_stack(m_ctx.get_status_reg());
        m_ctx.break_bit = true;
        m_ctx.interrupt = true;

        if (Processor() != ProcessorType::M6502)
            m_ctx.decimal = false; // 65C02 clears this bit

        m_ctx.uCycles += 7;
    }
    else
    {
        ASSERT(false);
    }
}

/*************************************************************************/

void CSym6502::SkipToAddr(uint16_t addr)
{
    ASSERT(!IsFinished());

    if (addr == sim::INVALID_ADDRESS)
        return;

    if (running)
        return;

    m_ctx.pc = addr;
    CurrentStatus = CSym6502::Status::OK;
}

/*************************************************************************/

void CSym6502::SkipInstr()
{
    ASSERT(!IsFinished());

    if (running)
        return;

    skip_cmd();
}

/*************************************************************************/

void CSym6502::AbortProg()
{
    if (!running)
        return; // Not running return

    stop_prog = true;

    if (m_thread.joinable())
    {
        m_thread.request_stop();

        // TODO: Replace with signal to UI that thread has stopped.
        m_thread.join();
    }

    ASSERT(!running);
}

/*************************************************************************/

void CSym6502::ExitSym()
{
    ASSERT(running == false);
    ResetPointer();
}

/*************************************************************************/

std::string CSym6502::GetStatMsg(CSym6502::Status stat) const
{
    wxString msg = _("Unknown Status");

    switch (stat)
    {
    case CSym6502::Status::OK:
    case CSym6502::Status::BPT_TEMP:
        msg = _("OK");
        break;

    case CSym6502::Status::BPT_EXECUTE:
        msg = _("Execution stopped on breakpoint");
        break;

    case CSym6502::Status::BPT_READ:
        msg = _("Execution stopped on breakpoint (caused by reading)");
        break;

    case CSym6502::Status::BPT_WRITE:
        msg = _("Execution stopped on breakpoint (caused by writing)");
        break;

    case CSym6502::Status::BPT_ILLEGAL_CODE:
        msg = _("Illegal byte code encountered");
        break;

    case CSym6502::Status::STOP:
        msg = _("Program execution stopped");
        break;

    case CSym6502::Status::FINISH:
        msg = _("Program finished");
        break;

    case CSym6502::Status::RUN:
        msg = _("Program is running...");
        break;

    case CSym6502::Status::INP_WAIT:
        msg = _("Program is waiting for input data...");
        break;

    case CSym6502::Status::ILL_WRITE:
        msg = _("Protected area write attempt detected");
        break;

    default:
        ASSERT(false);
        break;
    }

    return msg.ToStdString();
}

/*************************************************************************/

void CSym6502::Restart()
{
    m_ctx.Reset();
    CurrentStatus = CSym6502::Status::OK;

    //m_log.Clear();

    m_saveCycles = 0;
    m_ctx.set_status_reg_bits(0);
}

/*************************************************************************/

void CSym6502::SetStart(sim_addr_t address)
{
    if (address == sim::INVALID_ADDRESS)
    {
        // Use reset vector for start
        uint32_t addr = getVectorAddress(Vector::RESET);
        address = get_word(addr);
    }

    m_ctx.pc = address;
    m_ctx.s = 0x01FF;
    wxGetApp().m_global.m_bSRef = m_ctx.s;
    m_saveCycles = 0;
    m_ctx.set_status_reg_bits(0);

    if (debug)
    {
        CDebugLine dl;
        debug->GetLine(dl, address);
        SetPointer(dl.line, address);
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

/*************************************************************************/

void CSym6502::ClearCyclesCounter()
{
    ASSERT(running == false);
    m_ctx.uCycles = 0;
}

void CSym6502::AddBranchCycles(uint8_t arg)
{
    m_ctx.uCycles++; // Branch taken

    if (!cpu16() || (cpu16() && m_ctx.emm))
    {
        if (arg & 0x80) // jump back
        {
            if (m_ctx.pc >> 8 != static_cast<uint32_t>((m_ctx.pc - (0x100 - arg)) >> 8))
                m_ctx.uCycles++; // changing memory page -> additional cycle
        }
        else // jump forward
        {
            if (m_ctx.pc >> 8 != static_cast<uint32_t>((m_ctx.pc + arg) >> 8))
                m_ctx.uCycles++; // changing memory page -> additional cycle
        }
    }
}

/*************************************************************************/

void CSym6502::Interrupt(IntType eInt)
{
    m_nInterruptTrigger |= eInt;
}

/*************************************************************************/

void CSym6502::init()
{
    running = false;
    m_fuidLastView = 0;
    finish = CAsm::FIN_BY_BRK;
    CurrentStatus = CSym6502::Status::OK;
    //hThread = 0;
    m_nInterruptTrigger = NONE;
    m_vCodeToCommand = 0;
    m_vCodeToCycles = 0;
    m_vCodeToMode = 0;
    m_ctx.set_status_reg_bits(0);
}

void CSym6502::set_translation_tables()
{
    m_vCodeToCommand = CAsm::CodeToCommand();
    m_vCodeToCycles = CAsm::CodeToCycles();
    m_vCodeToMode = CAsm::CodeToMode();
}

/*************************************************************************/
