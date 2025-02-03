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

#pragma once
#pragma warning(disable: 4291)

class CLeksem : public CAsm
{
public:
    /*
      enum IdentInfo	// info o identyfikatorze
      {
        I_UNDEF,		// identyfikator niezdefiniowany
        I_ADDRESS,		// identyfikator zawiera adres
        I_VALUE		// identyfikator zawiera warto�� liczbow�
      };

      struct Ident
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


    enum InstrArg		// rodzaj argument�w dyrektywy
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


    enum NumType		// rodzaj liczby
    {
        N_DEC,
        N_HEX,
        N_BIN,
        N_CHR,	// znak
        N_CHR2	// dwa znaki, np. 'ab'
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
        ERR_NUM_BIG,	// b��d przekroczenia zakresu
        ERR_BAD_CHR,
        ERR_STR_UNLIM	// niezamkni�ty �a�cuch znak�w
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
        static char *s_ptr_1;
        static char *s_ptr_2;
        static char s_buf_1[];
        static char s_buf_2[];
        static const size_t s_cnMAX;
    public:
        CLString() : CString()
        {}
        CLString(const CString &str) : CString(str)
        {}
        CLString(const TCHAR *ptr, int size) : CString(ptr, size)
        {}
#ifdef _DEBUG
        void *operator new(size_t size, LPCSTR /*lpszFileName*/, int /*nLine*/)
#else
        void *operator new(size_t size)
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
        void operator delete(void *ptr)
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
        num.type = type;  num.value = val;
    }
    CLeksem(CLString *str) : type(L_STR), str(str)
    {}
    CLeksem(CLString *str, int dummy) : type(L_IDENT), str(str)
    {}
    CLeksem(CLString *str, long dummy) : type(L_IDENT_N), str(str)
    {}
    //  CLeksem(const CLeksem &leks, long dummy) : type(L_IDENT_N)
    //  { ASSERT(leks.type == L_IDENT);  str = new CString(*leks.str); }

    CLeksem &operator=(const CLeksem &);

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

    const CString *GetString() const
    {
        ASSERT(type == L_STR || type == L_IDENT || type == L_IDENT_N);
        return str;
    }

    void Format(SINT32 val)	// znormalizowanie postaci etykiety numerycznej
    {
        ASSERT(type == L_IDENT_N);
        CString num(' ', 9);
        num.Format("#%08X", (int)val);
        *str += num;		// do��czenie numeru
    }

    //debugging log  use= leks.Logger(cs);
    void Logger(const char *logMsg)
    {
        FILE *pFile = fopen("logfile.txt", "a");
        fprintf(pFile, "%s\n", logMsg);
        fclose(pFile);
    }

};

//-----------------------------------------------------------------------------

// tablica asocjacyjna identyfikator�w
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

//=============================================================================

class CInputBase	// klasa bazowa dla klas odczytu danych �r�d�owych
{
protected:
    int m_nLine;
    CString m_strFileName;
    bool m_bOpened;

public:
    CInputBase(const TCHAR *str = NULL) : m_strFileName(str)
    {
        m_nLine = 0;  m_bOpened = false;
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

    virtual LPTSTR read_line(LPTSTR str, UINT max_len) = 0;

    virtual int get_line_no()
    {
        return m_nLine - 1;
    }		// numeracja wierszy od 0

    virtual const CString &get_file_name()
    {
        return CInputBase::m_strFileName;
    }
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
        ASSERT(m_bOpened == false);	    // plik nie mo�e by� jeszcze otwarty
        CFileException *ex = new CFileException;
        if (!Open(CInputBase::m_strFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText, ex))
            throw (ex);
        m_bOpened = true;
        /*delete*/ ex->Delete();
    }

    virtual void close()
    {
        ASSERT(m_bOpened == true);	    // plik musi by� otworzony
        Close();
        m_bOpened = false;
    }

    virtual void seek_to_begin()
    {
        SeekToBegin();
        m_nLine = 0;
    }

    virtual LPTSTR read_line(LPTSTR str, UINT max_len);

    //  virtual int get_line_no()

    //  virtual const CString &get_file_name()
};

//-----------------------------------------------------------------------------

class CInputWin : public CInputBase, public CAsm	// odczyt danych z okna dokumentu
{
    CWnd *m_pWnd;
public:
    CInputWin(CWnd *pWnd) : m_pWnd(pWnd)
    {}
    virtual LPTSTR read_line(LPTSTR str, UINT max_len);
    virtual const CString &get_file_name();
    virtual void seek_to_begin();
};

//-----------------------------------------------------------------------------

class CInput : CList<CInputBase *, CInputBase *>, CAsm
{
    CInputBase *tail;
    FileUID fuid;

    int calc_index(POSITION pos);

public:
    void open_file(const CString &fname);
    void open_file(CWnd *pWin);
    void close_file();

    CInput(const CString &fname) : fuid(0)
    {
        open_file(fname);
    }
    CInput(CWnd *pWin) : fuid(0)
    {
        open_file(pWin);
    }
    CInput() : fuid(0), tail(NULL)
    {}
    ~CInput();

    LPTSTR read_line(LPTSTR str, UINT max_len)
    {
        return tail->read_line(str, max_len);
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

    const CString &get_file_name()
    {
        return tail->get_file_name();
    }

    FileUID get_file_UID()
    {
        return fuid;
    }

    void set_file_UID(FileUID fuid)
    {
        this->fuid = fuid;
    }

    bool is_present()
    {
        return tail != NULL;
    }
};

//-----------------------------------------------------------------------------

struct Expr		 // klasa do opisu wyra�enia arytmetycznego/logicznego/tekstowego
{
    SINT32 value;
    CString string;
    enum
    {
        EX_UNDEF,		// warto�� nieznana
        EX_BYTE,		// bajt, tj. od -255 do 255 (sic!)
        EX_WORD,		// s�owo, od -65535 do 65535
        EX_LONG,		// outside the above range
        EX_STRING		// ci�g znak�w
    } inf;

    Expr() : inf(EX_UNDEF)
    {}
    Expr(SINT32 value) : inf(EX_LONG), value(value)
    {}
};

//-----------------------------------------------------------------------------
class CConditionalAsm : public CAsm	// asemblacja warunkowa (automat ze stosem)
{
public:
    enum State
    {
        BEFORE_ELSE, AFTER_ELSE
    };

private:
    CByteArray stack;
    int level;

    State get_state()
    {
        ASSERT(level >= 0);  return stack.GetAt(level) & 1 ? BEFORE_ELSE : AFTER_ELSE;
    }
    bool get_assemble()
    {
        ASSERT(level >= 0);  return stack.GetAt(level) & 2 ? true : false;
    }
    bool get_prev_assemble()
    {
        ASSERT(level > 0);  return stack.GetAt(level - 1) & 2 ? true : false;
    }
    void set_state(State state, bool assemble)
    {
        stack.SetAtGrow(level, BYTE((state == BEFORE_ELSE ? 1 : 0) + (assemble ? 2 : 0)));
    }
public:
    CConditionalAsm() : level(-1)
    {
        stack.SetSize(16, 16);
    }
    Stat instr_if_found(Stat condition);
    Stat instr_else_found();
    Stat instr_endif_found();
    bool in_cond() const { return level >= 0; }
    int get_level() const { return level; }
    void restore_level(int level);
};

//-----------------------------------------------------------------------------

  // elementy wsp�lne odczytu wierszy dla .MACRO, .REPEAT i normalnego odczytu
class CSource : public CObject, public CAsm
{
    FileUID m_fuid;
    int cond_level_;
public:
    CSource() : m_fuid(0)
    {}
    virtual ~CSource()
    {}

    virtual void Start(CConditionalAsm *cond)			// rozpocz�cie odczytu wierszy
    {
        cond_level_ = cond ? cond->get_level() : INT_MAX;
    }

    virtual void Fin(CConditionalAsm *cond)				// zako�czenie odczytu wierszy
    {
        if (cond && cond_level_ != INT_MAX)
            cond->restore_level(cond_level_);
    }

    virtual const TCHAR *GetCurrLine(CString &str) = 0;	// odczyt bie��cego wiersza

    virtual int GetLineNo() = 0;		// odczyt numeru wiersza

    virtual FileUID GetFileUID()	// odczyt ID pliku
    {
        return m_fuid;
    }

    void SetFileUID(FileUID fuid)	// ustawienie ID pliku
    {
        m_fuid = fuid;
    }

    virtual const CString &GetFileName() = 0;	// nazwa aktualnego pliku

    static const CString s_strEmpty;

private:
    // remember nesting level of conditional assemblation
    void StoreConditionLevel(int level) { cond_level_ = level; }
    int GetConditionLevel() const { return cond_level_; }
};

//.............................................................................

struct CStrLines : public CStringArray
{
    CStrLines(int nInitSize, int nGrowBy)
    {
        SetSize(nInitSize, nGrowBy);
    }
};


// elementy wymagane do zapami�tywania i odtwarzania wierszy �r�d�owych programu
class CRecorder
{
    CStrLines m_strarrLines;
    CDWordArray m_narrLineNums;
    int m_nLine;
public:
    CRecorder(int nInitSize = 10, int nGrowBy = 10) :	// pocz�tkowe rozmiary tablic
        m_strarrLines(nInitSize, nGrowBy), m_nLine(0)
    {
        m_narrLineNums.SetSize(nInitSize, nGrowBy);
    }
    virtual ~CRecorder()
    {}

    void AddLine(const CString &strLine, int num)	// zapami�tanie kolejnego wiersza
    {
        m_strarrLines.SetAtGrow(m_nLine, strLine); m_narrLineNums.SetAtGrow(m_nLine, num); m_nLine++;
    }

    const CString &GetLine(int nLineNo)	// odczyt wiersza 'nLineNo'
    {
        return m_strarrLines[nLineNo];
    }

    int GetLineNo(int nLineNo)	// odczyt numeru wiersza w pliku �r�d�owym
    {
        return m_narrLineNums[nLineNo];
    }

    int GetSize()			// odczyt ilo�ci wierszy w tablicy
    {
        //    ASSERT( m_narrLineNums.GetSize() == m_narrLineNums.GetSize() );
        return m_nLine;
    }

};

//.............................................................................
/*
  // obs�uga tablicy identyfikator�w etykiet lokalnych
class CLabels : CIdentTable
{

};
*/

//-----------------------------------------------------------------------------

class CAsm6502;

class CMacroDef : public CSource, public CRecorder
{
    CIdentTable param_names;	// tablica nazw parametr�w makra
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
    CString m_strName;		// nazwa makra
    bool m_bFirstCodeLine;	// flaga odczytu pierwszego wiersza makra zawieraj�cego instr. 6502

    CMacroDef() : param_names(2), m_nParams(0), m_nLineNo(0), m_nFirstLineNo(-1),
        m_nFirstLineFuid(FileUID(-1)), m_bFirstCodeLine(true)
    {}
    ~CMacroDef()
    {}

    int AddParam(const CString &strParam)	// dopisanie nazwy kolejnego parametru
    {
        if (strParam.Compare(MULTIPARAM) == 0)
        {
            m_nParams = -(m_nParams + 1);
            return 1;		// koniec listy parametr�w
        }
        if (!param_names.insert(strParam, CIdent(CIdent::I_VALUE, m_nParams)))
            return -1;	// powt�rzona nazwa parametru!
        m_nParams++;
        return 0;
    }

    int GetParamsFormat()			// ilo�� przekazywanych parametr�w
    {
        return m_nParams;
    }

    virtual const TCHAR *GetCurrLine(CString &str);	// odczyt aktualnego wiersza makra

    virtual int GetLineNo()
    {
        return CRecorder::GetLineNo(m_nLineNo - 1);
    }

    int GetFirstLineNo()
    {
        return m_nFirstLineNo;
    }

    FileUID GetFirstLineFileUID()
    {
        return m_nFirstLineFuid;
    }

    void Start(CConditionalAsm *cond, int line, FileUID file)	// przygotowanie do odczytu
    {
        CSource::Start(cond); m_nLineNo = 0; m_nFirstLineNo = line; m_nFirstLineFuid = file;
    }

    virtual void Fin(CConditionalAsm *cond)
    {
        CSource::Fin(cond);
    }				// zako�czenie rozwijania bie��cego makra

// wczytanie argument�w wywo�ania
    CAsm::Stat ParseArguments(CLeksem &leks, CAsm6502 &asmb);

    CAsm::Stat ParamLookup(CLeksem &leks, const CString &param_name, Expr &expr, bool &found, CAsm6502 &asmb);
    CAsm::Stat ParamLookup(CLeksem &leks, int param_number, Expr &expr, CAsm6502 &asmb);
    CAsm::Stat AnyParamLookup(CLeksem &leks, CAsm6502 &asmb);

    CAsm::Stat ParamType(const CString param_name, bool &found, int &type);
    CAsm::Stat ParamType(int param_number, bool &found, int &type);

    //  virtual bool IsMacro()		// �r�d�em danych jest rozwijane makro
    //  { return true; }

    CMacroDef &operator= (const CMacroDef &src)
    {
        ASSERT(false);	// nie wolno przypisywa� obiekt�w typu CMacroDef
        return *this;
    }

    virtual const CString &GetFileName()	// nazwa aktualnego pliku
    {
        return s_strEmpty;
    }
};


class CMacroDefs : public CArray<CMacroDef, CMacroDef &>
{
};

//-----------------------------------------------------------------------------

class CRepeatDef : public CSource, public CRecorder
{
    int m_nLineNo;		// nr aktualnego wiersza (przy odczycie)
    int m_nRepeat;		// ilo�� powt�rze� wierszy
public:

    CRepeatDef(int nRept = 0) : m_nLineNo(0), m_nRepeat(nRept)
    {}
    ~CRepeatDef()
    {}

    virtual const TCHAR *GetCurrLine(CString &str);	// odczyt aktualnego wiersza

    virtual int GetLineNo()
    {
        return CRecorder::GetLineNo(m_nLineNo - 1);
    }

    virtual void Start(CConditionalAsm *cond)
    {
        CSource::Start(cond); m_nLineNo = GetSize();
    }		// licznik wierszy na koniec
    virtual void Fin(CConditionalAsm *cond)			// zako�czenie powt�rki wierszy
    {
        CSource::Fin(cond); delete this;
    }

    //  virtual bool IsRepeat()		// �r�d�em danych jest powt�rzenie (.REPEAT)
    //  { return true; }

    CRepeatDef &operator= (const CRepeatDef &src)
    {
        ASSERT(false);	// nie wolno przypisywa� obiekt�w typu CRepeatDef
        return *this;
    }

    virtual const CString &GetFileName()	// nazwa aktualnego pliku
    {
        return s_strEmpty;
    }
};


class CRepeatDefs : public CArray<CRepeatDef, CRepeatDef &>
{
};

//-----------------------------------------------------------------------------

class CSourceText : public CSource
{
    CInput input;
public:
    CSourceText(const CString &file_in_name) : input(file_in_name)
    {}
    CSourceText(CWnd *pWnd) : input(pWnd)
    {}
    CSourceText()
    {}

    virtual void Start(CConditionalAsm *cond)			// rozpocz�cie odczytu wierszy
    {
        CSource::Start(cond); input.seek_to_begin();
    }

    virtual const TCHAR *GetCurrLine(CString &str)// odczyt bie��cego wiersza
    {
        const TCHAR *ret = input.read_line(str.GetBuffer(1024 + 4), 1024 + 4);
        str.ReleaseBuffer(-1);
        return ret;
    }

    virtual int GetLineNo()		// odczyt numeru wiersza
    {
        return input.get_line_no();
    }

    FileUID GetFileUID()			// odczyt ID pliku
    {
        return input.get_file_UID();
    }

    void SetFileUID(CDebugInfo *pDebugInfo)
    {
        if (pDebugInfo)
            input.set_file_UID(pDebugInfo->GetFileUID(input.get_file_name()));
    }

    void Include(const CString &fname, CDebugInfo *pDebugInfo = NULL)	// w��czenie pliku
    {
        input.open_file(fname);
        if (pDebugInfo)
            input.set_file_UID(pDebugInfo->GetFileUID(fname));
    }

    bool TextFin()			// zako�czony bie��cy plik
    {
        if (input.get_count() > 1)	// zagnie�d�ony odczyt (.include) ?
        {
            input.close_file();
            return true;
        }
        else
            return false;	// koniec plik�w �r�d�owych
    }
    /*
      bool IsPresent()			// spr. czy jest jaki� odczytywany plik
      { return input.is_present(); }
    */
    virtual const CString &GetFileName()	// nazwa aktualnego pliku
    {
        return input.get_file_name();
    }
};


//-----------------------------------------------------------------------------


class CSourceStack : CTypedPtrArray<CObArray, CSource *>	// Stos obiekt�w b�d�cych �r�d�em wierszy
{
    int m_nIndex;
public:
    CSourceStack()
    {
        m_nIndex = -1;
    }
    ~CSourceStack()
    {
        for (int i = m_nIndex; i >= 0; i--)
            GetAt(i)->Fin(0);
    }

    void Push(CSource *pSrc)		// Dodanie elementu na wierzcho�ku stosu
    {
        ++m_nIndex; SetAtGrow(m_nIndex, pSrc);
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

    CSource *FindMacro()			// odszukanie ostatniego makra
    {
        for (int i = m_nIndex; i >= 0; i--)
            if (CMacroDef *pSrc = dynamic_cast<CMacroDef *>(GetAt(i)))
                return pSrc;
        return NULL;
    }
    CSource *FindRepeat()			// odszukanie ostatniego powt�rzenia
    {
        for (int i = m_nIndex; i >= 0; i--)
            if (CRepeatDef *pSrc = dynamic_cast<CRepeatDef *>(GetAt(i)))
                return pSrc;
        //      if (GetAt(i)->IsRepeat())
        //        return GetAt(i);
        return NULL;
    }
};


//-----------------------------------------------------------------------------
class CMarkArea;

class CAsm6502 : public CAsm, public CObject
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
    CLeksem::CLString *get_ident();	// wyodr�bnienie napisu
    CLeksem get_string(TCHAR lim);	// wyodr�bnienie �a�cucha znak�w
    CLeksem eat_space();			// omini�cie odst�pu
    bool proc_instr(const CString &str, OpCode &code);
    bool asm_instr(const CString &str, InstrType &it);
    CLeksem next_leks(bool nospace = true);	// pobranie kolejnego symbolu
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
        return abort_asm ? abort_asm = false, true : false;
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
        macros.SetSize(macros.GetSize() + 1);  return &macros[macros.GetSize() - 1];
    }
    CMacroDef *get_last_macro_entry()
    {
        ASSERT(macros.GetSize() > 0);  return &macros[macros.GetSize() - 1];
    }
    int get_last_macro_entry_index()
    {
        ASSERT(macros.GetSize() > 0);  return macros.GetSize() - 1;
    }
    //  Stat (CLeksem &leks);
    int find_const(const CString &str);
    Stat predef_const(const CString &str, Expr &expr, bool &found);
    Stat predef_function(CLeksem &leks, Expr &expr, bool &fn);
    Stat constant_value(CLeksem &leks, Expr &expr, bool nospace);
    Stat factor(CLeksem &leks, Expr &expr, bool nospace = true);
    Stat mul_expr(CLeksem &leks, Expr &expr);
    Stat shift_expr(CLeksem &leks, Expr &expr);
    Stat add_expr(CLeksem &leks, Expr &expr);
    Stat bit_expr(CLeksem &leks, Expr &expr);
    Stat cmp_expr(CLeksem &leks, Expr &expr);
    Stat bool_expr_and(CLeksem &leks, Expr &expr);
    Stat bool_expr_or(CLeksem &leks, Expr &expr);
    Stat expression(CLeksem &leks, Expr &expr, bool str = false);	// interpretacja wyra�enia
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
            m_nLine = m_File.Open(fname, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
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
        void AddCodeBytes(UINT32 addr, int code1 = -1, int code2 = -1, int code3 = -1, int code4 = -1);
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
    ProcessorType m_procType;

	static bool caseinsense;
    static bool case_insensitive; // true -> lowercase and uppercase letters in labels are not distinguished
    static bool swapbin;
	static bool swap_bin;
    static UINT8 forcelong;
    static bool generateBRKExtraByte;	// generowa� dodatkowy bajt za instrukcj� BRK?
    static UINT8 BRKExtraByte;			// warto�� dodatkowego bajtu za instrukcj� BRK

    CAsm6502(const CString &file_in_name, 
        COutputMem *out = NULL, 
        CDebugInfo *debug = NULL,
        CMarkArea *area = NULL, 
        ProcessorType procType = ProcessorType::M6502, 
        const TCHAR *listing_file = NULL)
        : entire_text(file_in_name)
        , out(out)
        , debug(debug)
        , markArea(area)
        , m_procType(procType)
        , listing(listing_file)
    {
        init();
    }
    CAsm6502(CWnd *pWnd, 
        COutputMem *out = NULL, 
        CDebugInfo *debug = NULL, 
        CMarkArea *area = NULL,
        ProcessorType procType = ProcessorType::M6502, 
        const TCHAR *listing_file = NULL)
        : entire_text(pWnd)
        , out(out)
        , debug(debug)
        , markArea(area)
        , m_procType(procType)
        , listing(listing_file)
    {
        init();
    }

    CAsm6502()
    {
        m_procType = ProcessorType::M6502;
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
            void *p = out;
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
