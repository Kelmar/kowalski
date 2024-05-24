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

#ifndef HEX_VALIDATOR_H__
#define HEX_VALIDATOR_H__

/*************************************************************************/

#include "StdAfx.h"

#include "FormatNums.h"

/**
 * @brief Numeric validator but parses/displays in hexidecimal format.
 */
class HexValidator : public wxValidator
{
private:
    uint32_t *m_value;

    NumberFormat m_format;
    int m_digits;

    uint32_t m_maxValue;

    bool ReadValue(_Out_ uint32_t &value);

public:
    /* constructor */ HexValidator(
        uint32_t *value, 
        NumberFormat format = NumberFormat::DollarHex,
        int digits = 4);

    void SetMaxValue(uint32_t value) { m_maxValue = value; }

    virtual bool TransferFromWindow();
    virtual bool TransferToWindow();

    virtual bool Validate(wxWindow *);

    virtual wxObject *Clone() const
    {
        return new HexValidator(*this);
    }
};

/*************************************************************************/

#endif /* HEX_VALIDATOR_H__ */

/*************************************************************************/
