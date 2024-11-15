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

#ifndef SIM_DEV_RAM_6502_H__
#define SIM_DEV_RAM_6502_H__

/*************************************************************************/

class COutputMem; // Forward decl for output memory

namespace sim::dev
{
    class RAM : public sim::Device
    {
    private:
        COutputMem *m_memory;
        sim_addr_t m_start;
        size_t m_length;

    public:
        /* constructor */ RAM(COutputMem *memory, sim_addr_t start, size_t length);
        virtual          ~RAM();

        virtual size_t AddressSize() const { return m_length; }

        virtual void Reset() { }

        virtual uint8_t Peek(sim_addr_t address) const;

        virtual uint8_t GetByte(sim_addr_t address) { return Peek(address); }

        virtual void SetByte(sim_addr_t address, uint8_t value);
    };
}

/*************************************************************************/

#endif /* SIM_DEV_RAM_6502_H__ */

/*************************************************************************/
