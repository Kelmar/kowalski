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
#include "6502.h"

/*************************************************************************/

using namespace sim::dev;

/*************************************************************************/

SimpleIO::SimpleIO(CIOWindow *ioWindow)
    : m_ioWindow(ioWindow)
{
    ASSERT(m_ioWindow);
}

SimpleIO::~SimpleIO()
{
}

/*************************************************************************/

void SimpleIO::Reset()
{
}

/*************************************************************************/

uint8_t SimpleIO::Peek(sim_addr_t address) const
{
    IOFunc func = static_cast<IOFunc>(address);

    switch (func)
    {
    case IOFunc::TERMINAL_IN:
        return m_ioWindow->PeekInput();

    case IOFunc::TERMINAL_GET_X_POS:
        return m_ioWindow->GetCursorPosition().x;

    case IOFunc::TERMINAL_GET_Y_POS:
        return m_ioWindow->GetCursorPosition().y;

    default:
        return 0;
    }
}

/*************************************************************************/

uint8_t SimpleIO::GetByte(sim_addr_t address)
{
    IOFunc func = static_cast<IOFunc>(address);

    switch (func)
    {
    case IOFunc::TERMINAL_IN:
        return m_ioWindow->Input();

    case IOFunc::TERMINAL_GET_X_POS:
        return m_ioWindow->GetCursorPosition().x;

    case IOFunc::TERMINAL_GET_Y_POS:
        return m_ioWindow->GetCursorPosition().y;

    default:
        return 0;
    }
}

/*************************************************************************/

void SimpleIO::SetByte(sim_addr_t address, uint8_t value)
{
    IOFunc func = static_cast<IOFunc>(address);

    switch (func)
    {
    case IOFunc::TERMINAL_OUT:
        m_ioWindow->RawChar(value);
        break;

    case IOFunc::TERMINAL_OUT_CHR:
        m_ioWindow->PutChar(value);
        break;

    case IOFunc::TERMINAL_OUT_HEX:
    {
        std::string val = fmt::format("%02X", value);
        m_ioWindow->PutChar(val[0]);
        m_ioWindow->PutChar(val[1]);
    }
    break;

    case IOFunc::CLS:
        m_ioWindow->Cls();
        break;

    case IOFunc::TERMINAL_SET_X_POS:
    {
        wxPoint loc = m_ioWindow->GetCursorPosition();
        loc.x = value;
        m_ioWindow->SetCursorPosition(loc);
    }
    break;

    case IOFunc::TERMINAL_SET_Y_POS:
    {
        wxPoint loc = m_ioWindow->GetCursorPosition();
        loc.y = value;
        m_ioWindow->SetCursorPosition(loc);
    }
    break;

    default:
        break;
    }
}

/*************************************************************************/

