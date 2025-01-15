/*************************************************************************/
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
/*************************************************************************/

#include "StdAfx.h"
#include "sim.h"

#include "6502.h"

/*************************************************************************/

CContext::CContext(const SimulatorConfig &config)
    : m_config(config)
    , bus(config.Processor == ProcessorType::WDC65816 ? 24 : 16)
{
    Reset(false);
}

/*************************************************************************/

void CContext::SRegister(uint16_t value)
{
    uint16_t mask = 0x00FF;

    if (Processor() == ProcessorType::WDC65816 && !emm)
        mask = 0xFFFF;

    m_s = value & mask;

    wxGetApp().m_global.m_bSRef = StackPointer();
}

/*************************************************************************/

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

/*************************************************************************/

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

/*************************************************************************/

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

/*************************************************************************/

uint8_t CContext::PeekProgramByte(ssize_t offset /* = 0 */) const
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return PeekByte(programAddr);
}

/*************************************************************************/

uint8_t CContext::GetProgramByte(ssize_t offset /* = 0 */)
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return getByte(programAddr);
}

/*************************************************************************/

uint16_t CContext::GetProgramWord(ssize_t offset /* = 0 */)
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return getWord(programAddr);
}

/*************************************************************************/

uint32_t CContext::GetProgramLWord(ssize_t offset /* = 0 */)
{
    sim_addr_t programAddr = GetProgramAddress(offset);
    return getLWord(programAddr);
}

/*************************************************************************/

uint8_t CContext::ReadProgramByte()
{
    uint8_t rval = getByte(GetProgramAddress());
    PC(PC() + 1);
    return rval;
}

/*************************************************************************/

uint16_t CContext::ReadProgramWord()
{
    uint16_t rval = getWord(GetProgramAddress());
    PC(PC() + 2);
    return rval;
}

/*************************************************************************/

uint32_t CContext::ReadProgramLWord()
{
    uint16_t rval = getWord(GetProgramAddress());
    PC(PC() + 3);
    return rval;
}

/*************************************************************************/

void CContext::PushByte(uint8_t byte)
{
    sim_addr_t addr = StackPointer();
    setByte(addr, byte);
    SRegister(SRegister() - 1);
}

/*************************************************************************/

void CContext::PushWord(uint16_t word)
{
    sim_addr_t addr = StackPointer();
    setWord(addr, word);
    SRegister(SRegister() - 2);
}

/*************************************************************************/

uint8_t CContext::PullByte()
{
    sim_addr_t addr = StackPointer();
    uint8_t rval = getByte(addr);
    SRegister(SRegister() + 1);
    return rval;
}

/*************************************************************************/

uint16_t CContext::PullWord()
{
    sim_addr_t addr = StackPointer();
    uint16_t rval = getWord(addr);
    SRegister(SRegister() + 2);
    return rval;
}

/*************************************************************************/

