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

#ifndef SIM_CONTEXT_6502_H__
#define SIM_CONTEXT_6502_H__

/*=======================================================================*/

#include "Asm.h"

// TODO: Remove this naming conflict
#undef OVERFLOW

/*=======================================================================*/

struct SimulatorConfig
{
    ProcessorType Processor;     // Type of processor selected
    CAsm::Finish  SimFinish;     // How the simulator should terminate
    bool          IOEnable;      // Set if the I/O functions should be enabled
    sim_addr_t    IOAddress;     // I/O Address location
    bool          ProtectMemory; // Set if a section of memroy should be write protected
    sim_addr_t    ProtectStart;  // Start of write protected memory area
    sim_addr_t    ProtectEnd;    // End of write protected memory area
};

/*=======================================================================*/

struct ContextBase
{
    uint8_t a, b;
    uint16_t x, y;

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

    ContextBase()
    {
        memset(this, 0, sizeof(ContextBase));
        emm = true;
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

/*=======================================================================*/

class CContext : public ContextBase
{
private:
    // Disable copy & move.
    CContext(const CContext &) = delete;

    CContext &operator =(const CContext &) = delete;

    SimulatorConfig m_config;

    /**
     * @brief Program Counter
     */
    uint16_t m_pc;

    /**
     * @brief Stack Pointer
     * 
     * Holds the raw value of the stack pointer register.
     * This means for Non 65816 or in emm mode, this value will be from $00 to $FF
     */
    uint16_t m_s;

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

    CContext(const SimulatorConfig &config);
    CContext(CContext &&rhs);

    /**
     * @brief Resets the context.
     * @param isSignal If set, signal is sent to all devices on the BUS as well.
     */
    void Reset(bool isSignal);

    const SimulatorConfig &Config() const { return m_config; }

    ProcessorType Processor() const { return m_config.Processor; }


public: // Register access
    /**
     * @brief Get the value of the actual program counter register.
     */
    uint16_t PC() const 
    { 
        return m_pc;
    }

    /**
     * @brief Set the address of the actual program counter register.
     */
    void PC(uint16_t addr)
    {
        m_pc = addr;
    }

    /**
     * @brief Read the value of the C register
     * 
     * This behavior changes depending on the emm and mem16 status flags.
     * 
     * @return The value of the C register (or A register if not doing 16 bit opp)
     */
    uint16_t CRegister() const
    {
        uint16_t rval = (a & 0xFF);

        if (Processor() == ProcessorType::WDC65C02 && !emm && mem16)
            rval += (b & 0xFF) << 8;

        return rval;
    }

    /**
     * @brief Set the value of the C register.
     * 
     * This behavior changes depending on the emm and mem16 status flags.
     * 
     * @param value The value to set for the C register.
     * @param setStatus If set, the status register will be adjusted based on the values setin the C register.
     * 
     * @return The actual value that is set.
     * For 8-bit mode, this will have the MSB cleared.
     */
    uint16_t CRegister(uint16_t value, bool setStatus )
    {
        a = (value & 0xFF);

        if (Processor() == ProcessorType::WDC65816 && !emm && mem16)
            b = (value >> 8) & 0xFF;
        else
            value &= 0x00FF; // Mask off result

        if (setStatus)
        {
            zero = value == 0;
            negative = (value & 0x8000) != 0;
        }

        return value;
    }

    /**
     * @brief Get value of S register.
     * 
     * This gets the raw value of the S register, it does not attempt
     * to adjust it based on the emumulation mode set.
     */
    uint16_t SRegister() const { return m_s; }

    /**
     * @brief Set value of S register
     * 
     * This sets the raw value of the S register, it does not attempt
     * to adjust it based on the emumulation mode set.
     */
    void SRegister(uint16_t value);

    /**
     * @brief Get the stack pointer value based on the current processor type.
     * 
     * This gets the adjusted value of the S register based on the emmulation mode status flag.
     */
    sim_addr_t StackPointer() const
    {
        if (Processor() == ProcessorType::WDC65816)
            return m_s;

        return (m_s & 0xFF) | 0x0100;
    }

    /**
     * @brief Get the program counter based on the current processor type.
     * @remarks For the 65816 this addjusts based on the current program bank register.
     */
    inline
    sim_addr_t GetProgramAddress(ssize_t offset = 0) const
    {
        sim_addr_t rval = PC() + offset;

        if (Processor() == ProcessorType::WDC65816)
            rval += (pbr << 16);

        return rval;
    }

    /**
     * @brief Returns the value of the processor status register.
     */
    uint8_t GetStatus() const;

    /**
     * @brief Sets the value of the processor status register.
     */
    void SetStatus(uint8_t value);

public: // Memory Access
    /**
     * @brief Peeks at the byte located at the current program counter.
     * @param offset Optional offset in bytes from program counter to read from.
     * @remarks This accounts for the program bank for 65816
     */
    uint8_t PeekProgramByte(ssize_t offset = 0) const;

    /**
     * @brief Reads the byte located at the current program counter
     * @param offset Optional offset in bytes from program counter to read from.
     * @remarks This accoutns for the program bank for 65816
     */
    uint8_t GetProgramByte(ssize_t offset = 0);

    /**
     * @brief Reads the word located at the current program counter
     * @param offset Optional offset in bytes from program counter to read from.
     * @remarks This accoutns for the program bank for 65816
     */
    uint16_t GetProgramWord(ssize_t offset = 0);

    /**
     * @brief Reads the LWord located at the current program counter
     * @param offset Optional offset in bytes from program counter to read from.
     * @remarks This accoutns for the program bank for 65816
     */
    uint32_t GetProgramLWord(ssize_t offset = 0);

    /**
     * @brief Reads the byte located at the current program counter and increments the counter by one
     */
    uint8_t ReadProgramByte();

    /**
     * @brief Reads the word located at the current program counter and increments the counter by two
     */
    uint16_t ReadProgramWord();

    /**
     * @brief Reads the long word located at the current program counter and increments the counter by three
     */
    uint32_t ReadProgramLWord();

    /**
     * @brief Push a byte into the virtual machine's stack.
     * @remarks This function updates the stack pointer.
     * @param byte The value to push.
     */
    void PushByte(uint8_t byte);

    /**
     * @brief Push a word into the virtual machine's stack.
     * @remarks This function updates the stack pointer.
     * @param word The value to push.
     */
    void PushWord(uint16_t word);

    /**
     * @brief Pull a byte from the virtual machine's stack.
     * @remarks This function updates the stack pointer.
     * @return The byte read from the stack.
     */
    uint8_t PullByte();

    /**
     * @brief Pull a word from the virtual machine's stack.
     * @remarks This function updates the stack pointer.
     * @return The 16-bit value read from the stack.
     */
    uint16_t PullWord();

    /**
     * @brief Peek at the byte at the given address on the bus.
     * @remarks This operation is non distructive.
     * @param address The address to read.
     * @return The value read.
     */
    inline
    uint8_t PeekByte(sim_addr_t address) const
    {
        return bus.PeekByte(address);
    }

    /**
     * @brief Peek at a word at the given address on the bus.
     * @remarks This operation is non distructive.
     * @param address The address to read
     * @return The 16-bit word to read.
     */
    uint16_t PeekWord(sim_addr_t address) const;

    /**
     * @brief Peek at an LWord (24-bit value) at the given address on the bus.
     * @remarks This operation is non distructive.
     * @param address The address to read from.
     * @return The 24-bit value read.
     */
    uint32_t PeekLWord(sim_addr_t address) const;

    /**
     * @brief Gets a byte from the bus.
     * @param addr The address to get
     */
    inline
    uint8_t GetByte(sim_addr_t address)
    {
        return bus.GetByte(address);
    }

    /**
     * @brief Gets a 16-bit word from the bus.
     * @param addr The address to get
     * @remarks Words are always read little endian.
     */
    uint16_t GetWord(sim_addr_t addr);

    /**
     * @brief Gets a 24-bit word from the bus.
     * @param addr The address to get
     * @remarks Words are always read little endian.
     */
    uint32_t GetLWord(sim_addr_t addr);

    /**
     * @brief Sets a byte on the bus.
     * @param addr The address to set
     * @param value The value to set
     */
    inline
    void SetByte(sim_addr_t address, uint8_t value)
    {
        bus.SetByte(address, value);
    }

    /**
     * @brief Sets a 16-bit word on the bus.
     * @param addr The address to set
     * @param word The word value to write.
     * @remarks Words are always written little endian.
     */
    void SetWord(sim_addr_t addr, uint16_t word);

    /**
     * @brief Sets a 24-bit word on the bus
     * @param addr The address to set
     * @param word The 24-bit word to write.
     * @remarks Words are always written little endian.
     */
    void SetLWord(sim_addr_t addr, uint32_t word);

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

    
};

/*=======================================================================*/

#endif /* SIM_CONTEXT_6502_H__ */

/*=======================================================================*/
