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

// Odczyt i zapis kodu wynikowego w postaci Intel-HEX

#include "StdAfx.h"
#include "IntelHex.h"
#include "resource.h"
#include <TCHAR.h>
#include "MarkArea.h"
#include "Sym6502.h"


void CIntelHex::SaveHexFormat(CArchive &archive, COutputMem &mem, CMarkArea &area, int prog_start/*= -1*/)
{
  TCHAR buf[80], *ptr;

  for (UINT part=0; part<area.GetSize(); part++)
  {
     int start,end;

     area.GetPartition(part,start,end);
     ASSERT(start >= 0 && start <= 0xFFFF);
     ASSERT(end >= 0 && end <= 0xFFFF);
     ASSERT(start <= end);

     if (start > 0xFFFF) // 1.3.3  support for 24 bit addressing
     {
        int sum = -(((start>>24) & 0xFF) + ((start>>16) & 0xFF) + 6) & 0xFF;
	    ptr = buf + wsprintf(buf,_T(":%02X%02X%02X%02X%02X%02X%02X\r\n"),2,0,0,4,    //generates bank address line
	       0, (start>>16) & 0xFF, sum);
	    archive.Write(buf,sizeof(TCHAR)*(ptr-buf));
     }

     const int STEP= 0x10;
     for (int i=start; i<=end; i+=STEP)
     {
        int sum= 0;				// suma kontrolna
        int lim= min(i+STEP-1, end);
	    // pocz¹tek wiersza: iloœæ danych, adres (hi, lo), zero
        int cnt= i+STEP<=end ? STEP : end-i+1;	// iloœæ bajtów do wys³ania (w wierszu)
		  
	    ptr = buf + wsprintf(buf,_T(":%02X%02X%02X%02X"),cnt,(i>>8)&0xFF,i&0xFF,0);
        sum += cnt + ((i>>8)&0xFF) + (i&0xFF);

	    for (int j=i; j<=lim; j++)
        {
          ptr += wsprintf(ptr,_T("%02X"),mem[j]);
	      sum += mem[j];
        }		// suma wszystkich bajtów w wierszu musi byæ równa zeru
        ptr += wsprintf(ptr,_T("%02X\r\n"),-sum & 0xFF);	// wygenerowanie bajtu kontrolnego
        archive.Write(buf,sizeof(TCHAR)*(ptr-buf));
     }
  }
  if (prog_start == -1)
     prog_start = 0;
  
  ASSERT(prog_start >= 0 && prog_start <= 0xFFFF);
  if (prog_start > 0xFFFF) // 1.3.3  added support for 24-bit start address
  {
    int sum= -( ((prog_start>>16)&0xFF)+((prog_start>>8)&0xFF) + (prog_start&0xFF) + 9 ) & 0xFF;
	ptr = buf + wsprintf(buf,_T(":%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n:%02X%02X%02X%02X%02X\r\n"),4, 0, 0, 5, 0,
      ((prog_start>>16)&0xFF),(prog_start>>8)&0xFF,prog_start&0xFF,sum, 0,0,0,1,0xFF);
  }
  else
  {
    int sum= -( ((prog_start>>8)&0xFF) + (prog_start&0xFF) + 1 ) & 0xFF;
    ptr = buf + wsprintf(buf,_T(":%02X%02X%02X%02X%02X\r\n"),0,
      (prog_start>>8)&0xFF,prog_start&0xFF,1,sum);
  }
  archive.Write(buf,sizeof(TCHAR)*(ptr-buf));
}

//-----------------------------------------------------------------------------

UINT CIntelHex::geth(const TCHAR *&ptr, UINT &sum)	// interpretacja dwucyfrowej liczby hex
{
  UINT res= 0;
  for (int i=0; i<2; i++)
  {
    res <<= 4;
    switch (*ptr++)
    {
      case _T('0'):
	break;
      case _T('1'):
	res++;
	break;
      case _T('2'):
	res += 2;
	break;
      case _T('3'):
	res += 3;
	break;
      case _T('4'):
	res += 4;
	break;
      case _T('5'):
	res += 5;
	break;
      case _T('6'):
	res += 6;
	break;
      case _T('7'):
	res += 7;
	break;
      case _T('8'):
	res += 8;
	break;
      case _T('9'):
	res += 9;
	break;
      case _T('A'):
	res += 10;
	break;
      case _T('B'):
	res += 11;
	break;
      case _T('C'):
	res += 12;
	break;
      case _T('D'):
	res += 13;
	break;
      case _T('E'):
	res += 14;
	break;
      case _T('F'):
	res += 15;
	break;
      case _T('a'):
	res += 10;
	break;
      case _T('b'):
	res += 11;
	break;
      case _T('c'):
	res += 12;
	break;
      case _T('d'):
	res += 13;
	break;
      case _T('e'):
	res += 14;
	break;
      case _T('f'):
	res += 15;
	break;
      default:
	throw CIntelHexException(CIntelHexException::E_FORMAT,row);
    }
  }
  sum += res;		// do liczenia sumy kontrolnej
  sum &= 0xFF;
  return res;
}

//-----------------------------------------------------------------------------

void CIntelHex::LoadHexFormat(CArchive &archive, COutputMem &mem, CMarkArea &area, int &prog_start)
{
  TCHAR buf[256];
  int bank = 0;
  bool ps = false;  // 1.3.3 added flag to show 24 bit prog_start was used.
  
  for (row=1; ; row++)
  {
    if (!archive.ReadString(buf,sizeof buf))
      break;

    if (_tcsclen(buf) == sizeof(buf)-1)
      CIntelHexException(CIntelHexException::E_BAD_FORMAT,row);		// za d³ugi wiersz

    const TCHAR *ptr= buf;
    if (*ptr++ != _T(':'))
      throw CIntelHexException(CIntelHexException::E_BAD_FORMAT,row);	// nierozpoznany format
    UINT sum= 0;		// zmienna do liczenia sumy kontrolnej
    UINT cnt= geth(ptr,sum);	// iloœæ bajtów danych
    UINT addr= geth(ptr,sum);
    addr <<= 8;
    addr += geth(ptr,sum);	// adres danych
    UINT info= geth(ptr,sum);	// bajt informacyjny
    switch (info)
    {
      case 0:		// kod programu
       {
	     if (cnt)
           if (theApp.m_global.m_bProc6502==2)
	         area.SetStart(((bank&0xff)<<16) + addr);
		   else 
             area.SetStart(addr);

	     for (UINT i=0; i<cnt; i++)
	     {
	       if (addr > 0xFFFF)
	         throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// za du¿y adres
		   if (theApp.m_global.m_bProc6502==2)   // 1.3.3 added support to store 24-bit address range
	         mem[((bank&0xff)<<16) + addr++] = (UINT8)geth(ptr,sum);
		   else
			 mem[addr++] = (UINT8)geth(ptr,sum);
	     }
	     geth(ptr,sum);		// bajt sumy kontrolnej
	     if (sum)		// b³¹d sumy kontrolnej
	       throw CIntelHexException(CIntelHexException::E_CHKSUM,row);
	     if (cnt)
           if (theApp.m_global.m_bProc6502==2)  // 1.3.3 added support for 24-bit addressing
	         area.SetEnd((bank<<16)&0xff+addr-1);
		   else 
             area.SetEnd(addr-1);
	     break;
       }

      case 1:		// adres uruchomienia
	    if (cnt)
	      throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane
	    geth(ptr,sum);		// bajt sumy kontrolnej
	    if (sum)		// b³¹d sumy kontrolnej
	      throw CIntelHexException(CIntelHexException::E_CHKSUM,row);
	    if (addr > 0xFFFF)
	      throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// za du¿y adres
	    if (!ps) 
		  prog_start = (int)addr;
	    break;

	  case 4:  // 1.3.3 used to set new address bank
        if (theApp.m_global.m_bProc6502!=2)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane 
		if (cnt != 2)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane
		if (addr)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane
        addr= geth(ptr,sum);
        addr <<= 8;
        addr += geth(ptr,sum);	
		if (addr > 0xFF)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane
	    geth(ptr,sum);		// bajt sumy kontrolnej
	    if (sum)		// b³¹d sumy kontrolnej
	      throw CIntelHexException(CIntelHexException::E_CHKSUM,row);
	    bank = (int)addr;
		break;

	  case 5:  // 1.3.3 used to set new prog start for 24 bit addresses
        if (theApp.m_global.m_bProc6502!=2)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane 
        if (cnt != 4)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane
		if (addr)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane
        addr= geth(ptr,sum);
        if (addr)
          throw CIntelHexException(CIntelHexException::E_FORMAT,row);	// nieoczekiwane dane  
        addr = geth(ptr,sum);	
        addr <<= 8;
        addr += geth(ptr,sum);	
        addr <<= 8;
        addr += geth(ptr,sum);	
	    geth(ptr,sum);		// bajt sumy kontrolnej
	    if (sum)		// b³¹d sumy kontrolnej
	      throw CIntelHexException(CIntelHexException::E_CHKSUM,row);
	    prog_start = (int)addr;
		ps = true;
        break;

	  default:		// nieznana wartoœæ
	    throw CIntelHexException(CIntelHexException::E_FORMAT,row);
    }
  }

}

//-----------------------------------------------------------------------------

/*virtual*/ bool CIntelHex::CIntelHexException::GetErrorMessage(LPTSTR lpszError,
  UINT nMaxError, PUINT pnHelpContext/*= NULL*/)
{
  CString msg;
  TCHAR num[16];
  if (pnHelpContext != NULL)
    *pnHelpContext = 0;
  wsprintf(num,_T("%u"),row);
  switch (error)
  {
    case E_BAD_FORMAT:		// b³êdny format pliku
      msg.LoadString(IDS_INTEL_HEX_ERR_2);
      break;
    case E_CHKSUM:		// b³¹d sumy kontrolnej
      AfxFormatString1(msg,IDS_INTEL_HEX_ERR_1,num);
      break;
    case E_FORMAT:		// b³êdny format danych
      AfxFormatString1(msg,IDS_INTEL_HEX_ERR_3,num);
      break;
    default:
      return FALSE;
  }
  _tcsnccpy(lpszError,msg,nMaxError);
  return TRUE;
}
