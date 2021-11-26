#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* DoGoto searches through the headers of the cards to find a card with */
/* the same pattern as pchBuf */

void FAR DoGoto(pchBuf)
PSTR pchBuf;
    {
    int i;
    int j;
    LPCARDHEADER lpCards;
    LPCARDHEADER lpCardsTmp;
    char buf[80];
    int iNextFirst;

    /* get the header information */
    lpCards = (LPCARDHEADER) GlobalLock(hCards);
    /* for each card starting with the one after the front card */
    lpCardsTmp = lpCards+iFirstCard+1;
    for (i = 1; i <= cCards; ++i)
        {
        if (i + iFirstCard == cCards)
            lpCardsTmp = lpCards;
        /* look for the pattern */
        if (AnsiFind(lpCardsTmp->line, pchBuf, LINELENGTH))
                goto DoneSearching;
        lpCardsTmp++;
        }
DoneSearching:
    GlobalUnlock(hCards);
    if (i <= cCards)     /* found it */
        {
        iNextFirst = iFirstCard + i;
        if (iNextFirst >= cCards)
            iNextFirst -= cCards;
        GetNewCard(iFirstCard, iNextFirst);
        }
    else
        {
        /* couln't find pattern */
        Mylstrcpy((LPSTR)buf, (LPSTR)"\"");
        Mylstrcat((LPSTR)buf, (LPSTR)pchBuf);
        Mylstrcat((LPSTR)buf, (LPSTR)"\"");
        MyMessageBox(IDS_ECANTFIND, buf, MB_OK | MB_ICONEXCLAMATION);
        }
    }

/* FindStrCard looks through the body of each card to find the pattern */
void FAR FindStrCard()
    {
    int i;
    int fFound;
    LPSTR lpch1;
    LPSTR lpText;
    int iCard;
    char buf[80];
    HANDLE hText;
    int ichStart;
    CARDHEADER CardHead;
    CARD Card;
    LPCARDHEADER lpCards;

    /* get a buffer for reading in each card's text */
    hText = GlobalAlloc(GHND, (long)CARDTEXTSIZE);
    if (!hText)
FSC_INSMEM:
        {
        CardfileOkError(IDS_EINSMEMORY);
        return;
        }
    lpText = GlobalLock(hText);
    fFound = FALSE;
    /* get the front card's text */
    GetWindowText(hCardWnd, lpText, CARDTEXTSIZE);
    /* get the current selection (and the starting spot for the search */
    ichStart = HIWORD(SendMessage(hCardWnd, EM_GETSEL, 0, 0L));
    iCard = iFirstCard;
    /* look in for the pattern */
    if (lpch1 = AnsiFind(lpText+ichStart, CurIFind, -1))
        {
        fFound = TRUE;
        goto FS_DONE;
        }
    /* try following cards */
    iCard++;
    if (iCard == cCards)
        iCard = 0;
    /* for each card back to current card */
    for (i = 0; i < cCards-1; ++i)
        {
        /* get text */
        lpCards = (LPCARDHEADER) GlobalLock(hCards);
        lpCards += iCard;
        CardHead = *lpCards;
        GlobalUnlock(hCards);
        ReadCurCardData(&CardHead, &Card, lpText);
        /* get rid of any bitmap */
        if (Card.hBitmap)
            DeleteObject(Card.hBitmap);
        /* look for text */
        if (lpch1 = AnsiFind(lpText, CurIFind, -1))
            {
            fFound = TRUE;
            goto FS_DONE;
            }
        iCard++;
        if (iCard == cCards)
            iCard = 0;
        }
    /* if we still haven't found it, get current text again */
    GetWindowText(hCardWnd, lpText, CARDTEXTSIZE);
    /* look up to beginning of selection */
    if (lpch1 = AnsiFind(lpText, CurIFind, ichStart))
        {
        fFound = TRUE;
        goto FS_DONE;
        }

FS_DONE:
    /* if we found it */
    if (fFound)
        {
        /* and not on font card, get new data */
        if (iCard != iFirstCard && !GetNewCard(iFirstCard, iCard))
            goto FSC_INSMEM;
        /* set selection */
        SendMessage(hCardWnd, EM_SETSEL, 0, MAKELONG((int)(lpch1-lpText), (int)(lpch1-lpText)+Mylstrlen((LPSTR)CurIFind)));
        }
    else
        {
        /* couldn't find it */
        Mylstrcpy((LPSTR)buf, (LPSTR)"\"");
        Mylstrcat((LPSTR)buf, (LPSTR)CurIFind);
        Mylstrcat((LPSTR)buf, (LPSTR)"\"");
        MyMessageBox(IDS_ECANTFIND, buf, MB_OK | MB_ICONEXCLAMATION);
        }
    GlobalUnlock(hText);
    GlobalFree(hText);
    }

/*
    Finds the pKey string within lpSrc.
    SrcLen specifies the length of the lpSrc to be searched.
    return LPSTR to where the Key is if pKey is found in lpSrc, FALSE otherwise.
*/
LPSTR NEAR AnsiFind(lpSrc, pKey, SrcLen)
LPSTR lpSrc;
PSTR  pKey;
short SrcLen;
{
    register LPSTR lpch;
    register PSTR  pKeyTmp;

    for (; *lpSrc && SrcLen; lpSrc++, SrcLen--)
        {
        for (lpch = lpSrc, pKeyTmp = pKey; *pKeyTmp; pKeyTmp++, lpch++)
            {
            if (AnsiUpper((LPSTR)(DWORD)(BYTE)*lpch) != AnsiUpper((LPSTR)(DWORD)(BYTE)*pKeyTmp))
                break;
            if (((PSTR)AnsiNext(pKeyTmp)) - pKeyTmp > 1)
                {
                if (pKeyTmp[1] != lpch[1])
                    break;
                pKeyTmp++;
                lpch++;
                }
            }
        if (!*pKeyTmp)
            return lpSrc;
        if (AnsiNext(lpSrc) - lpSrc > 1)
            {
            lpSrc++;
            SrcLen--;
            }
        }
    return FALSE;
}
