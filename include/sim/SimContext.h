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

#ifndef SIM_CONTEXT_6502_H__
#define SIM_CONTEXT_6502_H__

/*************************************************************************/

#include "Asm.h"

// TODO: Remove this naming conflict
#undef OVERFLOW

/*************************************************************************/

struct ContextBase
{
    uint8_t b;
    uint16_t a, x, y, s;
    uint32_t pc;

    ULONG uCycles;
    bool intFlag;  //% bug Fix 1.2.13.18 - command log assembly not lined up with registers

    bool negative, overflow, zero, carry;
    bool reserved, break_bit, decimal, interrupt;

    bool emm;   // Emulation mode
    bool mem16; // 8 bit mode
    bool xy16;  // 8 bit mode

    uint8_t dbr; // Data Bank Register
    uint8_t pbr; // Program Bank Register
    uint16_t dir; // Direct Register

    // contains ones in place of valid address bits
    bool io;
    uint16_t io_from, io_to;

    ContextBase()
    {
        memset(this, 0, sizeof(ContextBase));
    }

    ContextBase(const ContextBase &rhs)
    {
        memcpy(this, &rhs, sizeof(ContextBase));
    }

    const ContextBase &operator =(const ContextBase &rhs)
    {
        memcpy(this, &rhs, sizeof(ContextBase));
        return *this;
    }
};

class CContext : public ContextBase
{
private:
    // Disable copy & move.
    CContext(const CContext &) = delete;
    CContext(CContext &&) = delete;

    CContext &operator =(const CContext &) = delete;
    CContext &operator =(CContext &&) = delete;

    ProcessorType m_processor;

public:
    enum Flags
    {
        NEGATIVE = 0x80,
        OVERFLOW = 0x40, // TODO: Remove this naming conflict
        MEMORY = 0x20,
        RESERVED = 0x20,
        INDEX = 0x10,
        BREAK = 0x10,
        DECIMAL = 0x08,
        INTERRUPT = 0x04,
        ZERO = 0x02,
        EMMULATION = 0x01,
        CARRY = 0x01,
        NONE = 0x00,
        ALL = 0xFF,

        // bit numbers
        N_NEGATIVE = 7,
        N_OVERFLOW = 6,
        N_MEMORY = 5,
        N_RESERVED = 5,
        N_INDEX = 4,
        N_BREAK = 4,
        N_DECIMAL = 3,
        N_INTERRUPT = 2,
        N_ZERO = 1,
        N_EMMULATION = 1,
        N_CARRY = 0
    };

public:
    Bus bus;

    CContext(ProcessorType processor);

    void Reset()
    {
        bus.Reset();
        reset();
    }

    ProcessorType getProcessorType() const { return m_processor; }

    /**
     * @brief Get the stack pointer value based on the current processor type.
     */
    sim_addr_t getStackPointer() const
    {
        if (m_processor == ProcessorType::WDC65816)
            return s;

        return (s & 0xFF) | 0x0100;
    }

    /**
     * @brief Get the program counter based on the current processor type.
     */
    sim_addr_t getProgramAddress() const
    {
        if (m_processor == ProcessorType::WDC65816)
            return pc | (pbr << 16);

        return pc;
    }

    uint8_t peekByte(sim_addr_t address) const
    {
        return bus.PeekByte(address);
    }

    uint16_t peekWord(sim_addr_t address) const
    {
        std::array<uint8_t, 2> bytes;
        std::span<uint8_t> spn(bytes);

        bus.PeekRange(address, _Out_ spn);

        return bytes[1] << 8 | bytes[0];
    }

    uint32_t peekLWord(sim_addr_t address) const
    {
        std::array<uint8_t, 3> bytes;
        std::span<uint8_t> spn(bytes);

        bus.PeekRange(address, _Out_ spn);

        return bytes[2] << 16 | bytes[1] << 8 | bytes[0];
    }

    /**
     * @brief Gets a byte from the bus.
     * @param addr The address to get
     */
    uint8_t getByte(sim_addr_t address)
    {
        return bus.GetByte(address);
    }

    /**
     * @brief Gets a 16-bit word from the bus.
     * @param addr The address to get
     * @remarks Words are always read little endian.
     */
    uint16_t getWord(sim_addr_t addr)
    {
        std::array<uint8_t, 2> bytes;
        std::span<uint8_t> spn(bytes);

        bus.GetRange(addr, _Out_ spn);

        return bytes[1] << 8 | bytes[0];
    }

    /**
     * @brief Gets a 24-bit word from the bus.
     * @param addr The address to get
     * @remarks Words are always read little endian.
     */
    uint32_t getLWord(sim_addr_t addr)
    { 
        std::array<uint8_t, 3> bytes;
        std::span<uint8_t> spn(bytes);

        bus.GetRange(addr, _Out_ spn);

        return bytes[2] << 16 | bytes[1] << 8 | bytes[0];
    }

    /**
     * @brief Sets a byte on the bus.
     * @param addr The address to set
     * @param value The value to set
     */
    void setByte(sim_addr_t address, uint8_t value)
    {
        bus.SetByte(address, value);
    }

    /**
     * @brief Sets a 16-bit word on the bus.
     * @param addr The address to set
     * @param word The word value to write.
     * @remarks Words are always written little endian.
     */
    void setWord(sim_addr_t addr, uint16_t word)
    {
        std::array<uint8_t, 2> bytes =
        {
            (uint8_t)(word & 0x00FF),
            (uint8_t)((word >> 8) & 0x00FF)
        };

        bus.SetRange(addr, _In_ bytes);
    }

    /**
     * @brief Sets a 24-bit word on the bus
     * @param addr The address to set
     * @param word The 24-bit word to write.
     * @remarks Words are always written little endian.
     */
    void setLWord(sim_addr_t addr, uint32_t word)
    {
        std::array<uint8_t, 3> bytes =
        {
            (uint8_t)(word & 0x0000'00FF),
            (uint8_t)((word >> 8) & 0x0000'00FF),
            (uint8_t)((word >> 16) & 0x0000'00FF)
        };

        bus.SetRange(addr, _In_ bytes);
    }

    /**
     * @brief Sets a 32-bit word to the bus
     * @param addr The address to set
     * @param word The 32-bit word to write.
     * @remarks Words are always written little endian.
     */
    void setDWord(sim_addr_t addr, uint32_t word)
    {
        std::array<uint8_t, 4> bytes =
        {
            (uint8_t)(word & 0x0000'00FF),
            (uint8_t)((word >> 8) & 0x0000'00FF),
            (uint8_t)((word >> 16) & 0x0000'00FF),
            (uint8_t)((word >> 24) & 0x0000'00FF)
        };

        bus.SetRange(addr, _In_ bytes);
    }

    // functions that change the contents of the flag register
    void set_status_reg_VZNC(bool v, bool z, bool n, bool c)
    {
        negative = !!n;
        overflow = !!v;
        zero = !!z;
        carry = !!c;
    }

    void set_status_reg_ZNC(bool z, bool n, bool c)
    {
        negative = !!n;
        zero = !!z;
        carry = !!c;
    }

    void set_status_reg_VZN(bool v, bool z, bool n)
    {
        negative = !!n;
        overflow = !!v;
        zero = !!z;
    }

    void set_status_reg_ZN(bool z, bool n)
    {
        negative = !!n;
        zero = !!z;
    }

    void set_status_reg(uint8_t val)
    {
        zero = val == 0;
        negative = !!(val & 0x80);
    }

    void set_status_reg16(uint16_t val)
    {
        zero = val == 0;
        negative = !!(val & 0x8000);
    }

    uint8_t get_status_reg() const;

    void set_status_reg_bits(uint8_t reg);

private:
    void reset()
    {
        pc = 0;
        b = a = x = y = s = 0;
        io = false;
        negative = overflow = zero = carry = false;
        break_bit = decimal = interrupt = false;
        reserved = true;
        uCycles = 0;
    }
};


/*************************************************************************/

#endif /* SIM_CONTEXT_6502_H__ */

/*************************************************************************/
