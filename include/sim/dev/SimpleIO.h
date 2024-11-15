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

#ifndef SIMPLE_IO_6502_H__
#define SIMPLE_IO_6502_H__

/*************************************************************************/

// Forward decl for UI class
class CIOWindow;

/*************************************************************************/

namespace sim::dev
{
    /**
     * @brief Device interface to CIOWindow
     */
    class SimpleIO : public sim::Device
    {
    private:
        enum class IOFunc
        {
            CLS                 = 0,

            TERMINAL_OUT        = 1,
            TERMINAL_OUT_CHR    = 2,
            TERMINAL_OUT_HEX    = 3,

            TERMINAL_IN         = 4,
            TERMINAL_GET_X_POS  = 5,
            TERMINAL_GET_Y_POS  = 6,
            TERMINAL_SET_X_POS  = 7,
            TERMINAL_SET_Y_POS  = 8,

            LAST_FUNC
        };

    private:
        CIOWindow *m_ioWindow;

    public:
        /* constructor */ SimpleIO(CIOWindow *ioWindow);
        virtual          ~SimpleIO();

        virtual size_t AddressSize() const
        {
            size_t s = (size_t)(IOFunc::LAST_FUNC);
            return s - 1;
        }

        virtual void Reset();

        virtual uint8_t Peek(sim_addr_t address) const;

        virtual uint8_t GetByte(sim_addr_t address);

        virtual void SetByte(sim_addr_t address, uint8_t value);
    };
}

/*************************************************************************/

#endif /* SIMPLE_IO_6502_H__ */

/*************************************************************************/
