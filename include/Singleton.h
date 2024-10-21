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

#ifndef SINGLETON_6502_H__
#define SINGLETON_6502_H__

/*************************************************************************/

template <typename TBase>
class Singleton
{
private:
    static TBase *s_self;

protected:
    /* constructor */ Singleton()
    {
        ASSERT(s_self == nullptr);
        s_self = static_cast<TBase *>(this);
    }

public:
    virtual ~Singleton()
    {
        s_self = nullptr;
    }

    static TBase &Get()
    {
        ASSERT(s_self);
        return *s_self;
    }

    static TBase *Ptr() { return s_self; }
};

#define IMPLEMENT_SINGLETON(T_) template<> T_ *Singleton<T_>::s_self = nullptr

/*************************************************************************/

#endif /* SINGLETON_6502_H__ */

/*************************************************************************/
