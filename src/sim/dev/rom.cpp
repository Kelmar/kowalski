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

using namespace sim::dev;

/*=======================================================================*/

ROM::ROM(COutputMem *memory, sim_addr_t start, size_t length)
    : m_memory(memory)
    , m_start(start)
    , m_length(length)
{
}

ROM::~ROM()
{
}

/*=======================================================================*/

uint8_t ROM::Peek(sim_addr_t address) const
{
    sim_addr_t a = address + m_start;

    if (a > m_memory->size())
        return 0;

    return m_memory->get(a);
}

/*=======================================================================*/

void ROM::SetByte(sim_addr_t address, uint8_t value)
{
    UNUSED(address);
    UNUSED(value);

    // TODO: Rework to not use exceptions
    throw CSym6502::Status::ILL_WRITE;
}

/*=======================================================================*/
