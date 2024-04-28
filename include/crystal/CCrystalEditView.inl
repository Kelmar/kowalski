////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalEditView.inl
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Inline functions of Crystal Edit classes
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef CCrystalEditView_INL_INCLUDED__
#define CCrystalEditView_INL_INCLUDED__

#include "CCrystalEditView.h"

CE_INLINE bool CCrystalEditView::GetOverwriteMode() const
{
	return m_bOvrMode;
}

CE_INLINE void CCrystalEditView::SetOverwriteMode(bool bOvrMode /*= true */)
{
	m_bOvrMode = bOvrMode;
}

CE_INLINE bool CCrystalEditView::GetDisableBSAtSOL() const
{
	return m_bDisableBSAtSOL;
}

CE_INLINE bool CCrystalEditView::GetAutoIndent() const
{
	return m_bAutoIndent;
}

CE_INLINE void CCrystalEditView::SetAutoIndent(bool bAutoIndent)
{
	m_bAutoIndent = bAutoIndent;
}

#endif /* CCrystalEditView_INL_INCLUDED__ */
