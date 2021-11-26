#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* in card mode, cardfile uses an edit control for the body */
/* of the frontmost card.  Since a bitmap can be included on the card */
/* and edit controls don't support bitmaps, cardfile needs to subclass */
/* the edit control and intercept a number of messages */

long far PASCAL CardWndProc(hwnd, message, wParam, lParam)
HWND hwnd;
unsigned message;
WORD wParam;
DWORD lParam;
{
    PAINTSTRUCT ps;

    switch (message)
        {
        case WM_LBUTTONDOWN:
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
            /* if we're editing the bitmap, we want to handle these */
            if (EditMode == IDM_BITMAP)
                BMMouse(hwnd, message, wParam, MAKEPOINT(lParam));
            /* otherwise, just give them to the edit control */
            else
                goto PassMessageOn;
            break;
        case WM_CHAR:
            /* could be a ctrl-letter combination, which means */
            /* move to the next card starting with that letter. */
            /* check, and if not and if in text mode, pass it on */
            if (!CardChar(wParam) && EditMode == IDM_TEXT)
                goto PassMessageOn;
            break;
        case WM_KEYDOWN:
            /* could be page up-down, home, end, or move bitmap */
            if (!CardKey(wParam) && EditMode == IDM_TEXT)
                /* if not, and if in text mode, pass message on */
                goto PassMessageOn;
            break;
        case WM_PAINT:
            /* if setting window text, don't bother painting bitmap */
            if (fSettingText)
                break;
            BeginPaint(hwnd, (LPPAINTSTRUCT)&ps);
            /* first let edit control draw text */
            CallWindowProc(lpEditWndProc, hCardWnd, message, ps.hdc, 0L);
            /* then lay bitmap down on top of it */
            CardPaint(ps.hdc);
            EndPaint(hwnd, (LPPAINTSTRUCT)&ps);
            break;
        case WM_SETFOCUS:
            if (EditMode == IDM_TEXT)
                goto PassMessageOn;
            /* if in bitmap mode, draw rectangle around bitmap */
            else
                TurnOnBitmapRect();
            break;
        case WM_KILLFOCUS:
            if (EditMode == IDM_TEXT)
                goto PassMessageOn;
            /* take away rectangle around bitmap */
            else
                TurnOffBitmapRect();
            break;
        default:
PassMessageOn:
            return(CallWindowProc(lpEditWndProc, hCardWnd, message, wParam, lParam));
            break;
        }
    return(0L);
}

/* paint the bitmap on top of the text in the edit control */
void FAR CardPaint(hDC)
HDC hDC;
    {
    HANDLE hOldObject;
    HDC hMemoryDC;
    long Selection;
    long rgbText;
    int i;
    int cOnes;
    HWND hWndFocus;

    if (CardPhone == IDM_PHONEBOOK)
        return;
    hWndFocus = GetFocus();
    /* draw bitmap if there is one */
    if (CurCard.hBitmap)
        {
        /* if currently have focus, turn it off (so text selection goes away) */
        if (hWndFocus == hCardWnd)
            SetFocus(NULL);
        if (hMemoryDC = CreateCompatibleDC(hDC))
            {
            /* save old bitmap */
            hOldObject = SelectObject(hMemoryDC, CurCard.hBitmap);
            /* blt current bitmap on top of text */
            BitBlt(hDC, CurCard.xBitmap, CurCard.yBitmap, CurCard.cxBitmap, CurCard.cyBitmap, hMemoryDC, 0, 0, SRCAND);
            /* select in old object */
            SelectObject(hMemoryDC, hOldObject);
            DeleteDC(hMemoryDC);
            }
        /* if had focus, turn it back on, so text is selected */
        if (hWndFocus == hCardWnd)
            SetFocus(hCardWnd);
        }
    /* if in bitmap mode, and have focus, draw rectangle around bitmap */
    if (EditMode == IDM_BITMAP && hWndFocus == hCardWnd)
        DrawXorRect(hDC, &dragRect);
    }

/* get rid of a card, and remove it from the data structure */
void DeleteCard(iCard)
int iCard;
    {
    LPCARDHEADER lpCards;

    cCards--;
    lpCards = (LPCARDHEADER) GlobalLock(hCards);
    RepMov(lpCards+iCard, lpCards+iCard+1, (cCards-iCard)*sizeof(CARDHEADER));
    GlobalUnlock(hCards);
    }

/* adding a new card, insert it in the linked list in alphabetical order */
/* based on the header */
int FAR AddCurCard()
    {
    LPCARDHEADER lpCards;
    LPCARDHEADER lpTCards;
    char *pch1;
    unsigned char c1, c2;
    LPSTR lpch2;
    int i, fKj1, fKj2;

    lpCards = (LPCARDHEADER) GlobalLock(hCards);
    lpTCards = lpCards;
    /* scan for right place to insert the card */
    for (i = 0; i < cCards; i++)
        {
        fKj1 = fKj2 = 0;
        /* compare new header and current one */
        for(pch1 = CurCardHead.line, lpch2 = lpTCards->line; *pch1; ++pch1, ++lpch2)
            {
            /* do not call ansi upper if it is the second byte of the kanji stream */
            c1 = KanjiXlat((fKj1 ? *pch1 : AnsiUpper((LPSTR)(DWORD)(BYTE)*pch1)), (short *)&fKj1);
            c2 = KanjiXlat((fKj2 ? *lpch2 : AnsiUpper((LPSTR)(DWORD)(BYTE)*lpch2)), (short *)&fKj2);
            if (c1 != c2)
                break;
            }
        /* if less than or equal, found the spot */
        if (c1 <= c2)
            break;
        lpTCards++;
        }
    if (i != cCards)
        RepMovUp(lpTCards+1, lpTCards, (cCards - i) * sizeof(CARDHEADER));
    *lpTCards = CurCardHead;
    GlobalUnlock(hCards);
    cCards++;
    return(i);
    }

KanjiXlat(c, fKj)
unsigned char c;
short *fKj;
{
    /* if (fKj) ==> this is the second byte of a kanji char,
       no translation */
    if (*fKj)
            {
            *fKj = 0;
            return c;
            }

    if (c < 0x80)
        return c;
    if (c <= 0x9f)
        {
        *fKj = 1;
        return c + 0x40;
        }
    if (c <= 0xdf)
        return c - 0x20;
    *fKj = 1;
    return c;
}

/* save the data on the current card */
BOOL SaveCurrentCard(iCard)
int iCard;
    {
    LPCARDHEADER lpCards;
    HANDLE hText;
    LPSTR lpText;

    /* save the card if it's dirty */
    /* dirty if bitmap has changed or edittext has changed */
    if (CurCardHead.flags & (FDIRTY+FNEW) || SendMessage(hCardWnd, EM_GETMODIFY, 0, 0L))
        {
        /* get a buffer for text on card */
        hText = GlobalAlloc(GHND, (long)CARDTEXTSIZE);
        if (!hText)
            {
            CardfileOkError(IDS_EINSMEMSAVE);
            return(FALSE);
            }
        lpText = GlobalLock(hText);
        /* get the text in the edit control */
        GetWindowText(hCardWnd, lpText, CARDTEXTSIZE);
        /* save the card */
        if (WriteCurCard(&CurCardHead, &CurCard, lpText))
            {
            if (CurCardHead.flags & FDIRTY || SendMessage(hCardWnd, EM_GETMODIFY, 0, 0L))
                fFileDirty = TRUE;
            SendMessage(hCardWnd, EM_SETMODIFY, FALSE, 0L);
            CurCardHead.flags &= (!FNEW);
            CurCardHead.flags &= (!FDIRTY);
            CurCardHead.flags |= FTMPFILE;
            /* save the card in the linked list.  information about */
            /* location of card's data has changed, which is why the */
            /* header needs to be saved */
            lpCards = (LPCARDHEADER) GlobalLock(hCards);
            lpCards += iCard;
            *lpCards = CurCardHead;
            GlobalUnlock(hCards);
            }
        else
            return(FALSE);
        GlobalUnlock(hText);
        GlobalFree(hText);
        }
    /* if it has a bitmap, get rid of it */
    if (CurCard.hBitmap)
        {
        DeleteObject(CurCard.hBitmap);
        CurCard.hBitmap = 0;
        }
    return(TRUE);
    }

/* get the data for the current front card */
void SetCurCard(iCard)
int iCard;
    {
    LPCARDHEADER lpCards;
    LPSTR lpText;
    HANDLE hText;

    /* allocate a buffer to read the text into */
    hText = GlobalAlloc(GHND, (long)CARDTEXTSIZE);
    if (!hText)
        {
        CardfileOkError(IDS_EINSMEMREAD);
        return;
        }
    lpCards = (LPCARDHEADER) GlobalLock(hCards);
    lpCards += iCard;
    /* get the current header */
    CurCardHead = *lpCards;
    GlobalUnlock(hCards);
    lpText = GlobalLock(hText);
    /* read the data */
    if (!ReadCurCardData(&CurCardHead, &CurCard, lpText))
        CardfileOkError(IDS_ECANTREADPICT);
    /* set the contents of the edit control */
    SetEditText(lpText);
    GlobalUnlock(hText);
    GlobalFree(hText);
    /* if there's a bitmap */
    if (CurCard.hBitmap)
        /* calculate surrounding rect */
        SetRect((LPRECT)&dragRect, CurCard.xBitmap, CurCard.yBitmap, CurCard.xBitmap+CurCard.cxBitmap, CurCard.yBitmap+CurCard.cyBitmap);
    else
        /* otherwise it's tiny, just to show where it will be pasted */
        SetRect((LPRECT)&dragRect, 5, 5, 5+CharFixWidth, 5+CharFixHeight);
    }
