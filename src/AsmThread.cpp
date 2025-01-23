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

#include "StdAfx.h"
#include "6502.h"

#include "Events.h"
#include "MainFrm.h"
#include "M6502.h"

#include "AsmThread.h"

/*=======================================================================*/

wxThread::ExitCode AsmThread::Entry()
{
    COutputMem &mainMem = wxGetApp().m_global.GetMemory();
    CMemoryPtr asmMem(new COutputMem());
    CDebugInfo *debug = wxGetApp().simulatorController().DebugInfo();
    SimulatorConfig config = wxGetApp().simulatorController().GetConfig();

    auto assembler = std::make_unique<CAsm6502>(
        m_path.c_str(),
        m_output,
        asmMem.get(),
        debug,
        nullptr,
        config.Processor);

    CAsm::Stat res = assembler->assemble();

    if (res == CAsm::Stat::OK)
    {
        // Copy result to actual memory.
        mainMem = *asmMem;
    }

    wxThreadEvent event(wxEVT_THREAD, evTHD_ASM_COMPLETE);

    wxQueueEvent(m_parent, event.Clone());

    return reinterpret_cast<wxThread::ExitCode>(res);
}

/*=======================================================================*/
