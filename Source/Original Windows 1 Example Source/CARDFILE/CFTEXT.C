#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* this routine handles all virtual keys passed to cardfile.  These */
/* include pageup, down, home, end, and cursor keys. The first four */
/* bring a new card to the front.  The last four will move the bitmap */
/* around on the card if in bitmap mode */

BOOL CardKey(wParam)
WORD wParam;
    {
    switch(wParam)
        {
        case VK_HOME:
            CardfileScroll(hCardfileWnd, SB_THUMBTRACK, 0);
            goto FinishScroll;
        case VK_END:
            CardfileScroll(hCardfileWnd, SB_THUMBTRACK, cCards-1);
            goto FinishScroll;
        case VK_PRIOR:
            CardfileScroll(hCardfileWnd, SB_LINEUP, 0);
            goto FinishScroll;
        case VK_NEXT:
            CardfileScroll(hCardfileWnd, SB_LINEDOWN, 0);
FinishScroll:
            CardfileScroll(hCardfileWnd, SB_ENDSCROLL, 0);
            return(TRUE);
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
            if (EditMode == IDM_TEXT)
                return(FALSE);
            else
                BMKey(wParam);
            return(TRUE);
        default:
            return(FALSE);
        }
    }

/* this routine looks at characters coming into cardfile, and decides */
/* whether it is a ctrl-character, which tells cardfile to bring the */
/* next card starting with the letter to the front.  If it handles the */
/* character, it returns true.  If not it returns FALSE, so cardfile will */
/* know to pass the character on to the edit control. */

BOOL CardChar(ch)
int ch;
    {
    LPCARDHEADER lpCards;
    LPCARDHEADER lpTmpCard;
    int i;
    int iCardTmp;
    int y;
    RECT rect;
    HDC hDC;

    /* if control key not down, or character is greater than ' ' */
    if ((GetKeyState(VK_CONTROL) >= 0) || ch >= ' ')
        /* don't want to handle it */
        return(FALSE);

    /* find out the letter of the control character */
    ch += 'A' - 1;
    /* scan all cards for the character, starting at the front one */
    lpCards = (LPCARDHEADER) GlobalLock(hCards);
    iCardTmp = iFirstCard+1;
    lpTmpCard = lpCards + iCardTmp;
    for (i = 0; i < cCards; ++i, lpTmpCard++, iCardTmp++)
        {
        /* if at end */
        if (iCardTmp == cCards)
            {
            /* go back to beginning */
            iCardTmp = 0;
            lpTmpCard = lpCards;
            }
        /* case insensitive */
        if (AnsiUpper((LPSTR)(DWORD)(BYTE)*(lpTmpCard->line)) == ch)
            break;
        }
    GlobalUnlock(hCards);
    /* if found one, i will be < cCards */
    if (i < cCards)
        {
        /* if card mode */
        if (CardPhone != IDM_PHONEBOOK)
            {
            /* save current front data */
            SaveCurrentCard(iFirstCard);
            /* get new card's data */
            SetCurCard(iCardTmp);
            }
        else
            {
            /* if phonebook, just remove selection */
            y = (iFirstCard - iTopCard) * CharFixHeight;
            SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y + CharFixHeight);
            hDC = GetDC(hCardfileWnd);
            InvertRect(hDC, (LPRECT)&rect);
            ReleaseDC(hCardfileWnd, hDC);
            }
        iFirstCard = iCardTmp;
        /* if in card mode, repaint headers and redraw */
        if (CardPhone == IDM_CARDFILE)
            {
            SetScrollPos(hCardfileWnd, SB_HORZ, iFirstCard, TRUE);
            PaintNewHeaders();
            InvalidateRect(hCardWnd, (LPRECT)NULL, TRUE);
            }
        /* otherwise bring new card on screen */
        else
            BringCardOnScreen(iFirstCard);  /* will highlight it too */
        }
    return(TRUE);
    }

/* this routine is called in phonebook mode to bring a particular card */
/* into the visible window */
void BringCardOnScreen(iCard)
int iCard;
    {
    int dLines;
    int cLines;
    RECT rect;
    HDC hDC;
    int y;

    /* figure out how many lines there are in the window */
    GetClientRect(hCardfileWnd, (LPRECT)&rect);
    cLines = rect.bottom / CharFixHeight;

    /* put up new highlight in case it's already partly on the screen */
    y = (iCard - iTopCard) * CharFixHeight;
    SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y + CharFixHeight);
    hDC = GetDC(hCardfileWnd);
    InvertRect(hDC, (LPRECT)&rect);
    ReleaseDC(hCardfileWnd, hDC);

    /* see if we have to move the list within the window */
    if (iCard < iTopCard || iCard > iTopCard + cLines - 1)
        {
        if (iCard < iTopCard)
            dLines = (iCard - iTopCard);
        else
            dLines = iCard - iTopCard - cLines + 1;
        iTopCard += dLines;
        /* reset everything */
        SetScrollPos(hCardfileWnd, SB_VERT, iTopCard, TRUE);
        ScrollWindow(hCardfileWnd, 0, -dLines * CharFixHeight, (LPRECT)NULL, (LPRECT)NULL);
        UpdateWindow(hCardfileWnd);
        }
    }

/* this is called to handle virtual keys when cardfile is in phonebook mode. */
/*  These keys just change the current selection */
BOOL PhoneKey(hwnd, wParam)
HWND hwnd;
WORD wParam;
    {
    HDC hDC;
    RECT rect;
    int y;
    int tmpCurCard;
    int cLines;

    GetClientRect(hwnd, (LPRECT)&rect);
    cLines = rect.bottom / CharFixHeight;

    switch(wParam)
        {
        case VK_NEXT:
            tmpCurCard = iFirstCard + cLines;
            if (tmpCurCard >= cCards)
                tmpCurCard = cCards - 1;
            goto SelectNewObject;
        case VK_PRIOR:
            tmpCurCard = iFirstCard - cLines;
            if (tmpCurCard >= 0)
                goto SelectNewObject;
        case VK_HOME:
            tmpCurCard = 0;
            goto SelectNewObject;
        case VK_END:
            tmpCurCard = cCards - 1;
            goto SelectNewObject;
        case VK_UP:
            tmpCurCard = iFirstCard - 1;
            goto SelectNewObject;
        case VK_DOWN:
            tmpCurCard = iFirstCard + 1;
SelectNewObject:
            if (tmpCurCard >= 0 && tmpCurCard < cCards && tmpCurCard != iFirstCard)
                {
                y = (iFirstCard - iTopCard) * CharFixHeight;
                SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y + CharFixHeight);
                hDC = GetDC(hwnd);
                InvertRect(hDC, (LPRECT)&rect);
                ReleaseDC(hwnd, hDC);
                BringCardOnScreen(iFirstCard = tmpCurCard); /* will highlight the right one */
                }
            return(TRUE);
        default:
            return(FALSE);
        }
    }
