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

#ifndef BUS_6502_H__
#define BUS_6502_H__

/*************************************************************************/

#include "sim/dev/base.h"

/*************************************************************************/
/**
 * @brief Simulated address/memory bus.
 */
class Bus
{
public:
    enum class MapError
    {
        /**
         * @brief No error
         */
        None = 0,

        /**
         * @brief Device would fall outside of the valid address range for this bus.
         */
        OutOfRange,

        /**
         * @brief Another device is already mapped to to that address space, or overlaps with another device somehow.
         */
        Overlap,

        /**
         * @brief This device instance is already mapped to this bus.
         */
        AlreadyMapped
    };

private:
    struct Node
    {
        sim::PDevice device;
        sim_addr_t start;
        sim_addr_t end;

        Node(const Node &n)
            : device(n.device)
            , start(n.start)
            , end(n.end)
        {
        }

        Node(Node &&n)
            : device(std::move(n.device))
            , start(std::move(n.start))
            , end(std::move(n.end))
        {
        }

        Node(sim::PDevice d, sim_addr_t s, sim_addr_t e)
            : device(d)
            , start(s)
            , end(e)
        {
        }

        virtual ~Node() { }

        inline void Reset() { device->Reset(); }

        inline 
        sim_addr_t makeLocal(sim_addr_t global) const
        {
            return global - start;
        }

        inline
        uint8_t Peek(sim_addr_t global) const
        {
            return device->Peek(makeLocal(global));
        }

        inline
        void SetByte(sim_addr_t global, uint8_t value)
        {
            device->SetByte(makeLocal(global), value);
        }

        inline
        uint8_t GetByte(sim_addr_t global)
        {
            return device->GetByte(makeLocal(global));
        }
    };

    typedef std::function<void(sim_addr_t, size_t, Node *)> RangeOp;

private:
    int m_width;
    sim_addr_t m_maxAddress;

    // TODO: Use radix search?
    std::vector<Node*> m_devices;

    /**
     * @brief Find a node that falls in the given range
     * @param start The start address to search for.
     * @param cnt The number of addresses to search within.
     * @return The node found if any, otherwise NULL.
     */
    Node *RangeFind(sim_addr_t start, int cnt = 1) const;

    /**
     * @brief Performs an operation over a given range
     * @param start 
     * @param cnt 
     */
    void RunRange(sim_addr_t start, size_t sz, RangeOp operation) const;

public:
    /**
     * @brief Initialize a new bus.
     * @param width The width of the address bus. (in number of lines)
     */
    Bus(int width);
    Bus(Bus &&rhs);

    virtual ~Bus();

    /**
     * @brief Get the width of the address bus in number of lines.
     */
    int width() const { return m_width; }

    /**
     * @brief Returns the maximum valid address for this bus.
     */
    sim_addr_t maxAddress() const { return m_maxAddress; }

    /**
     * @brief Checks to see if the supplied device is already mapped to the bus.
     * @param device The device to check for.
     * @return true if the device was found, false if not.
     */
    bool Contains(sim::PDevice device) const;

    /**
     * @brief Checks if the supplied address is in a valid range for this bus.
     * @param address The address to check
     * @return true if the address is valid, false if not.
     * @remark This function does not check to see if there is a device
     * that will respond to the given address, just that the address falls
     * within the valid range of addresses that this bus can handle.
     */
    bool InRange(sim_addr_t address) const
    { 
        return address <= m_maxAddress;
    }

    /**
     * @brief Checks if there is a device at the otherwise valid address.
     * @param address The address to check
     * @return true if there is a valid device at the supplied address, false
     * if not or the address is invalid.
     * @remark Unlike InRange(), this function checks to see if anything
     * is listening at a valid address.
     */
    bool IsMapped(sim_addr_t address) const
    {
        Node *node = RangeFind(address);
        return (node != nullptr);
    }

    /**
     * @brief Add a new device instance to the bus.
     * @param device The device to add
     * @param address The starting address of the device.
     * @return true if the device was added, false if the device falls outside
     * of a valid address range for this bus at the supplied address.
     */
    MapError AddDevice(sim::PDevice device, sim_addr_t address);

    /**
     * @brief Send reset to all devices on the bus.
     */
    void Reset();

    /**
     * @brief Peeks at a byte on the address bus.
     * @param address The global address to read from.
     * @return The byte at that address.
     */
    uint8_t PeekByte(sim_addr_t address) const;

    /**
     * @brief Read a range of values from the starting address without modifying device state.
     * @param start The address to start from
     * @param values The target to write to.
     */
    void PeekRange(sim_addr_t start, _Out_ std::span<uint8_t> &values) const;

    /**
     * @brief Read a single byte from the address bus.
     * @param address The global address to read from.
     * @return The byte at that address.
     * @remarks
     * Unlike PeekByte(), this operation is allowed to adjust a device's state.
     */
    uint8_t GetByte(sim_addr_t address);

    /**
     * @brief Read a range of values from the starting address.
     * @param start The address to start from
     * @param values The target to write to.
     */
    void GetRange(sim_addr_t start, _Out_ std::span<uint8_t> &values);

    /**
     * @brief Write a byte to the global address.
     * @param address The global address to write to.
     * @param value The value to write.
     */
    void SetByte(sim_addr_t address, uint8_t value);

    /**
     * @brief Sets a range of values.
     * @param start The starting address to set
     * @param values The values to set.
     * @remark Any values not properly mapped will not get written.
     */
    void SetRange(sim_addr_t start, _In_ const std::span<uint8_t> &values);
};

/*************************************************************************/

#endif /* BUS_6502_H__ */

/*************************************************************************/
