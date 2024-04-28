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

// LogBuffer.h: interface for the CLogBuffer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LOG_BUFFER_H__
#define LOG_BUFFER_H__

// Logging class: recording consecutive items in the buffer overwriting
// old values

template <class T>
class CLogBuffer
{
public:
    CLogBuffer(int nSize = 100000)
    {
        m_vBuffer.resize(nSize);
        m_pHead = &m_vBuffer.front();
        m_bFull = false;
    }

    ~CLogBuffer() {}

    // record item
    void Record(const T& t)
    {
        *m_pHead++ = t;
        if (m_pHead >= &m_vBuffer.back() + 1)
        {
            m_pHead = &m_vBuffer.front();
            m_bFull = true;
        }
    }

    // no of items recorded
    int GetCount() const
    {
        return m_bFull ? m_vBuffer.size() : m_pHead - &m_vBuffer.front();
    }

    // get n-th item
    const T& operator [] (int nIndex) const
    {
        ASSERT(nIndex >= 0 && nIndex < GetCount());
        const T* pStart = m_bFull ? m_pHead : &m_vBuffer.front();
        pStart += nIndex;
        if (pStart >= &m_vBuffer.back() + 1)
            return pStart[0-m_vBuffer.size()];
        else
            return *pStart;
    }

    // empty buffer
    void Clear()
    {
        m_pHead = &m_vBuffer.front();
        m_bFull = false;
    }

private:
    std::vector<T> m_vBuffer;
    T* m_pHead;
    bool m_bFull;
};

#endif /* LOG_BUFFER_H__ */
