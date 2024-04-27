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

#ifndef CX_MULTI_DOC_TEMPLATE_H__
#define CX_MULTI_DOC_TEMPLATE_H__

/*
 * It isn't entirely clear from the Microsoft docs what the CMultiDocTemplate 
 * actually DOES.  It appears to have something to do with how it builds up
 * the list of file types that can be created in the File->New/Open/Save but
 * I'm not sure.
 * 
 *              -- B.Simonds (April 27, 2024)
 */

#if 0

// modified CMultiDocTemplate class -replaced recognition function
// opened documents

class CXMultiDocTemplate : public CMultiDocTemplate
{
private:
    bool m_bNormalMatch;

public:
    CXMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
                       CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass, bool bNormalMatch = true)
        : m_bNormalMatch(bNormalMatch)
        //, CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
    {
    }

    virtual ~CXMultiDocTemplate()
    {
    }

    virtual Confidence MatchDocType(const char *pathName, CDocument*& rpDocMatch)
    {
        if (m_bNormalMatch)
            return CMultiDocTemplate::MatchDocType(pathName, rpDocMatch);
        else
            return CDocTemplate::noAttempt;
    }

    virtual bool GetDocString(std::string& rString, enum DocStringIndex i) const;

    static bool s_bRegistrationExt;
};

#endif

#endif /* CX_MULTI_DOC_TEMPLATE_H__ */
