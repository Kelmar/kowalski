/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

// DynamicHelp.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DynamicHelp.h"
#include "CCrystalTextBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// CDynamicHelp

const int nMinW = 80;
const int nMinH = 50;

static const char* s_REGISTRY_SECTION = "DynamicHelp";
static const char* s_REGISTRY_WIDTH = "Height";

//static const char *s_pcszTitle = _("Dynamic Help");

std::string GetConfigPath()
{
    std::string path = "/";

    path += s_REGISTRY_SECTION;
    path += '/';
    path += s_REGISTRY_WIDTH;

    return path;
}

CDynamicHelp::CDynamicHelp()
{
    //m_cxLeftBorder = m_cxRightBorder = 0;
    //m_cyTopBorder = m_cyBottomBorder = 0;
    //m_nMRUWidth = 300;

    //m_sizeDefault = wxSize(m_nMRUWidth, wxGetApp().Config().Read(GetConfigPath(), 500));

    m_sizeDefault = wxSize(300, wxGetApp().Config().Read(GetConfigPath(), 500));
    m_nHeaderHeight = 16;
}

CDynamicHelp::~CDynamicHelp()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDynamicHelp message handlers

//-----------------------------------------------------------------------------

wxSize CDynamicHelp::CalcFixedLayout(bool stretch, bool horz)
{
    wxSize sizeBar;

    if (horz)
    {
        sizeBar.SetWidth(stretch ? 32767 : nMinW);
        sizeBar.SetHeight(nMinH);
    }
    else
    {
        sizeBar.SetWidth(nMinW);
        sizeBar.SetHeight(stretch ? 32767 : nMinH);
    }

    return sizeBar;
}

//-----------------------------------------------------------------------------

wxSize CDynamicHelp::CalcDynamicLayout(int nLength, uint32_t dwMode)
{
    return CalcLayout(dwMode, nLength);
}

wxSize CDynamicHelp::CalcLayout(uint32_t dwMode, int nLength)
{
    UNUSED(dwMode);
    UNUSED(nLength);

#if 0
    wxSize sizeResult(50, 50);
    if (dwMode & (LM_HORZDOCK | LM_VERTDOCK))
    {
        wxRect rect;
        CWnd* pMainWnd = AfxGetMainWnd();
        if (CMDIFrameWnd* pFrameWnd= dynamic_cast<CMDIFrameWnd*>(pMainWnd))
            ::GetClientRect(pFrameWnd->m_hWndMDIClient, rect);
        else
            pMainWnd->GetClientRect(rect);
        sizeResult = rect.Size();
        sizeResult += wxSize(10, 10);
        if (dwMode & 0x8000)
            m_nMRUWidth = nLength;
        sizeResult.cx = m_nMRUWidth;
    }

    if (dwMode & LM_MRUWIDTH)
        SizeToolBar(m_nMRUWidth);
    else if (dwMode & LM_HORZDOCK)
        return CSize(sizeResult.cx, nMinH);
    else if (dwMode & LM_VERTDOCK)
        return CSize(m_nMRUWidth, sizeResult.cy);
    else if (nLength != -1)
        SizeToolBar(nLength, !!(dwMode & LM_LENGTHY));
    else if (m_dwStyle & CBRS_FLOATING)
        SizeToolBar(m_nMRUWidth);
    else
        SizeToolBar(dwMode & LM_HORZ ? 32767 : 0);

    sizeResult = m_sizeDefault;

    if (dwMode & LM_COMMIT)
        m_nMRUWidth = sizeResult.cx;

    return sizeResult;
#endif

    return wxSize(0, 0);
}

void CDynamicHelp::SizeToolBar(int nLength, bool vert/*= false*/)
{
    if (vert)
        m_sizeDefault.SetHeight(nLength);
    else
        m_sizeDefault.SetWidth(nLength);

    if (m_sizeDefault.GetWidth() < nMinW)
        m_sizeDefault.SetWidth(nMinW);
    if (m_sizeDefault.GetHeight() < nMinH)
        m_sizeDefault.SetHeight(nMinH);

    Refresh();
}

//-----------------------------------------------------------------------------

void CDynamicHelp::OnNcPaint()
{
    //EraseNonClient();
}

//-----------------------------------------------------------------------------

bool CDynamicHelp::Create(wxWindow* pParentWnd, UINT nID)
{
    UNUSED(pParentWnd);
    UNUSED(nID);

#if 0
    CControlBar::Create(s_strWndClass, s_pcszTitle,
                        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        CRect(10, 10, 100, 100), pParentWnd, nID);

    SetBarStyle(CBRS_ALIGN_RIGHT | CBRS_FLYBY | CBRS_GRIPPER | CBRS_SIZE_DYNAMIC);

    m_wndClose.Create(WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
                      CRect(0, 0, 0, 0), this, -1);
    m_wndClose.SetButtonStructSize(sizeof(TBBUTTON));
    m_wndClose.AddBitmap(1, IDB_CLOSE_TB);
    m_wndClose.SetBitmapSize(CSize(8, 8));
    TBBUTTON btn;
    btn.iBitmap = 0;
    btn.idCommand = IDCLOSE;
    btn.fsState = TBSTATE_ENABLED;
    btn.fsStyle = TBSTYLE_BUTTON;
    btn.dwData = 0;
    btn.iString = 0;
    m_wndClose.AddButtons(1, &btn);

    Resize();
#endif

    return true;
}

//-----------------------------------------------------------------------------

bool CDynamicHelp::OnEraseBkgnd(wxDC* dc)
{
    if (!IsFloating())
    {
        wxRect rect = GetClientRect();
        //dc->SetBackground(::GetSysColor(COLOR_3DFACE));
        dc->DrawRectangle(rect.GetRight() - m_nHeaderHeight - 2, rect.GetTop(), m_nHeaderHeight, m_nHeaderHeight);
    }

    return true;
}

//-----------------------------------------------------------------------------
static wxColour s_rgbHelpBkgnd = wxColour(255, 255, 240);

void CDynamicHelp::DoPaint(wxDC* dc)
{
    wxRect rect = GetClientRect();

    if (IsFloating())
    {
        //dc->SetBackground(s_rgbHelpBkgnd);
        dc->DrawRectangle(rect);
    }
    else
    {
        //dc->SetBackground(::GetSysColor(COLOR_3DFACE));
        //dc->DrawRectangle(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), m_nHeaderHeight);

        int nW = rect.GetWidth() - 9 - m_nHeaderHeight;
        int nX = rect.GetLeft() + 5;

        UNUSED(nX);

        if (nW > 0)
        {
            //wxColour rgbLight = wxColour(255, 255, 255);
            //wxColour rgbDark = ::GetSysColor(COLOR_3DSHADOW);

            //dc->Draw3dRect(nX, rect.top + 5, nW, 3, rgbLight, rgbDark);
            //dc->Draw3dRect(nX, rect.top + 9, nW, 3, rgbLight, rgbDark);
        }

        int nHeight = rect.GetHeight() - m_nHeaderHeight;
        if (nHeight > 0)
        {
            //dc->SetBackground(s_rgbHelpBkgnd);
            dc->DrawRectangle(rect.GetLeft(), rect.GetTop() + m_nHeaderHeight, rect.GetWidth(), nHeight);
        }
    }
}

#if 0
bool CDynamicHelp::OnToolTipGetText(UINT uId, NMHDR* pNmHdr, LRESULT* pResult)
{
    NMTTDISPINFO* pTTT= (NMTTDISPINFO*)pNmHdr;

    pTTT->lpszText = pTTT->szText;
    pTTT->szText[0] = 0;
    pTTT->hinst = NULL;

    *pResult = 0;
    return TRUE;
}
#endif

void CDynamicHelp::Resize()
{
    wxRect rect = GetClientRect();

    if (IsFloating()) // if it's floating it has a title already
    {
        // correction for afxData.cxBorder2 junk
#if 0
        rect.DeflateRect(2, 2, 2, 2);

        if (m_wndClose.m_hWnd)
            m_wndClose.Hide();
#endif

        //m_wndHelp.SetWindowPos(0, rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
    }
    else
    {
        // it needs it's own title

        // correction for afxData.cxBorder2 junk
#if 0
        rect.DeflateRect(2, 0, 2, 8);
#endif

        if (rect.GetBottom() < rect.GetTop())
            rect.SetBottom(rect.GetTop());

#if 0
        if (m_wndClose)
            m_wndClose.SetWindowPos(0, rect.right - 16, rect.top + 2, 15, 14, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
#endif

        rect.SetTop(rect.GetTop() + m_nHeaderHeight);

        m_wndHelp.SetPosition(rect.GetTopLeft());
        m_wndHelp.SetSize(rect.GetSize());
    }
}

void CDynamicHelp::OnSize(UINT nType, int cx, int cy)
{
    UNUSED(nType);
    UNUSED(cx);
    UNUSED(cy);

    //CControlBar::OnSize(nType, cx, cy);

    Resize();
}

void CDynamicHelp::OnDestroy()
{
    wxGetApp().Config().Write(GetConfigPath(), m_sizeDefault.GetHeight());

    //CControlBar::OnDestroy();
}

#if 0
int CDynamicHelp::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CControlBar::OnCreate(lpCreateStruct) == -1)
        return -1;

    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL;
    m_wndHelp.CWnd::Create("RichEdit20A", NULL, dwStyle, CRect(0, 0, 0, 0), this, IDS_DYNAMIC_HELP);

    if (m_wndHelp.m_hWnd == 0)
        return -1;

    m_wndHelp.SetReadOnly();
    m_wndHelp.SetBackgroundColor(false, s_rgbHelpBkgnd);
    m_wndHelp.SendMessage(EM_SETTEXTMODE, TM_RICHTEXT | TM_SINGLECODEPAGE);
    m_wndHelp.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);

    //SetContextHelp("<b>LDA</b> blah blah<p><u>importante</u> reioioi khjkcxn opoiiso fjoo.");
    /*
    CFile file("C:\\LDA2.rtf", CFile::modeRead);
    std::string str;
    file.Read(str.GetBuffer(file.GetLength()), file.GetLength());
    str.ReleaseBuffer(file.GetLength());
    SetContextHelp(str);
    */

    return 0;
}
#endif

void CDynamicHelp::SetContextHelp(const char* pcszText, const char* pcszHeader/*= 0*/)
{
    std::string strText = 
        "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033"
        "{\\fonttbl"
        "{\\f0\\fmodern\\fprq1\\fcharset0 Courier New;}"
        "{\\f1\\fswiss\\fcharset0 Arial;}"
        "}"
        "{"
        "\\colortbl ;\\red255\\green255\\blue255;\\red192\\green192\\blue192;\\red160\\green160\\blue160;\\red224\\green224\\blue224;"
        "}"
        "\\viewkind4\\uc1\\pard\\li0";

    const char* pcszHeaderBefore = "\\f1\\fs20\\sa25\\sb25\\cf1\\cb3\\highlight3\\b\\ql\\~  \\~\\~\\~";
    const char* pcszHeaderAfter = "\\~\\~\\~  \\~\\highlight0\\par";
    const char* pcszFont = "\\pard\\cf0\\b0\\f0\\fs20\\ql";

    if (pcszHeader)
    {
        strText += pcszHeaderBefore;
        strText += pcszHeader;
        strText += pcszHeaderAfter;
        strText += pcszFont;
        strText += "\\par\\li60 ";
    }
    else
    {
        strText += pcszFont;
    }

#define PLAIN	"\\f1\\fs20\\b0 "

    wxString str = pcszText;

    str.Replace("{", "\\{");
    str.Replace("}", "\\}");
    str.Replace("#title#", "\\fi350\\f0\\fs30\\b ");
    str.Replace("#text#", "\\par\\fi0\\f1\\fs20\\b0 ");
    str.Replace("#syntax#", PLAIN "\\par\\par\\sa70 Syntax:\\par\\sa0\\f0\\fs20\\b ");
    str.Replace("#exmpl#", PLAIN "\\par\\sa70 Example:\\par\\sa0\\f0\\fs20\\b0 ");
    str.Replace("#desc#", PLAIN "\\par\\sa70 Description:\\par\\sa0\\f1\\fs20\\b0 ");
    str.Replace("#flags#", PLAIN "\\par\\par Affects flags: \\f0\\fs24\\b0 ");
    str.Replace("#modes#", PLAIN "\\par\\par\\sa70 Addressing Modes:\\par\\sa0\\f0\\fs20\\b0 ");
    str.Replace("<pre>", "\\f0 ");
    str.Replace("<small>", "\\f1\\fs16 ");
    str.Replace("|", "\\f1 ");
    str.Replace("\n", "\\par ");

    strText += str;
    strText += "}";

#if 0
    std::string strOld;
    m_wndHelp.GetWindowText(strOld);

    if (strOld != strText)
        m_wndHelp.SetWindowText(strText);

    CRect rect(0, 0, 0, 0);
    m_wndHelp.GetRect(rect);
#endif
}


void CDynamicHelp::DisplayHelp(const std::string& strLine, int nWordStart, int nWordEnd)
{
    UNUSED(strLine);
    UNUSED(nWordStart);
    UNUSED(nWordEnd);

#if 0
    std::string strHelp;
    const char* pcszHeader = 0;

    if (strLine.IsEmpty() || nWordStart >= nWordEnd)
        ;
    else if (nWordStart == 0)	// no instruction can start here; also almost all directives are prohibited in the first column
    {
        // test io_area only
        //
    }
    else
    {
        int nComment = strLine.Find(';');

        if (nComment < 0 || nComment > nWordEnd)	// not inside a comment?
        {
            std::string strWord = strLine.Mid(nWordStart, nWordEnd - nWordStart);

            extern int MatchingDirectives(const std::string& strWord, std::string& strOut);
            std::string GetDirectiveDesc(const std::string& strDirective);

            extern int MatchingInstructions(const std::string& strWord, std::string& strResult);
            std::string GetInstructionDesc(const std::string& strInstruction);

            int nMatching = MatchingInstructions(strWord, strHelp);

            if (nMatching == 1)
                strHelp = GetInstructionDesc(strHelp), pcszHeader = "Instruction";
            else if (nMatching > 1)
                pcszHeader = "Instructions";
            else
            {
                int nMatching = MatchingDirectives(strWord, strHelp);
                if (nMatching == 1)
                    strHelp = GetDirectiveDesc(strHelp), pcszHeader = _T("Directive");
                else if (nMatching > 1)
                    pcszHeader = _T("Directives");
            }
        }
    }

    SetContextHelp(strHelp, pcszHeader);
#endif
}

void CDynamicHelp::OnBarStyleChange(uint32_t dwOldStyle, uint32_t dwNewStyle)
{
    UNUSED(dwOldStyle);
    UNUSED(dwNewStyle);

    //PostMessage(WM_USER);
}

LRESULT CDynamicHelp::OnDelayedResize(WPARAM, LPARAM)
{
    Resize();
    return 0;
}

void CDynamicHelp::OnCloseWnd()
{
#if 0
    if (wxFrame* pFrameWnd = GetDockingFrame())
        pFrameWnd->ShowControlBar(this, false, true);
#endif
}
