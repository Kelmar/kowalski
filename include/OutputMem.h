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
    UINT m_uMask;

public:
    COutputMem()
        : m_data(nullptr)
        , m_size(0x1000000)
        , m_uMask(0xFFFFFF)
    {
        m_data = new uint8_t[m_size];
        memset(m_data, 0, m_size);
    }

    virtual ~COutputMem()
    {
        delete[] m_data;
    }

    void ClearMem()	// wyzerowanie pami�ci
    {
        memset(m_data, 0, m_size);
    }

    void ClearMem(int nByte)
    {
        memset(m_data, nByte, m_size);
    }

    COutputMem &operator= (const COutputMem &src)
    {
        ASSERT(m_size == src.m_size); // Dimensions must be identical
        memcpy(m_data, src.m_data, m_size);
        m_uMask = src.m_uMask;
        return *this;
    }

#if 0
    void Save(CArchive &archive, uint32_t start, uint32_t end)
    {
        ASSERT(end >= start);
        archive.Write(m_data + start, int(end) - start + 1);
    }

    void Load(CArchive &archive, uint32_t start, uint32_t end)
    {
        ASSERT(end >= start);
        archive.Read(m_data + start, int(end) - start + 1);
    }
#endif

    const uint8_t *Mem() const { return m_data; }

    void SetMask(uint32_t uMask)
    {
        m_uMask = uMask;
    }

    uint8_t& operator[] (uint32_t uAddr)
    {
        ASSERT(m_uMask > 0);
        return m_data[uAddr & m_uMask];
    } // ElementAt(uAddr & m_uMask); }

    uint8_t operator[] (uint32_t uAddr) const
    {
        ASSERT(m_uMask > 0);
        return m_data[uAddr & m_uMask];
    } // GetAt(uAddr & m_uMask); }

    uint16_t GetWord(uint16_t uLo, uint16_t uHi) const
    {
        ASSERT(m_uMask > 0);
        return m_data[uLo & m_uMask] + uint32_t(m_data[uHi & m_uMask] << 8);
        //return GetAt(uLo & m_uMask) + (uint32_t(GetAt(uHi & m_uMask)) << 8);
    }

    uint16_t GetWord(uint32_t uAddr) const
    {
        ASSERT(m_uMask > 0);
        return m_data[uAddr & m_uMask] + uint32_t(m_data[(uAddr + 1) & m_uMask] << 8);
        //return GetAt(uAddr & m_uMask) + (uint32_t(GetAt((uAddr + 1) & m_uMask)) << 8);
    }

    uint16_t GetWordInd(uint8_t uZpAddr) const
    {
        ASSERT(m_uMask > 0);
        return m_data[uZpAddr] + (m_data[uZpAddr + uint8_t(1)] << 8);
        //return GetAt(uZpAddr) + (GetAt(uZpAddr + uint8_t(1)) << 8);
    }
};

#endif
