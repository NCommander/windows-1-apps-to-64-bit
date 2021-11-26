#include "cardfile.h"

/*********************************************************************/
/*                                                                   */
/*  Windows Cardfile                                                 */
/*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
/*                                                                   */
/*********************************************************************/

/* the open dialog box, look in SAMPLE/DLGOPEN.C for better documentation */

int far PASCAL fnOpen(hwnd, msg, wParam, lParam)
HWND hwnd;
unsigned msg;
WORD wParam;
DWORD lParam;
    {
    int item;
    int cchFile, cchDir;
    char *pchFile;
    BOOL    fWild;
    char *pResultBuf = NULL;
    char szNewName[PATHMAX];
    int len;

    switch (msg)
        {
        case WM_INITDIALOG:
            SetDlgItemText(hwnd, IDD_EDIT, (LPSTR)"*.CRD");
            SendDlgItemMessage(hwnd, IDD_EDIT, EM_LIMITTEXT, 128, 0L);
            DlgDirList(hwnd, (LPSTR)"*.CRD", IDD_LISTBOX, IDD_PATH, ATTRDIRLIST);
            break;

        case WM_COMMAND:
            switch (wParam)
                {
                case IDOK:
LoadIt:
                    if (IsWindowEnabled(GetDlgItem(hwnd,IDOK)))
                        {
                        len = 7+GetWindowTextLength(GetDlgItem(hwnd, IDD_EDIT));
                        if(pResultBuf = (char *)LocalAlloc(LPTR, len))
                            {
                            GetDlgItemText(hwnd, IDD_EDIT, (LPSTR)pResultBuf, len);
                            Mylstrcpy((LPSTR)szNewName, (LPSTR)pResultBuf);

                            /* Append appropriate extension to user's entry */
                            DlgAddCorrectExtension(szNewName, TRUE);

                            /* Try to open directory.  If successful, fill listbox with
                               contents of new directory.  Otherwise, open datafile. */
                            if (FSearchSpec(szNewName))
                                {
                                if (DlgDirList(hwnd, (LPSTR)szNewName, IDD_LISTBOX, IDD_PATH, ATTRDIRLIST))
                                    {
                                    SetDlgItemText(hwnd, IDD_EDIT, (LPSTR)szNewName);
                                    break;
                                    }
                                }

                            DlgAddCorrectExtension(pResultBuf, FALSE);
                            /* If no directory list and filename contained search spec,
                               honk and don't try to open. */
                            if (FSearchSpec(pResultBuf))
                                {
                                MessageBeep(0);
                                break;
                                }
                            AnsiUpper((LPSTR)pResultBuf);
                            }
                        }
                    EndDialog(hwnd, (WORD)pResultBuf);
                    break;
                case IDCANCEL:
                    EndDialog(hwnd, NULL);
                    break;

                case IDD_LISTBOX:
                    switch (HIWORD(lParam))
                        {
                        case 1:
                            len = GetWindowTextLength(GetDlgItem(hwnd, IDD_EDIT));
                            if(pResultBuf = (char *)LocalAlloc(LPTR, ++len))
                                {
                                GetDlgItemText(hwnd, IDD_EDIT, (LPSTR)pResultBuf, len);
                                if (DlgDirSelect(hwnd, (LPSTR)szNewName, IDD_LISTBOX))
                                    {
                                    cchDir = Mylstrlen((LPSTR)szNewName);
                                    cchFile = Mylstrlen((LPSTR)pResultBuf);
                                    pchFile = pResultBuf+cchFile;
                                    fWild = (*pchFile == '*' || *pchFile == ':');
                                    while (pchFile > pResultBuf)
                                        {
                                        pchFile = (char *)AnsiPrev((LPSTR)(pResultBuf), (LPSTR)pchFile);
                                        if (*pchFile == '*' || *pchFile == '?')
                                            fWild = TRUE;
                                        if (*pchFile == '\\' || *pchFile == ':')
                                            {
                                            pchFile = (char *)AnsiNext((LPSTR)pchFile);
                                            break;
                                            }
                                        }
                                    if (fWild)
                                        Mylstrcpy((LPSTR)szNewName + cchDir, (LPSTR)pchFile);
                                    else
                                        Mylstrcpy((LPSTR)szNewName + cchDir, (LPSTR)"*.CRD");
                                    }
                                SetDlgItemText(hwnd, IDD_EDIT, (LPSTR)szNewName);
                                LocalFree((HANDLE)pResultBuf);
                                }
                            break;
                        case 2:
                            if (DlgDirList(hwnd, (LPSTR)szNewName, IDD_LISTBOX, IDD_PATH, 0x4010))
                                {
                                SetDlgItemText(hwnd, IDD_EDIT, (LPSTR)szNewName);
                                break;
                                }
                            goto LoadIt;    /* go load it up */
                        }
                    break;
                case IDD_EDIT:
                    CheckOkEnable(hwnd, HIWORD(lParam));
                    break;
                default:
                    return(FALSE);
                }
            break;
        default:
            return FALSE;
        }
    return(TRUE);
    }

/* ** Given filename or partial filename or search spec or partial
      search spec, add appropriate extension. */
void DlgAddCorrectExtension(szEdit, fSearching)
PSTR    szEdit;
BOOL    fSearching;
{
    register char    *pchLast;
    register char    *pchT;
    int ichExt;
    BOOL    fDone = FALSE;
    int     cchEdit;

    pchT = pchLast = (char *)AnsiPrev((LPSTR)szEdit, (LPSTR)(szEdit + (cchEdit = Mylstrlen((LPSTR)szEdit))));

    if ((*pchLast == '.' && *(AnsiPrev((LPSTR)szEdit, (LPSTR)pchLast)) == '.') && cchEdit == 2)
        ichExt = 0;
    else if (*pchLast == '\\' || *pchLast == ':')
        ichExt = 1;
    else {
        ichExt = fSearching ? 0 : 2;
        for (; pchT > szEdit; pchT = (char *)AnsiPrev((LPSTR)szEdit, (LPSTR)pchT)) {
            /* If we're not searching and we encounter a period, don't add
               any extension.  If we are searching, period is assumed to be
               part of directory name, so go ahead and add extension. However,
               if we are searching and find a search spec, do not add any
               extension. */
            if (fSearching) {
                if (*pchT == '*' || *pchT == '?')
                    return;
            } else if (*pchT == '.'){
                return;
            }
            /* Quit when we get to beginning of last node. */
            if (*pchT == '\\')
                break;
        }
        /* Special case hack fix since AnsiPrev can not return value less than
           szEdit. If first char is wild card, return without appending. */
        if (fSearching && (*pchT == '*' || *pchT == '?'))
            return;
    }
    Mylstrcpy((LPSTR)(pchLast+1), (LPSTR)(szExtSave+ichExt));
}

/* ** return TRUE iff 0 terminated string contains a '*' or '\' */
BOOL    FSearchSpec(sz)
register PSTR sz;
{
    for (; *sz;sz++) {
        if (*sz == '*' || *sz == '?')
            return TRUE;
    }
    return FALSE;
}

/* ** Dialog function for "Save as" .  User must specify new name of file. */
int far PASCAL fnSave(hwnd, msg, wParam, lParam)
HWND hwnd;
unsigned msg;
WORD wParam;
DWORD lParam;
    {
    char *pResultBuf;
    int len;
    char rgch[128];
    char    *pchFileName;
    char    *pchCmp;
    char    *pchTest;
    char    *PFileInPath();

    switch (msg)
        {
        case WM_INITDIALOG:
            /* Initialize Path field with current directory */
            DlgDirList(hwnd, (LPSTR)0, 0, IDD_PATH, 0);

            if (CurIFile[0])
                {
                /* rgch gets current directory string, terminated with "\\\0" */
                len = GetDlgItemText(hwnd, IDD_PATH, (LPSTR)rgch, 128);
                if (rgch[len-1] != '\\')
                    {
                    rgch[len] = '\\';
                    rgch[++len] = 0;
                    }

                /* Now see if path in reopen buffer matches current directory. */
                for (pchFileName = CurIFile,
                     pchTest = PFileInPath(CurIFile),
                     pchCmp = rgch;
                     pchFileName < pchTest;
                     pchFileName = (PSTR)AnsiNext(pchFileName), pchCmp = (PSTR)AnsiNext(pchCmp))
                    {
                    if (*pchFileName != *pchCmp)
                        break;
                    }
                /* If paths don't match, reset pchFileName to point to fully qualified
                   path. (Otherwise, pchFileName already points to filename. */
                if (*pchCmp || pchFileName < pchTest)
                    pchFileName = CurIFile;
                SetDlgItemText(hwnd, IDD_EDIT, (LPSTR)pchFileName);
                }
            else
                EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
            break;

        case WM_COMMAND:
            switch (wParam)
                {
                case IDOK:
                    if (IsWindowEnabled(GetDlgItem(hwnd, IDOK)))
                        {
                        len = 4+GetWindowTextLength(GetDlgItem(hwnd, IDD_EDIT));
                        if(pResultBuf = (char *)LocalAlloc(LPTR, ++len))
                            {
                            GetDlgItemText(hwnd, IDD_EDIT, (LPSTR)pResultBuf, len);
                            AppendExtension(pResultBuf, pResultBuf);
                            }
                        EndDialog(hwnd, (WORD)pResultBuf);
                        }
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, NULL);
                    break;
                case IDD_EDIT:
                    CheckOkEnable(hwnd, HIWORD(lParam));
                    break;
                default:
                    return(FALSE);
                }
            break;
        default:
            return FALSE;
        }
    return(TRUE);
    }

/* ** Given filename which may or maynot include path, return pointer to
      filename (not including path part.) */
PSTR PFileInPath(sz)
PSTR sz;
{
    PSTR pch;

    /* Strip path/drive specification from name if there is one */
    pch = (PSTR)AnsiPrev((LPSTR)sz, (LPSTR)(sz + Mylstrlen((LPSTR)sz)));
    while (pch > sz) {
        pch = (PSTR)AnsiPrev((LPSTR)sz, (LPSTR)pch);
        if (*pch == '\\' || *pch == ':') {
            pch = (PSTR)AnsiNext((LPSTR)pch);
            break;
        }
    }
    return(pch);
}



void CheckOkEnable(hwnd, message)
HWND    hwnd;
unsigned message;
    {
            if (message == EN_CHANGE)
                EnableWindow(GetDlgItem(hwnd, IDOK), (SendMessage(GetDlgItem(hwnd, IDD_EDIT), WM_GETTEXTLENGTH, 0, 0L)));
    }

/* the about dialog box: very simple, only shows the number of */
/* cards in the file */

int far PASCAL fnAbout(hwnd, msg, wParam, lParam)
HWND hwnd;
unsigned msg;
WORD wParam;
DWORD lParam;
    {
    char buf[40];
    int len;
    int id;

    if (msg == WM_INITDIALOG)
        {
        len = IntegerToAscii(cCards, buf);
        if (cCards == 1)
            id = IDS_CARD;
        else
            id = IDS_CARDS;
        LoadString(hCardfileInstance, id, (LPSTR)(buf+len), 40-len);
        SetDlgItemText(hwnd, IDD_EDIT, (LPSTR)buf);
        return(TRUE);
        }
    else if (msg == WM_COMMAND && wParam == IDOK)
        {
        EndDialog(hwnd, NULL);
        return(TRUE);
        }
    return(FALSE);
    }

/* convert an int to ascii */
int IntegerToAscii(n, psz)
unsigned n;
char *psz;
    {
    char *pch = psz;
    char ch;
    int len;

    do
        {
        *pch++ = n % 10 + '0';
        n /= 10;
        }
    while (n > 0);

    len = pch - psz;
    *pch-- = '\0';
    /* reverse the digits */

    while (psz < pch)
        {
        ch = *psz;
        *psz++ = *pch;
        *pch-- = ch;
        }
    return(len);
    }

/* generic dialog proc */
BOOL far PASCAL DlgProc(hDB, message, wParam, lParam)
HWND hDB;
unsigned message;
WORD wParam;
DWORD lParam;
    {
    char *pResultBuf;
    char *pchInit;
    int len;
    char PhoneNumber[30];

    switch (message)
        {
        case WM_INITDIALOG:
            /* initialize edit item based on which dialog it is */
            switch(DBcmd)
                {
                case DTHEADER:
                    pchInit = CurCardHead.line;
                    break;
                case DTFIND:
                    pchInit = CurIFind;
                    break;
                case DTDIAL:
                    pchInit = PhoneNumber;
                    GetPhoneNumber(PhoneNumber,30);
                    break;
                default:
                    pchInit = "";
                }
            SetDlgItemText(hDB, IDD_EDIT, (LPSTR)pchInit);
            SetFocus(GetDlgItem(hDB, IDD_EDIT));
            return(TRUE);
            break;

        case WM_COMMAND:
            /* all these get a single string */
            pResultBuf = NULL;
            switch (wParam)
                {
                case IDOK:
                    /* allocate buffer, read text and pass it back */
                    if ((len = GetWindowTextLength(GetDlgItem(hDB, IDD_EDIT))) || DBcmd == DTHEADER || DBcmd == DTADD)
                        if(pResultBuf = (char *)LocalAlloc(LPTR, ++len))
                            GetDlgItemText(hDB, IDD_EDIT, (LPSTR)pResultBuf, len);
                    break;
                case IDCANCEL:
                    break;
                default:
                    return(FALSE);
                }
            EndDialog(hDB, (int)pResultBuf);     /* return pointer to buffer */
            return(TRUE);
            break;
        default:
            return(FALSE);
        }
    }

/* handles the dial dialog */
BOOL far PASCAL fnDial(hDB, message, wParam, lParam)
HWND hDB;
unsigned message;
WORD wParam;
DWORD lParam;
    {
    char *pResultBuf;
    char *pch;
    int len;
    char PhoneNumber[40];
    char ComPortandDialType[15];
    int tmp;

    switch (message)
        {
        case WM_INITDIALOG:
            GetPhoneNumber(PhoneNumber,40);
            SetDlgItemText(hDB, IDD_EDIT, (LPSTR)PhoneNumber);
            SendMessage(GetDlgItem(hDB, IDD_EDIT), EM_SETSEL, 0, MAKELONG(0, Mylstrlen((LPSTR)PhoneNumber)));
            if (!fWinIniModem)
                {
                if (GetProfileString((LPSTR)rgchWindows, (LPSTR)"Modem", (LPSTR)"", (LPSTR)ComPortandDialType, 40))
                    {
                    for (pch = ComPortandDialType; *pch && *pch != ',' && *pch != ' '; ++pch)
                        ;
                    if (*pch)
                        *pch++ = 0;
                    AnsiUpper((LPSTR)ComPortandDialType);
                    if (Mylstrcmp(ComPortandDialType, "COM1"))
                        fModemOnCom1 = FALSE;
                    while (*pch == ',' || *pch == ' ')
                        pch++;
                    if (*pch == 'P' || *pch == 'p')
                        fTone = FALSE;
                    while (*pch && *pch != ',')
                        pch++;
                    while (*pch == ',' || *pch == ' ')
                        pch++;
                    if (*pch == 'F' || *pch == 'F')
                        fFastModem = TRUE;
                    fWinIniModem = TRUE;
                    }
                }
            CheckRadioButton(hDB, IDD_TONE, IDD_PULSE, fTone ? IDD_TONE : IDD_PULSE);
            CheckRadioButton(hDB, IDD_COM1, IDD_COM2, fModemOnCom1 ? IDD_COM1 : IDD_COM2);
            CheckRadioButton(hDB, IDD_1200, IDD_300, fFastModem ? IDD_1200 : IDD_300);
            SetFocus(GetDlgItem(hDB, IDD_EDIT));
            return(TRUE);

        case WM_COMMAND:
            pResultBuf = NULL;
            switch (wParam)
                {
                case IDOK:
                    tmp = IsDlgButtonChecked(hDB, IDD_TONE);
                    if (tmp != fTone)
                        fWinIniModem = FALSE;
                    fTone = tmp;
                    tmp = IsDlgButtonChecked(hDB, IDD_COM1);
                    if (tmp != fModemOnCom1)
                        fWinIniModem = FALSE;
                    fModemOnCom1 = tmp;
                    tmp = IsDlgButtonChecked(hDB, IDD_1200);
                    if (tmp != fFastModem)
                        fWinIniModem = FALSE;
                    fFastModem = tmp;

                    if (!fWinIniModem)
                        {
                        Mylstrcpy((LPSTR)ComPortandDialType, (LPSTR)"COM2,P,S");
                        if (fModemOnCom1)
                            ComPortandDialType[3] = '1';
                        if (fTone)
                            ComPortandDialType[5] = 'T';
                        if (fFastModem)
                            ComPortandDialType[7] = 'F';
                        /* write stuff out in win.ini */
                        WriteProfileString((LPSTR)rgchWindows, (LPSTR)"Modem", (LPSTR)ComPortandDialType);
                        fWinIniModem = TRUE;
                        }

                    if ((len = GetWindowTextLength(GetDlgItem(hDB, IDD_EDIT))) || DBcmd == DTHEADER || DBcmd == DTADD)
                        if(pResultBuf = (char *)LocalAlloc(LPTR, ++len))
                            GetDlgItemText(hDB, IDD_EDIT, (LPSTR)pResultBuf, len);
                case IDCANCEL:
                    EndDialog(hDB, (int)pResultBuf);
                    break;
                case IDD_TONE:
                case IDD_PULSE:
                    CheckRadioButton(hDB, IDD_TONE, IDD_PULSE, wParam);
                    break;
                case IDD_COM1:
                case IDD_COM2:
                    CheckRadioButton(hDB, IDD_COM1, IDD_COM2, wParam);
                    break;
                case IDD_300:
                case IDD_1200:
                    CheckRadioButton(hDB, IDD_1200, IDD_300, wParam);
                    break;
                default:
                    return(FALSE);
                }
            return(TRUE);
            break;
        default:
            return(FALSE);
        }
    }
