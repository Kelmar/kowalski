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

#ifndef SIM_DEVICE_BASE_H__
#define SIM_DEVICE_BASE_H__

/*************************************************************************/

namespace sim
{
    /**
     * @brief Abstraction for a device on simulated bus.
     *
     * @remarks
     * Addresses presented to the device are mapped to that device's local
     * address space.  For example, if a UART is mapped to the bus locations $8000-$800F
     * the device will receive $0-$F as the requested addresses; the bus handles
     * mapping from the global address space.
     */
    class Device
    {
    private:
        // Remove copy/move construction

        /* constructor */ Device(const Device &) = delete;
        /* constructor */ Device(Device &&) = delete;

        const Device &operator =(const Device &) = delete;
        const Device &operator =(Device &&) = delete;

    protected:
        /* constructor */ Device() { }

    public:
        virtual ~Device() { }

        /**
         * @brief Gets number of addresses this device needs.
         */
        virtual size_t AddressSize() const = 0;

        /**
         * @brief Called by the simulator to indicate that the device should reset itself its initial state.
         */
        virtual void Reset() = 0;

        /**
         * @brief Read a byte from the device at the given local address without affecting the device's state.
         * @param address The local address to read from.
         * @return The byte value at that address.
         * @remarks
         * This method is used by the UI and debugger to display the device's current state
         * without affecting it's state so as not to cause issues with the simulated program.
         */
        virtual uint8_t Peek(sim_addr_t address) const = 0;

        /**
         * @brief Read a byte from the device at the given local address.
         * @param address The device local address to read from.
         * @return The byte value read from that device.
         */
        virtual uint8_t GetByte(sim_addr_t address) = 0;

        /**
         * @brief Write a byte to the device at the given local address.
         * @param address The local address to write to.
         * @param value The value to be written.
         */
        virtual void SetByte(sim_addr_t address, uint8_t value) = 0;
    };

    typedef std::shared_ptr<Device> PDevice;
}

/*************************************************************************/

#endif /* SIM_DEVICE_BASE_H__ */

/*************************************************************************/
