////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextBuffer.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Interface of the CCrystalTextBuffer class, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef CRYSTAL_TEXT_BUFFER_H__
#define CRYSTAL_TEXT_BUFFER_H__

#include "cedefs.h"
#include "CCrystalTextView.h"

#define UNDO_DESCRIP_BUF 32

enum LINEFLAGS
{
    LF_BOOKMARK_FIRST           = 0x00000001L,
    LF_EXECUTION                = 0x00010000L,
    LF_BREAKPOINT               = 0x00020000L,
    LF_COMPILATION_ERROR        = 0x00040000L,
    LF_BOOKMARKS                = 0x00080000L,
    LF_INVALID_BREAKPOINT       = 0x00100000L,
    LF_COLLAPSIBLE_BLOCK_START  = 0x01000000L,
    LF_COLLAPSIBLE_BLOCK_END    = 0x02000000L,
    LF_COLLAPSED_BLOCK          = 0x04000000L
};

#define LF_BOOKMARK(id) (LF_BOOKMARK_FIRST << id)

enum CRLFSTYLE
{
    CRLF_STYLE_AUTOMATIC = -1,
    CRLF_STYLE_DOS       = 0,
    CRLF_STYLE_UNIX      = 1,
    CRLF_STYLE_MAC       = 2
};

enum
{
    CE_ACTION_UNKNOWN    = 0,
    CE_ACTION_PASTE      = 1,
    CE_ACTION_DELSEL     = 2,
    CE_ACTION_CUT        = 3,
    CE_ACTION_TYPING     = 4,
    CE_ACTION_BACKSPACE  = 5,
    CE_ACTION_INDENT     = 6,
    CE_ACTION_DRAGDROP   = 7,
    CE_ACTION_REPLACE    = 8,
    CE_ACTION_DELETE     = 9,
    CE_ACTION_AUTOINDENT = 10
                              //	...
                              //	Expandable: user actions allowed
};


/////////////////////////////////////////////////////////////////////////////
// CUpdateContext class

class CUpdateContext
{
public:
    virtual void RecalcPoint(wxPoint &ptPoint) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// CCrystalTextBuffer command target

class CCrystalTextBuffer : public wxEvtHandler
{
private:
    bool m_bInit;
    bool m_bReadOnly;
    bool m_bModified;
    int m_nCRLFMode;
    bool m_bCreateBackupFile;
    int m_nUndoBufSize;
    int FindLineWithFlag(uint32_t flag);

protected:
#pragma pack(push, 1)
    // Nested class declarations
    struct SLineInfo
    {
        char *m_pcLine;
        int m_nLength, m_nMax;
        uint32_t m_flags;

        SLineInfo()
        {
            memset(this, 0, sizeof(SLineInfo));
        };
    };

    enum
    {
        UNDO_INSERT     = 0x0001,
        UNDO_BEGINGROUP = 0x0100
    };

    // [JRT] Support For Descriptions On Undo/Redo Actions
    struct SUndoRecord
    {
    private:
        /*
         * TODO: Rewrite this
         * A) Needs to be a proper reusable string class, we have a couple
         * of places where we're using this.
         * 
         * B) We're relying on a quark of the Win32 behavior; that won't fly
         * with other OSes like Linux or Mac OS X.
         * 
         *              -- B.Simonds (May 4, 2024)
         */ 

        //	char	*m_pcText;
        //	Since in most cases we have 1 character here,
        //	we should invent a better way. Note: 2 * sizeof(WORD) <= sizeof(char*)
        //
        //	Here we will use the following trick: on Win32 platforms high-order word
        //	of any pointer will be != 0. So we can store 1 character strings without
        //	allocating memory.
        //

        union
        {
            char *m_pszText;  // For cases when we have > 1 character strings
            char m_szText[2]; // For single-character strings
        };

    public:
        uint32_t m_flags;

        wxPoint m_ptStartPos, m_ptEndPos; // Block of text participating

        int m_action; // For information only: action type

        //	constructor/destructor for this struct
        SUndoRecord()
            : m_pszText(nullptr)
            , m_flags(0)
            , m_ptStartPos(0, 0)
            , m_ptEndPos(0, 0)
            , m_action(0)
        {
        };

        void SetText(const std::string &text);
        void FreeText();

        std::string GetText() const
        {
            ASSERT(false); // This class needs to be rewritten to not use Win32 hack -- B.Simonds (May 12, 2024)
            /*
            if (HIWORD((uint32_t)m_pszText) != 0)
                return m_pszText;
            */

            return m_szText;
        };
    };

#pragma pack(pop)

    class CInsertContext : public CUpdateContext
    {
    public:
        wxPoint m_ptStart, m_ptEnd;
        virtual void RecalcPoint(wxPoint &ptPoint);
    };

    class CDeleteContext : public CUpdateContext
    {
    public:
        wxPoint m_ptStart, m_ptEnd;
        virtual void RecalcPoint(wxPoint &ptPoint);
    };

    // Lines of text
    std::vector<SLineInfo> m_aLines;

    // Undo
    std::vector<SUndoRecord> m_aUndoBuf;
    int m_nUndoPosition;
    int m_nSyncPosition;
    bool m_bUndoGroup, m_bUndoBeginGroup;

    // Connected views
    std::vector<CCrystalTextView *> m_lpViews;

    // Helper methods
    void InsertLine(const std::string &line, int nLength = -1, int nPosition = -1);
    void AppendLine(int nLineIndex, const std::string &chars, int nLength = -1);

    // Implementation
    bool InternalInsertText(CCrystalTextView *pSource, int nLine, int nPos, const std::string &text, int &nEndLine, int &nEndChar);
    bool InternalDeleteText(CCrystalTextView *pSource, int nStartLine, int nStartPos, int nEndLine, int nEndPos);

    // [JRT] Support For Descriptions On Undo/Redo Actions
    void AddUndoRecord(bool bInsert, const wxPoint &ptStartPos, const wxPoint &ptEndPos,
                       const std::string &text, int nActionType = CE_ACTION_UNKNOWN);

    // Overridable: provide action description
    virtual bool GetActionDescription(int nAction, std::string &desc);

// Operations
public:
    // Construction/destruction code
    /* constructor */ CCrystalTextBuffer();
    virtual          ~CCrystalTextBuffer();

    // Basic functions
    bool InitNew(int nCrlfStyle = CRLF_STYLE_DOS);
    bool LoadFromFile(const std::string &fileName, int nCrlfStyle = CRLF_STYLE_AUTOMATIC);
    bool SaveToFile(const std::string &fileName, int nCrlfStyle = CRLF_STYLE_AUTOMATIC, bool bClearModifiedFlag = true);
    void FreeAll();

    // 'Dirty' flag
    virtual void SetModified(bool modified = true);
    bool IsModified() const;

    // Connect/disconnect views
    void AddView(CCrystalTextView *pView);
    void RemoveView(CCrystalTextView *pView);

    // Text access functions
    size_t GetLineCount();
    int GetLineLength(int line);
    const char *GetLineChars(int line);
    uint32_t GetLineFlags(int line);
    int GetLineWithFlag(uint32_t flag);
    void SetLineFlag(int nLine, uint32_t flag, bool set, bool bRemoveFromPreviousLine = true);
    void GetText(int nStartLine, int nStartChar, int nEndLine, int nEndChar, std::string &text, const char *pszCRLF = nullptr);
    void SetLinesFlags(int nLineFrom, int nLineTo, uint32_t addFlags, uint32_t removeFlags);
    void GetText(std::string &text, const char *pszCRLF = nullptr);

    // Attributes
    int GetCRLFMode();
    void SetCRLFMode(int nCRLFMode);
    bool GetReadOnly() const;
    void SetReadOnly(bool bReadOnly = true);

    // Text modification functions
    bool InsertText(CCrystalTextView *pSource, int nLine, int nPos, const std::string &text, int &nEndLine, int &nEndChar, int nAction = CE_ACTION_UNKNOWN);
    bool DeleteText(CCrystalTextView *pSource, int nStartLine, int nStartPos, int nEndLine, int nEndPos, int nAction = CE_ACTION_UNKNOWN);

    // Undo/Redo
    bool CanUndo();
    bool CanRedo();
    bool Undo(wxPoint &ptCursorPos);
    bool Redo(wxPoint &ptCursorPos);

    // Undo grouping
    void BeginUndoGroup(bool bMergeWithPrevious = false);
    void FlushUndoGroup(CCrystalTextView *pSource);

    // Browse undo sequence
    POSITION GetUndoDescription(std::string &desc, POSITION pos);
    POSITION GetRedoDescription(std::string &desc, POSITION pos);

    // Notify all connected views about changes in text
    void UpdateViews(CCrystalTextView *pSource, CUpdateContext *pContext,
                     uint32_t dwUpdateFlags, int nLineIndex = -1);

    // More bookmarks
    int FindNextBookmarkLine(int nCurrentLine = 0);
    int FindPrevBookmarkLine(int nCurrentLine = 0);
};

inline bool CCrystalTextBuffer::IsModified() const
{
    return m_bModified;
}

#endif /* CRYSTAL_TEXT_BUFFER_H__ */
