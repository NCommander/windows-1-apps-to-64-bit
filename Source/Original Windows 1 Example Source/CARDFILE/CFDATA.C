#include "cardfile.h"

HANDLE hCardfileInstance;  /* instance handle */

/* variables for cardfile's main window */
HWND   hCardfileWnd;       /* cardfile's main window */
HANDLE hArrowCurs;      /* handle to the arrow cursor */
HANDLE hWaitCurs;       /* handle to the hourglass cursor */
HANDLE hAccel;          /* handle of accelerator table */
char rgchCardfileClass[] = "Cardfile"; /* Class name - not translatable. */

int CardPhone = IDM_CARDFILE;   /* mode: either IDM_CARDFILE or IDM_PHONEBOOK */

/* font metrics */
int     CharFixWidth;   /* width of the system font */
int     CharFixHeight;  /* height of the system font */
int ExtLeading;         /* external leading of the system font */

/* variables relating to display ************************************/
/*               |                 |  |  |                   -      */
/*            +-----------------+  |  |--+                   |      */
/*            |                 |  |  |                      |      */
/*         +-----------------+  |  |--+                      |      */
/*         |                 |  |  |             yFirstCard--+      */
/*       +----------------+  |  |--+   -                     |      */
/*       |                |  |  |      +-ySpacing            |      */
/*     +---------------+  |  |--+      -            -        -      */
/*     |               |  |  |                      |               */
/*     |---------------|  |--+                      |               */
/*     |               |  |                     CardHeight          */
/*     |               |--+                         |               */
/*     |               |                            |               */
/*     +---------------+                            -               */
/*                                                                  */
/*     |---CardWidth---|                                            */
/*                                                                  */
/*|-+--|                                                            */
/*  |                                                               */
/*  +--xFirstCard                                                   */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/*  In this case:                                                   */
/*      cScreenHeads = 5   (there are 5 partially visible)          */
/*      cFSHeads = 4       (only 4 are fully visible)               */
/*      cScreenCards = 7                                            */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/********************************************************************/

int     CardWidth;      /* the screen width of a card */
int     CardHeight;     /* the screen height of a card */
int cScreenHeads;       /* the number of headers that are partially visible (>=1) */
int cFSHeads;           /* the number of fully visible headers */
int cScreenCards;       /* the number of cards that are partially visible (>=1) */
int xFirstCard;         /* the x */
int yFirstCard;         /* and y coordinates of the front card */
int ySpacing;           /* the y offset of each card (CharFixHeight + 1) */

int cxMainWindow;       /* current width of main window */
int cyMainWindow;       /* current height of main window */

int iFirstCard = 0;     /* front card on screen */
int iTopCard   = 0;     /* in phonemode, the first visible line */

/* variables relating to list of cards */
int     cCards;                 /* the current number of cards */
HANDLE  hCards;                 /* the handle of the header buffer */
CARDHEADER CurCardHead;         /* the header struct for the front card */
CARD       CurCard;             /* the data for the front card */

/* Headings for sections of win.ini */
char rgchWindows[] = "Windows";
char rgchDevice[] =  "Device";

unsigned wMerge;        /* value used by MergeStrings */

/* for card area.  Either the edit control is working, or the user */
/* is editing the bitmap */
int     fSettingText = FALSE;        /* true when setting text of edit control */
HANDLE  hEditCurs;                   /* place to save cursor for edit control */
int     EditMode = IDM_TEXT;         /* either IDM_TEXT or IDM_BITMAP */
int     fNeedToUpdateBitmap = FALSE; /* true when bitmap on card is dirty */
HWND    hCardWnd;                    /* the edit control window handle */

/* bitmap movement variables */
RECT    dragRect;           /* current rectangle of bitmap */
POINT   dragPt;             /* last mouse location */
BOOL    fBMDown;            /* TRUE if the mouse button is down on bitmap */
int     xmax;               /* maximum x value for bitmap movement */
int     ymax;               /* maximum y value for bitmap movement */

/* brush handles */
HBRUSH hbrBack;         /* handle of brush with background color */
                        /* this will be BLUE on a color system, gray on B/W */
HBRUSH hbrLine;         /* handle of brush with color of line on card */
                        /* this will be RED on a color system, black on B/W */
HBRUSH hbrWhite;        /* white brush */
HBRUSH hbrBlack;        /* black brush */
HBRUSH hbrGray;         /* gray brush */


int fFileDirty = FALSE; /* TRUE is the user has made changes */

/* dialog procs */
FARPROC lpDlgProc;
FARPROC lpfnOpen;
FARPROC lpfnSave;
FARPROC lpfnAbout;
FARPROC lpEditWndProc;
FARPROC lpfnAbortProc;
FARPROC lpfnAbortDlgProc;
FARPROC lpfnDial;

/* strings of interest */
char rgchCardfile[40];                 /* title bar name - translatable */
char CurIFile[PATHMAX];                /* pathname of current file */
char CurIFind[40];                     /* current search pattern */
char rgchUntitled[30];                 /* "Untitled" string - translatable */
char NotEnoughMem[50];                 /* "Not enough Memory" */
char rgchCardData[40];                 /* "Cardfile data" for temp file */
char TmpFile[PATHMAX];                 /* the tempfilename */
char szExtSave[] = "\\*.CRD";          /* used by open and save */

/* relating to scrolling */
int fCardCleared = FALSE;   /* TRUE if the card has been erased */
int iCardStartScroll;       /* the front card before start of scroll */
int fScrolling = FALSE;     /* true if scrolling */

OFSTRUCT tmpreopen;         /* saves pathname of Cardfile */
OFSTRUCT mainreopen;        /* saves pathname of Tempfile */
int fReadOnly = FALSE;      /* TRUE in weird cases */


/* printing variables */
int fCanPrint = FALSE;      /* FALSE if no printer */
HWND hAbortDlgWnd;          /* handle for CANCEL printing dialog */
int fAbort;                 /* true if user hits CANCEL */
int fError;                 /* true if somekind of error while printing */
HANDLE hSysMenu;            /* menu handle for disabling CLOSE */
/* font metrics for printer */
int CharPrintWidth;         /* printer font width */
int CharPrintHeight;        /* printer font height */
int ExtPrintLeading;        /* external leading for printer font */
/* display variables for printing */
int CardPrintWidth;         /* cardwidth on printed page */
int CardPrintHeight;        /* cardheight on printed page */

int DBcmd;                  /* dialog function needs to know which command */

/* autodialing */
int fWinIniModem = FALSE;   /* TRUE if knows win.ini info about MODEM */
int fModemOnCom1 = TRUE;    /* if FALSE, modem is on COM2 */
int fTone = TRUE;           /* if FALSE, pulse dialing */
int fFastModem = TRUE;      /* if FALSE, 300 baud modem */
