#define NORASTEROPS
#define NOSYSCOMMANDS
#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* This routine handles all the menu commands in Cardfile. */

void CardfileInput(hWindow, event)
HWND hWindow;
int event;
    {
    PSTR pchBuf;
    PSTR pchFile;
    LPCARDHEADER lpCards;
    long ltmp;
    OFSTRUCT ofStruct;

    /* which menu command was it */
    switch(event)
        {
        case IDM_ABOUT:
            /* about dialog box */
            PutUpDB(DTABOUT);
            break;
        case IDM_NEW:
            /* create a new cardfile */
            if (fReadOnly)
                break;

            /* make sure we save any changes the user made */
            if (!MaybeSaveFile(FALSE))
                /* the user cancelled in the save changes dialog */
                break;

            SetCursor(hWaitCurs);

            CurIFile[0] = 0;
            CurIFind[0] = 0;
            SetCaption();

            /* get rid of the old temp file */
            Fdelete(TmpFile);
            /* create a new one */
            MakeTmpFile(hCardfileInstance);

            /* there's only one card in a new cardfile */
            /* shrinking or leaving the same size, so this should always work */
            GlobalReAlloc(hCards, (long)sizeof(CARDHEADER),GMEM_MOVEABLE);
            cCards = 1;
            iFirstCard = 0;
            iTopCard = 0;
            SetScrRangeAndPos();

            /* Set up CurCardHead */
            MakeBlankCard();

            /* Save blank card in data structure */
            lpCards = (LPCARDHEADER) GlobalLock(hCards);
            *lpCards = CurCardHead;
            GlobalUnlock(hCards);

            /* when in phone book mode, not making changes so must save now */
            if (CardPhone == IDM_PHONEBOOK)
                SaveCurrentCard(iFirstCard);

            /* make sure the window gets repainted */
            InvalidateRect(hWindow, (LPRECT)NULL, TRUE);
            SetCursor(hArrowCurs);

            /* clean file */
            fFileDirty = FALSE;

            /* make sure that any selection gets repainted for empty card */
            if (CardPhone == IDM_CARDFILE)
                {
                SetFocus(NULL);
                SetFocus(hCardWnd);
                }
            break;
        case IDM_OPEN:
            /* opening an existing file */
            /* make sure that any changes get saved */
            if(MaybeSaveFile(FALSE))
                {
                /* put up dialog box, and if user hits ok, open file */
                if (pchBuf = PutUpDB(DTOPEN))
                    {
                    if (!DoOpen(pchBuf))
                        SetCurCard(iFirstCard);
                    LocalFree((HANDLE)pchBuf);
                    }
                else
                    /* user cancelled open dialog box */
                    SetCurCard(iFirstCard);
                }
            break;
        case IDM_MERGE:
            /* merge in another file */
            DoMerge();
            break;
        case IDM_PRINT:
            /* print the front card */
            PrintCards(1);
            break;
        case IDM_PRINTALL:
            /* print all the cards in the file */
            if (CardPhone == IDM_CARDFILE)
                PrintCards(cCards);
            else
                PrintList();
            break;
        case IDM_SAVE:
        case IDM_SAVEAS:
            /* save the file */
            pchFile = NULL;
            /* if doing a SAVE, and file is named, use current name */
            if (event == IDM_SAVE && CurIFile[0])
                pchFile = CurIFile;
            /* else ask the user for a name */
            else if (GetNewFileName(&ofStruct))
                pchFile = ofStruct.szPathName;
            /* if there is a name to save it to */
            if (pchFile)
                {
                SetCursor(hWaitCurs);
                /* get the current selection if in cardfile mode */
                if (CardPhone == IDM_CARDFILE)
                    ltmp = SendMessage(hCardWnd, EM_GETSEL, 0, 0L);
                /* if in phonebook mode, or if in cardfile mode and */
                /* able to save the front card */
                if (CardPhone == IDM_PHONEBOOK || SaveCurrentCard(iFirstCard))
                    {
                    /* write the file out */
                    if (WriteCardFile(pchFile))
                        {
                        /* reset the caption */
                        SetCaption();
                        /* start a new temp file */
                        Fdelete(TmpFile);
                        MakeTmpFile(hCardfileInstance);
                        }
                    /* if in cardfile mode */
                    if (CardPhone == IDM_CARDFILE)
                        {
                        /* get contents of front card again, and set selection*/
                        SetCurCard(iFirstCard);
                        SendMessage(hCardWnd, EM_SETSEL, 0, ltmp);
                        }
                    }
                SetCursor(hArrowCurs);
                }
            break;
        case IDM_CARDFILE:
        case IDM_PHONEBOOK:
            /* change display modes */
            if (event != CardPhone)
                {
                /* if changing to phonebook */
                if (event == IDM_PHONEBOOK)
                    {
                    /* save data in card */
                    if (!SaveCurrentCard(iFirstCard))
                        /* if it doesn't work, stop */
                        break;
                    }
                /* else changing to cardfile mode */
                else
                    {
                    /* set the text in the front card */
                    SetCurCard(iFirstCard);
                    }
                /* save new mode */
                CardPhone = event;
                /* turn off old scroll bar */
                SetScrollRange(hWindow, event == IDM_PHONEBOOK ? SB_HORZ : SB_VERT, 0, 0, FALSE);

                /* if phonebook, get rid of edit control, otherwise reactivate */
                ShowWindow(hCardWnd, event == IDM_PHONEBOOK ? HIDE_WINDOW : SHOW_OPENWINDOW);
                SendMessage(hCardWnd, WM_SETREDRAW, event == IDM_CARDFILE, 0L);

                /* if phonbook, make sure that topcard is valid */
                if (event == IDM_PHONEBOOK)
                    SetTopCard();
                /* set up new scroll bar */
                SetScrRangeAndPos();
                /* set focus to right place */
                SetFocus(event == IDM_PHONEBOOK ? hCardfileWnd : hCardWnd);
                /* force a repaint */
                InvalidateRect(hWindow, (LPRECT)NULL, TRUE);
                }
            break;
        case IDM_UNDO:
            /* just send it on to the edit control */
            if (CardPhone == IDM_CARDFILE)
                SendMessage(hCardWnd, EM_UNDO, 0, 0L);
            break;
        case IDM_HEADER:
            /* if in phone book mode, make sure information is in right place */
            if (CardPhone == IDM_PHONEBOOK)
                {
                SetCurCard(iFirstCard);
                SaveCurrentCard(iFirstCard);
                }
            /* put up dialog box */
            if(pchBuf = PutUpDB(DTHEADER))
                {
                /* save new header */
                Mylstrcpy((LPSTR)CurCardHead.line, (LPSTR)pchBuf);
                DeleteCard(iFirstCard);     /* take it out of it's current position */
                iFirstCard = AddCurCard();  /* and put it back in the right place */
                SetScrRangeAndPos();        /* set position */
                fFileDirty = TRUE;
                InvalidateRect(hWindow, (LPRECT)NULL, TRUE);
                LocalFree((HANDLE)pchBuf);
                }
            break;
        case IDM_RESTORE:
            /* if card has a bitmap, make sure memory is freed up */
            if (CurCard.hBitmap)
                {
                DeleteObject(CurCard.hBitmap);
                CurCard.hBitmap = 0;
                }
            /* get the old information */
            SetCurCard(iFirstCard);
            InvalidateRect(hCardWnd, (LPRECT)NULL, TRUE);
            break;
        case IDM_CUT:
        case IDM_COPY:
            DoCutCopy(event);
            break;
        case IDM_PASTE:
            DoPaste();
            break;
        case IDM_TEXT:
        case IDM_BITMAP:
            if (event != EditMode)
                {
                if (event == IDM_BITMAP)
                    hEditCurs = SetCursor(hArrowCurs);
                else
                    SetCursor(hEditCurs);
                SetFocus(NULL);     /* make sure the old caret is off */
                EditMode = event;
                SetFocus(hCardWnd); /* turn caret back on */
                }
            break;
        case IDM_ADD:
            /* get the new header for the new card */
            if(pchBuf = PutUpDB(DTADD))
                {
                /* allocate space for new card */
                if (!GlobalReAlloc(hCards, (unsigned long)((cCards+1)*sizeof(CARDHEADER)),GMEM_MOVEABLE))
                    CardfileOkError(IDS_EINSMEMORY);
                else
                    {
                    /* if in phone book mode, or can save current card */
                    if (CardPhone == IDM_PHONEBOOK || SaveCurrentCard(iFirstCard))
                        {
                        /* make a blank card */
                        MakeBlankCard();
                        /* save the header */
                        Mylstrcpy((LPSTR)CurCardHead.line, (LPSTR)pchBuf);
                        /* the card is dirty */
                        CurCardHead.flags |= (FDIRTY + FNEW);
                        /* add the card */
                        iFirstCard = AddCurCard();
                        /* make sure that top card is correct */
                        if (CardPhone == IDM_PHONEBOOK)
                            SetTopCard();
                        /* set new scroll information */
                        SetScrRangeAndPos();
                        /* if in phone book mode, need to save immediately */
                        if (CardPhone == IDM_PHONEBOOK)
                            SaveCurrentCard(iFirstCard);
                        /* else, cause highlighting to be redrawn correctly */
                        else
                            {
                            SetFocus(NULL);
                            SetFocus(hCardWnd);
                            }
                        /* force repaint */
                        InvalidateRect(hWindow, (LPRECT)NULL, TRUE);
                        }
                    }
                LocalFree((HANDLE)pchBuf);
                }
            break;
        case IDM_DELETE:
            /* if in phone book mode */
            if (CardPhone == IDM_PHONEBOOK)
                {
                /* get information */
                SetCurCard(iFirstCard);
                /* save it in case of cancel */
                SaveCurrentCard(iFirstCard);
                }
            /* ask user for confirmation */
            if (MyMessageBox(IDS_DELCURCARD,
                    CurCardHead.line,
                    MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) == IDOK)
                {
                /* if card has a bitmap */
                if (CurCard.hBitmap)
                    {
                    /* get rid of it */
                    DeleteObject(CurCard.hBitmap);
                    CurCard.hBitmap = 0;
                    }
                /* if not last card */
                if (cCards > 1)
                    {
                    /* get rid of it */
                    DeleteCard(iFirstCard);
                    if (iFirstCard == cCards)
                        iFirstCard = (CardPhone == IDM_CARDFILE ? 0 : cCards-1);
                    /* change scroll information */
                    SetScrRangeAndPos();
                    /* if in card mode, get card's information */
                    if (CardPhone == IDM_CARDFILE)
                        SetCurCard(iFirstCard);
                    /* shrinking, so don't have to check */
                    GlobalReAlloc(hCards, (unsigned long)(cCards*sizeof(CARDHEADER)),GMEM_MOVEABLE);
                    }
                /* else, last card */
                else
                    {
                    /* make it blank */
                    MakeBlankCard();
                    /* store the info */
                    lpCards = (LPCARDHEADER) GlobalLock(hCards);
                    *lpCards = CurCardHead;
                    GlobalUnlock(hCards);
                    /* save the info in phonebook mode */
                    if (CardPhone == IDM_PHONEBOOK)
                        SaveCurrentCard(iFirstCard);
                    }
                /* dirty file and window */
                fFileDirty = TRUE;
                InvalidateRect(hWindow, (LPRECT)NULL, TRUE);
                }
            break;
        case IDM_DUPLICATE:
            /* make sure there's room */
            if (!GlobalReAlloc(hCards, (unsigned long)((cCards+1)*sizeof(CARDHEADER)),GMEM_MOVEABLE))
                CardfileOkError(IDS_EINSMEMORY);
            else
                {
                /* there is, save current card */
                if (CardPhone == IDM_PHONEBOOK || SaveCurrentCard(iFirstCard))
                    {
                    /* get information */
                    SetCurCard(iFirstCard);
                    CurCardHead.flags |= (FDIRTY + FNEW);
                    /* add it */
                    iFirstCard = AddCurCard();
                    if (CardPhone == IDM_PHONEBOOK)
                        {
                        SaveCurrentCard(iFirstCard);
                        SetTopCard();
                        }
                    /* set new scroll information */
                    SetScrRangeAndPos();
                    InvalidateRect(hWindow, (LPRECT)NULL, TRUE);
                    fFileDirty = TRUE;
                    }
                }
            break;
        case IDM_DIAL:
            /* if in phonebook mode, need to get info */
            if (CardPhone == IDM_PHONEBOOK)
                {
                SetCurCard(iFirstCard);
                SaveCurrentCard(iFirstCard);
                }
            if (pchBuf = PutUpDB(DTDIAL))
                {
                DoDial(pchBuf);
                LocalFree((HANDLE)pchBuf);
                }
            break;
        case IDM_GOTO:
            /* search headers */
            if (pchBuf = PutUpDB(DTGOTO))
                {
                DoGoto(pchBuf);
                LocalFree((HANDLE)pchBuf);
                }
            break;
        case IDM_FINDNEXT:
            /* search body of card */
            if (CurIFind[0])
                {
                FindStrCard();
                break;
                }
        case IDM_FIND:
            /* search for a pattern */
            if(pchBuf = PutUpDB(DTFIND))
                {
                /* save the pattern */
                Mylstrcpy((LPSTR)CurIFind, (LPSTR)pchBuf);
                FindStrCard();
                LocalFree((HANDLE)pchBuf);
                }
            break;
        default:
            break;
        }
    }

/* this routine is called in phone mode to figure out which header */
/* should appear at the top of the window */
void SetTopCard()
    {
    RECT rect;
    int i;

    GetClientRect(hCardfileWnd, (LPRECT)&rect);
    i = rect.bottom / CharFixHeight;
    if (!i)
        i = 1;
    iTopCard = min(iFirstCard-(i-1)/2, (cCards - i));
    if (iTopCard < 0)
        iTopCard = 0;
    }

PSTR PutUpDB(idb)
int idb;
    {
    FARPROC lpdbProc;

    DBcmd = idb;
    switch(idb)
        {
        case DTMERGE:
        case DTOPEN:
            lpdbProc = lpfnOpen;
            break;
        case DTSAVE:
            lpdbProc = lpfnSave;
            break;
        case DTABOUT:
            lpdbProc = lpfnAbout;
            break;
        case DTDIAL:
            lpdbProc = lpfnDial;
            break;
        default:
            lpdbProc = lpDlgProc;
            break;
        }
    return((PSTR)DialogBox(hCardfileInstance, MAKEINTRESOURCE(idb), hCardfileWnd, lpdbProc));
    }

/* this routine initializes the menu every time it is about */
/* to be displayed for the user */
void UpdateMenu()
    {
    HMENU hMenu;
    long lSelection;
    int wFmt;
    int mfPaste;

    hMenu = GetMenu(hCardfileWnd);
    CheckMenuItem(hMenu, IDM_CARDFILE, CardPhone == IDM_CARDFILE ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_PHONEBOOK, CardPhone == IDM_PHONEBOOK ? MF_CHECKED : MF_UNCHECKED);

    CheckMenuItem(hMenu, IDM_TEXT, EditMode == IDM_TEXT ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_BITMAP, EditMode == IDM_BITMAP ? MF_CHECKED : MF_UNCHECKED);
    if (CardPhone == IDM_CARDFILE)
        {
        if (EditMode == IDM_TEXT && SendMessage(hCardWnd, EM_CANUNDO, 0, 0L))
            EnableMenuItem(hMenu, IDM_UNDO, MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_UNDO, MF_GRAYED);

        mfPaste = MF_GRAYED;
        if (OpenClipboard(hCardfileWnd))
            {
            wFmt = 0;

            while (wFmt = EnumClipboardFormats(wFmt))
                {
                if (wFmt == (EditMode == IDM_BITMAP ? CF_BITMAP : CF_TEXT))
                    {
                    mfPaste = MF_ENABLED;
                    break;
                    }
                }

            CloseClipboard();
            }
        EnableMenuItem(hMenu, IDM_PASTE, mfPaste);

        EnableMenuItem(hMenu, IDM_TEXT, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_BITMAP, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_RESTORE, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_FIND, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_FINDNEXT, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_PRINT, fCanPrint ? MF_ENABLED : MF_GRAYED);
        if (EditMode == IDM_TEXT)
            {
            lSelection = SendMessage(hCardWnd, EM_GETSEL, 0, 0L);
            if (HIWORD(lSelection) == LOWORD(lSelection))
                goto Disabled;
            else
                goto Enabled;
            }
        else
            if (CurCard.hBitmap)
                {
Enabled:
                EnableMenuItem(hMenu, IDM_CUT, MF_ENABLED);
                EnableMenuItem(hMenu, IDM_COPY, MF_ENABLED);
                }
            else
                {
Disabled:
                EnableMenuItem(hMenu, IDM_CUT, MF_GRAYED);
                EnableMenuItem(hMenu, IDM_COPY, MF_GRAYED);
                }
        }
    else
        {
        EnableMenuItem(hMenu, IDM_UNDO, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_TEXT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_BITMAP, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_RESTORE, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_CUT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_COPY, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PASTE, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_FIND, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_FINDNEXT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PRINT, MF_GRAYED);
        }
    }

/* this routine sets the contents of the edit control, but first */
/* sets a flag so we will know not to redraw any bitmap on the card */
void SetEditText(lpText)
LPSTR lpText;
    {
    fSettingText = TRUE;
    SetWindowText(hCardWnd, lpText);
    }

/* this handles all mouse input to main window.  Mouse input in the */
/* edit control will get sent to CardWndProc */
void CardfileMouse(hWindow, message, wParam, pt)
HWND hWindow;
int message;
WORD wParam;
POINT pt;
    {
    RECT rect;
    int iCard;
    HDC hDC;
    int y;
    MSG msg;

    if (CardPhone == IDM_CARDFILE)
        {
        /* see if click on a card or background */
        if ((iCard = MapPtToCard(pt)) > -1)
            {
            /* if on another card */
            if (iCard != iFirstCard)
                {
                /* bring it to front */
                CardfileScroll(hCardfileWnd, SB_THUMBTRACK, iCard);
                CardfileScroll(hCardfileWnd, SB_THUMBPOSITION, iCard);
                }
            /* else if double click on first */
            else if (message == WM_LBUTTONDBLCLK)
                {
                /* bring up header box */
                SetCapture(hCardfileWnd);
                while(GetKeyState(VK_LBUTTON) < 0)
                    {
                    PeekMessage((LPMSG)&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, TRUE);
                    PeekMessage((LPMSG)&msg, NULL, WM_KEYFIRST, WM_KEYLAST, TRUE);
                    }
                ReleaseCapture();
                CardfileInput(hWindow, IDM_HEADER);
                }
            }
        }
    else        /* phone book mode */
        {
        /* figure out which header the user clicked on */
        iCard = iTopCard + (pt.y / CharFixHeight);
        if (message == WM_LBUTTONDOWN || iCard != iFirstCard)
            {
            /* single click, change selection */
            if (iCard < cCards)
                {
                y = (iFirstCard - iTopCard) * CharFixHeight;
                hDC = GetDC(hWindow);
                SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y+CharFixHeight);
                InvertRect(hDC, (LPRECT)&rect);
                iFirstCard = iCard;
                y = (iFirstCard - iTopCard) * CharFixHeight;
                SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y+CharFixHeight);
                InvertRect(hDC, (LPRECT)&rect);
                ReleaseDC(hWindow, hDC);
                }
            }
        else /* double click on cur card */
            {
            /* bring up header box */
            if (iCard < cCards)
                {
                while(!PeekMessage((LPMSG)&msg, NULL, WM_LBUTTONUP, WM_LBUTTONUP, TRUE))
                    ;
                CardfileInput(hWindow, IDM_HEADER);
                }
            }
        }
    }

/* this routine is called when a card which is brought forward */
/* via a keystroke (i.e. ctrl-letter) */
BOOL FAR GetNewCard(iOldCard, iNewCard)
int iOldCard;
int iNewCard;
    {
    HDC hDC;
    RECT rect;
    int y;

    /* if phonebook mode */
    if (CardPhone == IDM_PHONEBOOK)
        {
        /* unselect old card */
        y = (iOldCard - iTopCard) * CharFixHeight;
        hDC = GetDC(hCardfileWnd);
        SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y+CharFixHeight);
        InvertRect(hDC, (LPRECT)&rect);
        ReleaseDC(hCardfileWnd, hDC);
        /* get new one */
        BringCardOnScreen(iFirstCard = iNewCard);
        }
    else
        {
        /* send scroll messages */
        CardfileScroll(hCardfileWnd, SB_THUMBTRACK, iNewCard);
        return(CardfileScroll(hCardfileWnd, SB_THUMBPOSITION, iNewCard));
        }
    return(TRUE);
    }

/* map mouse coordinates to a particular card.  This is called in */
/* card mode only */
int MapPtToCard(pt)
POINT pt;
    {
    int idCard;
    int xCur;
    int yCur;
    int i;
    RECT rect;

    yCur = yFirstCard - (cScreenCards - 1)*ySpacing;
    xCur = xFirstCard + (cScreenCards - 1)* (2 * CharFixWidth);
    idCard = (iFirstCard + cScreenCards-1) % cCards;

    for (i = 0; i < cScreenCards; ++i)
        {
        SetRect((LPRECT)&rect, xCur+1, yCur+1, xCur+CardWidth-1, yCur+CharFixHeight+1);
        if (PtInRect((LPRECT)&rect, pt))
            return(idCard);
        SetRect((LPRECT)&rect, rect.right - 2*CharFixWidth + 2, rect.top,rect.right,rect.top+CardHeight-2);
        if (PtInRect((LPRECT)&rect, pt))
            return(idCard);
        xCur -= (2*CharFixWidth);
        yCur += ySpacing;
        idCard--;
        if (idCard < 0)
            idCard = cCards - 1;
        }
    return(-1);
    }

/* puts up a dialog box with only an ok button */
void FAR CardfileOkError(strid)
int strid;
    {
    MyMessageBox(strid, NULL, MB_OK | MB_ICONEXCLAMATION);
    }

/* puts up error message for strid, possibly merging in a string, */
/* and asks for the appropriate buttons and icons */
WORD FAR MyMessageBox(strid, pchMerge, style)
WORD strid;
PSTR pchMerge;
WORD style;
    {
    char buf1[128];
    char buf2[128];

    if (strid == IDS_EINSMEMORY)
        Mylstrcpy((LPSTR)buf1, (LPSTR)NotEnoughMem);
    else
        LoadString(hCardfileInstance, strid, (LPSTR)buf1, 128);
    MergeStrings((LPSTR)buf1, (LPSTR)pchMerge, (LPSTR)buf2);
    return(MessageBox(hCardfileWnd, (LPSTR)buf2, (LPSTR)rgchCardfile, style));
    }


/* ** Scan sz1 for merge spec.  If found, insert string sz2 at that point.
      Then append rest of sz1 NOTE! Merge spec guaranteed to be two chars.
      returns TRUE if it does a merge, false otherwise. */
BOOL  FAR MergeStrings(lpszSrc, lpszMerge, lpszDst)
LPSTR   lpszSrc;
LPSTR   lpszMerge;
LPSTR   lpszDst;
{
    LPSTR lpchSrc;
    LPSTR lpchDst;

    lpchSrc = lpszSrc;
    lpchDst = lpszDst;

    /* Find merge spec if there is one. */
    while (*(unsigned far *)lpchSrc != wMerge) {
        *lpchDst++ = *lpchSrc;

        /* If we reach end of string before merge spec, just return. */
        if (!*lpchSrc++)
            return FALSE;

    }
    /* If merge spec found, insert sz2 there. (check for null merge string */
    if (lpszMerge) {
        while (*lpszMerge)
            *lpchDst++ = *lpszMerge++;

    }

    /* Jump over merge spec */
    lpchSrc++,lpchSrc++;


    /* Now append rest of Src String */
    while (*lpchDst++ = *lpchSrc++);
    return TRUE;

}


void MakeBlankCard()
    {
    CurCardHead.line[0] = 0;
    CurCard.hBitmap = 0;
    SetEditText((LPSTR)"");
    CurCardHead.flags = FNEW;
    }

/* sets cardfile's window caption */
void SetCaption()
    {
    char buf[40];
    BuildCaption(buf);
    SetWindowText(hCardfileWnd, (LPSTR)buf);
    }

/* creates the string that will appear in the window, or in the */
/* spooled file list */
void FAR BuildCaption(pchBuf)
char *pchBuf;
    {
    char *pch;
    char *pch3;

    /* always starts off "Cardfile - " */
    Mylstrcpy((LPSTR)pchBuf, (LPSTR)rgchCardfile);
    Mylstrcat((LPSTR)pchBuf, (LPSTR)" - ");
    /* if named, append just filename */
    if (CurIFile[0])
        {
        pch = CurIFile;
        pch3 = pch;
        /* scan to end */
        for ( ; *pch; ++pch)
            ;
        /* run backwards looking for beginning of filename */
        while (pch > pch3 && *pch != '\\')
            pch = (PSTR)AnsiPrev(pch3, pch);
        /* if at slash, point to filename */
        if (*pch == '\\')
            pch++;
        Mylstrcat((LPSTR)pchBuf, (LPSTR)pch);
        }
    /* if unnamed, append "Untitled" */
    else
        Mylstrcat((LPSTR)pchBuf, (LPSTR)rgchUntitled);
    }

/* this routine is called when cardfile finds out that win.ini */
/* has changed */
void FAR CardfileWinIniChange()
    {
    char buf[3];
    HMENU hMenu;

    hMenu = GetMenu(hCardfileWnd);
    if (!GetProfileString((LPSTR)rgchWindows, (LPSTR)rgchDevice, (LPSTR)"", buf, 2))
        {
        fCanPrint = FALSE;
        EnableMenuItem(hMenu, IDM_PRINT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PRINTALL, MF_GRAYED);
        }
    else
        {
        fCanPrint = TRUE;
        EnableMenuItem(hMenu, IDM_PRINT, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_PRINTALL, MF_ENABLED);
        }
    }
