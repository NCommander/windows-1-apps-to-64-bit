/* declarations file for Cardfile */

extern HANDLE hCardfileInstance;


extern HWND   hCardfileWnd;
extern HANDLE hArrowCurs;
extern HANDLE hWaitCurs;
extern HANDLE hAccel;
extern char rgchCardfileClass[];

extern int  CardPhone;

extern int  CharFixWidth;
extern int  CharFixHeight;
extern int  ExtLeading;

extern int  CardWidth;
extern int  CardHeight;

extern int cScreenHeads;
extern int cFSHeads;
extern int cScreenCards;
extern int xFirstCard;
extern int yFirstCard;
extern int ySpacing;

extern int cxMainWindow;
extern int cyMainWindow;

extern int iFirstCard;
extern int iTopCard;

extern int  cCards;
extern HANDLE  hCards;
extern CARDHEADER CurCardHead;
extern CARD       CurCard;

extern char rgchWindows[];
extern char rgchDevice[];

extern unsigned wMerge;

extern int      fSettingText;
extern HANDLE   hEditCurs;
extern int      EditMode;
extern int      fNeedToUpdateBitmap;
extern HWND     hCardWnd;

extern RECT dragRect;
extern POINT   dragPt;
extern int     fBMDown;
extern int     xmax;
extern int     ymax;

extern HBRUSH hbrBack;
extern HBRUSH hbrLine;
extern HBRUSH hbrWhite;
extern HBRUSH hbrBlack;
extern HBRUSH hbrGray;

extern int fFileDirty;

/* dialog procs */
extern FARPROC lpDlgProc;
extern FARPROC lpfnOpen;
extern FARPROC lpfnSave;
extern FARPROC lpfnAbout;
extern FARPROC lpEditWndProc;
extern FARPROC lpfnAbortProc;
extern FARPROC lpfnAbortDlgProc;
extern FARPROC lpfnDial;

extern char rgchCardfile[];
extern char CurIFile[];
extern char CurIFind[];
extern char rgchUntitled[];
extern char NotEnoughMem[];
extern char rgchCardData[];
extern char TmpFile[];
extern char szExtSave[];

extern CATCHBUF CatchBuf;

extern int fCardCleared;
extern int iCardStartScroll;
extern int fScrolling;

extern OFSTRUCT tmpreopen;
extern OFSTRUCT mainreopen;
extern int fReadOnly;

extern int fCanPrint;
extern HWND hAbortDlgWnd;
extern int fAbort;
extern int fError;
extern HANDLE hSysMenu;
extern int CharPrintWidth;
extern int CharPrintHeight;
extern int ExtPrintLeading;
extern int CardPrintWidth;
extern int CardPrintHeight;

extern int DBcmd;

extern int fWinIniModem;
extern int fModemOnCom1;
extern int fTone;
extern int fFastModem;

/* in cfres.c */
int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);

/* in cfnew.c */
void FAR     GetOldData(HANDLE);
BOOL FAR     InitInstance(HANDLE, LPSTR, int);
BOOL FAR     CardfileInit(void);

/* in cfmain.c */
long far PASCAL CardfileWndProc(HWND, unsigned, WORD, DWORD);

/* in cfpaint.c */
void CardfileEraseBkGnd(HWND, HDC);
void CardfilePaint(HWND, HDC, LPRECT);
void PhonePaint(HWND, HDC, LPRECT);
void PaintNewHeaders(void);
void CardfileSize(HWND, int, int);

/* in cfinput.c */
void CardfileInput(HWND, int);
void SetTopCard(void);
PSTR PutUpDB(int);
void UpdateMenu(void);
void SetEditText(LPSTR);
void CardfileMouse(HWND, int, WORD, POINT);
BOOL FAR GetNewCard(int, int);
int MapPtToCard(POINT);
void FAR CardfileOkError(int);
WORD FAR MyMessageBox(WORD, PSTR, WORD);
BOOL  FAR MergeStrings(LPSTR, LPSTR, LPSTR);
void MakeBlankCard(void);
void SetCaption(void);
void FAR BuildCaption(PSTR);
void FAR CardfileWinIniChange();

/* in cfopen.c */
void DoMerge(void);
BOOL FAR DoOpen(PSTR);
BOOL MaybeSaveFile(int);
BOOL GetNewFileName(POFSTRUCT);

/* in cfscroll.c */
BOOL CardfileScroll(HWND, int, int);
void PhoneScroll(HWND, int, int);
void SetScrRangeAndPos(void);

/* in cfcard.c */
long far PASCAL CardWndProc(HWND, unsigned, WORD, DWORD);
void FAR CardPaint(HDC);
void DeleteCard(int);
int FAR AddCurCard();
KanjiXlat(unsigned char, short *);
BOOL SaveCurrentCard(int);
void SetCurCard(int);

/* in cftext.c */
BOOL CardKey(WORD);
BOOL CardChar(int);
void BringCardOnScreen(int);
BOOL PhoneKey(HWND, WORD);

/* in cffile.c */
BOOL FAR MergeCardFile(PSTR);
BOOL FAR ReadCardFile(PSTR);
void FAR AppendExtension(PSTR, PSTR);
BOOL FAR WriteCardFile(PSTR);
BOOL FAR WriteCurCard(PCARDHEADER, PCARD, LPSTR);
BOOL FAR ReadCurCardData(PCARDHEADER, PCARD, LPSTR);
void FAR MakeTmpFile(HANDLE);

/* in cfbitmap.c */
void FAR BMMouse(HWND, unsigned, WORD, POINT);
void FAR TurnOnBitmapRect(void);
void FAR TurnOffBitmapRect(void);
void XorBitmapRect(void);
void FAR DrawXorRect(HDC, PRECT);
BOOL FAR BMKey(WORD);

/* in cfclip.c */
void FAR DoCutCopy(int);
void FAR DoPaste();
HBITMAP MakeBitmapCopy( HBITMAP, PBITMAP);

/* in cfprint.c */
HDC SetupPrinting(void);
void FinishPrinting(HDC);
void FAR PrintList(void);
void FAR PrintCards(int);
void PrintCurCard(HDC, HDC, int, PCARDHEADER, PCARD, HWND);
int FAR PASCAL fnAbortProc(HDC, int);
int FAR PASCAL fnAbortDlgProc(HWND, unsigned, WORD, DWORD);
void PrintError(int);

/* in cffind.c */
void FAR DoGoto(PSTR);
void FAR FindStrCard(void);
LPSTR NEAR AnsiFind(LPSTR, PSTR, short);

/* in cfdb.c */
int far PASCAL fnOpen(HWND, unsigned, WORD, DWORD);
void DlgAddCorrectExtension(PSTR, BOOL);
BOOL    FSearchSpec(PSTR);
int far PASCAL fnSave(HWND, unsigned, WORD, DWORD);
PSTR PFileInPath(PSTR);
void CheckOkEnable(HWND, unsigned);
int far PASCAL fnAbout(HWND, unsigned, WORD, DWORD);
int IntegerToAscii(unsigned, PSTR);
BOOL far PASCAL DlgProc(HWND, unsigned, WORD, DWORD);
BOOL far PASCAL fnDial(HWND, unsigned, WORD, DWORD);

/* in cfdial.c */
void FAR GetPhoneNumber(PSTR, int);
BOOL ParseNumber(LPSTR, PSTR, int);
void FAR DoDial(PSTR);
void SetPortState(int);
int MakeDialCmd(PSTR, int, PSTR);

/* in asmsubs.asm */
RepMov(LPCARDHEADER, LPCARDHEADER, int);
RepMovUp(LPCARDHEADER, LPCARDHEADER, int);

/* in cfdos.asm */
Frename(PSTR, PSTR);
int FAR Fdelete(PSTR);
mylread(int, LPSTR, int);
myread(int, PSTR, int);
mylwrite(int, LPSTR, int);
mywrite(int, PSTR, int);
int FAR MyOpen(LPSTR, int);
void FAR MyClose(int);
long FAR MyLseek(int, long, int);
int FAR Mylstrlen(LPSTR);
LPSTR FAR Mylstrcpy(LPSTR, LPSTR);
LPSTR FAR Mylstrcat(LPSTR, LPSTR);
int FAR Mylstrcmp(LPSTR, LPSTR);
