/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

#ifndef OUTPUT_MEM_H__
#define OUTPUT_MEM_H__

/*************************************************************************/

#include <sigslot/signal.hpp>

/*************************************************************************/

class COutputMem
{
private:
    uint8_t *m_data;
    size_t m_size;

    /**
     * @brief Memory modified flag
     */
    bool m_dirty;

public:
    COutputMem()
        : m_data(nullptr)
        , m_size(0x1000000)
        , m_dirty(false)
    {
        m_data = new uint8_t[m_size];
        memset(m_data, 0, m_size);
    }

    virtual ~COutputMem()
    {
        delete[] m_data;
    }

    /**
     * @brief Notification signal for when memory has been modified.
     */
    sigslot::signal<> onUpdate;

    /**
     * @brief Size of the memory in bytes.
     */
    size_t size() const { return m_size; }

    /**
     * @brief Check if memory has been modified sense last call to validate()
     * @return True if the memory has been modified, false if not.
     */
    bool isDirty() const { return m_dirty; }

    /**
     * @brief Marks memory as being modified.
     */
    void invalidate()
    {
        m_dirty = true;
        onUpdate();
    }

    /**
     * @brief Clear's dirty flag
     */
    void validate() { m_dirty = false; }

    /**
     * @brief Resets memory to all zeroes.
     */
    void Clear()
    {
        memset(m_data, 0, m_size);
        invalidate();
    }

    // TODO: Deprecate this, don't want to be copying huge chunks of memory around.
    COutputMem &operator= (const COutputMem &src)
    {
        ASSERT(m_size == src.m_size); // Dimensions must be identical
        memcpy(m_data, src.m_data, m_size);
        invalidate();
        return *this;
    }

    const uint8_t *Mem() const { return m_data; }

    // Copy from data block
    void copy(size_t start, uint8_t *data, size_t sz)
    {
        ASSERT((start + sz) <= m_size);
        memcpy(m_data + start, data, sz);
        invalidate();
    }

    // Copy from a span
    void copy(size_t start, const std::span<uint8_t> &span)
    {
        copy(start, span.data(), span.size());
    }

    uint8_t get(size_t addr) const
    {
        ASSERT(addr < m_size);
        return m_data[addr];
    }

    uint8_t operator[] (size_t addr) const
    {
        return get(addr);
    }

    uint16_t getWord(size_t addr) const
    {
        ASSERT(addr < (m_size - 1));

        uint16_t lo = m_data[addr];
        uint16_t hi = m_data[addr + 1];

        return lo | (hi << 8);
    }

    uint32_t getLWord(size_t addr) const
    {
        ASSERT(addr < (m_size - 2));

        uint16_t lo = m_data[addr];
        uint16_t mid = m_data[addr + 1];
        uint16_t hi = m_data[addr + 2];

        return lo | (mid << 8) | (hi << 16);
    }

    std::span<uint8_t> getSpan(size_t from, size_t to)
    {
        ASSERT(from < m_size);
        to = std::min(to, m_size);
        return std::span(&m_data[from], to);
    }

    /**
     * @brief Sets a byte in memory
     * @param addr The address to set
     * @param value The value to set the memory to.
     */
    void set(size_t addr, uint8_t value)
    {
        ASSERT(addr < m_size);
        m_data[addr] = value;

        invalidate();
    }

    /**
     * @brief Sets a 16-bit word in memory.
     * @param addr The address to set
     * @param word The word value to write.
     * @remarks Words are always written little endian.
     */
    void setWord(size_t addr, uint16_t word)
    {
        ASSERT(addr < (m_size - 1));

        m_data[addr] = (uint8_t)(word & 0x00FF);
        m_data[addr + 1] = (uint8_t)((word >> 8) & 0x00FF);

        invalidate();
    }

    /**
     * @brief Sets a 24-bit word in memory
     * @param addr The address to set
     * @param word The 24-bit word to write.
     * @remarks Words are always written little endian.
     */
    void setLWord(size_t addr, uint32_t word)
    {
        ASSERT(addr < (m_size - 2));

        m_data[addr] = (uint8_t)(word & 0x0000'00FF);
        m_data[addr + 1] = (uint8_t)((word >> 8) & 0x0000'00FF);
        m_data[addr + 2] = (uint8_t)((word >> 16) & 0x0000'00FF);

        invalidate();
    }

    /**
     * @brief Sets a 32-bit word in memory
     * @param addr The address to set
     * @param word The 32-bit word to write.
     * @remarks Words are always written little endian.
     */
    void setDWord(size_t addr, uint32_t word)
    {
        ASSERT(addr < (m_size - 3));

        m_data[addr] = (uint8_t)(word & 0x0000'00FF);
        m_data[addr + 1] = (uint8_t)((word >> 8) & 0x0000'00FF);
        m_data[addr + 2] = (uint8_t)((word >> 16) & 0x0000'00FF);
        m_data[addr + 3] = (uint8_t)((word >> 24) & 0x0000'00FF);

        invalidate();
    }
};

typedef std::shared_ptr<COutputMem> CMemoryPtr;

/*************************************************************************/

#endif /* OUTPUT_MEM_H__ */

/*************************************************************************/
