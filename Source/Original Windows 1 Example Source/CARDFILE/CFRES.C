#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

int PASCAL WinMain(hInstance, hPrevInstance, lpszCommandLine, cmdShow)
HANDLE hInstance, hPrevInstance;
LPSTR lpszCommandLine;
int cmdShow;
    {
    MSG msg;
    HDC hDC;

    /* save instance handle for future use*/
    hCardfileInstance = hInstance;

    /* if first instance of cardfile */
    if (!hPrevInstance)
        {
        /* do all one-time initialization */
        if (!CardfileInit())
            goto InitError;
        }
    else
        /* otherwise get interesting data from previous instance */
        GetOldData(hPrevInstance);

    /* do per-instance initialization */
    if (InitInstance(hInstance, lpszCommandLine, cmdShow))
        {
        /* message loop */
        while(TRUE)
            {
            /* if edit control exists, and no keyboard messages left to handle*/
            if (IsWindow(hCardWnd) && !PeekMessage((LPMSG)&msg, hCardWnd, WM_KEYFIRST, WM_KEYLAST, FALSE))
                {
                /* see if need to update bitmap */
                /* as the user types into the edit control, the edit control*/
                /* blasts over any bitmap that is underneath it.  Cardfile */
                /* therefore needs to redraw the bitmap.  It waits until it */
                /* sees that there are no keys left to handle, and then does */
                /* the drawing.  This is faster than redrawing once per */
                /* keystroke */
                if (fNeedToUpdateBitmap)
                    {
                    hDC = GetDC(hCardWnd);
                    CardPaint(hDC);
                    ReleaseDC(hCardWnd, hDC);
                    fNeedToUpdateBitmap = FALSE;
                    }
                }
            if (GetMessage((LPMSG)&msg, NULL, 0, 0))
                {
                if (TranslateAccelerator(hCardfileWnd, hAccel, (LPMSG)&msg) == 0)
                    {
                    TranslateMessage((LPMSG)&msg);
                    DispatchMessage((LPMSG)&msg);
                    }
                }
            else
                break;
            }
        }
    else
        {
InitError:
        MessageBox(NULL, (LPSTR)NotEnoughMem, (LPSTR)NULL, MB_OK | MB_ICONEXCLAMATION);
        }
    return(0);
    }
