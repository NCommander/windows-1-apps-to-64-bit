#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* this is called to handle scroll messages */
BOOL CardfileScroll(hWindow, cmd, pos)
HWND hWindow;
int cmd;
int pos;
    {
    int OldFirst = iFirstCard;
    int NewFirst;
    HDC hDC;
    RECT rect;
    int result = TRUE;

    /* if less that 2 cards, scrolling is a waste */
    if (cCards < 2)
        return(result);
    /* if just started scrolling, save starting position */
    if (!fScrolling)
        iCardStartScroll = iFirstCard;
    /* we're definitely scrolling now */
    fScrolling = TRUE;
    switch (cmd)
        {
        case SB_LINEUP:
            /* go back one card, with a possible wrap */
            iFirstCard--;
            if (iFirstCard < 0)
                iFirstCard = cCards-1;
            break;
        case SB_LINEDOWN:
            /* move ahead one card */
            iFirstCard++;
            if (iFirstCard == cCards)
                iFirstCard = 0;
            break;
        case SB_PAGEUP:
            /* move back one page (of fully visible cards) */
            if (cFSHeads == cCards)
                return(result);
            iFirstCard -= cFSHeads;
            if (iFirstCard < 0)
                iFirstCard += cCards; /* a negative number */
            break;
        case SB_PAGEDOWN:
            /* move forward one page */
            if (cFSHeads == cCards)
                return(result);
            iFirstCard += cFSHeads;
            if (iFirstCard >= cCards)
                iFirstCard -= cCards;
            break;
        case SB_THUMBPOSITION:
        case SB_ENDSCROLL:
            /* if we finish on a different card than at start */
            if (iFirstCard != iCardStartScroll)
                {
                /* save info, and read it back */
                if (SaveCurrentCard(iCardStartScroll))
                    SetCurCard(iFirstCard);
                else
                    {
                    /* otherwise we won't do anything */
                    iFirstCard = iCardStartScroll;
                    result = FALSE;
                    }
                }
            /* set new position */
            SetScrollPos(hWindow, SB_HORZ, iFirstCard, TRUE);
            InvalidateRect(hCardWnd, (LPRECT)NULL, TRUE);
            fCardCleared = FALSE;
            fScrolling = FALSE;
            return(result);
        case SB_THUMBTRACK:
            /* follow thumb position */
            iFirstCard = pos;
            break;
        default:
            return(result);
        }
    /* only gets here on lineup-down, pageup-down, and thumbtrack */
    /* if we're on a different card than at start */
    if (iFirstCard != OldFirst)
        {
        /* and it hasn't been whited out */
        if (!fCardCleared)
            {
            /* clear it */
            hDC = GetDC(hCardWnd);
            GetClientRect(hCardWnd, (LPRECT)&rect);
            FillRect(hDC, (LPRECT)&rect, hbrWhite);
            ReleaseDC(hCardWnd, hDC);
            fCardCleared = TRUE;
            }
        /* set new position, if not conflicting with scroll bar */
        if (cmd != SB_THUMBTRACK)
            SetScrollPos(hWindow, SB_HORZ, iFirstCard, TRUE);
        /* paint the new headers */
        PaintNewHeaders();
        }
    return(result);
    }

/* this handles scrolling in phone book mode */
void PhoneScroll(hWindow, cmd, pos)
HWND hWindow;
int cmd;
int pos;
    {
    int OldTop = iTopCard;
    HDC hDC;
    RECT rect;
    int cLines;
    int dCards;

    GetClientRect(hWindow, (LPRECT)&rect);
    cLines = rect.bottom / CharFixHeight;

    dCards = 0;
    switch (cmd)
        {
        case SB_LINEUP:
            dCards = -1;
            break;
        case SB_LINEDOWN:
            dCards = 1;
            break;
        case SB_PAGEUP:
            dCards = -cLines;
            break;
        case SB_PAGEDOWN:
            dCards = cLines;
            break;
        case SB_THUMBTRACK:
            dCards = pos - iTopCard;
            break;
        case SB_THUMBPOSITION:
            SetScrollPos(hWindow, SB_VERT, pos, TRUE);
            return;
        }
    iTopCard += dCards;
    if (iTopCard > cCards - cLines)
        iTopCard = cCards - cLines;
    else if (iTopCard < 0)
        iTopCard = 0;

    dCards = OldTop - iTopCard;

    if (dCards)
        {
        if (cmd != SB_THUMBTRACK)
            SetScrollPos(hWindow, SB_VERT, iTopCard, TRUE);
        ScrollWindow(hWindow, 0, dCards * CharFixHeight, (LPRECT)NULL, (LPRECT)NULL);
        UpdateWindow(hWindow);
        }
    }

/* this resets the range and position of the scroll bars, depending upon */
/* the number of cards in the file, and the current card */
void SetScrRangeAndPos()
    {
    int range;
    RECT rect;
    int cLines;

    if (CardPhone == IDM_PHONEBOOK)
        {
        GetClientRect(hCardfileWnd, (LPRECT)&rect);
        if ((cLines = rect.bottom / CharFixHeight) >= cCards)
            range = 0;
        else
            {
            if (!cLines)
                cLines = 1;
            range = cCards - cLines;
            }
        }
    else
        range = cCards-1;
    SetScrollRange(hCardfileWnd, CardPhone == IDM_PHONEBOOK ? SB_VERT : SB_HORZ, 0, range, FALSE);
    SetScrollPos(hCardfileWnd, CardPhone == IDM_PHONEBOOK ? SB_VERT : SB_HORZ, CardPhone == IDM_PHONEBOOK ? iTopCard : iFirstCard, TRUE);
    }
