#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* This file contains all of cardfile's painting procedures, as well as */
/* the size proc. */

/* CardfileEraseBkGnd handles the EraseBkGnd message.  Most applications */
/* just pass the message on through to DefWindowProc.  However, Cardfile in */
/* card mode always paints the background BLUE, so it has to intercept the */
/* message.  In phonebook mode, it does the default, namely paint the */
/* background the default color. */

void CardfileEraseBkGnd(hWnd, hDC)
HWND hWnd;
HDC  hDC;
    {
    HBRUSH  hbr;            /* brush used to paint background */
    HBRUSH  hbrOld;         /* to save brush which was in DC */
    RECT    rect;           /* filled with the window's rect */
    POINT   pt;             /* used for aligning brush */

    GetClientRect(hWnd, (LPRECT)&rect);

    /* Get the right brush */
    if (CardPhone == IDM_PHONEBOOK)
        hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    else
        hbr = hbrBack;
    if (hbr)
        {
        /* Unless you unrealize the brush, and set its origin, patterns */
        /* may not line up correctly on the screen.  For example, two */
        /* gray brushes that weren't lined up could create a line. */
        UnrealizeObject(hbr);
        pt.x = 0;
        pt.y = 0;
        ClientToScreen(hWnd, (LPPOINT)&pt);
        SetBrushOrg( hDC, pt.x, pt.y);
        hbrOld = SelectObject(hDC, hbr);
        FillRect(hDC, (LPRECT)&rect, hbr);

        /* We may delete the brush, so select in the old one.  (You can't */
        /* delete a locked object.) */
        SelectObject(hDC, hbrOld);

        if (CardPhone == IDM_PHONEBOOK)
            DeleteObject(hbr);
        }
    }

/* CardfilePaint is the main paint proc for Cardfile when it is in card */
/* mode. cfdata.c contains a picture which identifies a number of the */
/* variables this routine uses. */

void CardfilePaint(hWindow, hDC, lpRectangle)
HWND hWindow;
HDC  hDC;
LPRECT lpRectangle;
    {
    RECT rect;
    LPCARDHEADER lpCards;   /* far pointer to first card */
    LPCARDHEADER lpTCards;  /* far pointer to any card in file */
    int xCur, yCur;         /* coordinates of card being painted */
    int vcCards;            /* count of cards that will fit vertically */
    int hcCards;            /* count of cards that will fit horizontally */
    int width;              /* width of the window minus the margins */
    int height;             /* height of the window minus the margins */
    int idCard;             /* index of the card being painted */
    int i;                  /* loop variable */


    /* don't paint if routine was called because of call to SetWindowText*/
    if (fSettingText)
        return;

    /* always have a left margin, and if there's room for a right margin, */
    /* subtract that too. */
    width = cxMainWindow - LEFTMARGIN;
    if (width - RIGHTMARGIN > CardWidth)
        width -= RIGHTMARGIN;

    /* always have a top margin, and if there's room for a bottom margin, */
    /* subtract that too. */
    height = cyMainWindow;
    if (cyMainWindow - BOTTOMMARGIN > CardHeight)
        height -= BOTTOMMARGIN;

    /* always have at least one card */
    hcCards = 1;
    cFSHeads = cScreenHeads = 1;

    /* see how many full or partial headers will appear in vertical space */
    if (yFirstCard < height)
        {
        cScreenHeads += (yFirstCard+ySpacing-1) / ySpacing;
        cFSHeads += (yFirstCard / ySpacing);
        }
    /* limit header counts to number of cards in file */
    if (cScreenHeads > cCards)
        cScreenHeads = cCards;
    if (cFSHeads > cCards)
        cFSHeads = cCards;

    /* see how many cards will fit horizontally */
    if (width - CardWidth > 0)
        hcCards += ((width - CardWidth) / (2 * CharFixWidth));
    /* see how many cards will fit vertically */
    vcCards = height / ySpacing;
    /* the actual count should be the greater of the vertical and horizontal */
    cScreenCards = hcCards > vcCards ? hcCards : vcCards;
    /* limit the count to the number of cards in the file */
    cScreenCards = cCards < cScreenCards ? cCards : cScreenCards;

    /* get pointer to first card */
    lpCards = (LPCARDHEADER) GlobalLock(hCards);

    /* calculate the coordinates of the first card to be painted */
    yCur = yFirstCard - (cScreenCards - 1)*ySpacing;
    xCur = xFirstCard + (cScreenCards - 1)* (2 * CharFixWidth);
    /* figure out which card will be painted first.  Cards are painted from */
    /* the back right to the front left, i.e. from the end of the file */
    /* to the front of the file */
    idCard = (iFirstCard + cScreenCards-1) % cCards;
    lpTCards = lpCards + idCard;

    for (i = 0; i < cScreenCards; ++i)
        {
        /* draw the card */
        SetRect((LPRECT)&rect, xCur, yCur, xCur+CardWidth, yCur+CardHeight);
        FrameRect(hDC, (LPRECT)&rect, hbrBlack);
        InflateRect((LPRECT)&rect, -1, -1);
        FillRect(hDC, (LPRECT)&rect, hbrWhite);
        SetBkMode(hDC, TRANSPARENT);
        TextOut(hDC, xCur+1, yCur+1+(ExtLeading / 2), lpTCards->line, Mylstrlen(lpTCards->line));

        /* calculate coordinates for next card */
        xCur -= (2*CharFixWidth);
        yCur += ySpacing;

        /* get next card */
        lpTCards--;
        idCard--;

        /* if past beginning of file */
        if (idCard < 0)
            {
            /* get to last card */
            idCard = cCards - 1;
            lpTCards = lpCards+idCard;
            }
        }
    /* draw the red line on the front card */
    SetRect((LPRECT)&rect, xFirstCard, yFirstCard+1+CharFixHeight, xFirstCard+CardWidth, yFirstCard+2+CharFixHeight);
    FillRect(hDC, (LPRECT)&rect, hbrLine);
    SetRect((LPRECT)&rect, xFirstCard, yFirstCard+3+CharFixHeight, xFirstCard+CardWidth, yFirstCard+4+CharFixHeight);
    FillRect(hDC, (LPRECT)&rect, hbrLine);

    /* done with cards */
    GlobalUnlock(hCards);
    }

/* PhonePaint paints the main window when it is in phone book mode.  This */
/* routine is much simpler than Cardfile paint since all it has to do is */
/* draw a list of strings */

void PhonePaint(hWindow, hDC, lpRectangle)
HWND hWindow;
HDC  hDC;
LPRECT lpRectangle;
    {
    RECT rect;               /* the window's rectangle */
    LPCARDHEADER lpCards;    /* far pointer to card being painted */
    int y;                   /* y coordinate of header being painted */
    int i;
    int cScrCards;           /* count partially visible headers */

    /* figure out how many headers are at least partially visible.  This */
    /* is basically just the count of how many lines of text will fit in */
    /* the window. */
    GetClientRect(hWindow, (LPRECT)&rect);
    cScrCards = (rect.bottom + CharFixHeight - 1) / CharFixHeight;

    /* get to the header which is currently at the top of the window */
    lpCards = (LPCARDHEADER) GlobalLock(hCards);
    lpCards += iTopCard;

    /* set the default text color, and put it in transparent mode */
    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkMode(hDC, TRANSPARENT);

    /* always start at the top of the screen */
    y = 0;

    for (i = 0; i < cScrCards; ++i)
        {
        /* if it's going to paint a non-existant card, break */
        if (i + iTopCard >= cCards)
            break;

        /* paint it */
        TextOut(hDC, CharFixWidth, y, lpCards->line, Mylstrlen(lpCards->line));

        /* get to next header */
        y += CharFixHeight;
        lpCards++;
        }

    /* done with cards */
    GlobalUnlock(hCards);

    /* if have the focus, highlight the current card */
    if (GetFocus() == hCardfileWnd)
        {
        y = (iFirstCard - iTopCard) * CharFixHeight;
        SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y+CharFixHeight);
        InvertRect(hDC, (LPRECT)&rect);
        }
    }


/* This routine is called whenever a new card is brought to the front. */
/* Rather than repainting everything in the window, it simply overwrites */
/* the information displayed in the headers.  This routine is only called */
/* while in card mode. */

void PaintNewHeaders()
    {
    HDC hDC;                    /* the DC */
    int idCard;                 /* id of the card being painted */
    LPCARDHEADER lpCards;       /* far pointer to first card */
    LPCARDHEADER lpTCards;      /* far pointer to card being painted */
    int xCur, yCur;             /* coordinates of card being painted */
    int i;
    RECT rect;

    /* cards are painted from back to front, calculate first coordinates */
    yCur = yFirstCard - (cScreenHeads - 1)*ySpacing;
    xCur = xFirstCard + (cScreenHeads - 1)* (2 * CharFixWidth);

    /* get to the first card to be painted */
    idCard = (iFirstCard + cScreenHeads-1) % cCards;
    lpCards = (LPCARDHEADER) GlobalLock(hCards);
    lpTCards = lpCards + idCard;

    /* get a dc */
    hDC = GetDC(hCardfileWnd);
    SetBkMode(hDC, TRANSPARENT);

    /* for all cards with headers showing */
    for (i = 0; i < cScreenHeads; ++i)
        {
        /* clear out the old header */
        SetRect((LPRECT)&rect, xCur+1, yCur+1, xCur+CardWidth-1, yCur+CharFixHeight+1);
        FillRect(hDC, (LPRECT)&rect, hbrWhite);
        /* draw the new header */
        TextOut(hDC, xCur+1, yCur+1+(ExtLeading/2), lpTCards->line, Mylstrlen(lpTCards->line));

        /* calculate next coordinates */
        xCur -= (2*CharFixWidth);
        yCur += ySpacing;

        /* get to next card */
        lpTCards--;
        idCard--;
        /* if past beginning of file, get to end of file */
        if (idCard < 0)
            {
            idCard = cCards - 1;
            lpTCards = lpCards+idCard;
            }
        }

    /* done with both dc and cards */
    ReleaseDC(hCardfileWnd, hDC);
    GlobalUnlock(hCards);
    }


/* This routine is called when cardfile receives a size message */
void CardfileSize(hWindow, newWidth, newHeight)
HWND    hWindow;
int newWidth;
int newHeight;
    {
    int yCard;

    /* save width and height */
    cxMainWindow = newWidth;
    cyMainWindow = newHeight;

    /* calculate coordinates of bottom left card */
    yFirstCard = (newHeight - BOTTOMMARGIN) - CardHeight;
    yFirstCard = (CharFixHeight / 2) > yFirstCard ? (CharFixHeight / 2) : yFirstCard;
    xFirstCard = LEFTMARGIN;

    /* figure out where edit control should go */
    yCard = yFirstCard + 1 + CharFixHeight + 1 + 2;

    /* if edit control exists, move it to new location */
    if (hCardWnd)
        MoveWindow(hCardWnd, xFirstCard+1, yCard, (LINELENGTH*CharFixWidth)+1, CARDLINES*CharFixHeight, FALSE);
    }
