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

#ifdef WIN32

#include "StdAfx.h"
#include "config.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace config::source;

/*=======================================================================*/

WinRegistry::WinRegistry(const std::string_view &root)
    : Source(root)
    , m_key(nullptr)
{
}

/*=======================================================================*/

void WinRegistry::createKey(bool reading)
{
    ASSERT(m_key == nullptr);

    LSTATUS res = RegCreateKeyExA(
        HKEY_CURRENT_USER,
        root().c_str(),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        reading ? KEY_READ : KEY_WRITE,
        NULL,
        &m_key,
        NULL);

    if (res != ERROR_SUCCESS)
        throw std::runtime_error("Error creating registry key");
}

/*=======================================================================*/

void WinRegistry::releaseKey()
{
    if (m_key)
    {
        RegFlushKey(m_key);
        RegCloseKey(m_key);
        m_key = nullptr;
    }
}

/*=======================================================================*/

std::shared_ptr<config::Context> WinRegistry::createContext(bool reading)
{
    auto d = defer([this] () { releaseKey(); });

    createKey(reading);

    return config::details::Source::createContext(reading);
}

/*=======================================================================*/

bool WinRegistry::readReg(
    const std::string &path,
    DWORD type,
    _Out_ std::vector<BYTE> &buffer)
{
    DWORD inType = type;
    DWORD inSize = 0;

    LSTATUS res = RegGetValueA(
        m_key,
        path.c_str(),
        nullptr, // lpValue (name of the value)
        RRF_RT_ANY,
        &inType,
        nullptr,
        &inSize
    );

    if (res != ERROR_SUCCESS)
    {
        if (res == ERROR_FILE_NOT_FOUND)
        {
            // This is an expected condition where the key wasn't found.
            return true;
        }

        return false;
    }

    if (inType != type)
        return false;

    buffer.reserve(inSize + 1); // Reserve extra space for NULL character.

    res = RegGetValueA(
        m_key,
        path.c_str(),
        nullptr, // plValue
        RRF_RT_ANY,
        nullptr,
        buffer.data(),
        &inSize
    );

    if (res != ERROR_SUCCESS)
        return false;

    return true;
}

/*=======================================================================*/

void WinRegistry::readValue(const std::string &path, bool &value)
{
    int i;
    readValue(path, i);
    value = (i != 0);
}

/*=======================================================================*/

void WinRegistry::readValue(const std::string &path, int &value)
{
    std::vector<BYTE> buffer;

    if (!readReg(path, REG_DWORD, _Out_ buffer))
        return;

    std::memcpy(&value, buffer.data(), sizeof(value));
}

/*=======================================================================*/

void WinRegistry::readValue(const std::string &path, size_t &value)
{
    std::vector<BYTE> buffer;

    constexpr DWORD regType = sizeof(size_t) == 4 ? REG_DWORD : REG_QWORD;

    if (!readReg(path, regType, _Out_ buffer))
        return;

    std::memcpy(&value, buffer.data(), sizeof(value));
}

/*=======================================================================*/

void WinRegistry::readValue(const std::string &path, long &value)
{
    std::vector<BYTE> buffer;

    constexpr DWORD regType = sizeof(long) == 4 ? REG_DWORD : REG_QWORD;

    if (!readReg(path, regType, _Out_ buffer))
        return;

    std::memcpy(&value, buffer.data(), sizeof(value));
}

/*=======================================================================*/

void WinRegistry::readValue(const std::string &path, float &value)
{
    std::vector<BYTE> buffer;

    if (!readReg(path, REG_DWORD, _Out_ buffer))
        return;

    std::memcpy(&value, buffer.data(), sizeof(value));
}

/*=======================================================================*/

void WinRegistry::readValue(const std::string &path, double &value)
{
    std::vector<BYTE> buffer;

    if (!readReg(path, REG_QWORD, _Out_ buffer))
        return;

    std::memcpy(&value, buffer.data(), sizeof(value));
}

/*=======================================================================*/

void WinRegistry::readValue(const std::string &path, std::string &value)
{
    std::vector<BYTE> buffer;

    if (!readReg(path, REG_SZ, _Out_ buffer))
        return;

    buffer.push_back(0); // Ensure NULL terminated.

    const char *ptr = reinterpret_cast<const char *>(buffer.data());
    value.assign(ptr, buffer.size());
}

#endif

/*=======================================================================*/
