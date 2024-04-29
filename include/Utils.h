/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Several useful debugging functions/annotations for code.
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

#ifndef UTILS_H__
#define UTILS_H__

/*************************************************************************/
/**
 * @brief Inserts or replaces a value.
 * 
 * @returns true if value was replaced.
 */
template <typename TKey, typename TItem>
bool TryReplace(_In_ std::unordered_map<TKey, TItem> &map,
                _In_ const TKey &key,
                _In_ const TItem &item)
{
    auto itr = map.find(key);
    map[key] = item;
    return itr != map.end();
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

/*************************************************************************/

#endif /* UTILS_H__ */

/*************************************************************************/