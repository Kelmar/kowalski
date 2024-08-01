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

#ifndef EXCEPT_6502_H__
#define EXCEPT_6502_H__

/*************************************************************************/

#include <cerrno>
#include <stdexcept>

/*************************************************************************/

class ResourceError : public std::runtime_error
{
public:
    /* constructor */ ResourceError()
        : runtime_error(_("Unable to load resource"))
    { }
};

/*************************************************************************/

class FileError : public std::runtime_error
{
public:
    enum ErrorCode
    {
        // There was no error
        NoError = 0,

        // Check errno for error
        SysError,

        // File format is corrupt
        Corrupt,

        // Request to save in an unsupported format.
        Unsupported,

        // Unable to change to file directory.
        CannotChangeDir,
    };

private:
    ErrorCode m_error;
    int m_sysError;

    std::string getErrString(ErrorCode error, int sysError)
    {
        bool includeSysError = false;
        std::string rval;

        switch (error)
        {
        case NoError:
            rval = std::string(_("No error."));
            break;

        case SysError:
            rval = _("General system error:");
            includeSysError = true;
            break;

        case Corrupt:
            rval = _("The file format is corrupt.");
            break;

        case CannotChangeDir:
            rval = _("Cannot change to directory:");
            includeSysError = true;
            break;

        default:
            ASSERT(false);
            rval = _("Unknown error code:");
            includeSysError = true;
            break;
        }

        if (includeSysError)
        {
            if (!rval.empty())
                rval += "\r\n";

            rval += std::strerror(sysError);
        }

        return rval;
    }

public:
    /* constructor */ FileError(ErrorCode error)
        : runtime_error(getErrString(error, errno))
        , m_error(error)
        , m_sysError(errno)
    { }

    // Get's the error code that was raised.
    ErrorCode Code() const { return m_error; }

    // Get's the errno value for this exception.
    int ErrNo() const { return m_sysError; }
};

/*************************************************************************/

#endif /* EXCEPT_6502_H__ */

/*************************************************************************/
