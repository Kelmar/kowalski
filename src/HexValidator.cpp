/*************************************************************************/
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
/*************************************************************************/

#include "StdAfx.h"

#include "FormatNums.h"
#include "HexValidator.h"

/*************************************************************************/

HexValidator::HexValidator(
    uint32_t *value,
    NumberFormat format /* = NumberFormat::DollarHex */,
    int digits /* = 4 */)
    : m_value(value)
    , m_format(format)
    , m_digits(digits)
{
    ASSERT(m_value);
}

/*************************************************************************/

bool HexValidator::ReadValue(_Out_ uint32_t &value)
{
    auto valueCtrl = dynamic_cast<wxTextEntry *>(GetWindow());

    if (!valueCtrl)
        return false;

    std::string valStr = valueCtrl->GetValue().ToStdString();

    uint32_t test = 0; // Initialized to keep valgrind happy.

    NumberFormat type = NumberFormats::FromString(valStr, _Out_ test);
    bool inRange = m_maxValue > 0 ? (test < m_maxValue) : true;

    if ((type != NumberFormat::Error) && inRange)
    {
        value = test;
        return true;
    }

    return false;
}

/*************************************************************************/

bool HexValidator::Validate(wxWindow *)
{
    uint32_t dmy;
    bool isValid = ReadValue(_Out_ dmy);

    if (!isValid && !IsSilent())
    {
        auto control = GetWindow();
        control->SetFocus();

        wxMessageBox(_("Invalid hex value."));
    }

    return isValid;
}

/*************************************************************************/

bool HexValidator::TransferFromWindow()
{
    return ReadValue(_Out_ * m_value);
}

/*************************************************************************/

bool HexValidator::TransferToWindow()
{
    auto valueCtrl = dynamic_cast<wxTextEntry *>(GetWindow());

    if (!valueCtrl)
        return false;

    wxString valStr = NumberFormats::ToString(*m_value, m_format, m_digits);
    valueCtrl->SetValue(valStr);

    return true;
}

/*************************************************************************/
