#define NOCOMM
#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* GetOldData gets interesting data from a previous instance of Cardfile */
void FAR  GetOldData(hInstance)
HANDLE hInstance;
    {
    GetInstanceData(hInstance, (PSTR)&hbrWhite, 2);
    GetInstanceData(hInstance, (PSTR)&hbrBlack, 2);
    GetInstanceData(hInstance, (PSTR)&hbrGray, 2);
    GetInstanceData(hInstance, (PSTR)&hbrLine, 2);
    GetInstanceData(hInstance, (PSTR)&hbrBack, 2);
    GetInstanceData(hInstance, (PSTR)&CharFixHeight, 2);
    GetInstanceData(hInstance, (PSTR)&CharFixWidth, 2);
    GetInstanceData(hInstance, (PSTR)&ySpacing, 2);
    GetInstanceData(hInstance, (PSTR)&CardWidth, 2);
    GetInstanceData(hInstance, (PSTR)&CardHeight, 2);
    GetInstanceData(hInstance, (PSTR)&hArrowCurs, 2);
    GetInstanceData(hInstance, (PSTR)&hWaitCurs, 2);
    GetInstanceData(hInstance, (PSTR)&hAccel, 2);
    }

/* InitInstance does per-instance initialization */
BOOL FAR InitInstance(hInstance, lpszCommandLine, cmdShow)
HANDLE hInstance;
LPSTR lpszCommandLine;
int cmdShow;
    {
    int i;
    LPCARDHEADER lpCards;
    HWND hwnd = NULL;
    LPSTR lpchTmp;
    PSTR pchTmp;
    FARPROC lpCardWndProc;
    OFSTRUCT ofStruct;
    char buf[3];

    /* get resident strings */
    LoadString(hInstance, IDS_EINSMEMORY, (LPSTR)NotEnoughMem, 50);
    LoadString(hInstance, IDS_UNTITLED, (LPSTR)rgchUntitled, 30);
    LoadString(hInstance, IDS_CARDDATA, (LPSTR)rgchCardData, 40);
    LoadString(hInstance, IDS_STRINGINSERT, (LPSTR)buf, 3);
    if (!LoadString(hInstance, IDS_CARDFILE, (LPSTR)rgchCardfile, 40))
        goto InitError;

    /* this is used in MergeStrings */
    wMerge = *(unsigned *)buf;

    /* make ProcInstances for all dialog procedures, and for the */
    /* filter proc for the edit control */
    lpDlgProc = MakeProcInstance( DlgProc, hInstance );
    lpfnOpen = MakeProcInstance( fnOpen, hInstance );
    lpfnSave = MakeProcInstance( fnSave, hInstance );
    lpfnDial = MakeProcInstance( fnDial, hInstance );
    lpfnAbout = MakeProcInstance( fnAbout, hInstance );
    lpCardWndProc = MakeProcInstance( (FARPROC) CardWndProc, hInstance);
    lpfnAbortProc = MakeProcInstance( fnAbortProc, hInstance);
    lpfnAbortDlgProc = MakeProcInstance( fnAbortDlgProc, hInstance);
    /* unlikely that last one will work but others won't */
    if (!lpfnAbortDlgProc)
        goto InitError;

    /* allocate the basic data structure for storing the card headers */
    hCards = GlobalAlloc(GHND, (long)sizeof(CARDHEADER)); /* alloc first card */
    if (!hCards)
        goto InitError;

    /* make a single blank card */
    CurIFile[0] = 0;        /* file is untitled */
    cCards = 1;
    CurCardHead.line[0] = 0;
    CurCard.hBitmap = 0;
    CurCardHead.flags = FNEW;
    lpCards = (LPCARDHEADER)GlobalLock(hCards);
    *lpCards = CurCardHead;
    GlobalUnlock(hCards);

    /* create the Cardfile window */
    hwnd = CreateWindow(
              (LPSTR) rgchCardfileClass,
              (LPSTR) "",
              WS_TILEDWINDOW | WS_HSCROLL | WS_VSCROLL,
              0, 0, 0, 100,
              NULL, NULL,
              hInstance,
              (LPSTR)NULL);

    if (!hwnd)
        goto InitError;

    /* create the edit control window */
    hCardWnd = CreateWindow(
                (LPSTR)"Edit",
                (LPSTR)"",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE,
                0, 0, 0, 0,
                hwnd,
                IDM_EDITWINDOW,
                hInstance,
                (LPSTR)NULL);

    if (!hCardWnd)
        {
        /* some kind of failure */
InitError:
        MessageBox(hwnd, (LPSTR)NotEnoughMem, (LPSTR)NULL, MB_OK | MB_ICONEXCLAMATION);
        return(FALSE);
        }

    /* limit the number of characters that can be typed into the */
    /* edit control to the number that will fit on a card */
    SendMessage(hCardWnd, EM_LIMITTEXT, CARDTEXTSIZE, 0L);

    /* filter the messages going to the edit control */
    lpEditWndProc = (FARPROC)GetWindowLong(hCardWnd, GWL_WNDPROC);
    SetWindowLong(hCardWnd, GWL_WNDPROC, (LONG)lpCardWndProc);

    ShowWindow(hwnd, cmdShow);

    MakeTmpFile(hInstance);

    /* if a file was passed as a command line argument, try to read it in */
    for (lpchTmp = lpszCommandLine; *lpchTmp == ' '; lpchTmp++)
        ;
    for (pchTmp = CurIFile, i = 0; i < PATHMAX-1 && (BYTE)*lpchTmp > ' '; ++i)
        *pchTmp++ = *lpchTmp++;
    *pchTmp = 0;
    AnsiUpper((LPSTR)CurIFile);

    if (*CurIFile)
        if (!DoOpen(CurIFile))
            CurIFile[0] = 0;

    return(TRUE);
    }

/* CardfileInit does the one time initialization, such as class registration*/
BOOL FAR CardfileInit()
    {
    PWNDCLASS   pClass;
    HANDLE      hIcon;
    TEXTMETRIC  Metrics;    /* structure filled with font info */
    HDC hMemoryDC;
    LOGBRUSH logBrush;
    HDC hIC;

    /* initialize the brushes */
    hbrWhite    = GetStockObject(WHITE_BRUSH);
    hbrBlack    = GetStockObject(BLACK_BRUSH);
    hbrGray     = GetStockObject(GRAY_BRUSH);
    GetObject(hbrGray, sizeof(LOGBRUSH), (LPSTR)&logBrush);
    if (!(hbrGray = CreateBrushIndirect((LPLOGBRUSH)&logBrush)))
        hbrGray = hbrWhite;


    hIC = CreateIC((LPSTR)"DISPLAY", (LPSTR)NULL, (LPSTR)NULL, (LPSTR)NULL);
    if (!hIC)
        return(FALSE);

/* check to see if screen is a color device */
    if((GetDeviceCaps(hIC,BITSPIXEL)+GetDeviceCaps(hIC,PLANES)) < 3)
        {
        /* it isn't */
        /* background is gray, line is black */
        hbrBack = hbrGray;
        hbrLine = hbrBlack;
        }
    else
        {
        /* it is */
        /* background should be BLUE, and the line should be RED */
        if (!(hbrBack = (HBRUSH) CreateSolidBrush((unsigned long int) RGBBLUE)))
            hbrBack = hbrGray;
        if (!(hbrLine = (HBRUSH) CreateSolidBrush((unsigned long int) RGBRED)))
            hbrLine = hbrBlack;
        }

    /* Setup the font, and get metrics */
    GetTextMetrics(hIC, (LPTEXTMETRIC)(&Metrics));
    DeleteDC(hIC);

    CharFixHeight = Metrics.tmHeight+Metrics.tmExternalLeading;               /* the height */
    ExtLeading = Metrics.tmExternalLeading;
    CharFixWidth = Metrics.tmAveCharWidth;          /* the average width */
    ySpacing = CharFixHeight+1;

    CardWidth = (LINELENGTH * CharFixWidth) + 3;
    CardHeight = (CARDLINES*CharFixHeight) + CharFixHeight + 1 + 2 + 2;

    /* get the resource file info, such as icons, and accelerator tables */
    hArrowCurs  = LoadCursor(NULL, IDC_ARROW);
    hWaitCurs   = LoadCursor(NULL, IDC_WAIT);
    hIcon    = LoadIcon(hCardfileInstance,(LPSTR)CFICON);
    hAccel = LoadAccelerators(hCardfileInstance, (LPSTR)CFACCEL);
    if (!hArrowCurs || !hWaitCurs || !hIcon || !hAccel)
        return(FALSE);

    /* register the cardfile class */
    pClass = (PWNDCLASS) LocalAlloc(LPTR, sizeof(WNDCLASS));
    if (!pClass)
        return(FALSE);
    pClass->lpszClassName = (LPSTR)rgchCardfileClass;
    pClass->hCursor       = hArrowCurs;    /* normal cursor is arrow */
    pClass->hIcon         = hIcon;
    pClass->hbrBackground = NULL;
    pClass->style         = CS_VREDRAW | CS_DBLCLKS;
    pClass->lpfnWndProc   = CardfileWndProc;
    pClass->hInstance     = hCardfileInstance;
    pClass->lpszMenuName  = (LPSTR)CFMENU;
    if (!RegisterClass((LPWNDCLASS) pClass))
        return FALSE;
    LocalFree((HANDLE)pClass);

    return TRUE;
    }
