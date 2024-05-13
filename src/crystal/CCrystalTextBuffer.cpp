////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextBuffer.cpp
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Implementation of the CCrystalTextBuffer class, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	17-Feb-99
//	+	FIX: unnecessary 'HANDLE' in CCrystalTextBuffer::SaveToFile
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	21-Feb-99
//		Paul Selormey, James R. Twine:
//	+	FEATURE: description for Undo/Redo actions
//	+	FEATURE: multiple MSVC-like bookmarks
//	+	FEATURE: 'Disable backspace at beginning of line' option
//	+	FEATURE: 'Disable drag-n-drop editing' option
//
//	+	FEATURE: changed layout of SUndoRecord. Now takes less memory
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <malloc.h>
#include "editcmd.h"
#include "CCrystalTextBuffer.h"
#include "CCrystalTextView.h"

//	Line allocation granularity
#define CHAR_ALIGN 16
#define ALIGN_BUF_SIZE(size) ((size) / CHAR_ALIGN) * CHAR_ALIGN + CHAR_ALIGN;

#define UNDO_BUF_SIZE 1024

const char crlf[] = "\r\n";

#if 0
static const char *crlfs[] =
{
    "\x0d\x0a", // DOS/Windows style
    "\x0a",     // UNIX style
    "\x0d"      // Macintosh style
};
#endif

#ifdef _DEBUG
#define _ADVANCED_BUGCHECK	1
#endif

/////////////////////////////////////////////////////////////////////////////
// CCrystalTextBuffer::SUndoRecord

void CCrystalTextBuffer::SUndoRecord::SetText(const std::string &text)
{
    m_pszText = nullptr;

    if (text.size() >> 0)
    {
        m_pszText = new char[(text.size() + 1) * sizeof(char)];
        strcpy(m_pszText, text.c_str());
    }
}

void CCrystalTextBuffer::SUndoRecord::FreeText()
{
#if REWRITE_TO_WX_WIDGET
    if (HIWORD((DWORD) m_pszText) != 0)
        delete m_pszText;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CCrystalTextBuffer::CUpdateContext

void CCrystalTextBuffer::CInsertContext::RecalcPoint(wxPoint &ptPoint)
{
    ASSERT((m_ptEnd.y > m_ptStart.y) || ((m_ptEnd.y == m_ptStart.y) && (m_ptEnd.x >= m_ptStart.x)));

    if (ptPoint.y < m_ptStart.y)
        return;

    if (ptPoint.y > m_ptStart.y)
    {
        ptPoint.y += (m_ptEnd.y - m_ptStart.y);
        return;
    }

    if (ptPoint.x <= m_ptStart.x)
        return;

    ptPoint.y += (m_ptEnd.y - m_ptStart.y);
    ptPoint.x = m_ptEnd.x + (ptPoint.x - m_ptStart.x);
}

void CCrystalTextBuffer::CDeleteContext::RecalcPoint(wxPoint &ptPoint)
{
    ASSERT((m_ptEnd.y > m_ptStart.y) || ((m_ptEnd.y == m_ptStart.y) && (m_ptEnd.x >= m_ptStart.x)));

    if (ptPoint.y < m_ptStart.y)
        return;

    if (ptPoint.y > m_ptEnd.y)
    {
        ptPoint.y -= (m_ptEnd.y - m_ptStart.y);
        return;
    }

    if (ptPoint.y == m_ptEnd.y && ptPoint.x >= m_ptEnd.x)
    {
        ptPoint.y = m_ptStart.y;
        ptPoint.x = m_ptStart.x + (ptPoint.x - m_ptEnd.x);
        return;
    }

    if (ptPoint.y == m_ptStart.y)
    {
        if (ptPoint.x > m_ptStart.x)
            ptPoint.x = m_ptStart.x;
        return;
    }

    ptPoint = m_ptStart;
}

/////////////////////////////////////////////////////////////////////////////
// CCrystalTextBuffer

CCrystalTextBuffer::CCrystalTextBuffer()
{
    m_bInit = FALSE;
    m_bReadOnly = FALSE;
    m_bModified = FALSE;
    m_bCreateBackupFile = FALSE;
    m_nUndoPosition = 0;
}

CCrystalTextBuffer::~CCrystalTextBuffer()
{
    ASSERT(! m_bInit);			//	You must call FreeAll() before deleting the object
}

#if REWRITE_TO_WX_WIDGET

BEGIN_MESSAGE_MAP(CCrystalTextBuffer, CCmdTarget)
    //{{AFX_MSG_MAP(CCrystalTextBuffer)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CCrystalTextBuffer message handlers

void CCrystalTextBuffer::InsertLine(const std::string &line, int length /*= -1*/, int nPosition /*= -1*/)
{
    if (length == -1)
        length = line.size();

    SLineInfo li;
    li.m_nLength = length;
    li.m_nMax = ALIGN_BUF_SIZE(li.m_nLength);

    ASSERT(li.m_nMax >= li.m_nLength);

    if (li.m_nMax > 0)
        li.m_pcLine = new char[li.m_nMax];

    if (li.m_nLength > 0)
        memcpy(li.m_pcLine, line.c_str(), sizeof(char) * li.m_nLength);

    if (nPosition == -1)
        m_aLines.push_back(li);
    else
    {
        auto idx = m_aLines.begin() + nPosition;
        m_aLines.insert(idx, li);
    }

#ifdef _DEBUG
#if REWRITE_TO_WX_WIDGETS
    size_t nLines = m_aLines.size();

    if (nLines % 5000 == 0)
        TRACE1("%d lines loaded!\n", nLines);
# endif
#endif
}

void CCrystalTextBuffer::AppendLine(int nLineIndex, const std::string &chars, int length /*= -1*/)
{
    if (length == -1)
        length = chars.size();

    if (length == 0)
        return;

    SLineInfo &li = m_aLines[nLineIndex];
    int bufNeeded = li.m_nLength + length;

    if (bufNeeded > li.m_nMax)
    {
        li.m_nMax = ALIGN_BUF_SIZE(bufNeeded);

        ASSERT(li.m_nMax >= li.m_nLength + length);

        char *pcNewBuf = new char[li.m_nMax];

        if (li.m_nLength > 0)
            memcpy(pcNewBuf, li.m_pcLine, sizeof(char) * li.m_nLength);

        delete[] li.m_pcLine;
        li.m_pcLine = pcNewBuf;
    }

    memcpy(li.m_pcLine + li.m_nLength, chars.c_str(), sizeof(char) * length);
    li.m_nLength += length;

    ASSERT(li.m_nLength <= li.m_nMax);
}

void CCrystalTextBuffer::FreeAll()
{
    //	Free text
    size_t nCount = m_aLines.size();

    for (size_t i = 0; i < nCount; i++)
    {
        if (m_aLines[i].m_nMax > 0)
            delete m_aLines[i].m_pcLine;
    }
    m_aLines.clear();

    //	Free undo buffer
    size_t bufSize = m_aUndoBuf.size();

    for (size_t i = 0; i < bufSize; i++)
        m_aUndoBuf[i].FreeText();

    m_aUndoBuf.clear();

    m_bInit = false;
}

bool CCrystalTextBuffer::InitNew(int nCrlfStyle /*= CRLF_STYLE_DOS*/)
{
    ASSERT(!m_bInit);
    ASSERT(m_aLines.empty());
    ASSERT(nCrlfStyle >= 0 && nCrlfStyle <= 2);
    InsertLine("");
    m_bInit = true;
    m_bReadOnly = false;
    m_nCRLFMode = nCrlfStyle;
    m_bModified = false;
    m_nSyncPosition = m_nUndoPosition = 0;
    m_bUndoGroup = m_bUndoBeginGroup = FALSE;
    m_nUndoBufSize = UNDO_BUF_SIZE;
    ASSERT(m_aUndoBuf.empty());
    UpdateViews(nullptr, nullptr, UPDATE_RESET);

    return true;
}

bool CCrystalTextBuffer::GetReadOnly() const
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!
    return m_bReadOnly;
}

void CCrystalTextBuffer::SetReadOnly(bool bReadOnly /*= true */)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!
    m_bReadOnly = bReadOnly;
}

bool CCrystalTextBuffer::LoadFromFile(const std::string &fileName, int nCrlfStyle /*= CRLF_STYLE_AUTOMATIC*/)
{
    UNUSED(fileName);
    UNUSED(nCrlfStyle);

#if REWRITE_TO_WX_WIDGET
    ASSERT(!m_bInit);
    ASSERT(m_aLines.GetSize() == 0);

    HANDLE hFile = NULL;
    int nCurrentMax = 256;
    char *pcLineBuf = new char[nCurrentMax];

    BOOL bSuccess = FALSE;
    __try
    {
        DWORD dwFileAttributes = ::GetFileAttributes(pszFileName);
        if (dwFileAttributes == (DWORD) -1)
            __leave;

        hFile = ::CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            __leave;

        int nCurrentLength = 0;

        const DWORD dwBufSize = 32768;
        char *pcBuf = (char *) _alloca(dwBufSize);
        DWORD dwCurSize;
        if (! ::ReadFile(hFile, pcBuf, dwBufSize, &dwCurSize, NULL))
            __leave;

        if (nCrlfStyle == CRLF_STYLE_AUTOMATIC)
        {
            //	Try to determine current CRLF mode
            DWORD I= 0;
            for (I = 0; I < dwCurSize; I ++)
            {
                if (pcBuf[I] == _T('\x0a'))
                    break;
                if (pcBuf[I] == _T('\x0d'))
                    break;
            }
            if (I == dwCurSize)
            {
                //	By default (or in the case of empty file), set DOS style
                nCrlfStyle = CRLF_STYLE_DOS;
            }
            else
            {
                if (pcBuf[I] == _T('\x0d') && I < dwCurSize - 1)
                {
                    if (pcBuf[I + 1] == _T('\x0a'))		// 0xd, 0xa?
                        nCrlfStyle = CRLF_STYLE_DOS;
                    else
                        nCrlfStyle = CRLF_STYLE_MAC;	// 0xd alone is Mac
                }
                else if (pcBuf[I] == _T('\x0a'))
                {
                    // this is Unix; there's no 0xa, 0xd combination in normal text files
                    nCrlfStyle = CRLF_STYLE_UNIX;
                }
                /*
                				//	Otherwise, analyse the first occurance of line-feed character
                				if (I > 0 && pcBuf[I - 1] == _T('\x0d'))
                				{
                					nCrlfStyle = CRLF_STYLE_DOS;
                				}
                				else
                				{
                					if (I < dwCurSize - 1 && pcBuf[I + 1] == _T('\x0d'))
                						nCrlfStyle = CRLF_STYLE_UNIX;
                					else
                						nCrlfStyle = CRLF_STYLE_MAC;
                				}
                */
            }
        }

        ASSERT(nCrlfStyle >= 0 && nCrlfStyle <= 2);
        m_nCRLFMode = nCrlfStyle;
        const char *crlf = crlfs[nCrlfStyle];

        m_aLines.SetSize(0, 4096);

        DWORD dwBufPtr = 0;
        int nCrlfPtr = 0;
        USES_CONVERSION;
        while (dwBufPtr < dwCurSize)
        {
            int c = pcBuf[dwBufPtr];
            dwBufPtr ++;
            if (dwBufPtr == dwCurSize && dwCurSize == dwBufSize)
            {
                if (! ::ReadFile(hFile, pcBuf, dwBufSize, &dwCurSize, NULL))
                    __leave;
                dwBufPtr = 0;
            }

            pcLineBuf[nCurrentLength] = (char) c;
            nCurrentLength ++;
            if (nCurrentLength == nCurrentMax)
            {
                //	Reallocate line buffer
                nCurrentMax += 256;
                char *pcNewBuf = new char[nCurrentMax];
                memcpy(pcNewBuf, pcLineBuf, nCurrentLength);
                delete pcLineBuf;
                pcLineBuf = pcNewBuf;
            }

            if ((char) c == crlf[nCrlfPtr])
            {
                nCrlfPtr ++;
                if (crlf[nCrlfPtr] == 0)
                {
                    pcLineBuf[nCurrentLength - nCrlfPtr] = 0;
                    InsertLine(A2T(pcLineBuf));
                    nCurrentLength = 0;
                    nCrlfPtr = 0;
                }
            }
            else
                nCrlfPtr = 0;
        }

        pcLineBuf[nCurrentLength] = 0;
        InsertLine(A2T(pcLineBuf));

        ASSERT(m_aLines.GetSize() > 0);		//	At least one empty line must present

        m_bInit = TRUE;
        m_bReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
        m_bModified = FALSE;
        m_bUndoGroup = m_bUndoBeginGroup = FALSE;
        m_nUndoBufSize = UNDO_BUF_SIZE;
        m_nSyncPosition = m_nUndoPosition = 0;
        ASSERT(m_aUndoBuf.GetSize() == 0);
        bSuccess = TRUE;

        UpdateViews(NULL, NULL, UPDATE_RESET);
    }
    __finally
    {
        if (pcLineBuf != NULL)
            delete pcLineBuf;
        if (hFile != NULL)
            ::CloseHandle(hFile);
    }
    return bSuccess;
#endif

    return false;
}

bool CCrystalTextBuffer::SaveToFile(const std::string &fileName, int nCrlfStyle /*= CRLF_STYLE_AUTOMATIC*/, bool bClearModifiedFlag /*= true*/)
{
    UNUSED(fileName);
    UNUSED(nCrlfStyle);
    UNUSED(bClearModifiedFlag);

#if REWRITE_TO_WX_WIDGET

    ASSERT(nCrlfStyle == CRLF_STYLE_AUTOMATIC || nCrlfStyle == CRLF_STYLE_DOS||
           nCrlfStyle == CRLF_STYLE_UNIX || nCrlfStyle == CRLF_STYLE_MAC);
    ASSERT(m_bInit);
    HANDLE hTempFile = INVALID_HANDLE_VALUE;
    HANDLE hSearch = INVALID_HANDLE_VALUE;
    TCHAR szTempFileDir[_MAX_PATH + 1];
    TCHAR szTempFileName[_MAX_PATH + 1];
    TCHAR szBackupFileName[_MAX_PATH + 1];
    BOOL bSuccess = FALSE;
    __try
    {
        TCHAR drive[_MAX_PATH], dir[_MAX_PATH], name[_MAX_PATH], ext[_MAX_PATH];
#ifdef _UNICODE
        _wsplitpath(pszFileName, drive, dir, name, ext);
#else
        _splitpath(pszFileName, drive, dir, name, ext);
#endif
        lstrcpy(szTempFileDir, drive);
        lstrcat(szTempFileDir, dir);
        lstrcpy(szBackupFileName, pszFileName);
        lstrcat(szBackupFileName, _T(".bak"));

        if (::GetTempFileName(szTempFileDir, _T("CRE"), 0, szTempFileName) == 0)
            __leave;

        hTempFile = ::CreateFile(szTempFileName, GENERIC_WRITE, 0, NULL,
                                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTempFile == INVALID_HANDLE_VALUE)
            __leave;

        if (nCrlfStyle == CRLF_STYLE_AUTOMATIC)
            nCrlfStyle = m_nCRLFMode;

        ASSERT(nCrlfStyle >= 0 && nCrlfStyle <= 2);
        const char *pszCRLF = crlfs[nCrlfStyle];
        int nCRLFLength = strlen(pszCRLF);

        int nLineCount = m_aLines.GetSize();
        USES_CONVERSION;
        for (int nLine = 0; nLine < nLineCount; nLine ++)
        {
            int nLength = m_aLines[nLine].m_nLength;
            DWORD dwWrittenBytes;
            if (nLength > 0)
            {
                if (! ::WriteFile(hTempFile, T2A(m_aLines[nLine].m_pcLine), nLength, &dwWrittenBytes, NULL))
                    __leave;
                if (nLength != (int) dwWrittenBytes)
                    __leave;
            }
            if (nLine < nLineCount - 1)	//	Last line must not end with CRLF
            {
                if (! ::WriteFile(hTempFile, pszCRLF, nCRLFLength, &dwWrittenBytes, NULL))
                    __leave;
                if (nCRLFLength != (int) dwWrittenBytes)
                    __leave;
            }
        }
        ::CloseHandle(hTempFile);
        hTempFile = INVALID_HANDLE_VALUE;

        if (m_bCreateBackupFile)
        {
            WIN32_FIND_DATA wfd;
            hSearch = ::FindFirstFile(pszFileName, &wfd);
            if (hSearch != INVALID_HANDLE_VALUE)
            {
                //	File exist - create backup file
                ::DeleteFile(szBackupFileName);
                if (! ::MoveFile(pszFileName, szBackupFileName))
                    __leave;
                ::FindClose(hSearch);
                hSearch = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            ::DeleteFile(pszFileName);
        }

        //	Move temporary file to target name
        if (! ::MoveFile(szTempFileName, pszFileName))
            __leave;

        if (bClearModifiedFlag)
        {
            SetModified(FALSE);
            m_nSyncPosition = m_nUndoPosition;
        }
        bSuccess = TRUE;
    }
    __finally
    {
        if (hSearch != INVALID_HANDLE_VALUE)
            ::FindClose(hSearch);
        if (hTempFile != INVALID_HANDLE_VALUE)
            ::CloseHandle(hTempFile);
        ::DeleteFile(szTempFileName);
    }
    
    return bSuccess;
#endif

    return false;
}

int CCrystalTextBuffer::GetCRLFMode()
{
    return m_nCRLFMode;
}

void CCrystalTextBuffer::SetCRLFMode(int nCRLFMode)
{
    ASSERT(nCRLFMode == CRLF_STYLE_DOS||
           nCRLFMode == CRLF_STYLE_UNIX ||
           nCRLFMode == CRLF_STYLE_MAC);
    m_nCRLFMode = nCRLFMode;
}

size_t CCrystalTextBuffer::GetLineCount()
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    return m_aLines.size();
}

int CCrystalTextBuffer::GetLineLength(int line)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!
    
    return m_aLines[line].m_nLength;
}

const char *CCrystalTextBuffer::GetLineChars(int line)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    return m_aLines[line].m_pcLine;
}

uint32_t CCrystalTextBuffer::GetLineFlags(int line)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    return m_aLines[line].m_flags;
}

static int FlagToIndex(uint32_t flag)
{
    int idx = 0;

    while ((flag & 1) == 0)
    {
        flag = flag >> 1;

        idx++;

        if (idx == 32)
            return -1;
    }

    flag = flag & 0xFFFFFFFE;

    if (flag != 0)
        return -1;

    return idx;
}

int CCrystalTextBuffer::FindLineWithFlag(uint32_t flag)
{
    size_t size = m_aLines.size();

    for (size_t l = 0; l < size; l ++)
    {
        if ((m_aLines[l].m_flags & flag) != 0)
            return l;
    }

    return -1;
}

int CCrystalTextBuffer::GetLineWithFlag(uint32_t flag)
{
    int flagIdx = ::FlagToIndex(flag);

    if (flagIdx < 0)
    {
        ASSERT(false); // Invalid flag passed in
        return -1;
    }

    return FindLineWithFlag(flag);
}

void CCrystalTextBuffer::SetLineFlag(int nLine, uint32_t flag, bool set, bool bRemoveFromPreviousLine /*= true */)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    int flagIdx = ::FlagToIndex(flag);
    if (flagIdx < 0)
    {
        ASSERT(false); // Invalid flag passed in
        return;
    }

    if (nLine == -1)
    {
        ASSERT(!set);
        nLine = FindLineWithFlag(flag);

        if (nLine == -1)
            return;

        bRemoveFromPreviousLine = false;
    }

    uint32_t newFlags = m_aLines[nLine].m_flags;

    if (set)
        newFlags = newFlags | flag;
    else
        newFlags = newFlags & ~flag;

    if (m_aLines[nLine].m_flags != newFlags)
    {
        if (bRemoveFromPreviousLine)
        {
            int nPrevLine = FindLineWithFlag(flag);

            if (set)
            {
                if (nPrevLine >= 0)
                {
                    ASSERT((m_aLines[nPrevLine].m_flags & flag) != 0);
                    m_aLines[nPrevLine].m_flags &= ~flag;
                    UpdateViews(NULL, NULL, UPDATE_SINGLELINE | UPDATE_FLAGSONLY, nPrevLine);
                }
            }
            else
            {
                ASSERT(nPrevLine == nLine);
            }
        }

        m_aLines[nLine].m_flags = newFlags;
        UpdateViews(NULL, NULL, UPDATE_SINGLELINE | UPDATE_FLAGSONLY, nLine);
    }
}

void CCrystalTextBuffer::SetLinesFlags(int nLineFrom, int nLineTo, uint32_t addFlags, uint32_t removeFlags)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    for (int nLine= nLineFrom; nLine <= nLineTo; ++nLine)
    {
        m_aLines[nLine].m_flags |= addFlags;
        m_aLines[nLine].m_flags &= ~removeFlags;
    }

    UpdateViews(NULL, NULL, UPDATE_FLAGSONLY, 0);
}

void CCrystalTextBuffer::GetText(int nStartLine, int nStartChar, int nEndLine, int nEndChar, std::string &text, const char *crlfStr /*= NULL*/)
{
    UNUSED(nStartLine);
    UNUSED(nStartChar);
    UNUSED(nEndLine);
    UNUSED(nEndChar);
    UNUSED(text);
    UNUSED(crlfStr);

#if REWRITE_TO_WX_WIDGET
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    ASSERT(nStartLine >= 0 && nStartLine < m_aLines.GetSize());
    ASSERT(nStartChar >= 0 && nStartChar <= m_aLines[nStartLine].m_nLength);
    ASSERT(nEndLine >= 0 && nEndLine < m_aLines.GetSize());
    ASSERT(nEndChar >= 0 && nEndChar <= m_aLines[nEndLine].m_nLength);
    ASSERT(nStartLine < nEndLine || nStartLine == nEndLine && nStartChar < nEndChar);

    if (crlf == NULL)
        crlf = crlf;

    int crlfLen = strlen(crlf);
    ASSERT(crlfLen > 0);

    int bufSize = 0;

    for (int l = nStartLine; l <= nEndLine; l++)
    {
        bufSize += m_aLines[l].m_nLength;
        bufSize += crlfLen;
    }

    text.reserve(bufSize);

    char *pszBuf = text.c_str();
    char *pszCurPos = pszBuf;

    if (nStartLine < nEndLine)
    {
        int count = m_aLines[nStartLine].m_nLength - nStartChar;

        if (count > 0)
        {
            memcpy(pszBuf, m_aLines[nStartLine].m_pcLine + nStartChar, sizeof(char) * count);
            pszBuf += count;
        }

        memcpy(pszBuf, crlf, sizeof(char) * crlfLen);
        pszBuf += crlfLen;

        for (int i = nStartLine + 1; i < nEndLine; i++)
        {
            count = m_aLines[i].m_nLength;

            if (count > 0)
            {
                memcpy(pszBuf, m_aLines[i].m_pcLine, sizeof(char) * count);
                pszBuf += count;
            }

            memcpy(pszBuf, crlf, sizeof(char) * crlfLen);
            pszBuf += crlfLen;
        }
        if (nEndChar > 0)
        {
            memcpy(pszBuf, m_aLines[nEndLine].m_pcLine, sizeof(char) * nEndChar);
            pszBuf += nEndChar;
        }
    }
    else
    {
        int count = nEndChar - nStartChar;
        memcpy(pszBuf, m_aLines[nStartLine].m_pcLine + nStartChar, sizeof(char) * count);
        pszBuf += count;
    }

    pszBuf[0] = 0;
    text.clear();

    //text.ReleaseBuffer();
    //text.FreeExtra();
#endif
}

void CCrystalTextBuffer::GetText(std::string& text, const char *crlfStr/* = NULL*/)
{
    text.clear();

    size_t size = m_aLines.size();
    if (size == 0)
        return;

    int endLine = size - 1;
    GetText(0, 0, endLine, m_aLines[endLine].m_nLength, text, crlfStr);
}

void CCrystalTextBuffer::AddView(CCrystalTextView *pView)
{
    m_lpViews.push_back(pView);
}

void CCrystalTextBuffer::RemoveView(CCrystalTextView *pView)
{
    UNUSED(pView);

#if REWRITE_TO_WX_WIDGETS
    POSITION pos = m_lpViews.GetHeadPosition();

    //while (pos != NULL)
    while (pos != 0)
    {
        POSITION thispos = pos;
        CCrystalTextView *pvw = m_lpViews.GetNext(pos);

        if (pvw == pView)
        {
            m_lpViews.RemoveAt(thispos);
            return;
        }
    }
#endif

    ASSERT(false);
}

void CCrystalTextBuffer::UpdateViews(CCrystalTextView *pSource, CUpdateContext *pContext, uint32_t updateFlags, int nLineIndex /*= -1*/)
{
    UNUSED(pSource);
    UNUSED(pContext);
    UNUSED(updateFlags);
    UNUSED(nLineIndex);

#if REWRITE_TO_WX_WIDGET
    POSITION pos = m_lpViews.GetHeadPosition();

    //while (pos != NULL)
    while (pos != 0)
    {
        CCrystalTextView *pView = m_lpViews.GetNext(pos);
        pView->UpdateView(pSource, pContext, updateFlags, nLineIndex);
    }
#endif
}

bool CCrystalTextBuffer::InternalDeleteText(CCrystalTextView *pSource, int nStartLine, int nStartChar, int nEndLine, int nEndChar)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    ASSERT((nStartLine >= 0) && (static_cast<std::size_t>(nStartLine) < m_aLines.size()));
    ASSERT((nStartChar >= 0) && (nStartChar <= m_aLines[nStartLine].m_nLength));
    ASSERT((nEndLine >= 0) && (static_cast<std::size_t>(nEndLine) < m_aLines.size()));
    ASSERT((nEndChar >= 0) && (nEndChar <= m_aLines[nEndLine].m_nLength));
    ASSERT((nStartLine < nEndLine) || ((nStartLine == nEndLine) && (nStartChar < nEndChar)));

    if (m_bReadOnly)
        return false;

    CDeleteContext context;
    context.m_ptStart.y = nStartLine;
    context.m_ptStart.x = nStartChar;
    context.m_ptEnd.y = nEndLine;
    context.m_ptEnd.x = nEndChar;

    if (nStartLine == nEndLine)
    {
        SLineInfo &li = m_aLines[nStartLine];

        if (nEndChar < li.m_nLength)
        {
            memcpy(li.m_pcLine + nStartChar, li.m_pcLine + nEndChar, sizeof(char) * (li.m_nLength - nEndChar));
        }

        li.m_nLength -= (nEndChar - nStartChar);

        UpdateViews(pSource, &context, UPDATE_SINGLELINE | UPDATE_HORZRANGE, nStartLine);
    }
    else
    {
        int restCount = m_aLines[nEndLine].m_nLength - nEndChar;
        char *restChars = nullptr;

        if (restCount > 0)
        {
            restChars = new char[restCount];
            memcpy(restChars, m_aLines[nEndLine].m_pcLine + nEndChar, restCount * sizeof(char));
        }

        //int nDelCount = nEndLine - nStartLine;

        for (int l = nStartLine + 1; l <= nEndLine; l++)
            delete m_aLines[l].m_pcLine;

#if REWRITE_FOR_WX_WIDGET
        m_aLines.RemoveAt(nStartLine + 1, nDelCount);
#endif

        //	nEndLine is no more valid
        m_aLines[nStartLine].m_nLength = nStartChar;

        if (restCount > 0)
        {
            AppendLine(nStartLine, restChars, restCount);
            delete[] restChars;
        }

        UpdateViews(pSource, &context, UPDATE_HORZRANGE | UPDATE_VERTRANGE, nStartLine);
    }

    if (!m_bModified)
        SetModified(true);

    return true;
}

bool CCrystalTextBuffer::InternalInsertText(CCrystalTextView *pSource, int nLine, int nPos, const std::string &text, int &nEndLine, int &nEndChar)
{
    ASSERT(m_bInit); // Text buffer not yet initialized.
    // You must call InitNew() or LoadFromFile() first!

    ASSERT((nLine >= 0) && (static_cast<std::size_t>(nLine) < m_aLines.size()));
    ASSERT((nPos >= 0) && (nPos <= m_aLines[nLine].m_nLength));

    if (m_bReadOnly)
        return false;

    CInsertContext context;
    context.m_ptStart.x = nPos;
    context.m_ptStart.y = nLine;

    int restCount = m_aLines[nLine].m_nLength - nPos;
    char *restChars = NULL;

    if (restCount > 0)
    {
        restChars = new char[restCount];
        memcpy(restChars, m_aLines[nLine].m_pcLine + nPos, restCount * sizeof(char));
        m_aLines[nLine].m_nLength = nPos;
    }

    const char *ptr = text.c_str();
    int currentLine = nLine;
    bool newLines = false;
    int textPos;

    for (;;)
    {
        textPos = 0;

        while (ptr[textPos] != 0 && ptr[textPos] != '\r')
            textPos++;

        if (currentLine == nLine)
        {
            AppendLine(nLine, ptr, textPos);
        }
        else
        {
            InsertLine(ptr, textPos, currentLine);
            newLines = true;
        }

        if (ptr[textPos] == 0)
        {
            nEndLine = currentLine;
            nEndChar = m_aLines[currentLine].m_nLength;
            AppendLine(currentLine, restChars, restCount);
            break;
        }

        currentLine++;
        textPos++;

        if (ptr[textPos] == '\n')
        {
            textPos++;
        }
        else
        {
            ASSERT(false); // Invalid line-end format passed
        }

        ptr += textPos;
    }

    if (restChars != NULL)
        delete[] restChars;

    context.m_ptEnd.x = nEndChar;
    context.m_ptEnd.y = nEndLine;

    if (newLines)
        UpdateViews(pSource, &context, UPDATE_HORZRANGE | UPDATE_VERTRANGE, nLine);
    else
        UpdateViews(pSource, &context, UPDATE_SINGLELINE | UPDATE_HORZRANGE, nLine);

    if (! m_bModified)
        SetModified(true);

    return true;
}

bool CCrystalTextBuffer::CanUndo()
{
    ASSERT((m_nUndoPosition >= 0) && (static_cast<std::size_t>(m_nUndoPosition) <= m_aUndoBuf.size()));
    return m_nUndoPosition > 0;
}

bool CCrystalTextBuffer::CanRedo()
{
    ASSERT((m_nUndoPosition >= 0) && (static_cast<std::size_t>(m_nUndoPosition) <= m_aUndoBuf.size()));
    return static_cast<std::size_t>(m_nUndoPosition) < m_aUndoBuf.size();
}

POSITION CCrystalTextBuffer::GetUndoDescription(std::string &desc, POSITION pos /*= NULL*/)
{
    ASSERT(CanUndo()); // Please call CanUndo() first

    ASSERT((m_aUndoBuf[0].m_flags & UNDO_BEGINGROUP) != 0);

    int position;
    //if (pos == NULL)
    if (pos == 0)
    {
        // Start from beginning
        position = m_nUndoPosition;
    }
    else
    {
        position = (int)pos;
        ASSERT(position > 0 && position < m_nUndoPosition);
        ASSERT((m_aUndoBuf[position].m_flags & UNDO_BEGINGROUP) != 0);
    }

    // Advance to next undo group
    position--;
    while ((m_aUndoBuf[position].m_flags & UNDO_BEGINGROUP) == 0)
        position --;

    // Read description
    if (!GetActionDescription(m_aUndoBuf[position].m_action, desc))
        desc.clear(); // Use empty string as description

    // Now, if we stop at zero position, this will be the last action,
    // since we return (POSITION) nPosition
    return (POSITION)position;
}

POSITION CCrystalTextBuffer::GetRedoDescription(std::string &desc, POSITION pos /*= NULL*/)
{
    ASSERT(CanRedo()); // Please call CanRedo() before!

    ASSERT((m_aUndoBuf[0].m_flags & UNDO_BEGINGROUP) != 0);
    ASSERT((m_aUndoBuf[m_nUndoPosition].m_flags & UNDO_BEGINGROUP) != 0);

    int position;
    //if (pos == NULL)
    if (pos == 0)
    {
        // Start from beginning
        position = m_nUndoPosition;
    }
    else
    {
        position = (int)pos;
        ASSERT(position > m_nUndoPosition);
        ASSERT((m_aUndoBuf[position].m_flags & UNDO_BEGINGROUP) != 0);
    }

    //	Read description
    if (!GetActionDescription(m_aUndoBuf[position].m_action, desc))
        desc.clear(); // Use empty string as description

    // Advance to next undo group
    position++;
    while ((static_cast<std::size_t>(position) < m_aUndoBuf.size()) && 
           (m_aUndoBuf[position].m_flags & UNDO_BEGINGROUP) == 0)
    {
        position--;
    }

    if (static_cast<std::size_t>(position) >= m_aUndoBuf.size())
        return 0; //NULL; // No more redo actions!

    return (POSITION)position;
}

bool CCrystalTextBuffer::Undo(wxPoint &ptCursorPos)
{
    ASSERT(CanUndo());

    ASSERT((m_aUndoBuf[0].m_flags & UNDO_BEGINGROUP) != 0);

    for (;;)
    {
        m_nUndoPosition --;
        const SUndoRecord &ur = m_aUndoBuf[m_nUndoPosition];

        if (ur.m_flags & UNDO_INSERT)
        {
#ifdef _ADVANCED_BUGCHECK
            // Try to ensure that we undoing correctly...
            // Just compare the text as it was before Undo operation
            std::string text;
            GetText(ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x, text);
            ASSERT(text == ur.GetText());
#endif
            VERIFY(InternalDeleteText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x));
            ptCursorPos = ur.m_ptStartPos;
        }
        else
        {
            int nEndLine, nEndChar;
            VERIFY(InternalInsertText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.GetText(), nEndLine, nEndChar));
#ifdef _ADVANCED_BUGCHECK
            ASSERT(ur.m_ptEndPos.y == nEndLine);
            ASSERT(ur.m_ptEndPos.x == nEndChar);
#endif
            ptCursorPos = ur.m_ptEndPos;
        }

        if (ur.m_flags & UNDO_BEGINGROUP)
            break;
    }

    if (m_bModified && m_nSyncPosition == m_nUndoPosition)
        SetModified(false);

    if (! m_bModified && m_nSyncPosition != m_nUndoPosition)
        SetModified(true);

    return true;
}

bool CCrystalTextBuffer::Redo(wxPoint &ptCursorPos)
{
    ASSERT(CanRedo());

    ASSERT((m_aUndoBuf[0].m_flags & UNDO_BEGINGROUP) != 0);
    ASSERT((m_aUndoBuf[m_nUndoPosition].m_flags & UNDO_BEGINGROUP) != 0);

    for (;;)
    {
        const SUndoRecord &ur = m_aUndoBuf[m_nUndoPosition];

        if (ur.m_flags & UNDO_INSERT)
        {
            int nEndLine, nEndChar;
            VERIFY(InternalInsertText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.GetText(), nEndLine, nEndChar));
#ifdef _ADVANCED_BUGCHECK
            ASSERT(ur.m_ptEndPos.y == nEndLine);
            ASSERT(ur.m_ptEndPos.x == nEndChar);
#endif
            ptCursorPos = ur.m_ptEndPos;
        }
        else
        {
#ifdef _ADVANCED_BUGCHECK
            std::string text;
            GetText(ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x, text);
            ASSERT(text == ur.GetText());
#endif
            VERIFY(InternalDeleteText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x));
            ptCursorPos = ur.m_ptStartPos;
        }

        m_nUndoPosition++;

        if (static_cast<std::size_t>(m_nUndoPosition) == m_aUndoBuf.size())
            break;

        if ((m_aUndoBuf[m_nUndoPosition].m_flags & UNDO_BEGINGROUP) != 0)
            break;
    }

    if (m_bModified && m_nSyncPosition == m_nUndoPosition)
        SetModified(false);

    if (! m_bModified && m_nSyncPosition != m_nUndoPosition)
        SetModified(true);

    return true;
}

//	[JRT] Support For Descriptions On Undo/Redo Actions
void CCrystalTextBuffer::AddUndoRecord(bool insert, const wxPoint &ptStartPos, const wxPoint &ptEndPos, const std::string &text, int actionType)
{
    // Forgot to call BeginUndoGroup()?
    ASSERT(m_bUndoGroup);
    ASSERT(m_aUndoBuf.empty() || (m_aUndoBuf[0].m_flags & UNDO_BEGINGROUP) != 0);

    // Strip unnecessary undo records (edit after undo)
    int bufSize = m_aUndoBuf.size();
    if (m_nUndoPosition < bufSize)
    {
        auto start = m_aUndoBuf.end();

        for (int i = m_nUndoPosition; i < bufSize; ++i, --start)
            m_aUndoBuf[i].FreeText();

        //m_aUndoBuf.size(m_nUndoPosition);
        m_aUndoBuf.erase(start, m_aUndoBuf.end());
    }

    // If undo buffer size is close to critical, remove the oldest records
    ASSERT(m_aUndoBuf.size() <= static_cast<std::size_t>(m_nUndoBufSize));
    bufSize = m_aUndoBuf.size();

    if (bufSize >= m_nUndoBufSize)
    {
        auto end = m_aUndoBuf.begin();
        //int index = 0;

        for (;;)
        {
            end->FreeText();
            ++end;
            //m_aUndoBuf[index].FreeText();
            //index++;

            if (end == m_aUndoBuf.end())
                break;

            if ((end->m_flags & UNDO_BEGINGROUP) != 0)
                break;

            //if (index == bufSize || (m_aUndoBuf[index].m_flags & UNDO_BEGINGROUP) != 0)
            //    break;
        }

        //m_aUndoBuf.RemoveAt(0, index);
        m_aUndoBuf.erase(m_aUndoBuf.begin(), end);
    }

    ASSERT(m_aUndoBuf.size() < static_cast<std::size_t>(m_nUndoBufSize));

    // Add new record
    SUndoRecord ur;
    ur.m_flags = insert ? UNDO_INSERT : 0;
    ur.m_action = actionType;

    if (m_bUndoBeginGroup)
    {
        ur.m_flags |= UNDO_BEGINGROUP;
        m_bUndoBeginGroup = false;
    }

    ur.m_ptStartPos = ptStartPos;
    ur.m_ptEndPos = ptEndPos;
    ur.SetText(text);

    m_aUndoBuf.push_back(ur);
    m_nUndoPosition = m_aUndoBuf.size();

    ASSERT(m_aUndoBuf.size() <= static_cast<std::size_t>(m_nUndoBufSize));
}

bool CCrystalTextBuffer::InsertText(CCrystalTextView *pSource, int nLine, int nPos, const std::string &text,
                                    int &nEndLine, int &nEndChar, int action)
{
    if (!InternalInsertText(pSource, nLine, nPos, text, nEndLine, nEndChar))
        return false;

    bool grouped = false;
    if (!m_bUndoGroup)
    {
        BeginUndoGroup();
        grouped = true;
    }

    AddUndoRecord(true, wxPoint(nPos, nLine), wxPoint(nEndChar, nEndLine), text, action);

    if (grouped)
        FlushUndoGroup(pSource);

    return true;
}

bool CCrystalTextBuffer::DeleteText(CCrystalTextView *pSource, int nStartLine, int nStartChar,
                                    int nEndLine, int nEndChar, int action)
{
    std::string textToDelete;
    GetText(nStartLine, nStartChar, nEndLine, nEndChar, textToDelete);

    if (!InternalDeleteText(pSource, nStartLine, nStartChar, nEndLine, nEndChar))
        return false;

    bool grouped = false;
    if (!m_bUndoGroup)
    {
        BeginUndoGroup();
        grouped = true;
    }

    AddUndoRecord(false, wxPoint(nStartChar, nStartLine), wxPoint(nEndChar, nEndLine), textToDelete, action);

    if (grouped)
        FlushUndoGroup(pSource);

    return true;
}

bool CCrystalTextBuffer::GetActionDescription(int action, std::string &desc)
{
    UNUSED(action);
    UNUSED(desc);

#if REWRITE_TO_WX_WIDGET

    HINSTANCE hOldResHandle = AfxGetResourceHandle();
#ifdef CRYSEDIT_RES_HANDLE
    AfxSetResourceHandle(CRYSEDIT_RES_HANDLE);
#else
    if (CCrystalTextView::s_hResourceInst != NULL)
        AfxSetResourceHandle(CCrystalTextView::s_hResourceInst);
#endif
    bool success = false;

    switch (action)
    {
    case CE_ACTION_UNKNOWN:
        success = desc.LoadString(IDS_EDITOP_UNKNOWN);
        break;

    case CE_ACTION_PASTE:
        success = desc.LoadString(IDS_EDITOP_PASTE);
        break;

    case CE_ACTION_DELSEL:
        success = desc.LoadString(IDS_EDITOP_DELSELECTION);
        break;

    case CE_ACTION_CUT:
        success = desc.LoadString(IDS_EDITOP_CUT);
        break;

    case CE_ACTION_TYPING:
        success = desc.LoadString(IDS_EDITOP_TYPING);
        break;

    case CE_ACTION_BACKSPACE:
        success = desc.LoadString(IDS_EDITOP_BACKSPACE);
        break;

    case CE_ACTION_INDENT:
        success = desc.LoadString(IDS_EDITOP_INDENT);
        break;

    case CE_ACTION_DRAGDROP:
        success = desc.LoadString(IDS_EDITOP_DRAGDROP);
        break;

    case CE_ACTION_REPLACE:
        success = desc.LoadString(IDS_EDITOP_REPLACE);
        break;

    case CE_ACTION_DELETE:
        success = desc.LoadString(IDS_EDITOP_DELETE);
        break;

    case CE_ACTION_AUTOINDENT:
        success = desc.LoadString(IDS_EDITOP_AUTOINDENT);
        break;
    }

    AfxSetResourceHandle(hOldResHandle);

    return success;
#endif
    return false;
}

void CCrystalTextBuffer::SetModified(bool bModified /*= TRUE*/)
{
    m_bModified = bModified;
}

void CCrystalTextBuffer::BeginUndoGroup(bool bMergeWithPrevious /*= FALSE*/)
{
    ASSERT(!m_bUndoGroup);
    m_bUndoGroup = true;
    m_bUndoBeginGroup = m_nUndoPosition == 0 || !bMergeWithPrevious;
}

void CCrystalTextBuffer::FlushUndoGroup(CCrystalTextView *pSource)
{
    ASSERT(m_bUndoGroup);

    if (pSource != NULL)
    {
        ASSERT(static_cast<std::size_t>(m_nUndoPosition) == m_aUndoBuf.size());

        if (m_nUndoPosition > 0)
        {
            m_bUndoBeginGroup = true;
            pSource->OnEditOperation(m_aUndoBuf[m_nUndoPosition - 1].m_action, m_aUndoBuf[m_nUndoPosition - 1].GetText());
        }
    }

    m_bUndoGroup = false;
}

int CCrystalTextBuffer::FindNextBookmarkLine(int nCurrentLine)
{
    bool bWrapIt = true;
    uint32_t dwFlags = GetLineFlags(nCurrentLine);

    if ((dwFlags & LF_BOOKMARKS) != 0)
        nCurrentLine++;

    size_t size = m_aLines.size();
    for (;;)
    {
        while (static_cast<std::size_t>(nCurrentLine) < size)
        {
            if ((m_aLines[nCurrentLine].m_flags & LF_BOOKMARKS) != 0)
                return nCurrentLine;
            // Keep going
            nCurrentLine++;
        }
        // End of text reached
        if (!bWrapIt)
            return -1;

        // Start from the beginning of text
        bWrapIt = false;
        nCurrentLine = 0;
    }
}

int CCrystalTextBuffer::FindPrevBookmarkLine(int nCurrentLine)
{
    bool bWrapIt = true;
    uint32_t dwFlags = GetLineFlags(nCurrentLine);

    if ((dwFlags & LF_BOOKMARKS) != 0)
        nCurrentLine--;

    size_t size = m_aLines.size();
    for (;;)
    {
        while (nCurrentLine >= 0)
        {
            if ((m_aLines[nCurrentLine].m_flags & LF_BOOKMARKS) != 0)
                return nCurrentLine;
            // Keep moving up
            nCurrentLine--;
        }

        // Beginning of text reached
        if (!bWrapIt)
            return -1;

        // Start from the end of text
        bWrapIt = false;
        nCurrentLine = size - 1;
    }
}
