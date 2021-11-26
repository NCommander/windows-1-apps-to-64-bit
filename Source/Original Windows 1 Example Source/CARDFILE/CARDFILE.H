/* turn off some stuff Cardfile never uses */
#define NOSOUND
#define NOCREATESTRUCT
#define NOMETAFILE
#define NOPEN
#define NORGN
#define NOWH
#define NOKANJI
#include "windows.h"

/* these are the ID's for the commands in the menu */

/* File menu */
#define     IDM_NEW         101
#define     IDM_OPEN        102
#define     IDM_SAVE        103
#define     IDM_SAVEAS      104
#define     IDM_PRINT       105
#define     IDM_PRINTALL    106
#define     IDM_MERGE       107
/* Edit menu */
#define     IDM_UNDO        121
#define     IDM_HEADER      122
#define     IDM_CUT         123
#define     IDM_COPY        124
#define     IDM_PASTE       125
#define     IDM_RESTORE     126

#define     IDM_TEXT        127
#define     IDM_BITMAP      128
/* View menu */
#define     IDM_CARDFILE    141
#define     IDM_PHONEBOOK   142
/* Card menu */
#define     IDM_ADD         161
#define     IDM_DELETE      162
#define     IDM_DUPLICATE   163
#define     IDM_DIAL        164
/* Search menu */
#define     IDM_GOTO        181
#define     IDM_FIND        182
#define     IDM_FINDNEXT    183

#define     IDM_ABOUT       8887

#define     IDM_EDITWINDOW  200     /* this isn't really a command, but */
                                    /* the id is used in a WM_COMMAND   */
                                    /* message */

/* id's for controls in dialog boxes */
#define IDD_PATH     10
#define IDD_EDIT     11
#define IDD_LISTBOX  12
#define IDD_TONE     13
#define IDD_PULSE    14
#define IDD_COM1     15
#define IDD_COM2     16
#define IDD_1200     17
#define IDD_300      18

/* id's for strings */
/* prompts */
#define IDS_DELCURCARD     0
#define IDS_CREATEFILE     1
#define IDS_OKTOSAVE       2
#define IDS_ABOUT          3
#define IDS_CARDS          4
#define IDS_CARD           5
#define IDS_UNTITLED       6
#define IDS_CARDDATA       7
#define IDS_PICKUPPHONE    8
#define IDS_CARDFILE       9
#define IDS_STRINGINSERT   10
/* errors */
#define IDS_ECANTDIAL       16
#define IDS_ECANTPRINTPICT  17
#define IDS_EINSMEMORY      18
#define IDS_EFILEEXISTS     19
#define IDS_EINVALIDFILE    20
#define IDS_ECLIPEMPTYTEXT  21
#define IDS_ENOTEXTSEL      22
#define IDS_EDISKFULLFILE   23
#define IDS_ECANTMAKETEMP   24
#define IDS_EINSMEMSAVE     25
#define IDS_EDISKFULLSAVE   26
#define IDS_EOPENTEMPSAVE   27
#define IDS_ECANTREADPICT   28
#define IDS_EINSMEMRUN      29
#define IDS_ENOTVALIDFILE   30
#define IDS_ECANTFIND       31
#define IDS_EINSMEMREAD     32
#define IDS_ECANTPRINT      33
#define IDS_ECLIPEMPTYPICT  34
#define IDS_ENOMODEM        35
#define IDS_ENOPICTURES     36
#define IDS_ECANTMAKEFILE   37
#define IDS_EMEMPRINT       38
#define IDS_EDISKPRINT      39


/* dialog boxes */
#define DTOPEN      1
#define DTSAVE      2
#define DTHEADER    3
#define DTADD       5
#define DTMERGE     6
#define DTGOTO      7
#define DTFIND      8
#define DTABOUT     9
#define DTABORTDLG  10
#define DTDIAL      11

/* resource names */
#define CFMENU    1
#define CFICON    1
#define CFACCEL   1

/* Flags for MyOpen */
#define READ        0
#define WRITE       1
#define READ_WRITE  2

/* card definitions */
#define LINELENGTH 40               /* 40 characters per line */
#define CARDLINES 11                /* 11 lines per card */
#define CARDTEXTSIZE ((LINELENGTH+1) * CARDLINES)   /* characters per card */

/* brush colors on a color screen */
#define RGBRED     0x0000ff
#define RGBBLUE    0xff0000

/* flags for a card header */
#define FNEW     0x01               /* new card, not in file yet */
#define FDIRTY   0x02               /* card has changed from file */
#define FTMPFILE 0x04               /* it's data is in the tmpfile */

/* margins on card */
#define LEFTMARGIN 8
#define RIGHTMARGIN 8
#define BOTTOMMARGIN 8

#define PATHMAX 128                 /* maximum # of character in filename */

#define ATTRDIRLIST 0x4010          /* used by open dialog box */

/* Cardfile's two main data structures */
/* the first is the header.  Each card in a file has a header associated */
/* with it.  All headers are resident in memory while a file is in use */
/* the header has information about the header line on the card, and */
/* where the rest of the card's information can be found */
typedef struct
    {
    char            reserved[6];          /* bytes available for future use */
    unsigned long   lfData;               /* file offset of data */
    unsigned char   flags;                /* flags */
    char            line[LINELENGTH+1];   /* 40 character lines plus null */
    }CARDHEADER;
typedef CARDHEADER *PCARDHEADER;
typedef CARDHEADER far *LPCARDHEADER;

/* the second is for the card currently viewed.  It contains information */
/* about the card's bitmap (if any), and the card's main body of text */
typedef struct
    {
    int        cxBitmap;                /* width of the bitmap */
    int        cyBitmap;                /* height of the bitmap */
    int        xBitmap;                 /* left location of bitmap */
    int        yBitmap;                 /* top location of bitmap */
    unsigned    bmSize;                 /* count of bytes in the bitmap */
    HANDLE  hBitmap;                    /* handle to bitmap, NULL if none */
    HANDLE  hText;                      /* handle to the text block */
    }CARD;
typedef CARD *PCARD;

#include "declare.h"
