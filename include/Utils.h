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

#ifndef UTILS_H__
#define UTILS_H__

/*=======================================================================*/
/**
 * @brief Inserts or replaces a value.
 * 
 * @returns true if value is new.
 */
template <typename TKey, typename TItem>
bool TryReplace(_In_ std::unordered_map<TKey, TItem> &map,
                _In_ const TKey &key,
                _In_ const TItem &item)
{
    auto itr = map.find(key);
    map[key] = item;
    return itr == map.end();
}

template <typename TKey, typename TItem>
bool TryLookup(_In_ const std::unordered_map<TKey, TItem> &map, 
               _In_ const TKey &key,
               _Out_ TItem &item)
{
    auto itr = map.find(key);

    if (itr != map.end())
    {
        item = itr->second;
        return true;
    }

    item = TItem();
    return false;
}

/*=======================================================================*/
/**
 * @brief Ensures run of code on function exit.
 */
template <class F>
class deferred_action
{
private:
    F m_call;

public:
    explicit deferred_action(F call) noexcept
        : m_call(std::move(call))
    {
    }

    deferred_action(deferred_action &&other) noexcept
        : m_call(std::move(other.m_call))
    {
    }

    // Prevent copy
    deferred_action(const deferred_action &) = delete;
    deferred_action &operator =(const deferred_action &) = delete;

    ~deferred_action() noexcept
    {
        m_call();
    }
};

template <class F>
inline deferred_action<F> defer(const F &f) noexcept
{
    return deferred_action<F>(f);
}

template <class F>
inline deferred_action<F> defer(F &&f) noexcept
{
    return deferred_action<F>(std::forward<F>(f));
}

/*=======================================================================*/
/**
 * @brief Utility class to preserve the current configuration path and 
 * restore on function exit.
 */
class PushConfigPath
{
private:
    wxConfig &m_config;
    wxString m_old;

public:
    PushConfigPath(wxConfig &config)
        : m_config(config)
    {
        m_old = m_config.GetPath();
    }

    ~PushConfigPath()
    {
        m_config.SetPath(m_old);
    }
};

/*=======================================================================*/

#endif /* UTILS_H__ */

/*=======================================================================*/
