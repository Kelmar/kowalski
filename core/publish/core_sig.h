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
/*
 * A quick and dirty signal/slot mechanism.
 */
/*=======================================================================*/

#ifndef CORE_SIGNAL_H__
#define CORE_SIGNAL_H__

/*=======================================================================*/

#include <type_traits>
#include <functional>
#include <memory>

/*=======================================================================*/

template <typename... TArgs>
class Slot
{
public:
    typedef std::function<void(TArgs...)> function_type;

private:
    bool m_connected;
    function_type m_fn;

    static void no_op(TArgs...) noexcept { }

public:
    /* constructor */ Slot() noexcept
        : m_connected(false)
        , m_fn(no_op)
    {
    }

    /* constructor */ Slot(function_type fn) noexcept
        : m_connected(true)
        , m_fn(fn)
    {
    }

    /* constructor */ Slot(const Slot &s) noexcept
        : m_connected(false)
        , m_fn(no_op)
    {
        operator =(s);
    }

    /* constructor */ Slot(Slot &&s) noexcept
        : m_connected(false)
        , m_fn(no_op)
    {
        operator =(std::forward(s));
    }

    virtual ~Slot() noexcept
    {
        // Pedantic clean up
        clear();
    }

    bool isConnected() const noexcept { return m_connected; }

    void clear() noexcept
    {
        m_fn = no_op;
        m_connected = false;
    }

    void set(function_type fn) noexcept
    {
        m_fn = fn;
        m_connected = true;
    }

    const Slot &operator =(const Slot &rhs) noexcept
    {
        m_fn = rhs.m_fn;
        m_connected = rhs.m_connected;

        return *this;
    }

    const Slot &operator =(Slot &&rhs) noexcept
    { 
        std::swap(m_fn, rhs.m_fn);
        std::swap(m_connected, rhs.m_connected);

        return *this;
    }

    void call(TArgs... args) const
    {
        if (m_connected)
            m_fn(args...);
    }
};

/*=======================================================================*/

template <typename... TArgs>
class Connection
{
public:
    typedef Slot<TArgs...> slot_type;
    typedef std::weak_ptr<slot_type> slot_ptr;

protected:
    slot_ptr m_slot;

    void clear() noexcept
    {
        m_slot.reset();
    }

public:
    Connection() noexcept
        : m_slot()
    {
    }

    Connection(slot_ptr slot) noexcept
        : m_slot(slot)
    {
    }

    Connection(const Connection &c) noexcept
    {
        operator =(c);
    }

    Connection(Connection &&c) noexcept
    {
        operator =(std::forward(c));
    }

    virtual ~Connection() noexcept
    {
        clear();
    }

    bool isConnected() const noexcept { return !m_slot.expired(); }

    void disconnect() noexcept
    {
        auto ptr = m_slot.lock();

        if (ptr)
        {
            ptr->clear();
            clear();
        }
    }

    const Connection &operator =(slot_ptr slot) noexcept
    {
        m_slot = slot;
        return *this;
    }

    const Connection &operator =(const Connection &c) noexcept
    {
        m_slot = c.m_slot;
        return *this;
    }

    const Connection &operator =(Connection &&c) noexcept
    {
        std::swap(m_slot, c.m_slot);
        return *this;
    }
};

/*=======================================================================*/

template <typename... TArgs>
class Signal
{
public:
    typedef Slot<TArgs...> slot_type;
    typedef slot_type::function_type function_type;
    typedef Connection<TArgs...> connection_type;

private:
    typedef std::shared_ptr<slot_type> slot_ptr;

    std::vector<slot_ptr> m_slots;
    bool m_blocked;

public:
    Signal()
        : m_blocked(false)
    {
    }

    virtual ~Signal()
    {
        // Pedantic clean up.
        m_slots.clear();
    }

    void block() { m_blocked = true; }

    void open() { m_blocked = false; }

    bool isBlocked() const { return m_blocked; }

    connection_type connect(function_type fn)
    {
        for (slot_ptr s : m_slots)
        {
            if (!s->isConnected())
            {
                s->set(fn);
                return connection_type(s);
            }
        }

        slot_ptr slot = std::make_shared<slot_type>(fn);
        m_slots.push_back(slot);
        return connection_type(slot);
    }

    void operator ()(TArgs... args) const
    {
        if (m_blocked)
            return;

        for (slot_ptr i : m_slots)
            i->call(args...);
    }
};

/*=======================================================================*/

#endif /* CORE_SIGNAL_H__ */

/*=======================================================================*/
