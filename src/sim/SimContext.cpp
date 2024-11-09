/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
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

/*************************************************************************/

CContext::CContext(ProcessorType processor)
    : m_processor(processor)
    , bus(m_processor == ProcessorType::WDC65816 ? 24 : 16)
{
    reset();
}

/*************************************************************************/

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

    if (m_processor == ProcessorType::WDC65816)
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

void CContext::set_status_reg_bits(uint8_t reg)
{
    negative = !!(reg & NEGATIVE);
    overflow = !!(reg & OVERFLOW);
    zero = !!(reg & ZERO);
    carry = !!(reg & CARRY);
    decimal = !!(reg & DECIMAL);
    interrupt = !!(reg & INTERRUPT);

    if (m_processor == ProcessorType::WDC65816)
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
