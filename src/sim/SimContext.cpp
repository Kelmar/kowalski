/*=======================================================================*/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*=======================================================================*/

#include "StdAfx.h"
#include "sim.h"

#include "6502.h"

/*=======================================================================*/

CContext::CContext(const SimulatorConfig &config)
    : m_config(config)
    , bus(config.Processor == ProcessorType::WDC65816 ? 24 : 16)
{
    Reset(false);
}

CContext::CContext(CContext &&rhs)
    : ContextBase(rhs) // Use copy for now
    , m_config(std::move(rhs.m_config))
    , m_pc(std::move(rhs.m_pc))
    , m_s(std::move(rhs.m_s))
    , bus(std::move(rhs.bus))
{
}

/*=======================================================================*/

void CContext::SRegister(uint16_t value)
{
    uint16_t mask = 0x00FF;

    if (Processor() == ProcessorType::WDC65816 && !emm)
        mask = 0xFFFF;

    m_s = value & mask;

    wxGetApp().m_global.m_bSRef = StackPointer();
}

/*=======================================================================*/

uint8_t CContext::GetStatus() const
{
    if (Processor() == ProcessorType::WDC65816)
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

/*=======================================================================*/

void CContext::SetStatus(uint8_t reg)
{
    negative = !!(reg & NEGATIVE);
    overflow = !!(reg & OVERFLOW);
    zero = !!(reg & ZERO);
    carry = !!(reg & CARRY);
    decimal = !!(reg & DECIMAL);
    interrupt = !!(reg & INTERRUPT);

    if (Processor() == ProcessorType::WDC65816)
    {
        mem16 = !!(reg & MEMORY);
        xy16 = !!(reg & INDEX);
    }
    else
    {
        // Reserved bit should always be set for non 65816

        reserved = true; //% Bug fix 1.2.12.3 BRK bit trouble
        break_bit = true; //% Bug fix 1.2.12.3 BRK bit trouble
    }
}

/*=======================================================================*/

void CContext::Reset(bool isSignal)
{
    b = a = x = y = 0;

    emm = true;
    mem16 = false;
    xy16 = false;

    negative = overflow = zero = carry = false;
    break_bit = decimal = interrupt = false;
    reserved = true;
    uCycles = 0;

    if (isSignal)
        bus.Reset();

    m_pc = 0;
    m_s = 0;
}

/*=======================================================================*/

uint8_t CContext::PeekProgramByte(ssize_t offset /* = 0 */) const
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return PeekByte(programAddr);
}

/*=======================================================================*/

uint8_t CContext::GetProgramByte(ssize_t offset /* = 0 */)
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return GetByte(programAddr);
}

/*=======================================================================*/

uint16_t CContext::GetProgramWord(ssize_t offset /* = 0 */)
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return GetWord(programAddr);
}

/*=======================================================================*/

uint32_t CContext::GetProgramLWord(ssize_t offset /* = 0 */)
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return GetLWord(programAddr);
}

/*=======================================================================*/

uint8_t CContext::ReadProgramByte()
{
    uint8_t rval = GetByte(GetProgramAddress());
    PC(PC() + 1);
    return rval;
}

/*=======================================================================*/

uint16_t CContext::ReadProgramWord()
{
    uint16_t rval = GetWord(GetProgramAddress());
    PC(PC() + 2);
    return rval;
}

/*=======================================================================*/

uint32_t CContext::ReadProgramLWord()
{
    uint32_t rval = GetLWord(GetProgramAddress());
    PC(PC() + 3);
    return rval;
}

/*=======================================================================*/

void CContext::PushByte(uint8_t byte)
{
    sim_addr_t addr = StackPointer();
    SetByte(addr, byte);
    SRegister(SRegister() - 1);
}

/*=======================================================================*/

void CContext::PushWord(uint16_t word)
{
    // Word's are pushed in reverse order because stack grows downwards.
    PushByte(word & 0x00FF);
    PushByte((word & 0xFF00) >> 8);
}

/*=======================================================================*/

uint8_t CContext::PullByte()
{
    // Adjust stack pointer before doing pull.
    SRegister(SRegister() + 1);

    sim_addr_t addr = StackPointer();
    uint8_t rval = GetByte(addr);
    return rval;
}

/*=======================================================================*/

uint16_t CContext::PullWord()
{
    // Word's are pulled in reverse order because stack grows downwards.
    return PullByte() << 8 | PullByte();
}

/*=======================================================================*/

uint16_t CContext::PeekWord(sim_addr_t address) const
{
    std::array<uint8_t, 2> bytes;
    std::span<uint8_t> spn(bytes);

    bus.PeekRange(address, _Out_ spn);

    return bytes[1] << 8 | bytes[0];
}

/*=======================================================================*/

uint32_t CContext::PeekLWord(sim_addr_t address) const
{
    std::array<uint8_t, 3> bytes;
    std::span<uint8_t> spn(bytes);

    bus.PeekRange(address, _Out_ spn);

    return bytes[2] << 16 | bytes[1] << 8 | bytes[0];
}

/*=======================================================================*/

uint16_t CContext::GetWord(sim_addr_t addr)
{
    std::array<uint8_t, 2> bytes;
    std::span<uint8_t> spn(bytes);

    bus.GetRange(addr, _Out_ spn);

    return bytes[1] << 8 | bytes[0];
}

/*=======================================================================*/

uint32_t CContext::GetLWord(sim_addr_t addr)
{
    std::array<uint8_t, 3> bytes;
    std::span<uint8_t> spn(bytes);

    bus.GetRange(addr, _Out_ spn);

    return bytes[2] << 16 | bytes[1] << 8 | bytes[0];
}

/*=======================================================================*/

void CContext::SetWord(sim_addr_t addr, uint16_t word)
{
    std::array<uint8_t, 2> bytes =
    {
        (uint8_t)(word & 0x00FF),
        (uint8_t)((word >> 8) & 0x00FF)
    };

    bus.SetRange(addr, _In_ bytes);
}

/*=======================================================================*/

void CContext::SetLWord(sim_addr_t addr, uint32_t word)
{
    std::array<uint8_t, 3> bytes =
    {
        (uint8_t)(word & 0x0000'00FF),
        (uint8_t)((word >> 8) & 0x0000'00FF),
        (uint8_t)((word >> 16) & 0x0000'00FF)
    };

    bus.SetRange(addr, _In_ bytes);
}

/*=======================================================================*/


