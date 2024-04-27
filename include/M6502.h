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

#ifndef M6502_H__
#define M6502_H__

#include "Asm.h"
#include "Ident.h"
#include "DebugInfo.h"
#include "OutputMem.h"

// No idication as to what this POSITION type actually is, we're stubbing in an int for now. -- B.Simonds (April 25, 2024)
typedef int POSITION;

// Stub for now
class CFileException
{
public:
    CFileException(int err) { }
    virtual ~CFileException() { }
};

// Leksem == Lex

// This actually looks like a class to hold a token. -- B.Simonds (April 25, 2024)

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
        std::string *str;
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
        int32_t value;
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
        L_UNKNOWN,      // Unrecognized character

        L_NUM,          // Number (dec, hex, bin, or sign)
        L_STR,          // A string of characters in apostrophes or quotation marks
        L_IDENT,        // Identifier
        L_IDENT_N,      // Numbered identifier (ending with '#' and a number)
        L_SPACE,        // Space
        L_OPER,         // Operator
        L_BRACKET_L,    // Left paren '('
        L_BRACKET_R,    // Right paren ')'
        L_LBRACKET_L,   // Left bracket '('
        L_LBRACKET_R,   // Right bracket ')'
        L_EXPR_BRACKET_L, // Left square bracket '['
        L_EXPR_BRACKET_R, // Right square bracket ']'
        L_COMMENT,      // Comment character ';'
        L_LABEL,        // Label character ':'
        L_COMMA,        // Comma character ','
        L_STR_ARG,      // Dollar sign '$', ends text type parameter
        L_MULTI,        // Ellipsis character '...'
        L_INV_COMMAS,   // Sign cudzys�owu ????
        L_LHASH,		// '!#'
        L_HASH,         // Hash sign '#'
        L_EQUAL,        // Assignment character '='
        L_PROC_INSTR,   // Processor instruction
        L_ASM_INSTR,    // Assembler directive
        L_CR,           // End of line
        L_FIN,          // End of data
        L_ERROR         // Bad token type
    };

    const LeksType type;

#if 0

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

        CLString(const std::string& str) : CString(str)
        {}

        CLString(const char* ptr, int size) : CString(ptr ,size)
        {}

#ifdef _DEBUG
        void* operator new(size_t size, const std::string & /*lpszFileName*/, int /*nLine*/)
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

#endif

private:
    union
    {
        //Id id;            // Identifier
        OperType op;        // Binary or unary operator
        OpCode code;        // Polynomial
        InstrType instr;    // Directive
        Num num;            // Numeric or character constant
        std::string *str;   // Identifier or string
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

    CLeksem(NumType type, int32_t val) : type(L_NUM)
    {
        num.type = type;
        num.value = val;
    }

    CLeksem(std::string *str) : type(L_STR), str(str)
    {}

    CLeksem(std::string *str, int dummy) : type(L_IDENT), str(str)
    {}

    CLeksem(std::string *str, long dummy) : type(L_IDENT_N), str(str)
    {}

//  CLeksem(const CLeksem &leks, long dummy) : type(L_IDENT_N)
//  { ASSERT(leks.type == L_IDENT);  str = new std::string(*leks.str); }

    CLeksem & operator =(const CLeksem &);

    const std::string *GetIdent() const
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

    const std::string* GetString() const
    {
        ASSERT(type == L_STR || type == L_IDENT || type == L_IDENT_N);
        return str;
    }

    void Format(int32_t val) // Normalize the form of the numeric label
    {
#if 0
        ASSERT(type == L_IDENT_N);
        CString num(' ', 9);
        num.Format("#%08X", (int)val);
        *str += num; // Append number
#endif

        // TODO: Definately need to verify this change. -- B.Simonds (April 25, 2024)

        ASSERT(type == L_IDENT_N);

        char tmp[64];

        snprintf(tmp, sizeof(tmp), "       #%08X%d", (int)val, num.value);
        str = new std::string(tmp);
    }

    //debugging log  use= leks.Logger(cs);
    void Logger(const char *logMsg)
    {
        std::FILE* file = std::fopen("logfile.txt", "a");
        std::fprintf(file, "%s\n", logMsg);
        std::fclose(file);
    }
};

//-----------------------------------------------------------------------------

#if 0
// Associative array of identifiers
class CIdentTable : public CMap<std::string, LPCTSTR, CIdent, CIdent>
{

public:
    CIdentTable(int nSize = 500) : CMap<std::string, LPCTSTR, CIdent, CIdent>(nSize)
    {
        InitHashTable(1021);
    }
    
    ~CIdentTable()
    {}

    bool insert(const std::string &str, CIdent &ident);
    bool replace(const std::string &str, const CIdent &ident);

    bool lookup(const std::string &str, CIdent &ident) const
    {
        return Lookup(str, ident);
    }

    void clr_table()
    {
        RemoveAll();
    }
};
#endif

typedef std::unordered_map<std::string, CIdent> CIdentTable;

//=============================================================================

class CInputBase    // Base class for classes reading source data
{
protected:
    int m_nLine;
    std::string m_strFileName;
    bool m_bOpened;

public:
    CInputBase(const char *str = nullptr)
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

    virtual const std::string &read_line(std::string buffer) = 0;

    virtual int get_line_no() const
    {
        return m_nLine - 1; // Line numbering from 0
    }

    virtual const std::string &get_file_name() const { return CInputBase::m_strFileName; }
};

//-----------------------------------------------------------------------------

class CInputFile : public CInputBase
{
private:
    std::FILE *m_file;

public:
    CInputFile(const std::string &str) 
        : CInputBase(str.c_str())
        , m_file(nullptr)
    {
    }

    ~CInputFile()
    {
    }

    virtual void open()
    {
        ASSERT(m_bOpened == false); // File is already open

        m_file = std::fopen(m_strFileName.c_str(), "r");

        if (m_file == nullptr)
            throw new CFileException(errno);

        m_bOpened = true;
    }

    virtual void close()
    {
        ASSERT(m_bOpened == true);	    // The file must be opened

        std::fclose(m_file);
        m_file = nullptr;
        m_bOpened = false;
    }

    virtual void seek_to_begin()
    {
        std::fseek(m_file, 0, SEEK_SET);
        m_nLine = 0;
    }

    virtual const std::string &read_line(std::string &buffer);

//  virtual int get_line_no()

//  virtual const std::string &get_file_name()
};

//-----------------------------------------------------------------------------

// Reading data from the document window
class CInputWin : public CInputBase, public CAsm
{
private:
    wxWindow *m_pWnd;

public:
    CInputWin(wxWindow *pWnd) : m_pWnd(pWnd)
    {}

    virtual const std::string &read_line(std::string &buffer);
    virtual const std::string &get_file_name();
    virtual void seek_to_begin();
};

//-----------------------------------------------------------------------------

class CInput : std::list<CInputBase*>
{
    CInputBase* tail;
    CAsm::FileUID fuid;

    int calc_index(POSITION pos);

public:
    void open_file(const std::string& fname);
    void open_file(wxWindow* pWin);
    void close_file();

    CInput(const std::string& fname)
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

    const std::string &read_line(std::string &buffer)
    {
        return tail->read_line(buffer);
    }

    void seek_to_begin()
    {
        tail->seek_to_begin();
    }

    int get_line_no() const
    {
        return tail->get_line_no();
    }

    int get_count() const { return size(); }

    const std::string& get_file_name() const { return tail->get_file_name(); }

    CAsm::FileUID get_file_UID() const { return fuid; }
    void set_file_UID(CAsm::FileUID fuid) { this->fuid = fuid; }

    bool is_present() const { return tail != nullptr; }
};

//-----------------------------------------------------------------------------

/**
 * @brief A class to describe an arithmetic/logical/text expression
 */
struct Expr		 
{
    int32_t value;
    std::string string;
    
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

    Expr(int32_t value)
        : inf(EX_LONG)
        , value(value)
    {}
};

//-----------------------------------------------------------------------------

// Conditional assembly (stack machine)
class CConditionalAsm //: public CAsm
{
public:
    enum State
    {
        BEFORE_ELSE,
        AFTER_ELSE
    };

private:
    std::vector<uint8_t> stack;
    int level;

    State get_state()
    {
        ASSERT(level >= 0);
        return stack[level] & 1 ? BEFORE_ELSE : AFTER_ELSE;
    }

    bool get_assemble()
    {
        ASSERT(level >= 0);
        return stack[level] & 2 ? true : false;
    }

    bool get_prev_assemble()
    {
        ASSERT(level > 0);
        return stack[level - 1] & 2 ? true : false;
    }

    void set_state(State state, bool assemble)
    {
        if (stack.capacity() < level)
            stack.reserve(level);

        stack[level] = uint8_t((state == BEFORE_ELSE ? 1 : 0) + (assemble ? 2 : 0));
    }

public:
    CConditionalAsm()
        : stack()
        , level(-1)
    {
        stack.reserve(16);
    }

    CAsm::Stat instr_if_found(CAsm::Stat condition);
    CAsm::Stat instr_else_found();
    CAsm::Stat instr_endif_found();

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

    virtual const std::string &GetCurrLine(std::string &str) = 0;

    virtual int GetLineNo() const = 0;

    virtual FileUID GetFileUID() const
    {
        return m_fuid;
    }

    void SetFileUID(FileUID fuid)
    {
        m_fuid = fuid;
    }

    virtual const std::string& GetFileName() const = 0;

    static const std::string s_strEmpty;

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

// Elements required for storing and reproducing program source lines
class CRecorder
{
private:
    //CStrLines m_strarrLines;
    //CDWordArray m_narrLineNums;

    std::vector<std::string> m_strarrLines;
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

    void AddLine(const std::string &strLine, int num) // Save the next line
    {
        m_strarrLines[m_nLine] = strLine;
        m_narrLineNums[m_nLine] = num;

        ++m_nLine;
        
        //m_strarrLines.SetAtGrow(m_nLine, strLine);
        //m_narrLineNums.SetAtGrow(m_nLine, num);

        //m_nLine++;
    }

    const std::string& GetLine(int nLineNo) // reading the 'nLineNo' line
    {
        return m_strarrLines[nLineNo];
    }

    int GetLineNo(int nLineNo) const // read the line number in the source file
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
    CIdentTable param_names;    // Table of macro parameter names
    
    int m_nParams;              // Required number of parameters
    int m_nParamCount;          // Number of parameters in a macro call
    std::vector<std::string> m_strarrArgs;  // Subsequent call arguments -only strings
    std::vector<uint32_t> m_narrArgs;     // Subsequent call arguments -only expression values

    enum ArgType { NUM, STR, UNDEF_EXPR };

    std::vector<ArgType> m_arrArgType;    // Argument types (NUM -number, STR -string)

    int m_nLineNo;              // Current line number (when reading)
    int m_nFirstLineNo;         // The line number from which the macro is called
    FileUID m_nFirstLineFuid;   // The ID of the file from which the macro is called

public:
    std::string m_strName; // Macro name
    bool m_bFirstCodeLine; // Flag for reading the first line of the macro containing the instruction. 6502

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

    int AddParam(const std::string &strParam) // Adding the name of the next parameter
    {
        if (strParam == MULTIPARAM)
        {
            m_nParams = -(m_nParams + 1);
            return 1; // end of parameter list
        }

        auto intr = param_names.find(strParam);

        if (intr != param_names.end())
            return -1; // Repeated parameter name!

        param_names[strParam] = CIdent(CIdent::I_VALUE, m_nParams);

        ++m_nParams;
        return 0;
    }

    // Number of passed parameters
    int GetParamsFormat() const
    {
        return m_nParams;
    }

    virtual const std::string &GetCurrLine(std::string &str);

    virtual int GetLineNo() const
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

    CAsm::Stat ParamLookup(CLeksem &leks, const std::string& param_name, Expr &expr, bool &found, CAsm6502 &asmb);
    CAsm::Stat ParamLookup(CLeksem &leks, int param_number, Expr &expr, CAsm6502 &asmb);
    CAsm::Stat AnyParamLookup(CLeksem &leks, CAsm6502 &asmb);

    CAsm::Stat ParamType(const std::string &param_name, bool& found, int& type);
    CAsm::Stat ParamType(int param_number, bool& found, int& type);

    //virtual bool IsMacro() // The data source is the expanded macro
    //{ return true; }

    CMacroDef &operator= (const CMacroDef &src)
    {
        ASSERT(false); // objects of type C cannot be assigned to MacroDef
        return *this;
    }

    virtual const std::string &GetFileName() const // name of the current file
    {
        return s_strEmpty;
    }
};

typedef std::vector<CMacroDef> CMacroDefs;

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

    virtual const std::string &GetCurrLine(std::string &str); // Read the current line

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

    virtual const std::string &GetFileName() const { return s_strEmpty; }
};

#if 0
class CRepeatDefs : public CArray<CRepeatDef, CRepeatDef&>
{
};
#endif

typedef std::vector<CRepeatDef> CRepeatDefs;

//-----------------------------------------------------------------------------

class CSourceText : public CSource
{
private:
    CInput input;

public:
    CSourceText(const std::string &file_in_name)
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

    virtual const std::string &GetCurrLine(std::string &str) // reading less than an entire line
    {
        str.reserve(1024 + 4);

        return input.read_line(str);
    }

    virtual int GetLineNo() const // read the line number
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

    void Include(const std::string &fname, CDebugInfo* pDebugInfo= NULL) // include file
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
   
    virtual const std::string &GetFileName() const
    {
        return input.get_file_name();
    }
};

//-----------------------------------------------------------------------------

#if 0
class CSourceStack : CTypedPtrArray<CObArray,CSource *> // Stack of objects that are the source of the rows
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
        for (int i = m_nIndex; i >= 0; i--)
            GetAt(i)->Fin(0);
    }

    void Push(CSource *pSrc) // Add an item to the top of the stack
    {
        ++m_nIndex;
        SetAtGrow(m_nIndex,pSrc);
    }

    CSource *Peek() // Check the item at the top of the stack
    {
        return m_nIndex < 0 ? NULL : GetAt(m_nIndex);
    }

    CSource *Pop() // Remove an item from the stack
    {
        return GetAt(m_nIndex--);
    }

//  void RemoveAll()
//  { RemoveAll();  m_nIndex = -1; }

    CSource* FindMacro() // Find the last macro
    {
        for (int i=m_nIndex; i>=0; i--)
            if (CMacroDef* pSrc= dynamic_cast<CMacroDef*>(GetAt(i)))
                return pSrc;
        return NULL;
    }
    CSource* FindRepeat() // Find the last repetition
    {
        for (int i=m_nIndex; i>=0; i--)
            if (CRepeatDef* pSrc= dynamic_cast<CRepeatDef*>(GetAt(i)))
                return pSrc;
//      if (GetAt(i)->IsRepeat())
//        return GetAt(i);
        return NULL;
    }
};

#endif

typedef std::vector<CSource*> CSourceStack;

//-----------------------------------------------------------------------------
class CMarkArea;

class CAsm6502 : public CAsm /*, public CObject */
{
    friend class CMacroDef;

    std::string current_line;
    const char *ptr;                // To keep track of the current line
    const char *err_start;
    const char *ident_start;        // Position of the identifier on the line
    const char *ident_fin;          // Position of the end of the identifier on the line

    bool check_line;                // flag: true - Analysis of one line, false - Program
    uint32_t origin;
    bool originWrapped;             // true - if the command counter "wrapped"
    uint32_t program_start;         // Beginning of the program
    uint32_t mem_mask;              // Processor memory limit (mask), normally $FFFF
    int local_area;                 // Local label area number
    int proc_area;                  // Local label area number
    int macro_local_area;           // Area number of the local macro definitions
    int pass;                       // Transition number (1 or 2)
    //int conditional;              // Conditional assembly - depth level
    std::string include_fname;
    std::string user_error_text;    // User error text (.ERROR directives)

    const char *instr_start;        // To remember the beginning and 
    const char *instr_fin;          // end of an instruction in a line
    CMacroDefs macros;              // Macro definitions

    CSource *text;                  // Current source text
    CSourceText entire_text;        // First (initial) source text
    CRepeatDef *pRept;

    // leksyka:
    CLeksem get_dec_num();          // Interpretation of a decimal number
    CLeksem get_hex_num();          // Interpretation of a hexadecimal number
    CLeksem get_bin_num();          // Interpretation of binary number
    CLeksem get_char_num();         // Interpretation of the character constant
    std::string* get_ident();       // Extract the string
    CLeksem get_string(char lim);	// Extracting a string of characters
    CLeksem eat_space();			// Skip whitespace
    bool proc_instr(const std::string &str, OpCode &code);
    bool asm_instr(const std::string &str, InstrType &it);
    CLeksem next_leks(bool nospace= true); // Get the next symbol
    bool next_line();               // Load the next line

    COutputMem *out;				// memory for the object code
    CMarkArea *markArea;			// to mark used memory areas with 'out'
    CIdentTable local_ident;        // Array of local identifiers
    CIdentTable proc_local_ident;   // Array of local identifiers
    CIdentTable global_ident;       // Array of global identifiers
    CIdentTable macro_name;         // Array of macro definition names
    CIdentTable macro_ident;        // Array of IDs in macro expansions
    CDebugInfo *debug;              // Commissioning information for the simulator

    //std::string err_file;         // Name of the file whose reading resulted in an error
    //int err_line;                 // Line number where the error was encountered
    std::string err_ident;          // Name of the label that caused the error

    bool temporary_out;             // Flag -'out' has been allocated in the constructor
    bool abort_asm;                 // Variable to force assembly to be interrupted externally
    bool is_aborted()
    {
        return abort_asm ? abort_asm=false, true : false;
    }

    bool add_ident(const std::string &ident, CIdent &inf);
    Stat def_ident(const std::string &ident, CIdent &inf);
    Stat chk_ident(const std::string &ident, CIdent &inf);
    Stat chk_ident_def(const std::string &ident, CIdent &inf);
    Stat def_macro_name(const std::string &ident, CIdent &inf);
    Stat chk_macro_name(const std::string &ident);
    std::string format_local_label(const std::string &ident, int area);
    // Interpretation of processor instructions
    Stat proc_instr_syntax(CLeksem &leks, CodeAdr &mode, Expr &expr, Expr &expr_bit, Expr &expr_zpg);
    // Interpretation of the directive
    Stat asm_instr_syntax_and_generate(CLeksem &leks, InstrType it, const std::string *pLabel);

    CMacroDef *get_new_macro_entry()
    {
        macros.resize(macros.size() + 1);
        return &macros[macros.size() - 1];
    }

    CMacroDef *get_last_macro_entry()
    {
        ASSERT(!macros.empty());
        return &macros[macros.size() - 1];
    }

    int get_last_macro_entry_index()
    {
        ASSERT(!macros.empty());
        return macros.size() - 1;
    }

//  Stat (CLeksem &leks);
    int find_const(const std::string& str);
    Stat predef_const(const std::string &str, Expr &expr, bool &found);
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
    Stat expression(CLeksem &leks, Expr &expr, bool str= false); // Expression expansion
    bool is_expression(const CLeksem &leks);
    Stat assemble_line();           // Line interpretation
    Stat assemble();
    const char *get_next_line();    // Load the next line into the assembly
    const char *play_macro();       // Reading the next line of the macro
    const char *play_repeat();      // Reading the next replay line
    //CPtrStack<CSource> source;    // Stack of objects returning source rows
    CSourceStack source;            // Stack of objects returning source rows
    void asm_start();               // Start assembly
    void asm_fin();                 // Complete assembly
    void asm_start_pass();          // Start taking over assembly
    void asm_fin_pass();            // Complete assembly takeover
    Stat chk_instr_code(OpCode &code, CodeAdr &mode, Expr expr, int &length);
    void generate_code(OpCode code, CodeAdr mode, Expr expr, Expr expr_bit, Expr expr_zpg);
    Stat inc_prog_counter(int dist);
    Stat look_for_endif();          // Search for .ENDIF or .ELSE
    //int get_line_no();            // Line number (for debug info)
    //FileUID get_file_UID();       // File id (for debug info)
    void generate_debug(uint32_t addr, int line_no, FileUID file_UID);
    Stat generate_debug(InstrType it, int line_no, FileUID file_UID);
    void generate_debug();
    Stat look_for_endm();
    Stat record_macro();
    //void MacroExpandStart(CMacroDef *pMacro); // Switch to macro definition expansion mode
    //void MacroExpandFin();        // Ends macro expansion mode
    CMacroDef *in_macro;            // Currently recording macro or NULL
    CMacroDef *expanding_macro;     // Currently expanding macro or NULL
    //CPtrStack<CMacroDef> expand_macros; // List of drop-down macros
    CRepeatDef *repeating;          // Current replay (.REPEAT)
    //CPtrStack<CRepeat Def> repeats; // Replay list
    Stat record_rept(CRepeatDef *pRept);
    Stat look_for_repeat();         // Search for .ENDR or .REPEAT
    int reptInit;                   // Value to initialize the number of repetitions
    int reptNested;                 // Nest counter .REPEAT (when registering)
    //void RepeatStart(CRepeatDef *pRept);
    //void RepeatFin();
    bool b_listing;

    //static int __cdecl asm_str_key_cmp(const void *elem1, const void *elem2);
    static int asm_str_key_cmp(const void *elem1, const void *elem2);

    struct ASM_STR_KEY
    {
        const char *str;
        CAsm::InstrType it;
    };

    CConditionalAsm conditional_asm;

    class CListing
    {
    private:
        std::FILE *m_file;
        std::string m_Str; // Current listing line

        void Open(const char *fname)
        {
            m_file = std::fopen(fname, "w");
        }

        void Close()
        {
            if (m_file != nullptr)
            {
                fclose(m_file);
                m_file = nullptr;
            }
        }

    public:
        CListing()
            : m_file(nullptr)
        {
        }

        CListing(const char *fname);

        ~CListing()
        {
            if (m_file != nullptr)
                Close();
        }

        void Remove();
        void NextLine();
        void AddCodeBytes(uint32_t addr, int code1 = -1, int code2 = -1, int code3 = -1, int code4 = -1);
        void AddValue(uint32_t val);
        void AddBytes(uint32_t addr, uint16_t mask, const uint8_t mem[], int len);
        void AddSourceLine(const char *line);

        bool IsOpen() const { return m_file != nullptr; }
    } listing;

    void init();
    void init_members();

public:
    uint8_t bProc6502;  // 0=6502, 1=65C02 or 6501, 2=65816

    static bool case_insensitive; //true -> lowercase and uppercase letters in labels are not distinguished
    static bool swapbin;
    static uint8_t forcelong;
    static bool generateBRKExtraByte; // Generate an extra byte after the BRK instruction?
    static uint8_t BRKExtraByte;      // The value of the additional byte after the BRK instruction

    CAsm6502(const std::string &file_in_name,
             COutputMem *out = NULL,
             CDebugInfo *debug = NULL,
             CMarkArea *area = NULL,
             uint8_t proc6502 = 0,
             const char *listing_file = NULL)
        : entire_text(file_in_name)
        , out(out)
        , debug(debug)
        , markArea(area)
        , bProc6502(proc6502)
        , listing(listing_file)
    {
        init();
    }

    CAsm6502(wxWindow *pWnd, 
             COutputMem *out = NULL,
             CDebugInfo *debug = NULL,
             CMarkArea *area = NULL,
             uint8_t proc6502 = 0,
             const char *listing_file = NULL)
        : entire_text(pWnd)
        , out(out)
        , debug(debug)
        , markArea(area)
        , bProc6502(proc6502)
        , listing(listing_file)
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
           ::delete out;

        if (text)
            text->Fin(0);

        if (pRept)
            pRept->Fin(0);
    }

    // syntax check on 'str' line
    // in 'instr_idx_start' the returned position of the statement on the line or 0
    // in 'instr_idx_fin' the returned position of the end of the statement on the line or 0
    Stat CheckLine(const char *str, int &instr_idx_start, int &instr_idx_fin);

    void Abort()
    {
        abort_asm = true;
    }

    std::string GetErrMsg(Stat stat); // Error description

    Stat Assemble() // Assembly
    {
        return assemble();
    }

    uint32_t GetProgramStart() // Beginning of the program
    {
        return program_start;
    }
};

#endif /* M6502_H__ */
