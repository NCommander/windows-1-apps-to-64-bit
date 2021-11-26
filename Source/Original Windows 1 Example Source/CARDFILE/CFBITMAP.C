#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* this handles all mouse input in the edit control when in picture mode */
/* The user can use the mouse to move the card's bitmap, or to move the */
/* tiny rectangle to position where a bitmap would get pasted in, if */
/* there is no card. */
void FAR BMMouse(hWindow, message, wParam, pt)
HWND hWindow;
unsigned message;
WORD wParam;
POINT pt;
    {
    RECT rect;
    int iCard;
    HDC hDC;
    int t;

    switch(message)
        {
        case WM_LBUTTONDOWN:
            /* if mouse is in bitmap rectangle */
            if (PtInRect((LPRECT)&dragRect, pt))
                {
                /* make sure we don't miss upclick */
                SetCapture(hWindow);
                /* set maximum coordinates */
                GetClientRect(hWindow, (LPRECT)&rect);
                xmax = rect.right - CharFixWidth;
                ymax = rect.bottom - CharFixHeight;
                /* remember that user is dragging bitmap */
                fBMDown = TRUE;
                /* save the reference point */
                dragPt = pt;
                }
            break;
        case WM_LBUTTONUP:
            /* dragging */
            if (fBMDown)
                {
                /* done */
                ReleaseCapture();
                /* if bitmap moved */
                if (dragRect.top != CurCard.yBitmap || dragRect.left != CurCard.xBitmap)
                    {
                    /* remember old coords */
                    SetRect((LPRECT)&rect, CurCard.xBitmap, CurCard.yBitmap, CurCard.xBitmap+CurCard.cxBitmap, CurCard.yBitmap+CurCard.cyBitmap);
                    /* save new coords */
                    CurCard.xBitmap = dragRect.left;
                    CurCard.yBitmap = dragRect.top;
                    /* clear out old and new bitmap rectangles */
                    InvalidateRect(hCardWnd, (LPRECT)&rect, TRUE);
                    InvalidateRect(hCardWnd, (LPRECT)&dragRect, TRUE);
                    /* if there is a bitmap, mark card as dirty */
                    if (CurCard.hBitmap)
                        CurCardHead.flags |= FDIRTY;
                    }
                /* not dragging any more */
                fBMDown = FALSE;
                }
            break;
        case WM_MOUSEMOVE:
            /* if dragging */
            if (fBMDown)
                {
                /* allow for maximum and minimums */
                t = dragRect.left + pt.x - dragPt.x;
                if (t > xmax)
                    pt.x = xmax - dragRect.left + dragPt.x;
                else if (t < CharFixWidth - (dragRect.right - dragRect.left))
                    pt.x = CharFixWidth - (dragRect.right - dragRect.left) - dragRect.left + dragPt.x;

                t = dragRect.top + pt.y - dragPt.y;
                if (t > ymax)
                    pt.y = ymax - dragRect.top + dragPt.y;
                else if (t < CharFixHeight - (dragRect.bottom - dragRect.top))
                    pt.y = CharFixHeight - (dragRect.bottom - dragRect.top) - dragRect.top + dragPt.y;

                /* if new dragPt */
                if (dragPt.x != pt.x || dragPt.y != pt.y)
                    {
                    /* redraw selection rectangles */
                    hDC = GetDC(hCardWnd);
                    DrawXorRect(hDC, &dragRect);
                    OffsetRect((LPRECT)&dragRect, pt.x - dragPt.x, pt.y - dragPt.y);
                    dragPt = pt;
                    DrawXorRect(hDC, &dragRect);
                    ReleaseDC(hCardWnd, hDC);
                    }
                }
            break;
        }
    }

/* sets the bitmap rect and draws it */
void FAR TurnOnBitmapRect()
    {
    if (CurCard.hBitmap)
        SetRect((LPRECT)&dragRect, CurCard.xBitmap, CurCard.yBitmap, CurCard.xBitmap+CurCard.cxBitmap, CurCard.yBitmap+CurCard.cyBitmap);
    else
        SetRect((LPRECT)&dragRect, 5, 5, 5+CharFixWidth, 5+CharFixHeight);
    XorBitmapRect();
    }

/* just draws the bitmap rect */
void FAR TurnOffBitmapRect()
    {
    XorBitmapRect();
    }

void XorBitmapRect()
    {
    HDC hDC;

    hDC = GetDC(hCardWnd);
    DrawXorRect(hDC, &dragRect);
    ReleaseDC(hCardWnd, hDC);
    }

/* puts up a rectangle xor'ed with what is on screen */
void FAR DrawXorRect(hDC, pRect)
HDC hDC;
PRECT pRect;
    {
    int     x,y;
    POINT   point;

    SelectObject(hDC, hbrWhite);

    x = pRect->right  - (point.x = pRect->left);
    y = pRect->bottom - (point.y = pRect->top);

    PatBlt(hDC, point.x, point.y, x, 1, PATINVERT);
    point.y = pRect->bottom - 1;
    PatBlt(hDC, point.x, point.y, x, 1, PATINVERT);
    point.y = pRect->top;
    PatBlt(hDC, point.x, point.y, 1, y, PATINVERT);
    point.x = pRect->right - 1;
    PatBlt(hDC, point.x, point.y, 1, y, PATINVERT);
    }

/* handles arrow keys when in bitmap mode.  This is the keyboard interface */
/* to moving the bitmap */
BOOL FAR BMKey(wParam)
WORD wParam;
    {
    int x;
    int y;

    x = dragRect.left;
    y = dragRect.top;

    switch(wParam)
        {
        case VK_UP:
            y -= CharFixHeight;
            break;
        case VK_DOWN:
            y += CharFixHeight;
            break;
        case VK_LEFT:
            x -= CharFixWidth;
            break;
        case VK_RIGHT:
            x += CharFixWidth;
            break;
        default:
            return(FALSE);
        }

    if (x > (LINELENGTH-1) * CharFixWidth)
        x = (LINELENGTH-1) * CharFixWidth;
    else if (x < CharFixWidth - (dragRect.right - dragRect.left))
        x = CharFixWidth - (dragRect.right - dragRect.left);

    if (y > 10 * CharFixHeight)
        y = 10 * CharFixHeight;
    else if (y < CharFixHeight - (dragRect.bottom - dragRect.top))
        y = CharFixHeight - (dragRect.bottom - dragRect.top);

    if (x != dragRect.left || y != dragRect.top)
        {
        InvalidateRect(hCardWnd, (LPRECT)&dragRect, TRUE);
        CurCard.xBitmap = x;
        CurCard.yBitmap = y;
        OffsetRect((LPRECT)&dragRect, x-dragRect.left, y-dragRect.top);
        InvalidateRect(hCardWnd, (LPRECT)&dragRect, TRUE);
        }
    CurCardHead.flags |= FDIRTY;
    return(TRUE);
    }
