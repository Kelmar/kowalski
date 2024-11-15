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

#ifndef PERSIST_EX_6502_H__
#define PERSIST_EX_6502_H__

/*************************************************************************/

/**
 * @brief Extensions for saving/loading complex values.
 */
class PersistentExtended : public wxPersistentObject
{
protected:
    PersistentExtended(void *obj)
        : wxPersistentObject(obj)
    {
    }

    bool SavePoint(const wxString &name, const wxPoint &point) const
    {
        wxString xName = name + ".x";
        wxString yName = name + ".y";

        return SaveValue(xName, point.x) && SaveValue(yName, point.y);
    }

    bool RestorePoint(const wxString &name, wxPoint *point)
    {
        ASSERT(point);

        wxString xName = name + ".x";
        wxString yName = name + ".y";

        int x = 0;
        int y = 0;

        bool ok = RestoreValue(xName, &x) && RestoreValue(yName, &y);

        if (ok)
        {
            point->x = x;
            point->y = y;
        }
        
        return ok;
    }

    bool SaveSize(const wxString &name, const wxSize &size) const
    {
        wxString xName = name + ".x";
        wxString yName = name + ".y";

        return SaveValue(xName, size.x) && SaveValue(yName, size.y);
    }

    bool RestoreSize(const wxString &name, wxSize *size)
    {
        ASSERT(size);

        wxString xName = name + ".x";
        wxString yName = name + ".y";

        int x = 0;
        int y = 0;

        bool ok = RestoreValue(xName, &x) && RestoreValue(yName, &y);

        if (ok)
        {
            size->x = x;
            size->y = y;
        }

        return ok;
    }
};

/*************************************************************************/

#endif /* PERSIST_EX_6502_H__ */

/*************************************************************************/
