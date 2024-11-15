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

#ifndef MACRO_DEF_H__
#define MACRO_DEF_H__

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

    int m_nLineNo;                  // Current line number (when reading)
    int m_nFirstLineNo;             // The line number from which the macro is called
    CAsm::FileUID m_nFirstLineFuid; // The ID of the file from which the macro is called

public:
    std::string m_strName; // Macro name
    bool m_bFirstCodeLine; // Flag for reading the first line of the macro containing the instruction. 6502

    CMacroDef()
        : param_names()
        , m_nParams(0)
        , m_nLineNo(0)
        , m_nFirstLineNo(-1)
        , m_nFirstLineFuid(CAsm::FileUID(-1))
        , m_bFirstCodeLine(true)
    {}

    ~CMacroDef()
    {}

    int AddParam(const std::string &strParam) // Adding the name of the next parameter
    {
        if (strParam == CAsm::MULTIPARAM)
        {
            m_nParams = -(m_nParams + 1);
            return 1; // end of parameter list
        }

        CIdent intr;

        if (param_names.contains(strParam))
            return -1; // Repeated parameter name!

        param_names.set(strParam, CIdent(CIdent::I_VALUE, m_nParams));

        ++m_nParams;
        return 0;
    }

    // Number of passed parameters
    int GetParamsFormat() const
    {
        return m_nParams;
    }

    virtual bool GetCurrLine(std::string &str);

    virtual int GetLineNo() const
    {
        return CRecorder::GetLineNo(m_nLineNo - 1);
    }

    int GetFirstLineNo() const { return m_nFirstLineNo; }

    CAsm::FileUID GetFirstLineFileUID() { return m_nFirstLineFuid; }

    void MacroStart(CConditionalAsm* cond, int line, CAsm::FileUID file) // prepare for reading
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
    CAsm::Stat ParseArguments(CToken &leks, CAsm6502 &asmb);

    CAsm::Stat ParamLookup(CToken &leks, const std::string& param_name, Expr &expr, bool &found, CAsm6502 &asmb);
    CAsm::Stat ParamLookup(CToken &leks, int param_number, Expr &expr, CAsm6502 &asmb);
    CAsm::Stat AnyParamLookup(CToken &leks, CAsm6502 &asmb);

    CAsm::Stat ParamType(const std::string &param_name, _Out_ bool &found, _Out_ int &type);
    CAsm::Stat ParamType(int param_number, _Out_ bool &found, _Out_ int &type);

    //virtual bool IsMacro() // The data source is the expanded macro
    //{ return true; }

    CMacroDef &operator= (const CMacroDef &)
    {
        ASSERT(false); // objects of type C cannot be assigned to MacroDef
        return *this;
    }

    virtual std::string GetFileName() const // name of the current file
    {
        return "";
    }
};

#endif /* MACRO_DEF_H__ */
