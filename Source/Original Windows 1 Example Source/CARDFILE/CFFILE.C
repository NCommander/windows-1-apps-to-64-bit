#define NOCOMM
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* cardfile data files are layed out with the following format:      */
/* three magic bytes "MGC"                                           */
/* a word count of cards in the file                                 */
/* array of cardheaders, each one containing:                        */
/*                                                                   */
/*  char          reserved[6];        bytes available for future use */
/*  unsigned long lfData;             file offset of data            */
/*  unsigned char flags;              flags                          */
/*  char          line[LINELENGTH+1]; 40 character lines plus null   */
/*                                                                   */
/*  These are stored sorted in alphabetical order on the             */
/*  header (line).                                                   */
/*                                                                   */
/*  Following the headers are the cards' data.  lfData in a card's   */
/*  header points to somewhere in this area.  Each card's data is    */
/*  layed out like this:                                             */
/*                                                                   */
/*  unsigned bmSize;            size in bytes of the bitmap          */
/*      Ooops, this won't work so well when we get 64K bitmaps       */
/*                                                                   */
/*  If bmSize is > 0 then the card has a bitmap, and then next       */
/*  bytes contain:                                                   */
/*                                                                   */
/*  int cxBitmap;           cx in pixels of bitmap                   */
/*  int cyBitmap;           cy in pixels of bitmap                   */
/*  int cxEighths;          (x coord of bitmap * 8) / cxFont         */
/*  int cyEighths;          (y coord of bitmap * 8) / cyFont         */
/*  BYTE  bmBits[bmSize]    bitmap bits                              */
/*                                                                   */
/*  The x and y coords are in the funny format so that the bitmap    */
/*  will appear in roughly the same place on the card on different   */
/*  displays.                                                        */
/*                                                                   */
/*  Following the bitmap information is the card's text              */
/*                                                                   */
/*  unsigned tSize;         size in bytes                            */
/*  char  Text[tSize];      text characters                          */
/*                                                                   */
/*                                                                   */
/*********************************************************************/


/* MergeCardFile merges the file specified by pchName into the current */
/* file. */

BOOL FAR MergeCardFile(pchName)
PSTR pchName;
    {
    int fh;
    unsigned i;
    char buf[PATHMAX];
    char MaGiC[4];
    OFSTRUCT mergereopen;
    int cEighths;
    HBITMAP hBits;
    LPSTR lpBits;
    LPSTR lpText;
    HANDLE hText;
    unsigned cMergeCards;
    unsigned tSize;
    unsigned long lCurPos;
    HBITMAP hBitmap;
    int result = FALSE;
    unsigned long fhLoc;

    /* if no extension, append the default one */
    AppendExtension(pchName, buf);

    /* open file */
    if ((fh = OpenFile((LPSTR)buf, (LPOFSTRUCT)&mergereopen, OF_PROMPT | OF_CANCEL)) < 0)
        return(FALSE);

    /* now read it in */
    /* check that the first three bytes signify a Cardfile */
    MaGiC[3] = 0;
    myread(fh, MaGiC, 3);
    if (Mylstrcmp(MaGiC, "MGC"))
        {
        CardfileOkError(IDS_ENOTVALIDFILE);
        goto MergeClean;
        }

    /* read the number of cards in the file */
    myread(fh, (PSTR)&cMergeCards, sizeof(int));

    /* allocate enough memory for the file */
    if (!GlobalReAlloc(hCards, (unsigned long)((cCards+cMergeCards)*sizeof(CARDHEADER)),GMEM_MOVEABLE))
        {
MergeInsMem:
        CardfileOkError(IDS_EINSMEMORY);
        goto MergeClean;
        }

    /* allocate a buffer for the text on the cards */
    hText = GlobalAlloc(GHND, (long)CARDTEXTSIZE);
    if (!hText)
        goto MergeInsMem;
    lpText = GlobalLock(hText);

    /* for each card in the merged file */
    for(i = 0; i < cMergeCards; ++i)
        {
        /* read its header */
        myread(fh, (PSTR)&CurCardHead, sizeof(CARDHEADER));
        /* save current position, and seek to the data */
        lCurPos = MyLseek(fh, 0L, 1);
        MyLseek(fh, CurCardHead.lfData, 0);

        CurCard.hBitmap = 0;

        /* if there's a bitmap, read it in */
        myread(fh, (PSTR)&CurCard.bmSize, sizeof(int));
        if (CurCard.bmSize)
            {
            myread(fh, (PSTR)&CurCard.cxBitmap, sizeof(int));
            myread(fh, (PSTR)&CurCard.cyBitmap, sizeof(int));
            myread(fh, (PSTR)&cEighths, sizeof(int));
            CurCard.xBitmap = (cEighths * CharFixWidth) / 8;
            myread(fh, (PSTR)&cEighths, sizeof(int));
            CurCard.yBitmap = (cEighths * CharFixHeight) / 8;
            /* allocate space for the bits */
            if(hBits = GlobalAlloc(GHND, (unsigned long)CurCard.bmSize))
                {
                lpBits = (LPSTR)GlobalLock(hBits);
                mylread(fh, (LPSTR)lpBits, CurCard.bmSize);
                fhLoc = MyLseek(fh, 0L, 1);
                if (hBitmap = CreateBitmap(CurCard.cxBitmap, CurCard.cyBitmap, 1, 1, lpBits))
                    CurCard.hBitmap = hBitmap;
                /* CreateBitmap call may have caused disk swap, might */
                /* have to reopen a file */
                while (MyLseek(fh, fhLoc, 0) == -1)
                    fh = OpenFile((LPSTR)buf, (LPOFSTRUCT)&mergereopen, OF_PROMPT | OF_REOPEN);
                GlobalUnlock(hBits);
                GlobalFree(hBits);
                }
            else
                /* if no space, seek past bitmap */
                MyLseek(fh, (unsigned long)CurCard.bmSize, 1);
            }
        /* read text size */
        myread(fh, (PSTR)&tSize, sizeof(int));
        /* make sure text is no bigger than buffer */
        if (tSize >= CARDTEXTSIZE)
            tSize = CARDTEXTSIZE-1;
        /* read text */
        mylread(fh, (LPSTR)lpText, tSize);
        *(lpText+tSize) = 0;
        /* write out the card, so we'll be able to get it in the future */
        if (tSize = WriteCurCard(&CurCardHead, &CurCard, lpText))
            {
            iFirstCard = AddCurCard();
            }
        /* if card had a bitmap, free up the memory */
        if (CurCard.hBitmap)
            DeleteObject(CurCard.hBitmap);
        /* seek to next header */
        MyLseek(fh, lCurPos, 0);
        if (!tSize)
            goto MergeClean;
        }
    GlobalUnlock(hText);
    GlobalFree(hText);
    fFileDirty = TRUE;
    result = TRUE;
MergeClean:
    MyClose(fh);
    return(result);
    }

BOOL FAR ReadCardFile(pchName)
PSTR pchName;
    {
    int fh;
    LPCARDHEADER lpTCards;
    unsigned i;
    char buf[PATHMAX];
    char MaGiC[4];
    unsigned cNewCards;
    int result = FALSE;
    OFSTRUCT tmpmain;

    /* if no extension, append the default */
    AppendExtension(pchName, buf);

    /* open file */
    if ((fh = OpenFile((LPSTR)buf, (LPOFSTRUCT)&tmpmain, OF_PROMPT | OF_CANCEL)) < 0)
        return(FALSE);

    /* now read it in, and verify that it's a cardfile */
    MaGiC[3] = 0;
    myread(fh, (PSTR)MaGiC, 3);
    if (Mylstrcmp(MaGiC, "MGC"))
        {
        CardfileOkError(IDS_ENOTVALIDFILE);
        goto ReadClean;
        }

    /* read the number of cards in the file */
    myread(fh, (PSTR)&cNewCards, sizeof(int));

    /* allocate the object to hold all the headers */
    if (!GlobalReAlloc(hCards, (unsigned long)(cNewCards * sizeof(CARDHEADER)), GMEM_MOVEABLE))
        {
        CardfileOkError(IDS_EINSMEMORY);
        goto ReadClean;
        }
    /* save card count */
    cCards = cNewCards;
    lpTCards = (LPCARDHEADER) GlobalLock(hCards);

    /* for all cards */
    for(i = 0; i < cNewCards; ++i)
        {
        /* read the header, and save it */
        myread(fh, (PSTR)&CurCardHead, sizeof(CARDHEADER));
        *lpTCards++ = CurCardHead;
        }
    GlobalUnlock(hCards);
    fFileDirty = FALSE;

    mainreopen = tmpmain;     /* save new OFSTRUCT */
    Mylstrcpy((LPSTR)CurIFile, (LPSTR)mainreopen.szPathName);
    result = TRUE;
ReadClean:
    MyClose(fh);
    return(result);
    }

/* this routine checks for an extension, and if none exists, appends */
/* the default (CRD).  A single '.' denotes an extension, so here is what */
/* this routine will do:                    */
/* FOO     -> FOO.CRD                       */
/* FOO.    -> FOO.                          */
/* FOO.BAR -> FOO.BAR                       */

void FAR AppendExtension(pchName, pchBuf)
PSTR pchName;
PSTR pchBuf;
    {
    char *pch1;
    char ch;

    /* save name */
    Mylstrcpy((LPSTR)pchBuf, (LPSTR)pchName);
    /* get to end */
    pch1 = pchBuf + Mylstrlen((LPSTR)pchBuf);
    /* scan backwards for beginning or end of filename */
    while ((ch = *pch1) != '.' && ch != '\\' && ch != ':' && pch1 > pchBuf)
        pch1 = (char *)AnsiPrev((LPSTR)pchBuf, (LPSTR)pch1);
    /* if no '.', need to tack on extension */
    if (*pch1 != '.')
        Mylstrcat((LPSTR)pchBuf, (LPSTR)".CRD");
    /* what the heck, upper case it */
    AnsiUpper((LPSTR)pchBuf);
    }

/* this routin writes out a cardfile */
BOOL FAR WriteCardFile(pchName)
PSTR pchName;
    {
    LPCARDHEADER lpTCards = NULL;       /* so cleaning works */
    char bakName[PATHMAX];
    unsigned i;
    int fhBak;
    int fhDollar;
    int fhOld;
    int fh;
    unsigned long lCurLoc;
    unsigned long lCardData;
    unsigned bmSize;
    unsigned cchWritten;
    unsigned bmInfo[4];
    HANDLE hText;
    LPSTR lpText;
    HANDLE hBits;
    LPSTR lpBits;
    char buf[PATHMAX];
    unsigned tSize;
    int fSameFile;
    char *pchFileName;
    int result = FALSE;
    CARDHEADER CardHeader;
    LPCARDHEADER lpCards;
    OFSTRUCT bakofStruct;

    /* if no extension, append the default one */
    AppendExtension(pchName, buf);

    /* if saving to same file, need to save out to temporary one */
    if (fSameFile = !Mylstrcmp(buf, CurIFile))
        {
        /* get a tempfile name */
        if(!GetTempFileName(CurIFile[0] | TF_FORCEDRIVE, (LPSTR)"CRD", 0, (LPSTR)bakName))
            goto SaveTempProblem;
        }
    else
        /* otherwise, just save to specified file */
        Mylstrcpy((LPSTR)bakName, (LPSTR)buf);

    /* open the new file */
    fhBak = OpenFile((LPSTR)bakName, (LPOFSTRUCT)&bakofStruct, OF_CREATE);
    /* open files */
    if (CurIFile[0])
        pchFileName = CurIFile;
    else
        pchFileName = rgchCardData;
    /* open temporary file */
    if(fReadOnly || !(fhDollar = OpenFile((LPSTR)pchFileName, (LPOFSTRUCT)&tmpreopen, OF_PROMPT | OF_REOPEN | 2)) == -1)
        {
SaveTempProblem:
        CardfileOkError(IDS_EDISKFULLFILE);
        return(FALSE);
        }
    if (fhBak < 0)
        {
        goto CantMakeFile;
        }
    /* if curifile is null, then won't use fhOld */
    if (CurIFile[0])
        fhOld = OpenFile((LPSTR)CurIFile, (LPOFSTRUCT)&mainreopen, OF_PROMPT | OF_REOPEN);

    if (MyLseek(fhBak, 0L, 0) == -1)
        {
        if (CurIFile[0])
            MyClose(fhOld);
CantMakeFile:
        MyClose(fhDollar);
        CardfileOkError(IDS_ECANTMAKEFILE);
        return(FALSE);
        }
    /* truncate file */
    mywrite(fhBak, (PSTR)"", 0);

    /* write out cardfile magic bytes */
    mywrite(fhBak, (PSTR)"MGC", 3);

    /* write the number of cards in the file */
    mywrite(fhBak, (PSTR)&cCards, sizeof(int));
    lCardData = MyLseek(fhBak, 0L, 1) + (cCards * sizeof(CARDHEADER));

    /* allocate buf for card text */
    hText = GlobalAlloc(GHND, (long)CARDTEXTSIZE);
    if (!hText)
        goto WFInsMem;
    lpText = (LPSTR)GlobalLock(hText);

    /* lock down the card headers */
    lpCards = lpTCards = (LPCARDHEADER) GlobalLock(hCards);

    /* for each card in file */
    for(i = 0; i < cCards; ++i)
        {
        /* see where the card's data is */
        if (lpTCards->flags & FTMPFILE)
            fh = fhDollar;
        else
            fh = fhOld;
        /* seek to data */
        MyLseek(fh, lpTCards->lfData, 0);
        /* get header */
        CardHeader = *lpTCards++;
        /* clear out tmpfile bit */
        CardHeader.flags &= (!FTMPFILE);
        CardHeader.lfData = lCardData;
        /* write header */
        if (mylwrite(fhBak, (LPSTR)&CardHeader, sizeof(CARDHEADER)) < sizeof(CARDHEADER))
            goto WFDiskFull;
        lCurLoc = MyLseek(fhBak, 0L, 1);
        /* seek to data spot in new file */
        MyLseek(fhBak, lCardData, 0);
        myread(fh, (PSTR)&bmSize, sizeof(int));
        mywrite(fhBak, (PSTR)&bmSize, sizeof(int));

        /* if bitmap, save it */
        if (bmSize)
            {
            myread(fh, (PSTR)bmInfo, 4 * sizeof(int));
            mywrite(fhBak, (PSTR)bmInfo, 4 * sizeof(int));
            hBits = GlobalAlloc(GHND, (unsigned long)bmSize);
            if (!hBits)
                {
WFInsMem:
                MyClose(fhBak);
                MyClose(fhOld);
                MyClose(fhDollar);
                Fdelete(bakName);
                CardfileOkError(IDS_EINSMEMORY);
                goto WriteFileClean;
                }
            lpBits = (LPSTR)GlobalLock(hBits);
            mylread(fh, (LPSTR)lpBits, bmSize);
            cchWritten = mylwrite(fhBak, (LPSTR)lpBits, bmSize);
            GlobalUnlock(hBits);
            GlobalFree(hBits);
            if (cchWritten < bmSize)
                goto WFDiskFull;
            }
        /* save text */
        myread(fh, (PSTR)&tSize, sizeof(int));
        if (mywrite(fhBak, (PSTR)&tSize, sizeof(int)) < sizeof(int))
            goto WFDiskFull;
        mylread(fh, (LPSTR)lpText, tSize);
        if (mylwrite(fhBak, (LPSTR)lpText, tSize) < tSize)
            {
WFDiskFull:
            MyClose(fhBak);
            MyClose(fhOld);
            MyClose(fhDollar);
            Fdelete(bakName);
            CardfileOkError(IDS_EDISKFULLFILE);
            goto WriteFileClean;
            }

        lCardData = MyLseek(fhBak, 0L, 1);
        MyLseek(fhBak, lCurLoc, 0);
        }
    MyClose(fhBak);
    if (CurIFile[0])
        MyClose(fhOld);
    MyLseek(fhDollar, 0L, 0);
    mywrite(fhDollar, (PSTR)"", 0);
    MyClose(fhDollar);
    if (fSameFile)
        {
        Fdelete(buf);
        Frename(bakName, buf);
        fhOld = OpenFile((LPSTR)buf, (LPOFSTRUCT)&mainreopen, 2);
        }
    else
        {
        AnsiUpper((LPSTR)CurIFile);
        fhOld = OpenFile((LPSTR)bakName, (LPOFSTRUCT)&mainreopen, 2);
        }

    Mylstrcpy((LPSTR)CurIFile, (LPSTR)mainreopen.szPathName);

    MyLseek(fhOld, 5L, 0);
    lpTCards = lpCards;
    for(i = 0; i < cCards; ++i)
        {
        myread(fhOld, (PSTR)&CardHeader, sizeof(CARDHEADER));
        *lpTCards++ = CardHeader;
        }
    MyClose(fhOld);
    fFileDirty = FALSE;
    result = TRUE;
WriteFileClean:
    if (hText)
        {
        GlobalUnlock(hText);
        GlobalFree(hText);
        }
    if (lpTCards)
        GlobalUnlock(hCards);
    return(result);
    }

/* write out the data for the current card */
BOOL FAR WriteCurCard(pCardHead, pCard, lpText)
PCARDHEADER pCardHead;
PCARD pCard;
LPSTR lpText;
    {
    int fh;
    unsigned long lEnd;
    HANDLE hBits;
    LPSTR lpBits;
    int zero = 0;
    unsigned i;
    unsigned tSize;
    int cEighths;
    char *pchFileName;
    unsigned cchWritten;
    unsigned long fhLoc;

    /* get right string in case of error */
    if (CurIFile[0])
        pchFileName = CurIFile;
    else
        pchFileName = rgchCardData;

    /* open temp file, if necessary asking for it */
    if ((fh = OpenFile((LPSTR)pchFileName, (LPOFSTRUCT)&tmpreopen, OF_CANCEL | OF_PROMPT | OF_REOPEN | 2)) == -1)
        {
        CardfileOkError(IDS_EOPENTEMPSAVE);
        return(FALSE);
        }

    /* seek to end of temp file */
    lEnd = MyLseek(fh, 0L, 2);
    /* if bitmap, save it */
    if (pCard->hBitmap)
        {
        /* get buffer for bits */
        if (hBits = GlobalAlloc(GHND, (unsigned long)pCard->bmSize))
            {
            /* write out bitmap info */
            lpBits = (LPSTR)GlobalLock(hBits);
            mywrite(fh, (PSTR)&pCard->bmSize, sizeof(int));
            mywrite(fh, (PSTR)&pCard->cxBitmap, sizeof(int));
            if(mywrite(fh, (PSTR)&pCard->cyBitmap, sizeof(int)) < sizeof(int))
                goto WCDiskFull;
            cEighths = (pCard->xBitmap * 8) / CharFixWidth;
            mywrite(fh, (PSTR)&cEighths, sizeof(int));
            cEighths = (pCard->yBitmap * 8) / CharFixHeight;
            if(mywrite(fh, (PSTR)&cEighths, sizeof(int)) < sizeof(int))
                goto WCDiskFull;
            fhLoc = MyLseek(fh, 0L, 1);
            GetBitmapBits(pCard->hBitmap, (unsigned long)pCard->bmSize, lpBits);
            while (MyLseek(fh, fhLoc, 0) == -1)
                fh = OpenFile((LPSTR)pchFileName, (LPOFSTRUCT)&tmpreopen, OF_PROMPT | OF_REOPEN | OF_READWRITE);
            if (mylwrite(fh, (LPSTR)lpBits, pCard->bmSize) < pCard->bmSize)
                goto WCDiskFull;
            GlobalUnlock(hBits);
            GlobalFree(hBits);
            }
        else
            {
            MyClose(fh);
            CardfileOkError(IDS_EINSMEMSAVE);
            return(FALSE);
            }
        }
    else
        /* no bitmap */
        mywrite(fh, (PSTR)&zero, sizeof(int));

    /* save text */
    tSize = Mylstrlen((LPSTR)lpText);
    if (tSize >= CARDTEXTSIZE)
        tSize = CARDTEXTSIZE-1;
    if (mywrite(fh, (PSTR)&tSize, sizeof(int)) < sizeof(int))
        goto WCDiskFull;
    cchWritten = mylwrite(fh, (LPSTR)lpText, tSize);
    MyClose(fh);
    if (cchWritten < tSize)
        {
WCDiskFull:
        CardfileOkError(IDS_EDISKFULLSAVE);
        return(FALSE);
        }
    pCardHead->flags |= FTMPFILE;
    pCardHead->lfData = lEnd;
    return(TRUE);
    }

/* read current card's data */
BOOL FAR ReadCurCardData(pCardHead, pCard, lpText)
PCARDHEADER pCardHead;
PCARD pCard;
LPSTR lpText;
    {
    int fh;
    HANDLE hBits;
    LPSTR lpBits;
    HBITMAP hBitmap;
    unsigned i;
    unsigned tSize;
    int cEighths;
    int fZero;
    char *pchFileName;
    int result = TRUE;
    unsigned long fhLoc;

    /* assume no bitmap */
    pCard->hBitmap = 0;

    /* if a new card, then can't get it out of any file */
    if (pCardHead->flags & FNEW)
        {
        /* no text */
        lpText = (LPSTR)"";
        return(result);
        }

    /* if data is in temp file, get the right file handle */
    if (pCardHead->flags & FTMPFILE)
        {
        if (CurIFile[0])
            pchFileName = CurIFile;
        else
            pchFileName = rgchCardData;
        fh = OpenFile((LPSTR)pchFileName, (LPOFSTRUCT)&tmpreopen, OF_PROMPT | OF_REOPEN | 2);
        }
    else
        fh = OpenFile((LPSTR)CurIFile, (LPOFSTRUCT)&mainreopen, OF_PROMPT | OF_REOPEN);

    /* seek to data */
    MyLseek(fh, pCardHead->lfData, 0);

    /* is there a bitmap */
    myread(fh, (PSTR)&(pCard->bmSize), sizeof(int));
    if (pCard->bmSize)
        {
        /* yep, read it in */
        myread(fh, (PSTR)&(pCard->cxBitmap), sizeof(int));
        myread(fh, (PSTR)&(pCard->cyBitmap), sizeof(int));
        myread(fh, (PSTR)&cEighths, sizeof(int));
        pCard->xBitmap = (cEighths * CharFixWidth) / 8;
        myread(fh, (PSTR)&cEighths, sizeof(int));
        pCard->yBitmap = (cEighths * CharFixHeight) / 8;
        if(hBits = GlobalAlloc(GHND, (unsigned long)pCard->bmSize))
            {
            lpBits = (LPSTR)GlobalLock(hBits);
            mylread(fh, (LPSTR)lpBits, pCard->bmSize);
            fhLoc = MyLseek(fh, 0L, 1);
            if (!(pCard->hBitmap = CreateBitmap(pCard->cxBitmap, pCard->cyBitmap, 1, 1, lpBits)))
                result = FALSE;
            /* createbitmap may have caused disk swap, make user reinsert disk*/
            while (MyLseek(fh, fhLoc, 0) == -1)
                {
                if (pCardHead->flags & FTMPFILE)
                    {
                    if (CurIFile[0])
                        pchFileName = CurIFile;
                    else
                        pchFileName = rgchCardData;
                    fh = OpenFile((LPSTR)pchFileName, (LPOFSTRUCT)&tmpreopen, OF_PROMPT | OF_REOPEN | OF_READWRITE);
                    }
                else
                    fh = OpenFile((LPSTR)CurIFile, (LPOFSTRUCT)&mainreopen, OF_PROMPT | OF_REOPEN);
                }
            GlobalUnlock(hBits);
            GlobalFree(hBits);
            }
        else
            {
            MyLseek(fh, (unsigned long)pCard->bmSize, 1);
            result = FALSE;
            }
        }
    /* read text */
    myread(fh, (PSTR)&tSize, sizeof(int));
    if (tSize >= CARDTEXTSIZE)
        tSize = CARDTEXTSIZE-1;
    mylread(fh, (LPSTR)lpText, tSize);
    *(lpText+tSize) = 0;
    MyClose(fh);
    return (result);
    }

/* create a temporary file, with a random name provided by system */
void FAR MakeTmpFile(hInstance)
HANDLE hInstance;
    {
    int  fh;

    if(GetTempFileName(CurIFile[0], (LPSTR)"CRD", 0, (LPSTR)TmpFile))
        {
        if ((fh = OpenFile((LPSTR)TmpFile, (LPOFSTRUCT)&tmpreopen, OF_CREATE)) < 0)
            goto CantMakeTmp;
        else
            MyClose(fh);
        }
    else
        {
        /* if can't make temp file, mark "read only", although I'm sure */
        /* this readonly stuff doesn't work well.  Should rarely happen. */
CantMakeTmp:
        CardfileOkError(IDS_ECANTMAKETEMP);
        fReadOnly = TRUE;
        }
    }
