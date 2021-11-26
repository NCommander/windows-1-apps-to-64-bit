#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* this routine is called when the user selects the merge command */
void DoMerge()
    {
    char *pchBuf;
    OFSTRUCT ofStruct;

    if(!fReadOnly && (pchBuf = (char *)PutUpDB(DTMERGE)))
        {
        /* check the name that the user specifies */
        if (OpenFile((LPSTR)pchBuf, (LPOFSTRUCT)&ofStruct, OF_PARSE))
            {
            /* not a valid file name */
            CardfileOkError(IDS_EINVALIDFILE);
            LocalFree((HANDLE)pchBuf);
            return;
            }
        SetCursor(hWaitCurs);
        /* save current data */
        if (CardPhone == IDM_PHONEBOOK || SaveCurrentCard(iFirstCard))
            {
            /* merge the files */
            if(MergeCardFile(pchBuf))
                {
                /* successful, reset information */
                iTopCard = iFirstCard = 0;
                SetScrRangeAndPos();
                if (CardPhone == IDM_CARDFILE)
                    SetCurCard(iFirstCard);
                InvalidateRect(hCardfileWnd, (LPRECT)NULL, TRUE);
                }
            }
        LocalFree((HANDLE)pchBuf);
        SetCursor(hArrowCurs);
        }
    }

BOOL FAR DoOpen(pchBuf)
char *pchBuf;
    {
    int result = FALSE;
    OFSTRUCT ofStruct;

    /* check the filename the user specified */
    if (OpenFile((LPSTR)pchBuf, (LPOFSTRUCT)&ofStruct, OF_PARSE))
        {
        /* not a valid filename */
        CardfileOkError(IDS_EINVALIDFILE);
        LocalFree((HANDLE)pchBuf);
        return(0);
        }
    SetCursor(hWaitCurs);
    /* read the file */
    if(ReadCardFile(pchBuf))
        {
        /* successful, update caption and tempfile */
        SetCaption();
        Fdelete(TmpFile);
        MakeTmpFile(hCardfileInstance);
        /* reset scroll bars */
        iTopCard = iFirstCard = 0;
        SetScrRangeAndPos();
        /* get card's data */
        if (CardPhone == IDM_CARDFILE);
            SetCurCard(iFirstCard);
        CurCardHead.flags = 0;
        InvalidateRect(hCardfileWnd, (LPRECT)NULL, TRUE);
        result = TRUE;
        }
    SetCursor(hArrowCurs);
    return(result);
    }

/* see if the user wants to save any changes he's made */
BOOL MaybeSaveFile(fSystemModal)
int fSystemModal;
    {
    char *pchFile;
    int result;
    char *pch;
    OFSTRUCT ofStruct;

/* put up a message box that says "Do you wish to save your edits?" */
/* if so, save 'em */
/* if returns FALSE, means it couldn't save, and whatever is happening */
/* should not continue */
    if (fFileDirty || CurCardHead.flags & FDIRTY || SendMessage(hCardWnd, EM_GETMODIFY, 0, 0L))
        {
        if (CurIFile[0])
            {
            /* get to the end of the file */
            for (pch = CurIFile ; *pch; ++pch)
                ;
            /* scan backwards to beginning of filename */
            while (pch > CurIFile && *pch != '\\')
                pch = (PSTR)AnsiPrev(CurIFile, pch);
            if (*pch == '\\')
                pch++;

            AnsiUpper((LPSTR)pch);
            }
        else
            pch = rgchUntitled;
        /* put up the message, merging in filename */
        result = MyMessageBox(IDS_OKTOSAVE, pch, MB_YESNOCANCEL | MB_ICONQUESTION);

        /* if user wants to save */
        if (result == IDYES)
            {
            /* save the current information */
            if (SaveCurrentCard(iFirstCard))
                {
                /* if unnamed, need to get a filename */
                if (!CurIFile[0])
                    {
                    if (GetNewFileName(&ofStruct))
                        pchFile = ofStruct.szPathName;
                    else
                        {
                        SetCurCard(iFirstCard);
                        return(FALSE);      /* cancelled */
                        }
                    }
                else
                    pchFile = CurIFile;
                /* save file, if can't save don't continue */
                if (!WriteCardFile(pchFile))
                    return(FALSE);
                }
            else
                return(FALSE);
            }
        else if (result == IDCANCEL)
            return(FALSE);
        else if (CurCard.hBitmap)
            {
            /* get rid of any bitmap */
            DeleteObject(CurCard.hBitmap);
            CurCard.hBitmap = 0;
            }
        }
    else if (CurCard.hBitmap)
        {
        DeleteObject(CurCard.hBitmap);
        CurCard.hBitmap = 0;
        }

    return(TRUE);
    }


/* this asks the user for a filename */
BOOL GetNewFileName(pOFStruct)
POFSTRUCT pOFStruct;
    {
    PSTR pchBuf;
    BOOL fDone;
    int fh;

    fDone = FALSE;
    while (!fDone)
        {
        if(pchBuf = PutUpDB(DTSAVE))
            {
            /* check for valid filename */
            if (OpenFile((LPSTR)pchBuf, (LPOFSTRUCT)pOFStruct, OF_PARSE))
                {
                CardfileOkError(IDS_EINVALIDFILE);
                }
            /* check to see if file already exists */
            else if ((fh = MyOpen((LPSTR)pchBuf, READ)) > -1)
                {
                /* it does, close it and as if user wants to overwrite */
                MyClose(fh);
                AnsiUpper((LPSTR)pchBuf);
                if (MyMessageBox(IDS_EFILEEXISTS,
                                 pchBuf,
                                 MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
                    {
                    /* does, done */
                    fDone = TRUE;
                    }
                }
            else
                /* file does not exist, done */
                fDone = TRUE;
            /* free up the response buffer */
            LocalFree((HANDLE)pchBuf);
            }
        else
            /* cancelled save as dialog box */
            return(FALSE);
        }
    return(TRUE);
    }
