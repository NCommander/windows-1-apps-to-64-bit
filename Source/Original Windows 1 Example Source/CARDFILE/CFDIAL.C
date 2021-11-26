#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* this routine looks for a phone number on a card.  If there is a  */
/* selection, it tries that text, otherwise it looks first in the */
/* header and then the main body of the text */

void FAR GetPhoneNumber(pchBuf, cchMax)
PSTR pchBuf;
int cchMax;
    {
    LPCARDHEADER lpCard;
    int fFound = FALSE;
    LPSTR lpText;
    HANDLE hText;
    unsigned long lSelection;

    lSelection = SendMessage(hCardWnd, EM_GETSEL, 0, 0L);
    /* if no selection, look in header */
    if (HIWORD(lSelection) == LOWORD(lSelection))
        {
        lpCard = (LPCARDHEADER) GlobalLock(hCards) + iFirstCard;
        fFound = ParseNumber((LPSTR)(lpCard->line), pchBuf, cchMax);
        GlobalUnlock(hCards);
        }
    /* if haven't found one yet */
    if (!fFound)
        {
        /* make a buffer for the body */
        hText = GlobalAlloc(GHND, (long)CARDTEXTSIZE);
        if (hText)
            {
            /* get the text from the edit control */
            lpText = GlobalLock(hText);
            GetWindowText(hCardWnd, lpText, CARDTEXTSIZE);
            /* if a selection, look in it */
            if (HIWORD(lSelection) != LOWORD(lSelection))
                {
                Mylstrcpy(lpText, lpText+LOWORD(lSelection));
                *(lpText + (HIWORD(lSelection) - LOWORD(lSelection))) = 0;
                }
            fFound = ParseNumber(lpText, pchBuf, cchMax);
            GlobalUnlock(hText);
            GlobalFree(hText);
            }
        }
    if (!fFound)
        *pchBuf = 0;
    }

/* this routine finds a phone number in a string.  It will only find */
/* phone numbers which meet the following requirements: */
/* count of characters in number > 5 */
/* must be contiguous (i.e. no spaces allowed) */
/* must have a '-' in it */
/* can only contain a character in "-0123456789@,()*#" */

BOOL ParseNumber(lpSrc, pchBuf, cchMax)
LPSTR lpSrc;
PSTR pchBuf;
int cchMax;
    {
    LPSTR lpchTmp;
    LPSTR lpchEnd;
    PSTR pchTmp;
    int fValid;
    char ch;

    for (lpchTmp = lpSrc; *lpchTmp; ++lpchTmp)
        {
        pchTmp = pchBuf;
        lpchEnd = lpchTmp;
        while(pchTmp - pchBuf < cchMax)
            {
            ch = *lpchEnd++;
            if (ch == '-')
                {
                fValid = TRUE;
                *pchTmp++ = ch;
                }
            else if ((ch >= '0' && ch <= '9') || ch == '@' ||
                     ch == ',' || ch == '(' || ch == ')' || ch == '*' || ch == '#')
                *pchTmp++ = ch;
            else
                {
                *pchTmp = 0;
                break;
                }
            }
        if (fValid && pchTmp - pchBuf > 5)
            return(TRUE);
        }
    return(FALSE);
    }

/* tell the modem to dial a number.  This only works with fully */
/* Hayes compatible modems. */
void FAR DoDial(pchNumber)
PSTR pchNumber;
    {
    int cid;
    char rgchComm[5];
    char cmdBuf[80];
    char inBuf[80];
    int cch;
    COMSTAT ComStatInfo;
    long oldtime;

    Mylstrcpy((LPSTR)rgchComm, (LPSTR)"COM2");
    if (fModemOnCom1)
        rgchComm[3] = '1';
    if ((cid = OpenComm((LPSTR)rgchComm, 256, 256))>=0)
        {
        SetPortState(cid);
        GetCommError(cid, (COMSTAT FAR *)&ComStatInfo);
        cch = MakeDialCmd(cmdBuf, 80, pchNumber);
        while (WriteComm(cid, (LPSTR)cmdBuf, cch) <= 0)
            {
            GetCommError(cid, (COMSTAT FAR *)&ComStatInfo);
            FlushComm(cid, 0);
            FlushComm(cid, 1);
            }
        oldtime = GetCurrentTime();
        while(TRUE)
            {
            GetCommError(cid, (COMSTAT FAR *)&ComStatInfo);
            if (GetCurrentTime() - oldtime > 3000)
                {
                CardfileOkError(IDS_ENOMODEM);
                goto DoneDialing;
                }
            if (!ComStatInfo.cbOutQue)
                break;
            }
        FlushComm(cid, 1);
        MyMessageBox(IDS_PICKUPPHONE, NULL, MB_OKCANCEL | MB_ICONQUESTION);
        Mylstrcpy((LPSTR)cmdBuf, (LPSTR)"ATH0");
        cmdBuf[4] = 0x0d;
        while(WriteComm(cid, (LPSTR)cmdBuf, 5) <= 0)
            {
            GetCommError(cid, (COMSTAT FAR *)&ComStatInfo);
            FlushComm(cid, 0);
            FlushComm(cid, 1);
            }
        while(TRUE)
            {
            GetCommError(cid, (COMSTAT FAR *)&ComStatInfo);
            if (!ComStatInfo.cbOutQue)
                break;
            }
DoneDialing:
        CloseComm(cid);
        }
    else
        CardfileOkError(IDS_ECANTDIAL);
    }

/* initialize the comm port */
void SetPortState(cid)
int  cid;
    {
    DCB   dcb;
    char rgchPortInfo[30];
    char *pch;
    char rgchPort[6];

    if (GetCommState(cid, (DCB FAR *) &dcb)!=-1)
        {
        if (fFastModem)
            dcb.BaudRate = 1200;
        else
            dcb.BaudRate = 300;
        Mylstrcpy((LPSTR)rgchPort, (LPSTR)"COM1:");
        rgchPort[3] = '1' + cid;
        GetProfileString((LPSTR)"Ports", (LPSTR)rgchPort, (LPSTR)"300,n,8,1", (LPSTR)rgchPortInfo, 30);
        for (pch = rgchPortInfo; *pch && *pch != ','; ++pch)
            ;
        while(*pch == ',' || *pch == ' ')
            pch++;
        dcb.Parity = *pch == 'n' ? NOPARITY : (*pch == 'o' ? ODDPARITY : EVENPARITY);
        if (*pch)
            pch++;
        while(*pch == ',' || *pch == ' ')
            pch++;
        dcb.ByteSize = *pch == '8' ? 8 : 7;
        if (*pch)
            pch++;
        while (*pch == ',' || *pch == ' ')
            pch++;
        dcb.StopBits = *pch == '2' ? 2 : 0;
        SetCommState((DCB FAR *)&dcb);
        }
    }

/* Create a modem command for dialing */
int MakeDialCmd(pBuf, cchMax, pchNumber)
PSTR pBuf;
int  cchMax;
PSTR pchNumber;
    {
    PSTR pch1;
    PSTR pch2;
    int   cb;
    char  rgchCmd[80];
    char ch;

    Mylstrcpy((LPSTR)rgchCmd, (LPSTR)"ATD");
    for (pch2 = rgchCmd; *pch2; ++pch2)
        ;
    *pch2++ = fTone ? 'T' : 'P';
    for (pch1 = pchNumber; ch = *pch1++; )
        if ((ch >= '0' && ch <= '9') || (ch == ',') || (ch == '#') || (ch == '*'))
            *pch2++ = ch;
        else if (ch == '@')
            {
            *pch2++ = ',';
            *pch2++ = ',';
            *pch2++ = ',';
            }
        else if (ch == 'P' || ch == 'T')
            {
            *pch2++ = 'D';
            *pch2++ = ch;
            }

    *pch2++ = ';';
    *pch2++ = 0x0d;
    *pch2 = 0;

    cb = Mylstrlen((LPSTR)rgchCmd);
    if (cchMax < pch2 - rgchCmd)
        {
        rgchCmd[cchMax] = 0;
        cb = cchMax;
        }
    Mylstrcpy((LPSTR)pBuf, (LPSTR)rgchCmd);
    return cb;
    }
