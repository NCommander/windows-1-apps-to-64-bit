#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* this routine handles the cut and copy commands, putting the */
/* appropriate information out in the clipboard. */

void FAR DoCutCopy(event)
int event;
    {
    HBITMAP hBitmap;
    RECT    rect;
    BITMAP  bmInfo;

    /* if in text mode, just tell edit control to do it */
    if (EditMode == IDM_TEXT)
        SendMessage(hCardWnd, event == IDM_CUT ? WM_CUT : WM_COPY, 0, 0L);
    /* else if there's a bitmap, and can open clipboard */
    else if (CurCard.hBitmap && OpenClipboard(hCardfileWnd))
        {
        /* empty it */
        EmptyClipboard();
        hBitmap = CurCard.hBitmap;
        /* if copy, need to create a copy of the bitmap */
        if (event == IDM_COPY)
            {
            GetObject(CurCard.hBitmap, sizeof(BITMAP), (LPSTR)&bmInfo);
            if (!(CurCard.hBitmap = MakeBitmapCopy( CurCard.hBitmap, &bmInfo)))
                {
                /* save the copy for our own use */
                CurCard.hBitmap = hBitmap;
                CardfileOkError(IDS_EINSMEMORY);
                hBitmap = NULL;
                }
            }
        /* otherwise, get rid of bitmap from card */
        else
            {
            /* force repaint of bitmap area */
            SetRect((LPRECT)&rect, CurCard.xBitmap, CurCard.yBitmap, CurCard.cxBitmap+CurCard.xBitmap, CurCard.yBitmap+CurCard.cyBitmap);
            InflateRect((LPRECT)&rect, 1, 1);
            InvalidateRect(hCardWnd, (LPRECT)&rect, TRUE);
            /* no bitmap any more */
            CurCard.hBitmap = 0;
            CurCardHead.flags |= FDIRTY;
            /* tiny drag rectangle */
            dragRect.bottom = dragRect.top + CharFixHeight;
            dragRect.right = dragRect.left + CharFixWidth;
            }
        /* if there is a bitmap for the clipboard */
        if (hBitmap)
            /* give it away */
            SetClipboardData(CF_BITMAP, hBitmap);
        CloseClipboard();
        }
    }

/* this handles pasting */
void FAR DoPaste()
    {
    HBITMAP hBitmap;
    BITMAP bmInfo;
    RECT rect;

    /* if in text mode, let edit control handle it */
    if (EditMode == IDM_TEXT)
        {
        if (!SendMessage(hCardWnd, WM_PASTE, 0, 0L))
            CardfileOkError(IDS_ECLIPEMPTYTEXT);
        }
    /* otherwise, paste in a bitmap */
    else if (OpenClipboard(hCardfileWnd))
        {
        /* if there is a bitmap to paste */
        if (hBitmap = (HBITMAP) GetClipboardData(CF_BITMAP))
            {
            /* need to make a copy of it */
            GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bmInfo);
            hBitmap = MakeBitmapCopy( hBitmap, &bmInfo);
            if (!hBitmap)
                {
                CardfileOkError(IDS_EINSMEMORY);
                }
            else
                {
                /* if there's already a bitmap */
                if (CurCard.hBitmap)
                    {
                    /* force redraw on the area */
                    SetRect((LPRECT)&rect, CurCard.xBitmap, CurCard.yBitmap, CurCard.cxBitmap+CurCard.xBitmap, CurCard.yBitmap+CurCard.cyBitmap);
                    InvalidateRect(hCardWnd, (LPRECT)&rect, TRUE);
                    /* delete the old one */
                    DeleteObject(CurCard.hBitmap);
                    }
                /* save new bitmap information */
                CurCard.cxBitmap = bmInfo.bmWidth;
                CurCard.cyBitmap = bmInfo.bmHeight;
                CurCard.bmSize = bmInfo.bmHeight * bmInfo.bmWidthBytes;
                CurCard.xBitmap = dragRect.left;
                CurCard.yBitmap = dragRect.top;
                CurCard.hBitmap = hBitmap;
                /* make sure selection rectangle is good */
                SetFocus(NULL);
                SetFocus(hCardWnd);
                /* cause repaint */
                SetRect((LPRECT)&rect, CurCard.xBitmap, CurCard.yBitmap, CurCard.cxBitmap+CurCard.xBitmap, CurCard.yBitmap+CurCard.cyBitmap);
                InvalidateRect(hCardWnd, (LPRECT)&rect, TRUE);
                CurCardHead.flags |= FDIRTY;
                }
            }
        else
            CardfileOkError(IDS_ECLIPEMPTYPICT);
        CloseClipboard();
        }
    }


/* makes a copy of the bitmap whose handle is hbmSrc, and which is */
/* described by pBitmap */
HBITMAP MakeBitmapCopy( hbmSrc, pBitmap)
HBITMAP hbmSrc;
PBITMAP pBitmap;
    {
    HBITMAP hBitmap = NULL;
    HDC hDCSrc = NULL;
    HDC hDCDest = NULL;
    HDC hDC;

    hDC = GetDC(hCardfileWnd);
    hDCSrc = CreateCompatibleDC( hDC ); /* get memory dc */
    hDCDest = CreateCompatibleDC( hDC );
    ReleaseDC(hCardfileWnd, hDC);
    if (!hDCSrc || !hDCDest)
        goto MakeCopyEnd;

    /* select in passed bitmap */
    if (!SelectObject( hDCSrc, hbmSrc ))
        goto MakeCopyEnd;

    /* create new monochrome bitmap */

    if (!(hBitmap = CreateBitmap( pBitmap->bmWidth, pBitmap->bmHeight, 1, 1, (LPSTR) NULL )))
        goto MakeCopyEnd;

    /* Now blt the bitmap contents.  The screen driver in the source will
       "do the right thing" in copying color to black-and-white. */

    if (!SelectObject(hDCDest, hBitmap) ||
        !BitBlt( hDCDest, 0, 0, pBitmap->bmWidth, pBitmap->bmHeight, hDCSrc, 0, 0, SRCCOPY ))
        {
        DeleteObject(hBitmap);
        hBitmap = NULL;
        }

MakeCopyEnd:
    if (hDCSrc)
        DeleteObject(hDCSrc);
    if (hDCDest)
        DeleteObject(hDCDest);
    return (hBitmap);
    }
