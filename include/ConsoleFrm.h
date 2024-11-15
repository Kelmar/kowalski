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

#ifndef CONSOLE_FRAME_H__
#define CONSOLE_FRAME_H__

/*************************************************************************/

#include "Events.h"
#include "output.h"

/*************************************************************************/

/**
 * @brief Frame that displays the output of a console source.
 */
class ConsoleFrame : public wxPanel
{
public:
    class ConsoleOutput : public io::output
    {
        friend class ConsoleFrame;

    protected:
        ConsoleFrame *m_parent;
        std::string m_target;

        /* constructor */ ConsoleOutput(ConsoleFrame *frame, const std::string &target)
            : m_parent(frame)
            , m_target(target)
        {
        }

    public:
        virtual ~ConsoleOutput()
        {
        }

        virtual void write(const char *str)
        {
            MessageEvent event(evTHD_OUTPUT, evTHD_OUTPUT_ID, m_target.c_str());
            event.SetString(str);

            //m_parent->GetEventHandler()->AddPendingEvent(event);
            wxQueueEvent(m_parent, event.Clone());
        }
    };

    void OnMessage(MessageEvent &);

private:
    wxTextCtrl *m_text;

    ConsoleOutput m_output;

public:
    /* constructor */ ConsoleFrame(wxFrame *parent);
    virtual          ~ConsoleFrame();

    void AppendText(const char *txt);

    io::output &GetOutput(const char *target)
    {
        UNUSED(target); // We don't use targets (yet)
        return m_output;
    }
};

/*************************************************************************/

#endif /* CONSOLE_FRAME_H__ */

/*************************************************************************/
