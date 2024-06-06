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

#ifndef _output_mem_h_
#define _output_mem_h_

class COutputMem
{
private:
    uint8_t *m_data;
    size_t m_size;

public:
    COutputMem()
        : m_data(nullptr)
        , m_size(0x1000000)
    {
        m_data = new uint8_t[m_size];
        memset(m_data, 0, m_size);
    }

    virtual ~COutputMem()
    {
        delete[] m_data;
    }

    size_t size() const { return m_size; }

    void Clear()
    {
        memset(m_data, 0, m_size);
    }

    // TODO: Deprecate this, don't want to be copying huge chunks of memory around.
    COutputMem &operator= (const COutputMem &src)
    {
        ASSERT(m_size == src.m_size); // Dimensions must be identical
        memcpy(m_data, src.m_data, m_size);
        return *this;
    }

    const uint8_t *Mem() const { return m_data; }

    uint8_t operator[] (size_t addr) const
    {
        ASSERT(addr < m_size);
        return m_data[addr];
    }

    uint8_t &operator[] (size_t addr)
    {
        ASSERT(addr < m_size);
        return m_data[addr];
    }

    std::span<uint8_t> GetSpan(size_t from, size_t to)
    {
        ASSERT(from < m_size);
        to = std::min(to, m_size);
        return std::span(&m_data[from], to - from);
    }

    uint16_t GetWord(size_t addr) const
    {
        ASSERT(addr < (m_size - 1));

        uint16_t lo = m_data[addr];
        uint16_t hi = m_data[addr + 1];

        return lo | (hi << 8);
    }
};

typedef std::shared_ptr<COutputMem> CMemoryPtr;

#endif
