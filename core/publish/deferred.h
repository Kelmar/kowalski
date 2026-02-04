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
 * Deferred action class
 */
/*=======================================================================*/

#ifndef CORE_DEFERRED_H__
#define CORE_DEFERRED_H__

/*=======================================================================*/
/**
 * @brief Ensures run of code on function exit.
 */
template <class F>
class DeferredAction
{
private:
    F m_call;

public:
    explicit DeferredAction(F call) noexcept
        : m_call(std::move(call))
    { }

    DeferredAction(DeferredAction &&other) noexcept
        : m_call(std::move(other.m_call))
    { }

    // Prevent copy
    DeferredAction(const DeferredAction &) = delete;
    DeferredAction &operator =(const DeferredAction &) = delete;

    ~DeferredAction() noexcept
    {
        m_call();
    }
};

template <class F>
inline DeferredAction<F> defer(const F &f) noexcept
{
    return DeferredAction<F>(f);
}

template <class F>
inline DeferredAction<F> defer(F &&f) noexcept
{
    return DeferredAction<F>(std::forward<F>(f));
}

/*=======================================================================*/

#endif /* CORE_DEFERRED_H__ */

/*=======================================================================*/