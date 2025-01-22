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

#include "StdAfx.h"
#include "sim.h"

/*=======================================================================*/

Bus::Bus(int width)
    : m_width(width)
    , m_maxAddress(0)
    , m_devices()
{
    ASSERT((m_width >= 10) && (m_width <= 24));

    m_maxAddress = (1 << m_width) - 1;
}

Bus::Bus(Bus &&rhs)
    : m_width(std::move(rhs.m_width))
    , m_maxAddress(std::move(rhs.m_maxAddress))
    , m_devices(std::move(rhs.m_devices))
{
}

Bus::~Bus()
{
    for (auto i : m_devices)
        delete i;
}

/*=======================================================================*/

Bus::Node *Bus::RangeFind(sim_addr_t start, int cnt) const
{
    sim_addr_t end = start + (cnt - 1);

    for (auto i : m_devices)
    {
        if ((i->start < end) && (i->end > start))
            return i;
    }

    return nullptr;
}

/*=======================================================================*/

void Bus::RunRange(sim_addr_t start, size_t sz, RangeOp operation) const
{
    if (!InRange(start))
        return;

    sim_addr_t end = std::min(m_maxAddress, (sim_addr_t)(start + sz));
    sz = end - start; // Clamp to edge of address space
    Node *node = nullptr;

    for (size_t i = 0; i < sz; ++i)
    {
        sim_addr_t address = start + i;

        if (!node || node->end < address)
            node = RangeFind(address);

        operation(address, i, node);
    }
}

/*=======================================================================*/

bool Bus::Contains(sim::PDevice device) const
{
    for (auto i : m_devices)
    {
        if (i->device == device)
            return true;
    }

    return false;
}

/*=======================================================================*/

Bus::MapError Bus::AddDevice(sim::PDevice device, sim_addr_t start)
{
    ASSERT(device);

    if (Contains(device))
        return Bus::MapError::AlreadyMapped;

    sim_addr_t cnt = device->AddressSize();
    sim_addr_t end = start + (cnt - 1);

    if (!InRange(end) || (end < start))
        return Bus::MapError::OutOfRange;

    Node *node = RangeFind(start, cnt);

    if (node)
        return Bus::MapError::Overlap; // A device already exists at the given address

    node = new Node(device, start, end);
    m_devices.push_back(node);

    return Bus::MapError::None;
}

/*=======================================================================*/

void Bus::Reset()
{
    for (auto n : m_devices)
        n->Reset();
}

/*=======================================================================*/

uint8_t Bus::PeekByte(sim_addr_t address) const
{
    Node *node = RangeFind(address);
    return node ? node->Peek(address) : 0;
}

/*=======================================================================*/

void Bus::PeekRange(sim_addr_t start, _Out_ std::span<uint8_t> &values) const
{
    RunRange(start, values.size(), [&values] (sim_addr_t address, size_t i, Node *node)
    {
        values[i] = node ? node->Peek(address) : 0;
    });
}

/*=======================================================================*/

uint8_t Bus::GetByte(sim_addr_t address)
{
    if (!InRange(address))
        return 0;

    Node *node = RangeFind(address);
    return node ? node->GetByte(address) : 0;
}

/*=======================================================================*/

void Bus::GetRange(sim_addr_t start, _Out_ std::span<uint8_t> &values)
{
    RunRange(start, values.size(), [&values] (sim_addr_t address, size_t i, Node *node)
    {
        values[i] = node ? node->GetByte(address) : 0;
    });
}

/*=======================================================================*/

void Bus::SetByte(sim_addr_t address, uint8_t value)
{
    if (!InRange(address))
        return;

    Node *node = RangeFind(address);

    if (node)
        node->SetByte(address, value);
}

/*=======================================================================*/

void Bus::SetRange(sim_addr_t start, _In_ const std::span<uint8_t> &values)
{
    RunRange(start, values.size(), [&values] (sim_addr_t address, size_t i, Node *node)
    {
        if (node)
            node->SetByte(address, values[i]);
    });
}

/*=======================================================================*/
