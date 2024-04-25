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

#include "Asm.h"
#include "Ident.h"
#include "DebugInfo.h"
#include "OutputMem.h"

#pragma warning(disable: 4291)

class CLeksem : public CAsm
{
public:
    /*
    enum IdentInfo // Info about the identifier
    {
        I_UNDEF,    // Undefined identifier
        I_ADDRESS,  // The ID contains the address
        I_VALUE     // The identifier contains a numeric value
    };

    structIdent
    {
        IdentInfo info;
        int value;
        CString *str;
    };
    */

    struct Code
    {
        OpCode code;
        CodeAdr adr;
    };

    enum InstrArg   // Type of directive arguments
    {
        A_BYTE,
        A_NUM,
        A_LIST,
        A_STR,
        A_2NUM
    };

    struct Instr
    {
        InstrType type;
        InstrArg arg;
    };

    enum NumType    // Type of number
    {
        N_DEC,
        N_HEX,
        N_BIN,
        N_CHR,	// character
        N_CHR2	// Two characters, e.g. 'ab'
    };

    struct Num
    {
        NumType type;
        SINT32 value;
    };

    enum Error
    {
        ERR_NUM_HEX,
        ERR_NUM_DEC,
        ERR_NUM_BIN,
        ERR_NUM_CHR,
        ERR_NUM_BIG,    // Out of bounds error
        ERR_BAD_CHR,
        ERR_STR_UNLIM	// An unclosed string of characters
    };

    //...........................................................................

    enum LeksType
    {
        L_UNKNOWN,		// nierozpoznany znak

        L_NUM,		// liczba (dec, hex, bin, lub znak)
        L_STR,		// ci�g znak�w w apostrofach lub cudzys�owach
        L_IDENT,		// identyfikator
        L_IDENT_N,		// numerowany identyfikator (zako�czony przez '#' i numer)
        L_SPACE,		// odst�p
        L_OPER,		// operator
        L_BRACKET_L,	// lewy nawias '('
        L_BRACKET_R,	// prawy nawias ')'
        L_LBRACKET_L,	// lewy nawias '('
        L_LBRACKET_R,	// prawy nawias ')'
        L_EXPR_BRACKET_L,	// lewy nawias dla wyra�e� '['
        L_EXPR_BRACKET_R,	// prawy nawias dla wyra�e� ']'
        L_COMMENT,		// znak komentarza ';'
        L_LABEL,		// znak etykiety ':'
        L_COMMA,		// znak przecinka ','
        L_STR_ARG,		// znak dolara '$', ko�czy parametr typu tekstowego
        L_MULTI,		// znak wielokropka '...'
        L_INV_COMMAS,	// znak cudzys�owu
        L_LHASH,		// '!#'
        L_HASH,		// znak '#'
        L_EQUAL,		// znak przypisania '='
        L_PROC_INSTR,	// instrukcja procesora
        L_ASM_INSTR,	// dyrektywa asemblera
        L_CR,		// koniec wiersza
        L_FIN,		// koniec danych
        L_ERROR		// b��d
    };

    const LeksType type;

    class CLString : public CString
    {
        static char* s_ptr_1;
        static char* s_ptr_2;
        static char s_buf_1[];
        static char s_buf_2[];
        static const size_t s_cnMAX;

    public:
        CLString() : CString()
        {}

        CLString(const std::wstring& str) : CString(str)
        {}

        CLString(const TCHAR* ptr, int size) : CString(ptr ,size)
        {}

#ifdef _DEBUG
        void* operator new(size_t size, const std::wstring & /*lpszFileName*/, int /*nLine*/)
#else
        void* operator new(size_t size)
#endif
        {
            if (size > s_cnMAX)
                return CObject::operator new(size);
            else if (s_ptr_1 == NULL)
                return s_ptr_1 = s_buf_1;
            else if (s_ptr_2 == NULL)
                return  s_ptr_2 = s_buf_2;
            else
                return CObject::operator new(size);
        }

        void operator delete(void* ptr)
        {
            if (ptr == s_ptr_1)
                s_ptr_1 = NULL;
            else if (ptr == s_ptr_2)
                s_ptr_2 = NULL;
            else
                CObject::operator delete(ptr);
        }
    };

private:

    union
    {
//    Ident id;		// identyfikator
        OperType op;	// operator binarny lub unarny
        OpCode code;	// mnomonik
        InstrType instr;	// dyrektywa
//    int val;		// sta�a liczbowa lub znakowa
        Num num;		// sta�a liczbowa lub znakowa
        CLString *str;	// identyfikator lub ci�g znak�w
        Error err;
    };

public:
    CLeksem(const CLeksem &leks);

    ~CLeksem();

    CLeksem(LeksType type) : type(type)
    {}

    CLeksem(OperType oper) : type(L_OPER), op(oper)
    {}

    CLeksem(OpCode code) : type(L_PROC_INSTR), code(code)
    {}

    CLeksem(InstrType it) : type(L_ASM_INSTR), instr(it)
    {}

    CLeksem(Error err) : type(L_ERROR), err(err)
    {}

    CLeksem(NumType type, SINT32 val) : type(L_NUM)
    {
        num.type=type;
        num.value=val;
    }

    CLeksem(CLString *str) : type(L_STR), str(str)
    {}

    CLeksem(CLString *str, int dummy) : type(L_IDENT), str(str)
    {}

    CLeksem(CLString *str, long dummy) : type(L_IDENT_N), str(str)
    {}

//  CLeksem(const CLeksem &leks, long dummy) : type(L_IDENT_N)
//  { ASSERT(leks.type == L_IDENT);  str = new CString(*leks.str); }

    CLeksem & operator=(const CLeksem &);

    const CString *GetIdent() const
    {
        ASSERT(type == L_IDENT);
        return str;
    }

    OperType GetOper()
    {
        ASSERT(type == L_OPER);
        return op;
    }

    OpCode GetCode()
    {
        ASSERT(type == L_PROC_INSTR);
        return code;
    }

    InstrType GetInstr()
    {
        ASSERT(type == L_ASM_INSTR);
        return instr;
    }

    int GetValue()
    {
        ASSERT(type == L_NUM);
        return num.value;
    }

    const CString* GetString() const
    {
        ASSERT(type == L_STR || type == L_IDENT || type == L_IDENT_N);
        return str;
    }

    void Format(SINT32 val) // Normalize the form of the numeric label
    {
        ASSERT(type == L_IDENT_N);
        CString num(' ', 9);
        num.Format("#%08X", (int)val);
        *str += num;    // Adding a number
    }

    //debugging log  use= leks.Logger(cs);
    void Logger(const char *logMsg)
    {
        FILE* pFile = fopen("logfile.txt", "a");
        fprintf(pFile, "%s\n", logMsg);
        fclose(pFile);
    }
};

//-----------------------------------------------------------------------------

#if 0
// Associative array of identifiers
class CIdentTable : public CMap<CString, LPCTSTR, CIdent, CIdent>
{

public:
    CIdentTable(int nSize = 500) : CMap<CString, LPCTSTR, CIdent, CIdent>(nSize)
    {
        InitHashTable(1021);
    }
    
    ~CIdentTable()
    {}

    bool insert(const CString &str, CIdent &ident);
    bool replace(const CString &str, const CIdent &ident);

    bool lookup(const CString &str, CIdent &ident) const
    {
        return Lookup(str, ident);
    }

    void clr_table()
    {
        RemoveAll();
    }
};
#endif

typedef std::unordered_map<std::wstring, CIdent> CIdentTable;

//=============================================================================

class CInputBase    // Base class for classes reading source data
{
protected:
    int m_nLine;
    std::wstring m_strFileName;
    bool m_bOpened;

public:
    CInputBase(const wchar *str = nullptr)
        : m_strFileName(str)
    {
        m_nLine = 0;
        m_bOpened = false;
    }

    virtual ~CInputBase()
    {
        ASSERT(m_bOpened == false);
    }

    virtual void open()
    {
        m_bOpened = true;
    }

    virtual void close()
    {
        m_bOpened = false;
    }

    virtual void seek_to_begin()
    {
        m_nLine = 0;
    }

    virtual wchar *read_line(std::wstring buffer) = 0;

    virtual int get_line_no() const
    {
        return m_nLine - 1; // Line numbering from 0
    }

    virtual const std::wstring &get_file_name() const { return CInputBase::m_strFileName; }
};

//-----------------------------------------------------------------------------

class CInputFile : public CInputBase, CStdioFile
{
public:
    CInputFile(const CString &str) : CInputBase(str)
    {}

    ~CInputFile()
    {}

    virtual void open()
    {
        ASSERT(m_bOpened == false); // The file cannot be opened yet.
        CFileException *ex = new CFileException;

        if (!Open(CInputBase::m_strFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText, ex))
            throw (ex);

        m_bOpened = true;
        /*delete*/ ex->Delete();
    }

    virtual void close()
    {
        ASSERT(m_bOpened == true);	    // The file must be opened
        Close();
        m_bOpened = false;
    }

    virtual void seek_to_begin()
    {
        SeekToBegin();
        m_nLine = 0;
    }

    virtual wchar *read_line(wchar *str, UINT max_len);

//  virtual int get_line_no()

//  virtual const CString &get_file_name()
};

//-----------------------------------------------------------------------------

// Reading data from the document window
class CInputWin : public CInputBase, public CAsm
{
    wxWindow *m_pWnd;

public:
    CInputWin(CWnd *pWnd) : m_pWnd(pWnd)
    {}

    virtual LPTSTR read_line(LPTSTR str, UINT max_len);
    virtual const CString &get_file_name();
    virtual void seek_to_begin();
};

//-----------------------------------------------------------------------------

class CInput : CList<CInputBase*, CInputBase*>, CAsm
{
    CInputBase* tail;
    FileUID fuid;

    int calc_index(POSITION pos);

public:
    void open_file(const std::wstring& fname);
    void open_file(wxWindow* pWin);
    void close_file();

    CInput(const std::wstring& fname)
        : fuid(0)
        , tail(nullptr)
    {
        open_file(fname);
    }

    CInput(wxWindow* pWin)
        : fuid(0)
        , tail(nullptr)
    {
        open_file(pWin);
    }

    CInput()
        : fuid(0)
        , tail(nullptr)
    {}

    ~CInput();

    wchar *read_line(std::wstring &buffer)
    {
        return tail->read_line(buffer);
    }

    void seek_to_begin()
    {
        tail->seek_to_begin();
    }

    int get_line_no()
    {
        return tail->get_line_no();
    }

    int get_count()
    {
        return GetCount();
    }

    const std::wstring& get_file_name() const { return tail->get_file_name(); }

    FileUID get_file_UID() const { return fuid; }
    void set_file_UID(FileUID fuid) { this->fuid = fuid; }

    bool is_present() const {  return tail != nullptr; }
};

//-----------------------------------------------------------------------------

/**
 * @brief A class to describe an arithmetic/logical/text expression
 */
struct Expr		 
{
    SINT32 value;
    CString string;
    
    enum
    {
        EX_UNDEF,   // Unknown value
        EX_BYTE,    // Byte i.e. from -255 to 255 (sic!)
        EX_WORD,    // Word, from -65535 to 65535
        EX_LONG,    // outside the above range
        EX_STRING   // String of characters
    } inf;

    Expr()
        : inf(EX_UNDEF)
        , value(0)
    {}

    Expr(SINT32 value)
        : inf(EX_LONG)
        , value(value)
    {}
};

//-----------------------------------------------------------------------------

// Conditional assembly (stack machine)
class CConditionalAsm : public CAsm
{
public:
    enum State
    {
        BEFORE_ELSE,
        AFTER_ELSE
    };

private:
    CByteArray stack;
    int level;

    State get_state()
    {
        ASSERT(level >= 0);
        return stack.GetAt(level)&1 ? BEFORE_ELSE : AFTER_ELSE;
    }

    bool get_assemble()
    {
        ASSERT(level >= 0);
        return stack.GetAt(level)&2 ? true : false;
    }

    bool get_prev_assemble()
    {
        ASSERT(level > 0);
        return stack.GetAt(level-1)&2 ? true : false;
    }

    void set_state(State state, bool assemble)
    {
        stack.SetAtGrow(level,BYTE((state == BEFORE_ELSE ? 1 : 0) + (assemble ? 2 : 0)));
    }

public:
    CConditionalAsm()
        : level(-1)
    {
        stack.SetSize(16, 16);
    }

    Stat instr_if_found(Stat condition);
    Stat instr_else_found();
    Stat instr_endif_found();

    bool in_cond() const
    {
        return level >= 0;
    }

    int get_level() const
    {
        return level;
    }

    void restore_level(int level);
};

//-----------------------------------------------------------------------------

// Line reading common elements for .MACRO, .REPEAT and normal reading
class CSource : /*public CObject,*/ public CAsm
{
    FileUID m_fuid;
    int cond_level_;

public:
    CSource()
        : m_fuid(0)
    {}

    virtual ~CSource()
    {}

    virtual void Start(CConditionalAsm* cond)
    {
        cond_level_ = cond ? cond->get_level() : INT_MAX;
    }

    virtual void Fin(CConditionalAsm* cond)
    {
        if (cond && cond_level_ != INT_MAX)
            cond->restore_level(cond_level_);
    }

    virtual const wchar* GetCurrLine(std::wstring &str) = 0;

    virtual int GetLineNo() const = 0;

    virtual FileUID GetFileUID() const
    {
        return m_fuid;
    }

    void SetFileUID(FileUID fuid)
    {
        m_fuid = fuid;
    }

    virtual const std::wstring& GetFileName() const = 0;

    static const std::wstring s_strEmpty;

private:
    // remember nesting level of conditional assemblation
    void StoreConditionLevel(int level)
    {
        cond_level_ = level;
    }

    int GetConditionLevel() const
    {
        return cond_level_;
    }
};

//.............................................................................

#if 0
struct CStrLines : public CStringArray
{
    CStrLines(int nInitSize, int nGrowBy)
    {
        SetSize(nInitSize, nGrowBy);
    }
};
#endif

// Elements required for storing and reproducing program source lines
class CRecorder
{
private:
    //CStrLines m_strarrLines;
    //CDWordArray m_narrLineNums;

    std::vector<std::wstring> m_strarrLines;
    std::vector<uint32_t> m_narrLineNums;

    int m_growBy;

    int m_nLine;

public:
    CRecorder(int nInitSize = 10, int nGrowBy = 10) // Initial array sizes
        : m_strarrLines(nInitSize)
        , m_narrLineNums(nInitSize)
        , m_growBy(nGrowBy)
        , m_nLine(0)
    {
    }

    virtual ~CRecorder()
    {}

    void AddLine(const std::wstring &strLine, int num) // Save the next line
    {
        m_strarrLines[m_nLine] = strLine;
        m_narrLineNums[m_nLine] = num;

        ++m_nLine;
        
        //m_strarrLines.SetAtGrow(m_nLine, strLine);
        //m_narrLineNums.SetAtGrow(m_nLine, num);

        //m_nLine++;
    }

    const std::wstring& GetLine(int nLineNo) // reading the 'nLineNo' line
    {
        return m_strarrLines[nLineNo];
    }

    int GetLineNo(int nLineNo) // read the line number in the source file
    {
        return m_narrLineNums[nLineNo];
    }

    int GetSize() // Reading the number of rows in the array
    {
        //ASSERT(m_narrLineNums.GetSize() == m_narrLineNums.GetSize());
        return m_nLine;
    }
};

//............................................................. .............................
/*
// Support for an array of local label IDs
class CLabels : CIdentTable
{

};
*/

//-----------------------------------------------------------------------------

class CAsm6502;

class CMacroDef : public CSource, public CRecorder
{
private:
    CIdentTable param_names; // Table of macro parameter names
    
    int m_nParams;		// required number of parameters
    int m_nParamCount;		// number of parameters in a macro call
    CStringArray m_strarrArgs;	// kolejne argumenty wywo�ania - tylko �a�cuchy znak�w
    CDWordArray m_narrArgs;	// kolejne argumenty wywo�ania - tylko warto�ci wyra�e�
    enum ArgType { NUM, STR, UNDEF_EXPR };
    CByteArray m_arrArgType;	// typy argument�w (NUM - liczba, STR - �a�cuch znak�w)
    int m_nLineNo;		// nr aktualnego wiersza (przy odczycie)
    int m_nFirstLineNo;		// numer wiersza, z kt�rego wywo�ywane jest makro
    FileUID m_nFirstLineFuid;	// ID pliku, z kt�rego wywo�ywane jest makro

public:
    std::wstring m_strName; // nazwa makra
    bool m_bFirstCodeLine;  // flaga odczytu pierwszego wiersza makra zawieraj�cego instr. 6502

    CMacroDef()
        : param_names(2)
        , m_nParams(0)
        , m_nLineNo(0)
        , m_nFirstLineNo(-1)
        , m_nFirstLineFuid(FileUID(-1))
        , m_bFirstCodeLine(true)
    {}

    ~CMacroDef()
    {}

    int AddParam(const std::wstring &strParam)//adding the name of the next parameter
    {
        if (strParam.Compare(MULTIPARAM) == 0)
        {
            m_nParams = -(m_nParams + 1);
            return 1; // end of parameter list
        }

        std::pair<auto, bool> res = param_names.insert(strParam, CIdent(CIdent::I_VALUE, m_nParams));

        if (!res.second)
            return -1; // repeated parameter name!

        ++m_nParams;
        return 0;
    }

    // Number of passed parameters
    int GetParamsFormat() const
    {
        return m_nParams;
    }

    virtual const wchar* GetCurrLine(CString &str);

    virtual int GetLineNo()
    {
        return CRecorder::GetLineNo(m_nLineNo - 1);
    }

    int GetFirstLineNo() const { return m_nFirstLineNo; }

    FileUID GetFirstLineFileUID() { return m_nFirstLineFuid; }

    void Start(CConditionalAsm* cond, int line, FileUID file) // prepare for reading
    {
        CSource::Start(cond);
        m_nLineNo = 0;
        m_nFirstLineNo = line;
        m_nFirstLineFuid = file;
    }

    virtual void Fin(CConditionalAsm* cond)
    {
        CSource::Fin(cond); // End expansion of the current macro
    }

    // Load the arguments of the call
    CAsm::Stat ParseArguments(CLeksem &leks, CAsm6502 &asmb);

    CAsm::Stat ParamLookup(CLeksem &leks, const CString& param_name, Expr &expr, bool &found, CAsm6502 &asmb);
    CAsm::Stat ParamLookup(CLeksem &leks, int param_number, Expr &expr, CAsm6502 &asmb);
    CAsm::Stat AnyParamLookup(CLeksem &leks, CAsm6502 &asmb);

    CAsm::Stat ParamType(const CString param_name, bool& found, int& type);
    CAsm::Stat ParamType(int param_number, bool& found, int& type);

    //virtual bool IsMacro() // The data source is the expanded macro
    //{ return true; }

    CMacroDef &operator= (const CMacroDef &src)
    {
        ASSERT(false); // objects of type C cannot be assigned to MacroDef
        return *this;
    }

    virtual const CString &GetFileName()	// nazwa aktualnego pliku
    {
        return s_strEmpty;
    }
};

#if 0

class CMacroDefs : public CArray<CMacroDef, CMacroDef&>
{
};

#endif

//-----------------------------------------------------------------------------

class CRepeatDef : public CSource, public CRecorder
{
private:
    int m_nLineNo; // current line number (when reading)
    int m_nRepeat; // number of repeated lines

public:

    CRepeatDef(int nRept = 0)
        : m_nLineNo(0)
        , m_nRepeat(nRept)
    {}

    ~CRepeatDef()
    {}

    virtual const wchar* GetCurrLine(std::wstring &str); // Read the current line

    virtual int GetLineNo()
    {
        return CRecorder::GetLineNo(m_nLineNo - 1);
    }

    virtual void Start(CConditionalAsm* cond)
    {
        CSource::Start(cond); // Line counter at the end
        m_nLineNo = GetSize();
    }

    virtual void Fin(CConditionalAsm* cond) // end repeat lines
    {
        CSource::Fin(cond);
        delete this;
    }

    //virtual bool IsRepeat() //data source is repeat (.REPEAT)
    //{ return true; }

    CRepeatDef &operator= (const CRepeatDef &src)
    {
        ASSERT(false); // objects of type CRepeatDef cannot be assigned
        return *this;
    }

    virtual const std::wstring &GetFileName() const { return s_strEmpty; }
};


class CRepeatDefs : public CArray<CRepeatDef, CRepeatDef&>
{
};

//-----------------------------------------------------------------------------

class CSourceText : public CSource
{
private:
    CInput input;

public:
    CSourceText(const std::wstring &file_in_name)
        : input(file_in_name)
    {}

    CSourceText(wxWindow* pWnd)
        : input(pWnd)
    {}

    CSourceText()
    {}

    virtual void Start(CConditionalAsm* cond) // start reading lines
    {
        CSource::Start(cond);
        input.seek_to_begin();
    }

    virtual const wchar* GetCurrLine(std::wstring &str) // reading less than an entire line
    {
        str.reserve(1024 + 4);

        return input.read_line(str);
    }

    virtual int GetLineNo() // read the line number
    {
        return input.get_line_no();
    }

    FileUID GetFileUID() // read file ID
    {
        return input.get_file_UID();
    }

    void SetFileUID(CDebugInfo* pDebugInfo)
    {
        if (pDebugInfo)
            input.set_file_UID(pDebugInfo->GetFileUID(input.get_file_name()));
    }

    void Include(const std::wstring &fname, CDebugInfo* pDebugInfo= NULL) // include file
    {
        input.open_file(fname);

        if (pDebugInfo)
            input.set_file_UID(pDebugInfo->GetFileUID(fname));
    }

    bool TextFin() // Current file finished
    {
        if (input.get_count() > 1) // nested read (.include) ?
        {
            input.close_file();
            return true;
        }
        else
            return false; // End of source files
    }

    /*
    bool IsPresent() const // Check whether there is any file being read
    { return input.is_present(); }
    */
   
    virtual const std::wstring &GetFileName() const
    {
        return input.get_file_name();
    }
};

//-----------------------------------------------------------------------------

class CSourceStack : CTypedPtrArray<CObArray,CSource *>	// Stos obiekt�w b�d�cych �r�d�em wierszy
{
private:
    int m_nIndex;

public:
    CSourceStack()
    {
        m_nIndex = -1;
    }

    ~CSourceStack()
    {
        for (int i=m_nIndex; i>=0; i--)
            GetAt(i)->Fin(0);
    }

    void Push(CSource *pSrc)		// Dodanie elementu na wierzcho�ku stosu
    {
        ++m_nIndex;
        SetAtGrow(m_nIndex,pSrc);
    }

    CSource *Peek()			// Sprawdzenie elementu na szczycie stosu
    {
        return m_nIndex < 0 ? NULL : GetAt(m_nIndex);
    }

    CSource *Pop()			// Zdj�cie elementu ze stosu
    {
        return GetAt(m_nIndex--);
    }

//  void RemoveAll()
//  { RemoveAll();  m_nIndex = -1; }

    CSource* FindMacro()			// odszukanie ostatniego makra
    {
        for (int i=m_nIndex; i>=0; i--)
            if (CMacroDef* pSrc= dynamic_cast<CMacroDef*>(GetAt(i)))
                return pSrc;
        return NULL;
    }
    CSource* FindRepeat()			// odszukanie ostatniego powt�rzenia
    {
        for (int i=m_nIndex; i>=0; i--)
            if (CRepeatDef* pSrc= dynamic_cast<CRepeatDef*>(GetAt(i)))
                return pSrc;
//      if (GetAt(i)->IsRepeat())
//        return GetAt(i);
        return NULL;
    }
};


//-----------------------------------------------------------------------------
class CMarkArea;

class CAsm6502 : public CAsm /*, public CObject */
{
    friend class CMacroDef;

    CString current_line;
    const TCHAR *ptr;				// do �ledzenia aktualnego wiersza
    const TCHAR *err_start;
    const TCHAR *ident_start;		// po�o�enie identyfikatora w wierszu
    const TCHAR *ident_fin;			// po�o�enie ko�ca identyfikatora w wierszu

    bool check_line;				// flaga: true - analiza jednego wiersza, false - programu
    UINT32 origin;
    bool originWrapped;				// true - je�li licznik rozkaz�w "przewin�� si�"
    UINT32 program_start;			// pocz�tek programu
    UINT32 mem_mask;				// granica pami�ci procesora (maska), normalnie $FFFF
    int local_area;					// nr obszaru etykiet lokalnych
    int proc_area;					// nr obszaru etykiet lokalnych
    int macro_local_area;			// nr obszaru etykiet lokalnych makrodefinicji
    int pass;						// numer przej�cia (1 lub 2)
    //int conditional;				// asemblacja warunkowa - poziom zag��bienia
    CString include_fname;
    CString user_error_text;		// tekst b��du u�ytkownika (dyrektywy .ERROR)
    const TCHAR *instr_start;		// do zapami�tania pocz�tku
    const TCHAR *instr_fin;			// i ko�ca instrukcji w wierszu
    CMacroDefs macros;				// makrodefinicje

    CSource *text;					// bie��cy tekst �r�d�owy
    CSourceText entire_text;		// pierwszy (pocz�tkowy) tekst �r�d�owy
    CRepeatDef *pRept;

    // leksyka:
    CLeksem get_dec_num();			// interpretacja liczby dziesi�tnej
    CLeksem get_hex_num();			// interpretacja liczby szesnastkowej
    CLeksem get_bin_num();			// interpretacja liczby dw�jkowej
    CLeksem get_char_num();			// interpretacja sta�ej znakowej
//  CLeksem get_ident();			// wyodr�bnienie napisu
    CLeksem::CLString* get_ident();	// wyodr�bnienie napisu
    CLeksem get_string(TCHAR lim);	// wyodr�bnienie �a�cucha znak�w
    CLeksem eat_space();			// omini�cie odst�pu
    bool proc_instr(const CString &str, OpCode &code);
    bool asm_instr(const CString &str, InstrType &it);
    CLeksem next_leks(bool nospace= true);	// pobranie kolejnego symbolu
    bool next_line();				// wczytanie kolejnego wiersza

    COutputMem *out;				// memory for the object code
    CMarkArea *markArea;			// to mark used memory areas with 'out'
    CIdentTable local_ident;		// tablica identyfikator�w lokalnych
    CIdentTable proc_local_ident;		// tablica identyfikator�w lokalnych
    CIdentTable global_ident;		// tablica identyfikator�w globalnych
    CIdentTable macro_name;			// tablica nazw makrodefinicji
    CIdentTable macro_ident;		// tablica identyfikator�w w makrorozwini�ciach
    CDebugInfo *debug;				// commissioning information for the simulator

//	CString err_file;				// nazwa pliku, kt�rego odczyt spowodowa� b��d
//	int err_line;					// nr wiersza, w kt�rym napotkano b��d
    CString err_ident;				// nazwa etykiety, kt�ra spowodowa�a b��d

    bool temporary_out;				// flaga - 'out' zosta�o alokowane w konstruktorze
    bool abort_asm;					// zmienna do wymuszenia przerwania asemblacji z zewn�trz
    bool is_aborted()
    {
        return abort_asm ? abort_asm=false, true : false;
    }

    bool add_ident(const CString &ident, CIdent &inf);
    Stat def_ident(const CString &ident, CIdent &inf);
    Stat chk_ident(const CString &ident, CIdent &inf);
    Stat chk_ident_def(const CString &ident, CIdent &inf);
    Stat def_macro_name(const CString &ident, CIdent &inf);
    Stat chk_macro_name(const CString &ident);
    CString format_local_label(const CString &ident, int area);
    // interpretacja instrukcji procesora
    Stat proc_instr_syntax(CLeksem &leks, CodeAdr &mode, Expr &expr, Expr &expr_bit, Expr &expr_zpg);
    // interpretacja dyrektywy
    Stat asm_instr_syntax_and_generate(CLeksem &leks, InstrType it, const CString *pLabel);
    CMacroDef *get_new_macro_entry()
    {
        macros.SetSize(macros.GetSize()+1);
        return &macros[macros.GetSize()-1];
    }
    CMacroDef *get_last_macro_entry()
    {
        ASSERT(macros.GetSize() > 0);
        return &macros[macros.GetSize()-1];
    }
    int get_last_macro_entry_index()
    {
        ASSERT(macros.GetSize() > 0);
        return macros.GetSize()-1;
    }
//  Stat (CLeksem &leks);
    int find_const(const CString& str);
    Stat predef_const(const CString &str, Expr &expr, bool &found);
    Stat predef_function(CLeksem &leks, Expr &expr, bool &fn);
    Stat constant_value(CLeksem &leks, Expr &expr, bool nospace);
    Stat factor(CLeksem &leks, Expr &expr, bool nospace= true);
    Stat mul_expr(CLeksem &leks, Expr &expr);
    Stat shift_expr(CLeksem &leks, Expr &expr);
    Stat add_expr(CLeksem &leks, Expr &expr);
    Stat bit_expr(CLeksem &leks, Expr &expr);
    Stat cmp_expr(CLeksem &leks, Expr &expr);
    Stat bool_expr_and(CLeksem &leks, Expr &expr);
    Stat bool_expr_or(CLeksem &leks, Expr &expr);
    Stat expression(CLeksem &leks, Expr &expr, bool str= false);	// interpretacja wyra�enia
    bool is_expression(const CLeksem &leks);
    Stat assemble_line();			// interpretacja wiersza
    Stat assemble();
    const TCHAR *get_next_line();	// wczytanie kolejnego wiersza do asemblacji
    const TCHAR *play_macro();		// odczyt kolejnego wiersza makra
    const TCHAR *play_repeat();		// odczyt kolejnego wiersza powt�rki
//  CPtrStack <CSource> source;		// stos obiekt�w zwracaj�cych wiersze �r�d�owe
    CSourceStack source;			// stos obiekt�w zwracaj�cych wiersze �r�d�owe
    void asm_start();				// rozpocz�cie asemblacji
    void asm_fin();					// zako�czenie asemblacji
    void asm_start_pass();			// rozpocz�cie przej�cia asemblacji
    void asm_fin_pass();			// zako�czenie przej�cia asemblacji
    Stat chk_instr_code(OpCode &code, CodeAdr &mode, Expr expr, int &length);
    void generate_code(OpCode code, CodeAdr mode, Expr expr, Expr expr_bit, Expr expr_zpg);
    Stat inc_prog_counter(int dist);
    Stat look_for_endif();			// szukanie .ENDIF lub .ELSE
//  int get_line_no();				// numer wiersza (dla debug info)
//  FileUID get_file_UID();			// id pliku (dla debug info)
    void generate_debug(UINT32 addr, int line_no, FileUID file_UID);
    Stat generate_debug(InstrType it, int line_no, FileUID file_UID);
    void generate_debug();
    Stat look_for_endm();
    Stat record_macro();
//  void MacroExpandStart(CMacroDef *pMacro);	// przej�cie do trybu rozwijania makrodefinicji
//  void MacroExpandFin();			// zako�czenie trybu rozwijania makrodefinicji
    CMacroDef *in_macro;			// aktualnie rejestrowane makro lub NULL
    CMacroDef *expanding_macro;		// aktualnie rozwijane makro lub NULL
//  CPtrStack<CMacroDef> expand_macros;	// lista rozwijanych makr
    CRepeatDef *repeating;			// aktualna powt�rka (.REPEAT)
//  CPtrStack<CRepeatDef> repeats;	// lista powt�rek
    Stat record_rept(CRepeatDef *pRept);
    Stat look_for_repeat();			// szukanie .ENDR lub .REPEAT
    int reptInit;					// warto�� do zainicjowania ilo�ci powt�rze�
    int reptNested;					// licznik zagnie�d�e� .REPEAT (przy rejestracji)
//  void RepeatStart(CRepeatDef *pRept);
//  void RepeatFin();
    bool b_listing;

    static int __cdecl asm_str_key_cmp(const void *elem1, const void *elem2);
    struct ASM_STR_KEY
    {
        const TCHAR *str;
        CAsm::InstrType it;
    };

    CConditionalAsm conditional_asm;

    class CListing
    {
        CStdioFile m_File;	// wsk. do pliku z listingiem
        CString m_Str;		// bie��cy wiersz listingu
        int m_nLine;		// bie��cy wiersz

        void Open(const TCHAR *fname)
        {
            m_nLine = m_File.Open(fname,CFile::modeCreate|CFile::modeWrite|CFile::typeText);
        }
        void Close()
        {
            m_File.Close();
        }
    public:
        CListing()
        {
            m_nLine = -1;
        }
        CListing(const TCHAR *fname);
        ~CListing()
        {
            if (m_nLine != -1) Close();
        }

        void Remove();
        void NextLine();
        void AddCodeBytes(UINT32 addr, int code1= -1, int code2= -1, int code3= -1, int code4= -1);
        void AddValue(UINT32 val);
        void AddBytes(UINT32 addr, UINT16 mask, const UINT8 mem[], int len);
        void AddSourceLine(const TCHAR *line);

        bool IsOpen()
        {
            return m_nLine != -1;
        }
    } listing;

    void init();
    void init_members();

public:
    UINT8 bProc6502;						// 0=6502, 1=65C02 or 6501, 2=65816
    static bool case_insensitive;		// true -> ma�e i du�e litery w etykietach nie s� rozr�niane
    static bool swapbin;
    static UINT8 forcelong;
    static bool generateBRKExtraByte;	// generowa� dodatkowy bajt za instrukcj� BRK?
    static UINT8 BRKExtraByte;			// warto�� dodatkowego bajtu za instrukcj� BRK

    CAsm6502(const CString &file_in_name, COutputMem *out= NULL, CDebugInfo *debug= NULL,
             CMarkArea *area= NULL, UINT8 proc6502= 0, const TCHAR *listing_file= NULL) :
        entire_text(file_in_name), out(out), debug(debug), markArea(area), bProc6502(proc6502), listing(listing_file)
    {
        init();
    }
    CAsm6502(CWnd *pWnd, COutputMem *out= NULL, CDebugInfo *debug= NULL, CMarkArea *area= NULL,
             UINT8 proc6502= 0, const TCHAR *listing_file= NULL) :
        entire_text(pWnd), out(out), debug(debug), markArea(area), bProc6502(proc6502), listing(listing_file)
    {
        init();
    }
    CAsm6502()
    {
        bProc6502 = 0;
        swapbin = false;
        forcelong = 0;
        init_members();
        temporary_out = false;
        check_line = true;
        text = NULL;
        pRept = NULL;
    }
    ~CAsm6502()
    {
        if (temporary_out)
        {
            void* p = out;
            ::delete p;
        }
        if (text)
            text->Fin(0);
        if (pRept)
            pRept->Fin(0);
    }

    // sprawdzenie sk�adni w wierszu 'str'
    // w 'instr_idx_start' zwracane po�o�enie instrukcji w wierszu lub 0
    // w 'instr_idx_fin' zwracane po�o�enie ko�ca instrukcji w wierszu lub 0
    Stat CheckLine(const TCHAR *str, int &instr_idx_start, int &instr_idx_fin);

    void Abort()
    {
        abort_asm = true;
    }

    CString GetErrMsg(Stat stat);		// opis b��du

    Stat Assemble()						// asemblacja
    {
        return assemble();
    }

    UINT32 GetProgramStart()			// pocz�tek programu
    {
        return program_start;
    }
};
