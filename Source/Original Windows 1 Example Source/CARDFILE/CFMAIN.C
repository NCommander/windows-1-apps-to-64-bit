#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* this is the main Window Procedure for cardfile's window */
long far PASCAL CardfileWndProc(hwnd, message, wParam, lParam)
HWND hwnd;
unsigned message;
WORD wParam;
DWORD lParam;
{
    PAINTSTRUCT ps;
    LPCARDHEADER lpCards;
    int range;
    HMENU hMenu;
    char buf[30];
    HDC hDC;
    MSG msg;
    RECT rect;
    int y;

    switch (message)
        {
        case WM_CREATE:
            /* save the window handle */
            hCardfileWnd = hwnd;
            /* read in relevant win.ini information */
            CardfileWinIniChange();
            /* add "About" to the system menu */
            hMenu = GetSystemMenu(hwnd, FALSE);
            ChangeMenu(hMenu, 0, (LPSTR)NULL, -1, MF_APPEND | MF_SEPARATOR);
            LoadString(hCardfileInstance, IDS_ABOUT, (LPSTR)buf, 30);
            ChangeMenu(hMenu, 0, (LPSTR)buf, IDM_ABOUT, MF_APPEND | MF_STRING);
            /* set the caption */
            SetCaption();
            /* initialize the scroll bars */
            SetScrollRange(hwnd, SB_HORZ, 0, cCards-1, FALSE);
            SetScrollRange(hwnd, SB_VERT, 0, 0, FALSE);
            break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            /* handle mouse input */
            CardfileMouse(hwnd, message, wParam, MAKEPOINT(lParam));
            break;
        case WM_ENDSESSION:
            /* windows is about to close down */
            if (wParam)
                /* make sure temp file goes away */
                Fdelete(TmpFile);
            break;
        case WM_QUERYENDSESSION:
            /* windows is asking if it is ok to close down */
            /* if there are changes in cardfile, ask if user wants to */
            /* save them, and give user a chance to cancel end session */
            if (MaybeSaveFile(TRUE))
                {
                /* user says it's ok to end session.  make sure that */
                /* cardfile is in a position to continue in case user */
                /* cancels end session from some other application */
                SetCurCard(iFirstCard);
                return(TRUE);
                }
            else
                /* user cancelled.  Tell windows not to end session */
                return(FALSE);
            break;
        case WM_CLOSE:
            /* user is trying to close cardfile's window */
            /* if changes in cardfile, ask user if he wants to save, or */
            /* wants to cancel close */
            if (MaybeSaveFile(FALSE))
                {
                /* everything is ok to close */
                DestroyWindow(hwnd);
                }
            return(TRUE);
        case WM_DESTROY:
            /* cardfile's window is going away: clean up */
            /* get rid of temp file */
            Fdelete(TmpFile);
            /* if last instance of cardfile, get rid of brushes */
            /* if not last instance, then other's will still use them */
            if (GetModuleUsage(hCardfileInstance) == 1)
                {
                DeleteObject(hbrGray);
                DeleteObject(hbrBack);
                DeleteObject(hbrLine);
                }
            /* ok, time to quit */
            PostQuitMessage(0);
            return(TRUE);
        case WM_INITMENU:
            /* the user has clicked on a menu.  Enable and check everything*/
            /* that should be, before the menu is actually brought up */
            UpdateMenu();
            break;
        case WM_COMMAND:
            /* one of three things */
            /* the edit control may be reporting that it has run out of */
            /* memory and can't accept any more input */
            if (LOWORD(lParam) == hCardWnd && HIWORD(lParam) == EN_ERRSPACE)
                CardfileOkError(IDS_EINSMEMORY);
            /* or the edit control may be reporting that the user has */
            /* just changed the contents of the edit control */
            else if (wParam == IDM_EDITWINDOW)
                {
                /* the edit control also passes this message on a call */
                /* to SetWindowText.  We don't want to draw bitmap twice, */
                /* so check fSettingText */
                if (!fSettingText && HIWORD(lParam) == EN_CHANGE)
                    fNeedToUpdateBitmap = TRUE;
                fSettingText = FALSE;
                }
            else
                /* or, the common thing, the user has executed a menu command */
                CardfileInput(hwnd, wParam);
            break;
        case WM_CTLCOLOR:
            /* this message gives cardfile a chance to set the background */
            /* color of the edit control, which should always be white */
            if (LOWORD(lParam) == hCardWnd)
                {
                SetBkColor((HDC)wParam, 0x00ffffff);
                SetTextColor((HDC)wParam, 0L);
                return((long)hbrWhite);
                }
            /* if not edit control, pass message on */
            goto CallDefProc;
        case WM_ERASEBKGND:
            /* paint the background, which will be BLUE or GRAY */
            CardfileEraseBkGnd(hwnd, (HDC)wParam);
            break;
        case WM_PAINT:
            /* time to paint, either a phonebook paint, or a cardfile paint */
            BeginPaint(hwnd, (LPPAINTSTRUCT)&ps);
            if (CardPhone == IDM_PHONEBOOK)
                PhonePaint(hwnd, ps.hdc, (LPRECT)&ps.rcPaint);
            else
                CardfilePaint(hwnd, ps.hdc, (LPRECT)&ps.rcPaint);
            EndPaint(hwnd, (LPPAINTSTRUCT)&ps);
            break;
        case WM_SIZE:
            /* the main window has changed size */
            if (wParam != SIZEICONIC)
                CardfileSize(hwnd, LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_HSCROLL:
            /* there is a horizontal scroll bar only in cardfile mode */
            CardfileScroll(hwnd, wParam, LOWORD(lParam));
            break;
        case WM_VSCROLL:
            /* the vertical scroll bar only appears in phonebook mode */
            PhoneScroll(hwnd, wParam, LOWORD(lParam));
            break;
        case WM_CHAR:
            /* handle character input.  NOTE: this message will only */
            /* come through when Cardfile is in phonebook mode */
            CardChar(wParam);
            break;
        case WM_KEYDOWN:
            /* handle keys.  Again, this will only come through when */
            /* cardfile is in phonebook mode */
            PhoneKey(hwnd, wParam);
            break;
        case WM_ACTIVATE:
            /* activate, and set focus to either edit control, or main window*/
            /* depending upon which mode we are in */
            if (wParam && !HIWORD(lParam))
                if (CardPhone == IDM_CARDFILE)
                    SetFocus(hCardWnd);
                else
                    SetFocus(hCardfileWnd);
            break;
        case WM_WININICHANGE:
            /* the information in win.ini has changed.  Read new stuff */
            CardfileWinIniChange();
            break;
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            /* if in phonebook, take down highlighting */
            if(CardPhone == IDM_PHONEBOOK)
                {
                hDC = GetDC(hCardfileWnd);
                y = (iFirstCard - iTopCard) * CharFixHeight;
                SetRect((LPRECT)&rect, 0, y, (LINELENGTH+2)*CharFixWidth, y+CharFixHeight);
                InvertRect(hDC, (LPRECT)&rect);
                ReleaseDC(hCardfileWnd, hDC);
                }
            break;
        case WM_SYSCOMMAND:
            /* user has chosen ABOUT, put up dialog */
            if (wParam == IDM_ABOUT)
                {
                CardfileInput(hwnd, wParam);
                break;
                }
        default:
            /* some other message, let default handler take care of it */
CallDefProc:
            return(DefWindowProc(hwnd, message, wParam, lParam));
            break;
        }
    return(0L);
}
