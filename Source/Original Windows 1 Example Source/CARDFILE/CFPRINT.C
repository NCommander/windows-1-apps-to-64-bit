#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* note!!! the sample code in the SAMPLE application is better for printing */

/* setup for printing: get a dc, find out about the printer fonts, */
/* send the initial escapes, create the modeless dialog, and disable */
/* the main window */

HDC SetupPrinting()
    {
    char buf[40];
    char *pch;
    char *pchFile;
    char *pchPort;
    char fileandport[128];
    HDC hPrintDC;
    TEXTMETRIC Metrics;

    /* parse the device string */
    GetProfileString((LPSTR)rgchWindows, (LPSTR)rgchDevice, (LPSTR)"", (LPSTR)fileandport, 40);
    for (pch = fileandport; *pch && *pch != ','; ++pch)
        ;
    if (*pch)
        *pch++=0;
    while(*pch && *pch <= ' ')
        pch++;
    pchFile = pch;
    while(*pch && *pch != ',' && *pch > ' ')
        pch++;
    if (*pch)
        *pch++ = 0;
    while (*pch && (*pch <= ' ' || *pch == ','))
        pch++;
    pchPort = pch;
    while (*pch && *pch > ' ')
        pch++;
    *pch = 0;

    /* create the dc */
    if (!(hPrintDC = CreateDC((LPSTR)pchFile, (LPSTR)fileandport, (LPSTR)pchPort, (LPSTR)NULL)))
        {
CantPrint:
        if (hPrintDC)
            DeleteDC(hPrintDC);
        CardfileOkError(IDS_ECANTPRINT);
        return(NULL);
        }
    /* find out about the font */
    GetTextMetrics(hPrintDC, (LPTEXTMETRIC)&Metrics);
    CharPrintHeight = Metrics.tmHeight+Metrics.tmExternalLeading;
    ExtPrintLeading = Metrics.tmExternalLeading;
    CharPrintWidth = Metrics.tmAveCharWidth;          /* the average width */

    /* send the escapes */
    Escape(hPrintDC, SETABORTPROC, 0, (LPSTR)lpfnAbortProc, (LPSTR)0);
    BuildCaption(buf);
    if (Escape(hPrintDC, STARTDOC, Mylstrlen((LPSTR)buf), (LPSTR)buf, (LPSTR)0) < 0)
        goto CantPrint;

    /* put up the dialog box */
    fError = FALSE;
    fAbort = FALSE;
    hAbortDlgWnd = CreateDialog(hCardfileInstance,
                                (LPSTR)DTABORTDLG,
                                hCardfileWnd,
                                lpfnAbortDlgProc);
    if (!hAbortDlgWnd)
        goto CantPrint;

    /* disable parent window */
    EnableWindow(hCardfileWnd, FALSE);
    return(hPrintDC);
    }

/* done printing, clean up */
void FinishPrinting(hPrintDC)
HDC hPrintDC;
    {
    /* if user didn't abort */
    if (!fAbort)
        {
        /* if there was no error, tell spooler that we're done */
        if (!fError)
            Escape(hPrintDC, ENDDOC, 0, (LPSTR)0, (LPSTR)0);
        /* reenable main window */
        EnableWindow(hCardfileWnd, TRUE);
        /* get rid of dialog */
        DestroyWindow(hAbortDlgWnd);
        }
    DeleteDC(hPrintDC);
    }

/* PrintList prints the cardfile in phone list mode */
void FAR PrintList()
    {
    HDC hPrintDC;
    int xPrintRes;
    int yPrintRes;
    int curcard;
    int i;
    int y;
    int cCardsPerPage;
    LPCARDHEADER lpCards;
    int iError;

    if (!(hPrintDC = SetupPrinting()))
        return;

    /* find out how many lines will fit on a page */
    xPrintRes = GetDeviceCaps(hPrintDC, HORZRES);
    yPrintRes = GetDeviceCaps(hPrintDC, VERTRES);

    cCardsPerPage = yPrintRes / CharPrintHeight;
    if (!cCardsPerPage)
        cCardsPerPage++;    /* alway at least one */

    lpCards = (LPCARDHEADER)GlobalLock(hCards);
    /* while cards left to print */
    for (curcard = 0; curcard < cCards; )
        {
        /* start at the top of a page */
        y = 0;
        /* while there is room on page, and cards left to print */
        for (i = 0; i < cCardsPerPage && curcard < cCards; ++i)
            {
            /* print the card */
            TextOut(hPrintDC, CharPrintWidth, y, lpCards->line, Mylstrlen(lpCards->line));
            y += CharPrintHeight;
            lpCards++;
            curcard++;
            }
        /* done with a page, tell spooler */
        if ((iError = Escape(hPrintDC, NEWFRAME, 0, (LPSTR)NULL, (LPSTR)0)) < 0)
            {
            PrintError(iError);
            break;
            }
        /* if user aborted, quit */
        if (fAbort)
            break;
        }
    GlobalUnlock(hCards);
    FinishPrinting(hPrintDC);
    }

/* print cards in card mode.  Count will either equal the count */
/* of all cards in the file, or 1 to print a single card */
/* when printing cards, we have to be careful to make sure that */
/* all data in front card is valid.  The user can move the abort */
/* dialog box around, which may reveal part of the front card. */
/* This will cause the system to ask Cardfile to paint the card. */
/* If we reuse the data space set aside for the front card, Cardfile */
/* could end up drawing pieces of other random cards, which wouldn't */
/* look so good.  To get around this, Cardfile allocates other space.*/
/* It also uses a hidden edit control so that the edit control will */
/* handle word wrapping the text within it.  */

void FAR PrintCards(count)
int count;
    {
    HDC hPrintDC;
    char printer[40];
    char fileandport[40];
    char *pch;
    char ch;
    int xPrintRes;
    int yPrintRes;
    int curcard;
    int i;
    int y;
    int cCardsPerPage;
    char *pchFile;
    char *pchPort;
    HANDLE hText;
    LPSTR lpText;
    CARDHEADER CardHead;
    LPCARDHEADER lpCards;
    CARD Card;
    HDC hMemoryDC;
    HWND hPrintWnd;
    HANDLE hOldObject;
    int fPictureWarning;
    int iError;

    /* get a buffer for each card's text */
    hText = GlobalAlloc(GHND, (long)CARDTEXTSIZE);
    if (!hText)
        {
InsMemPrint:
        CardfileOkError(IDS_EINSMEMORY);
        return;
        }

    /* create an invisible window to load card's data into */
    hPrintWnd = CreateWindow(
                (LPSTR)"Edit",
                (LPSTR)"",
                WS_CHILD | ES_MULTILINE,
                0, 0, (LINELENGTH*CharFixWidth)+1, CARDLINES*CharFixHeight,
                hCardfileWnd,
                NULL,
                hCardfileInstance,
                (LPSTR)NULL);

    if (!hPrintWnd)
        {
        GlobalFree(hText);
        goto InsMemPrint;
        }

    /* set up */
    if (!(hPrintDC = SetupPrinting()))
        {
        GlobalFree(hText);
        return;
        }

    lpText = GlobalLock(hText);

    /* figure out printer sizes */
    CardPrintWidth = (LINELENGTH * CharPrintWidth) + 2;
    CardPrintHeight = (CARDLINES*CharPrintHeight) + CharPrintHeight + 1 + 2 + 2;

    hOldObject = SelectObject(hPrintDC, GetStockObject(HOLLOW_BRUSH));

    hMemoryDC = CreateCompatibleDC(hPrintDC);
    fPictureWarning = FALSE;

    /* if only doing one card */
    if (count == 1)
        {
        if (!hMemoryDC && CurCard.hBitmap)
            CardfileOkError(IDS_ENOPICTURES);
        /* print it */
        PrintCurCard(hPrintDC, hMemoryDC, 0, &CurCardHead, &CurCard, hCardWnd);
        /* done with page */
        if ((iError = Escape(hPrintDC, NEWFRAME, 0, (LPSTR)NULL, (LPSTR)0)) < 0)
            PrintError(iError);
        }
    else
        {
        /* figure out how many cards will fit on one page */
        xPrintRes = GetDeviceCaps(hPrintDC, HORZRES);
        yPrintRes = GetDeviceCaps(hPrintDC, VERTRES);

        cCardsPerPage = yPrintRes / (CardPrintHeight + CharPrintHeight);
        if (!cCardsPerPage)
            cCardsPerPage++;
        /* while cards left to print */
        for (curcard = 0; curcard < count; )
            {
            /* start at top */
            y = 0;
            /* while room left, and cards left to print */
            for (i = 0; i < cCardsPerPage && curcard < count; ++i)
                {
                /* if not front card */
                if (curcard != iFirstCard)
                    {
                    /* get data from file */
                    lpCards = (LPCARDHEADER) GlobalLock(hCards);
                    lpCards += curcard;
                    CardHead = *lpCards;
                    GlobalUnlock(hCards);
                    if (!ReadCurCardData(&CardHead, &Card, lpText))
                        CardfileOkError(IDS_ECANTPRINTPICT);
                    }
                else
                    {
                    /* get data from front card */
                    CardHead = CurCardHead;
                    Card = CurCard;
                    GetWindowText(hCardWnd, lpText, CARDTEXTSIZE);
                    }
                /* set text of hidden edit control */
                SetWindowText(hPrintWnd, lpText);
                /* tell user if can't draw pictures */
                if (!hMemoryDC && Card.hBitmap && !fPictureWarning)
                    {
                    fPictureWarning++;
                    CardfileOkError(IDS_ENOPICTURES);
                    }
                /* print the card */
                PrintCurCard(hPrintDC, hMemoryDC, y, &CardHead, &Card, hPrintWnd);
                /* if not first card, and there is a bitmap */
                if (curcard != iFirstCard && Card.hBitmap)
                    /* free the space */
                    DeleteObject(Card.hBitmap);
                y += CardPrintHeight + CharPrintHeight;
                curcard++;
                }
            /* done with a page */
            if ((iError = Escape(hPrintDC, NEWFRAME, 0, (LPSTR)NULL, (LPSTR)0)) < 0)
                {
                PrintError(iError);
                break;
                }
            if (fAbort)
                break;
            }
        }
PrintCardsDone:
    DestroyWindow(hPrintWnd);
    GlobalUnlock(hText);
    GlobalFree(hText);
    SelectObject(hPrintDC, hOldObject);
    FinishPrinting(hPrintDC);
    if (hMemoryDC)
        DeleteDC(hMemoryDC);
    }


/* print the current card */
void PrintCurCard(hPrintDC, hMemoryDC, line, pCardHead, pCard, hWnd)
HDC hPrintDC;
HDC hMemoryDC;
int line;
PCARDHEADER pCardHead;
PCARD   pCard;
HWND    hWnd;
    {
    int y;
    HANDLE hOldObject;
    RECT rect;
    int level;
    int xratio;
    int yratio;
    int i;
    int cLines;
    char buf[LINELENGTH];
    int cch;

    /* draw the header text */
    SetBkMode(hPrintDC, TRANSPARENT);
    TextOut(hPrintDC, 1, line + 1+(ExtPrintLeading / 2), (LPSTR)pCardHead->line, Mylstrlen((LPSTR)pCardHead->line));

    /* if there's a bitmap */
    if (pCard->hBitmap && hMemoryDC)
        {
        level = SaveDC(hPrintDC);
        IntersectClipRect(hPrintDC, 1, line+CharPrintHeight+4, CardPrintWidth-1, line+CardPrintHeight-1);
        hOldObject = SelectObject(hMemoryDC, pCard->hBitmap);
        /* stretch the bitmap out, so that it takes up roughly the same */
        /* percentage of the printed card as it does on the screen */
        if (!StretchBlt(hPrintDC,
                        (pCard->xBitmap*CharPrintWidth)/CharFixWidth,
                        line+CharPrintHeight+4+(pCard->yBitmap*CharPrintHeight)/CharFixHeight,
                        (pCard->cxBitmap * CharPrintWidth)/ CharFixWidth,
                        (pCard->cyBitmap * CharPrintHeight)/ CharFixHeight,
                        hMemoryDC,
                        0,
                        0,
                        pCard->cxBitmap,
                        pCard->cyBitmap,
                        SRCCOPY))
            CardfileOkError(IDS_ECANTPRINTPICT);
        SelectObject(hMemoryDC, hOldObject);
        RestoreDC(hPrintDC, level);
        }

    /* draw the text */
    y = line+CharPrintHeight + 4 + (ExtPrintLeading / 2);
    cLines = SendMessage(hWnd, EM_GETLINECOUNT, 0, 0L);
    /* for each line in the edit control */
    for (i = 0; i < cLines; ++i)
        {
        buf[0] = LINELENGTH;
        buf[1] = 0;
        /* ask the edit control for it */
        cch = SendMessage(hWnd, EM_GETLINE, i, (long)(LPSTR)buf);
        /* paint it */
        TextOut(hPrintDC, 1, y, (LPSTR)buf, cch);
        y += CharPrintHeight;
        }
    /* draw the bounding rectangles. */
    Rectangle(hPrintDC, 0, line, CardPrintWidth, line+CardPrintHeight);
    Rectangle(hPrintDC, 0, line+1+CharPrintHeight, CardPrintWidth, line+2+CharPrintHeight);
    Rectangle(hPrintDC, 0, line+3+CharPrintHeight, CardPrintWidth, line+4+CharPrintHeight);
    }

/*  spooler calls this so that we can handle messages while spooling */
int FAR PASCAL fnAbortProc(hPrintDC, iReserved)
HDC hPrintDC;
int iReserved;
    {
    MSG msg;

    while (!fAbort && PeekMessage((LPMSG)&msg, NULL, NULL, NULL, TRUE))
        if (!hAbortDlgWnd || !IsDialogMessage(hAbortDlgWnd, (LPMSG)&msg))
            {
            TranslateMessage((LPMSG)&msg);
            DispatchMessage((LPMSG)&msg);
            }
    return(!fAbort);
    }

/* this controls the Cancel Printing dialog box */
int FAR PASCAL fnAbortDlgProc(hwnd, msg, wParam, lParam)
HWND hwnd;
unsigned msg;
WORD wParam;
DWORD lParam;
    {
    HMENU hMenu;
    PSTR pchTmp;

    switch(msg)
        {
        case WM_COMMAND:
            /* the only command we can get is ok, which means cancel */
            /* printing */
            /* let the abort proc know */
            fAbort = TRUE;
            /* reenable the main window */
            EnableWindow(hCardfileWnd, TRUE);
            /* destroy the dialog */
            DestroyWindow(hwnd);
            hAbortDlgWnd = NULL;
            return(TRUE);
        case WM_INITDIALOG:
            /* keep the system menu around for disabling close */
            hSysMenu = GetSystemMenu(hwnd, FALSE);
            /* put the filename in the dialog */
            if (CurIFile[0])
                {
                for (pchTmp = CurIFile + Mylstrlen((LPSTR)CurIFile) - 1;
                    pchTmp >= CurIFile && *pchTmp != '\\' && *pchTmp != ':';
                    pchTmp--)
                    ;
                pchTmp++;
                SetDlgItemText(hwnd, 4, (LPSTR)pchTmp);
                }
            else
                SetDlgItemText(hwnd, 4, (LPSTR)rgchUntitled);
            SetFocus(hwnd);
            return(TRUE);
        case WM_INITMENU:
            EnableMenuItem(hSysMenu, SC_CLOSE, MF_GRAYED);
            return(TRUE);
        }
    return(FALSE);
    }

void PrintError(iError)
int iError;
    {
    fError = TRUE;
    if (iError & SP_NOTREPORTED)
        {
        switch(iError)
            {
            case SP_OUTOFMEMORY:
                CardfileOkError(IDS_EMEMPRINT);
                break;
            case SP_OUTOFDISK:
                CardfileOkError(IDS_EDISKPRINT);
                break;
            case SP_USERABORT:
            case SP_APPABORT:
                break;
            default:
                CardfileOkError(IDS_ECANTPRINT);
                break;
            }
        }
    }
