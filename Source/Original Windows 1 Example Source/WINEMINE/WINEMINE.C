/*
 * WineMine (main.c)
 *
 * Copyright 2000 Joshua Thielen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */


#include "winemine.h"
#include "winemres.h"
#include "Stdlib.h" 

/* Windows 1.x/2.x has no mechanisim for passing parameters to dialogs 
   so we must use globals instead */
   
BOARD *g_board; 


#define RAND_MAX 0x7fff

static const DWORD wnd_style =  WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU;

void CheckLevel(BOARD * p_board )
{
    if( p_board->rows < BEGINNER_ROWS )
        p_board->rows = BEGINNER_ROWS;

    if( p_board->rows > MAX_ROWS )
        p_board->rows = MAX_ROWS;

    if( p_board->cols < BEGINNER_COLS )
        p_board->cols = BEGINNER_COLS;

    if( p_board->cols > MAX_COLS )
        p_board->cols = MAX_COLS;

    if( p_board->mines < BEGINNER_MINES )
        p_board->mines = BEGINNER_MINES;

    if( p_board->mines > ( p_board->cols - 1 ) * ( p_board->rows - 1 ) )
        p_board->mines = ( p_board->cols - 1 ) * ( p_board->rows - 1 );
}

void LoadBoard( BOARD *p_board )
{
    char data[MAX_PLAYER_NAME_SIZE+1];
    char key_name[8];
    char ProfStr[40];
    char Default[40];
    unsigned i;

	GetProfileString("WineMine", "Xpos", "15", ProfStr, sizeof(ProfStr));
    p_board->pos.x = atoi(ProfStr);
            
   	GetProfileString("WineMine", "Ypos", "42", ProfStr, sizeof(ProfStr));
    p_board->pos.y = atoi(ProfStr);
   	
   	sprintf(Default, "%d", BEGINNER_ROWS);
    GetProfileString("WineMine", "Height", Default, ProfStr, sizeof(ProfStr));
    p_board->rows = atoi(ProfStr);

    sprintf(Default, "%d", BEGINNER_COLS);
    GetProfileString("WineMine", "Width", Default,ProfStr, sizeof(ProfStr));
    p_board->cols = atoi(ProfStr);
            
    sprintf(Default, "%d", BEGINNER_MINES);        
    GetProfileString("WineMine", "Mines", Default, ProfStr, sizeof(ProfStr));
    p_board->mines = atoi(ProfStr);
    
    sprintf(Default, "%d", BEGINNER);
    GetProfileString("WineMine", "Difficulty", Default, ProfStr, sizeof(ProfStr));
    p_board->difficulty = atoi(ProfStr);
    
    GetProfileString("WineMine", "Mark", "1", ProfStr, sizeof(ProfStr));
    p_board->IsMarkQ = atoi(ProfStr);

    LoadString( p_board->hInst, IDS_NOBODY, Default, MAX_PLAYER_NAME_SIZE+1 );
    for( i = 0; i < 3; i++ ) {
        sprintf( key_name, "Name%d", i+1 );
        GetProfileString("WineMine", key_name, Default, p_board->best_name[i], MAX_PLAYER_NAME_SIZE+1);
    }

    for( i = 0; i < 3; i++ ) {
        sprintf( key_name, "Time%d", i+1 ); 
        GetProfileString("WineMine", key_name, "999", ProfStr, sizeof(ProfStr));
        p_board->best_time[i] = atoi(ProfStr);
    }
}

void InitBoard( BOARD *p_board )
{
    HMENU hMenu;

    p_board->hMinesBMP = LoadBitmap( p_board->hInst, "mines");
    p_board->hFacesBMP = LoadBitmap( p_board->hInst, "faces");
    p_board->hLedsBMP = LoadBitmap( p_board->hInst, "leds");

    LoadBoard( p_board );

    hMenu = GetMenu( p_board->hWnd );
    CheckMenuItem( hMenu, IDM_BEGINNER + (unsigned) p_board->difficulty,
            MF_CHECKED );
    if( p_board->IsMarkQ )
        CheckMenuItem( hMenu, IDM_MARKQ, MF_CHECKED );
    else
        CheckMenuItem( hMenu, IDM_MARKQ, MF_UNCHECKED );
    CheckLevel( p_board );
}

void SaveBoard( BOARD *p_board )
{
    unsigned i;
    char data[MAX_PLAYER_NAME_SIZE+1];
    char key_name[8];
    char ProfStr[40];
    
    sprintf( ProfStr, "%d", p_board->pos.x);
    WriteProfileString("WineMine", "Xpos", ProfStr);
    
    sprintf( ProfStr, "%d", p_board->pos.y);
    WriteProfileString("WineMine", "Ypos", ProfStr);
    
    sprintf( ProfStr, "%d", p_board->difficulty);
    WriteProfileString("WineMine", "Difficulty", ProfStr);
    
    sprintf( ProfStr, "%d", p_board->rows);
    WriteProfileString("WineMine", "Height", ProfStr);
    
    sprintf( ProfStr, "%d", p_board->cols);
    WriteProfileString("WineMine", "Width", ProfStr);
  
    sprintf( ProfStr, "%d", p_board->mines);
    WriteProfileString("WineMine", "Mines", ProfStr);

    sprintf( ProfStr, "%d", p_board->IsMarkQ);
    WriteProfileString("WineMine", "Mark", ProfStr);

    for( i = 0; i < 3; i++ ) {
        sprintf( key_name, "Name%u", i+1 );
        strcpy( data, p_board->best_name[i], sizeof( data ) );
        WriteProfileString("WineMine", key_name, data);
    }

    for( i = 0; i < 3; i++ ) {
        sprintf( key_name, "Time%u", i+1 );
        sprintf( ProfStr, "%d", p_board->best_time[i]);
        WriteProfileString("WineMine", key_name, ProfStr);
    }
}

void DestroyBoard( BOARD *p_board )
{
    DeleteObject( p_board->hFacesBMP );
    DeleteObject( p_board->hLedsBMP );
    DeleteObject( p_board->hMinesBMP );
}

void SetDifficulty( BOARD *p_board, DIFFICULTY difficulty )
{
    HMENU hMenu;

    g_board = p_board;
    
    if ( difficulty == CUSTOM )

        if (DialogBox( p_board->hInst, "DLG_CUSTOM", p_board->hWnd,
                  p_board->lpCustomDlgProc) != 0)          
           return;

    hMenu = GetMenu( p_board->hWnd );
    CheckMenuItem( hMenu, IDM_BEGINNER + p_board->difficulty, MF_UNCHECKED );
    p_board->difficulty = difficulty;
    CheckMenuItem( hMenu, IDM_BEGINNER + difficulty, MF_CHECKED );

    switch( difficulty ) {
    case BEGINNER:
        p_board->cols = BEGINNER_COLS;
        p_board->rows = BEGINNER_ROWS;
        p_board->mines = BEGINNER_MINES;
        break;

    case ADVANCED:
        p_board->cols = ADVANCED_COLS;
        p_board->rows = ADVANCED_ROWS;
        p_board->mines = ADVANCED_MINES;
        break;

    case EXPERT:
        p_board->cols = EXPERT_COLS;
        p_board->rows = EXPERT_ROWS;

        p_board->mines = EXPERT_MINES;
        break;

    case CUSTOM:
        break;
    }
}

void ShiftBetween(LONG* x, LONG* y, LONG a, LONG b)
{
    if (*x < a) {
	*y += a - *x;
	*x = a;
    }
    else if (*y > b) {
	*x -= *y - b;
	*y = b;
    }
}

void MoveOnScreen(RECT* rect)
{
/* Windows 1 does not support multiple monitors. Who would have thought? */
     
/*    HMONITOR hMonitor;
    MONITORINFO mi; */

    /* find the nearest monitor ... */
/*    hMonitor = MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST); */

    /* ... and move it into the work area (ie excluding task bar)*/
/*    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

    ShiftBetween(&rect->left, &rect->right, mi.rcWork.left, mi.rcWork.right);
    ShiftBetween(&rect->top, &rect->bottom, mi.rcWork.top, mi.rcWork.bottom);*/
}

void CreateBoard( BOARD *p_board )
{
    int left, top, bottom, right;
    unsigned col, row;
    RECT wnd_rect;

    p_board->mb = MB_NONE;
    p_board->boxes_left = p_board->cols * p_board->rows - p_board->mines;
    p_board->num_flags = 0;

    /* Create the boxes...
     * We actually create them with an empty border,
     * so special care doesn't have to be taken on the edges
     */
    for( col = 0; col <= p_board->cols + 1; col++ )
      for( row = 0; row <= p_board->rows + 1; row++ ) {
        p_board->box[col][row].IsPressed = FALSE;
        p_board->box[col][row].IsMine = FALSE;
        p_board->box[col][row].FlagType = NORMAL;
        p_board->box[col][row].LastFlagType = NORMAL;
        p_board->box[col][row].NumMines = 0;
      }

    p_board->width = p_board->cols * MINE_WIDTH + BOARD_WMARGIN * 2;

    p_board->height = p_board->rows * MINE_HEIGHT + LED_HEIGHT
        + BOARD_HMARGIN * 3;

    /* setting the mines rectangle boundary */
    left = BOARD_WMARGIN;
    top = BOARD_HMARGIN * 2 + LED_HEIGHT;
    right = left + p_board->cols * MINE_WIDTH;
    bottom = top + p_board->rows * MINE_HEIGHT;
    SetRect( &p_board->mines_rect, left, top, right, bottom );

    /* setting the face rectangle boundary */
    left = p_board->width / 2 - FACE_WIDTH / 2;
    top = BOARD_HMARGIN;
    right = left + FACE_WIDTH;
    bottom = top + FACE_HEIGHT;
    SetRect( &p_board->face_rect, left, top, right, bottom );

    /* setting the timer rectangle boundary */
    left = BOARD_WMARGIN;
    top = BOARD_HMARGIN;
    right = left + LED_WIDTH * 3;
    bottom = top + LED_HEIGHT;
    SetRect( &p_board->timer_rect, left, top, right, bottom );

    /* setting the counter rectangle boundary */
    left =  p_board->width - BOARD_WMARGIN - LED_WIDTH * 3;
    top = BOARD_HMARGIN;
    right = p_board->width - BOARD_WMARGIN;
    bottom = top + LED_HEIGHT;
    SetRect( &p_board->counter_rect, left, top, right, bottom );

    p_board->status = WAITING;
    p_board->face_bmp = SMILE_BMP;
    p_board->time = 0;

    wnd_rect.left   = p_board->pos.x;
    wnd_rect.right  = p_board->pos.x + p_board->width ;
    wnd_rect.top    = p_board->pos.y;
    wnd_rect.bottom = p_board->pos.y + p_board->height;
    AdjustWindowRect(&wnd_rect, wnd_style, TRUE); 

   /* Under Windows 2.x and 3.x there is a crazy problem where AdjustWindowRect
      sometimes returns a window rect that is one pixel lower than the actual 
      window. Interestingly, it works perfectly under Windows 1! 
      If there is a root cause (such as a window style that shouldn't be used), 
      I can not find it. 
      So to work around this, we keep in mind that we are resizing, but not 
      actually moving the window here. We force the original top/left while 
      providing the new bottom/right */
      
    if (IsWindowVisible(p_board->hWnd)){  
        RECT winRC;
        GetWindowRect(p_board->hWnd,&winRC);
        wnd_rect.bottom = winRC.top + (wnd_rect.bottom - wnd_rect.top);
        wnd_rect.top = winRC.top;
        wnd_rect.left = winRC.left;
    }                               
   
    MoveWindow( p_board->hWnd, wnd_rect.left, wnd_rect.top,
		wnd_rect.right - wnd_rect.left,
		wnd_rect.bottom - wnd_rect.top,
		TRUE );  

    InvalidateRect(p_board->hWnd, (LPRECT)NULL, TRUE);
    UpdateWindow(p_board->hWnd);
}


/* Randomly places mines everywhere except the selected box. */
void PlaceMines ( BOARD *p_board, int selected_col, int selected_row )
{
    int i, j;
    unsigned col, row;

    srand (LOWORD (GetCurrentTime ()));

    /* Temporarily place a mine at the selected box until all the other
     * mines are placed, this avoids checking in the mine creation loop. */
    p_board->box[selected_col][selected_row].IsMine = TRUE;

    /* create mines */
    i = 0;
    while( (unsigned) i < p_board->mines ) {
        /* Integer math is needed here to make Windows 1.x happy */
        col = (rand () % (p_board->cols) + 1);
        row = (rand () % (p_board->rows) + 1); 
        if( !p_board->box[col][row].IsMine ) {
            i++;
            p_board->box[col][row].IsMine = TRUE;
        }
    }

    /* Remove temporarily placed mine for selected box */
    p_board->box[selected_col][selected_row].IsMine = FALSE;

    /*
     * Now we label the remaining boxes with the
     * number of mines surrounding them.
     */
    for( col = 1; col < p_board->cols + 1; col++ )
    for( row = 1; row < p_board->rows + 1; row++ ) {
        for( i = -1; i <= 1; i++ )
        for( j = -1; j <= 1; j++ ) {
            if( p_board->box[col + i][row + j].IsMine ) {
                p_board->box[col][row].NumMines++ ;
            }
        }
    }
}

void DrawMine( HDC hdc, HDC hMemDC, BOARD *p_board, unsigned col, unsigned row, BOOL IsPressed )
{
    MINEBMP_OFFSET offset = BOX_BMP;

    if( col == 0 || col > p_board->cols || row == 0 || row > p_board->rows )
           return;

    if( p_board->status == GAMEOVER ) {
        if( p_board->box[col][row].IsMine ) {
            switch( p_board->box[col][row].FlagType ) {
            case FLAG:
                offset = FLAG_BMP;
                break;
            case COMPLETE:
                offset = EXPLODE_BMP;
                break;
            case QUESTION:
                /* fall through */
            case NORMAL:
                offset = MINE_BMP;
            }
        } else {
            switch( p_board->box[col][row].FlagType ) {
            case QUESTION:
                offset = QUESTION_BMP;
                break;
            case FLAG:
                offset = WRONG_BMP;
                break;
            case NORMAL:
                offset = BOX_BMP;
                break;
            case COMPLETE:
                /* Do nothing */
                break;
            default:
                /* WINE_TRACE("Unknown FlagType during game over in DrawMine\n"); */
                break;
            }
        }
    } else {    /* WAITING or PLAYING */
        switch( p_board->box[col][row].FlagType ) {
        case QUESTION:
            if( !IsPressed )
                offset = QUESTION_BMP;
            else
                offset = QPRESS_BMP;
            break;
        case FLAG:
            offset = FLAG_BMP;
            break;
        case NORMAL:
            if( !IsPressed )
                offset = BOX_BMP;
            else
                offset = MPRESS_BMP;
            break;
        case COMPLETE:
            /* Do nothing */
            break;
        default:
            /*WINE_TRACE("Unknown FlagType while playing in DrawMine\n");*/
            break;
        }
    }

    if( p_board->box[col][row].FlagType == COMPLETE
        && !p_board->box[col][row].IsMine )
          offset = (MINEBMP_OFFSET) p_board->box[col][row].NumMines;

    /* if nothing has changed, then don't repaint the tile */

    if ((p_board->box[col][row].FlagType != p_board->box[col][row].LastFlagType) || 
        p_board->status == GAMEOVER ||
        p_board->IsFullRepaint )
        { 
        BitBlt( hdc,
                (col - 1) * MINE_WIDTH + p_board->mines_rect.left,
                (row - 1) * MINE_HEIGHT + p_board->mines_rect.top,
                MINE_WIDTH, MINE_HEIGHT,
                hMemDC, 0, offset * MINE_HEIGHT, SRCCOPY );  
        p_board->box[col][row].LastFlagType = p_board->box[col][row].FlagType; 
        }        
}

void DrawMines ( HDC hdc, HDC hMemDC, BOARD *p_board )
{
    HANDLE hOldObj;
    unsigned col, row;
    hOldObj = SelectObject (hMemDC, p_board->hMinesBMP);

    for( row = 1; row <= p_board->rows; row++ ) {
      for( col = 1; col <= p_board->cols; col++ ) {
        DrawMine( hdc, hMemDC, p_board, col, row, FALSE );
      }
    }
    SelectObject( hMemDC, hOldObj );
}

void DrawLeds( HDC hdc, HDC hMemDC, BOARD *p_board, int number, int x, int y )
{
    HANDLE hOldObj;
    unsigned led[3], i;
    int count;

    count = number;
    if( count < 1000 ) {
        if( count >= 0 ) {
            led[0] = count / 100 ;
            count -= led[0] * 100;
        }
        else {
            led[0] = 10; /* negative sign */
            count = -count;
        }
        led[1] = count / 10;
        count -= led[1] * 10;
        led[2] = count;
    }
    else {
        for( i = 0; i < 3; i++ )
            led[i] = 10;
    }

    hOldObj = SelectObject (hMemDC, p_board->hLedsBMP);

    for( i = 0; i < 3; i++ ) {
        BitBlt( hdc,
            i * LED_WIDTH + x,
            y,
            LED_WIDTH,
            LED_HEIGHT,
            hMemDC,
            0,
            led[i] * LED_HEIGHT,
            SRCCOPY);
    }

    SelectObject( hMemDC, hOldObj );
}


void DrawFace( HDC hdc, HDC hMemDC, BOARD *p_board )
{
    HANDLE hOldObj;
    
    
    hOldObj = SelectObject (hMemDC, p_board->hFacesBMP);

    BitBlt( hdc,
        p_board->face_rect.left,
        p_board->face_rect.top,
        FACE_WIDTH,
        FACE_HEIGHT,
        hMemDC, 0, p_board->face_bmp * FACE_HEIGHT, SRCCOPY);

    SelectObject( hMemDC, hOldObj );
    
}


void DrawBoard( HDC hdc, HDC hMemDC, PAINTSTRUCT *ps, BOARD *p_board )
{
    RECT tmp_rect;
        
    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->counter_rect ) )
        DrawLeds( hdc, hMemDC, p_board, p_board->mines - p_board->num_flags,
                  p_board->counter_rect.left,
                  p_board->counter_rect.top );

    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->timer_rect ) )
        DrawLeds( hdc, hMemDC, p_board, p_board->time,
                  p_board->timer_rect.left,
                  p_board->timer_rect.top );

    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->face_rect ) )
        DrawFace( hdc, hMemDC, p_board );

    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->mines_rect ) )
        DrawMines( hdc, hMemDC, p_board );
}


void AddFlag( BOARD *p_board, unsigned col, unsigned row )
{
    if( p_board->box[col][row].FlagType != COMPLETE ) {
        switch( p_board->box[col][row].FlagType ) {
        case FLAG:
            if( p_board->IsMarkQ )
                p_board->box[col][row].FlagType = QUESTION;
            else
                p_board->box[col][row].FlagType = NORMAL;
            p_board->num_flags--;
            break;

        case QUESTION:
            p_board->box[col][row].FlagType = NORMAL;
            break;

        default:
            p_board->box[col][row].FlagType = FLAG;
            p_board->num_flags++;
        }
    }
}


void UnpressBox( BOARD *p_board, unsigned col, unsigned row )
{
    HDC hdc;
    HANDLE hOldObj;
    HDC hMemDC;

    hdc = GetDC( p_board->hWnd );
    hMemDC = CreateCompatibleDC( hdc );
    hOldObj = SelectObject( hMemDC, p_board->hMinesBMP );

    DrawMine( hdc, hMemDC, p_board, col, row, FALSE );

    SelectObject( hMemDC, hOldObj );
    DeleteDC( hMemDC );
    ReleaseDC( p_board->hWnd, hdc );
}


void UnpressBoxes( BOARD *p_board, unsigned col, unsigned row )
{
    int i, j;

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        UnpressBox( p_board, col + i, row + j );
      }
}


void PressBox( BOARD *p_board, unsigned col, unsigned row )
{
    HDC hdc;
    HANDLE hOldObj;
    HDC hMemDC;

    hdc = GetDC( p_board->hWnd );
    hMemDC = CreateCompatibleDC( hdc );
    hOldObj = SelectObject (hMemDC, p_board->hMinesBMP);

    DrawMine( hdc, hMemDC, p_board, col, row, TRUE );

    SelectObject( hMemDC, hOldObj );
    DeleteDC( hMemDC );
    ReleaseDC( p_board->hWnd, hdc );
}


void PressBoxes( BOARD *p_board, unsigned col, unsigned row )
{
    int i, j;

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        p_board->box[col + i][row + j].IsPressed = TRUE;
        PressBox( p_board, col + i, row + j );
    }

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        if( !p_board->box[p_board->press.x + i][p_board->press.y + j].IsPressed )
            UnpressBox( p_board, p_board->press.x + i, p_board->press.y + j );
    }

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        p_board->box[col + i][row + j].IsPressed = FALSE;
        PressBox( p_board, col + i, row + j );
    }

    p_board->press.x = col;
    p_board->press.y = row;
}


void CompleteBox( BOARD *p_board, unsigned col, unsigned row )
{
    int i, j;
    
    if( p_board->box[col][row].FlagType != COMPLETE &&
            p_board->box[col][row].FlagType != FLAG &&
            col > 0 && col < p_board->cols + 1 &&
            row > 0 && row < p_board->rows + 1 ) {
        p_board->box[col][row].FlagType = COMPLETE;

        if( p_board->box[col][row].IsMine ) {
            p_board->face_bmp = DEAD_BMP;
            p_board->status = GAMEOVER;
        }
        else if( p_board->status != GAMEOVER )
            p_board->boxes_left--;

        if( p_board->box[col][row].NumMines == 0 )
        {
            for( i = -1; i <= 1; i++ )
            for( j = -1; j <= 1; j++ )
                CompleteBox( p_board, col + i, row + j ); 
                /* WARNING! This requires a LOT of stack! */
                /* if the stack size is too small, large boards will crash */
        }
    }
}


void CompleteBoxes( BOARD *p_board, unsigned col, unsigned row )
{
    unsigned numFlags = 0;
    int i, j;
    int c;

    if( p_board->box[col][row].FlagType == COMPLETE ) {
        for( i = -1; i <= 1; i++ )
          for( j = -1; j <= 1; j++ ) {
            if( p_board->box[col+i][row+j].FlagType == FLAG )
                numFlags++;
          }

        if( numFlags == p_board->box[col][row].NumMines ) {
            for( i = -1; i <= 1; i++ )
              for( j = -1; j <= 1; j++ ) {
                if( p_board->box[col+i][row+j].FlagType != FLAG )
                    CompleteBox( p_board, col+i, row+j ); 
              }
        }
    }
}


void TestMines( BOARD *p_board, POINT pt, int msg )
{
    HBRUSH hOldBrush;
    BOOL draw = TRUE;
    int col, row;

    col = (pt.x - p_board->mines_rect.left) / MINE_WIDTH + 1;
    row = (pt.y - p_board->mines_rect.top ) / MINE_HEIGHT + 1;

    switch ( msg ) {
    case WM_LBUTTONDOWN:
        if( p_board->press.x != col || p_board->press.y != row ) {
            UnpressBox( p_board,
                    p_board->press.x, p_board->press.y );
            p_board->press.x = col;
            p_board->press.y = row;
            PressBox( p_board, col, row );
        }
        draw = FALSE;
        break;

    case WM_LBUTTONUP:
        if( p_board->press.x != col || p_board->press.y != row )
            UnpressBox( p_board,
                    p_board->press.x, p_board->press.y );
        p_board->press.x = 0;
        p_board->press.y = 0;
        if( p_board->box[col][row].FlagType != FLAG
            && p_board->status != PLAYING )
        {
            p_board->status = PLAYING;
            PlaceMines( p_board, col, row );
        }
        CompleteBox( p_board, col, row );
        break;

    case WM_MBUTTONDOWN:
        PressBoxes( p_board, col, row );
        draw = FALSE; 
        break;

    case WM_MBUTTONUP:
        if( p_board->press.x != col || p_board->press.y != row ) 
            UnpressBoxes( p_board,
                    p_board->press.x, p_board->press.y );
        p_board->press.x = 0;
        p_board->press.y = 0;
        CompleteBoxes( p_board, col, row );
        break;

    case WM_RBUTTONDOWN:
        AddFlag( p_board, col, row );
        break;
    default:
        /*WINE_TRACE("Unknown message type received in TestMines\n");*/
        break;
    }

    if( draw ) 
    {
        /* Redraw and update the window. */
        /* Reduce flicker by disabling the background brush before redrawing */
        /* Note that we also update the mine counter as well as the mine field */
        /* and that we need to redraw after a middle-click selection */
        hOldBrush = GetClassWord(p_board->hWnd, GCW_HBRBACKGROUND);
        SetClassWord(p_board->hWnd, GCW_HBRBACKGROUND, NULL);
        if (msg != WM_MBUTTONUP) p_board->IsFullRepaint = FALSE;
        InvalidateRect(p_board->hWnd, (LPRECT)NULL, TRUE);
        UpdateWindow(p_board->hWnd);
        p_board->IsFullRepaint = TRUE;
        SetClassWord(p_board->hWnd, GCW_HBRBACKGROUND, hOldBrush);
    }
}


void TestFace( BOARD *p_board, POINT pt, int msg )
{
    HBRUSH hOldBrush;
    if( p_board->status == PLAYING || p_board->status == WAITING ) {
        if( msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN )
            p_board->face_bmp = OOH_BMP;
        else p_board->face_bmp = SMILE_BMP;
    }
    else if( p_board->status == GAMEOVER )
        p_board->face_bmp = DEAD_BMP;
    else if( p_board->status == WON )
            p_board->face_bmp = COOL_BMP;

    if( PtInRect( &p_board->face_rect, pt ) ) {
        if( msg == WM_LBUTTONDOWN )
            p_board->face_bmp = SPRESS_BMP;

        if( msg == WM_LBUTTONUP )
            CreateBoard( p_board );
    }
    
    hOldBrush = GetClassWord(p_board->hWnd, GCW_HBRBACKGROUND);
    SetClassWord(p_board->hWnd, GCW_HBRBACKGROUND, NULL);
    InvalidateRect(p_board->hWnd, &p_board->face_rect, TRUE);
    UpdateWindow(p_board->hWnd);
    SetClassWord(p_board->hWnd, GCW_HBRBACKGROUND, hOldBrush); 
}


void TestBoard( HWND hWnd, BOARD *p_board, int x, int y, int msg )
{
    POINT pt;
    unsigned col,row;

    pt.x = x;
    pt.y = y;

    if( PtInRect( &p_board->mines_rect, pt ) && p_board->status != GAMEOVER
    && p_board->status != WON )
        TestMines( p_board, pt, msg );
    else {
        UnpressBoxes( p_board,
            p_board->press.x,
            p_board->press.y );
        p_board->press.x = 0;
        p_board->press.y = 0;
    }

    if( p_board->boxes_left == 0 ) {
        p_board->status = WON;

        if (p_board->num_flags < p_board->mines) {
            for( row = 1; row <= p_board->rows; row++ ) {
                for( col = 1; col <= p_board->cols; col++ ) {
                    if (p_board->box[col][row].IsMine && p_board->box[col][row].FlagType != FLAG)
                        p_board->box[col][row].FlagType = FLAG;
                }
            }

            p_board->num_flags = p_board->mines;

            InvalidateRect(p_board->hWnd, (LPRECT)NULL, TRUE);
        }

        if( p_board->difficulty != CUSTOM &&
                    p_board->time < p_board->best_time[p_board->difficulty] ) {
            p_board->best_time[p_board->difficulty] = p_board->time;
            
            g_board = p_board;

            DialogBox( p_board->hInst, "DLG_CONGRATS", hWnd,
                    p_board->lpCongratsDlgProc);
                    
            g_board = p_board;        

            DialogBox( p_board->hInst, "DLG_TIMES", hWnd,
                    p_board->lpTimesDlgProc);
        }
    }
    TestFace( p_board, pt, msg );
}


long FAR PASCAL MainProc( HWND hWnd, unsigned msg, WORD wParam, LONG lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HMENU hMenu;
    static BOARD board;

    switch( msg ) {
    case WM_CREATE:
        board.hInst = ((LPCREATESTRUCT) lParam)->hInstance;
        board.hWnd = hWnd;
        board.IsFullRepaint = TRUE;
        
        /* Dialog boxes need this for Windows 1.x/2.x */
        board.lpCustomDlgProc = MakeProcInstance((FARPROC)CustomDlgProc, board.hInst);
        board.lpCongratsDlgProc = MakeProcInstance((FARPROC)CongratsDlgProc, board.hInst);
        board.lpTimesDlgProc = MakeProcInstance((FARPROC)TimesDlgProc, board.hInst);
        board.lpAboutDlgProc = MakeProcInstance((FARPROC)AboutDlgProc, board.hInst);
        
        InitBoard( &board );
        CreateBoard( &board );
        return 0;

    case WM_PAINT:
      {
        HDC hMemDC;

        /*WINE_TRACE("WM_PAINT\n"); */
        hdc = BeginPaint( hWnd, &ps );
        hMemDC = CreateCompatibleDC( hdc );

        DrawBoard( hdc, hMemDC, &ps, &board );

        DeleteDC( hMemDC );
        EndPaint( hWnd, &ps );

        return 0;
      }

    case WM_MOVE:
        /* WINE_TRACE("WM_MOVE\n"); */
        if (IsWindowVisible(hWnd)){      
        board.pos.x = (short)LOWORD(lParam);
        board.pos.y = (short)HIWORD(lParam); 
        }
        return 0;

    case WM_DESTROY:
        SaveBoard( &board );
        DestroyBoard( &board );
        PostQuitMessage( 0 );
        return 0;

    case WM_TIMER:
        if( board.status == PLAYING ) {
            board.time++; {

            /* Redraw and update the timer rect */
            /* Temporarily disable the background to avoid flicker */
        
            HBRUSH hOldBrush;
            hOldBrush = GetClassWord(board.hWnd, GCW_HBRBACKGROUND);
            SetClassWord(board.hWnd, GCW_HBRBACKGROUND, NULL);
            InvalidateRect(board.hWnd, &board.timer_rect, TRUE);
            UpdateWindow(board.hWnd);
            SetClassWord(board.hWnd, GCW_HBRBACKGROUND, hOldBrush); 
            }    
        }
        return 0;

    case WM_LBUTTONDOWN:
        /*WINE_TRACE("WM_LBUTTONDOWN\n");*/
        if( wParam & (MK_RBUTTON | MK_SHIFT) )
            msg = WM_MBUTTONDOWN;
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        SetCapture( hWnd );
        return 0;

    case WM_LBUTTONUP:
        /*WINE_TRACE("WM_LBUTTONUP\n");*/
        if( wParam & (MK_RBUTTON | MK_SHIFT) )
            msg = WM_MBUTTONUP;
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        ReleaseCapture();
        return 0;

    case WM_RBUTTONDOWN:
        /* WINE_TRACE("WM_RBUTTONDOWN\n"); */
        if( wParam & MK_LBUTTON ) {
            board.press.x = 0;
            board.press.y = 0;
            msg = WM_MBUTTONDOWN;
        }
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        return 0;

    case WM_RBUTTONUP:
        /* WINE_TRACE("WM_RBUTTONUP\n"); */
        if( wParam & MK_LBUTTON )
            msg = WM_MBUTTONUP;
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        return 0;

    case WM_MBUTTONDOWN:
        /* WINE_TRACE("WM_MBUTTONDOWN\n");*/
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        return 0;

    case WM_MBUTTONUP:
        /*WINE_TRACE("WM_MBUTTONUP\n");*/
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        return 0;

    case WM_MOUSEMOVE:
    {
        if( ( wParam & MK_MBUTTON ) ||
            ( ( wParam & MK_LBUTTON ) && ( wParam & MK_RBUTTON ) ) ) {
            msg = WM_MBUTTONDOWN;
        }
        else if( wParam & MK_LBUTTON ) {
            msg = WM_LBUTTONDOWN;
        }
        else {
            return 0;
        }

        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam),  msg );

        return 0;
    }
    
    case WM_SETFOCUS:     
    {
        UpdateWindow(board.hWnd);
        return 0;
    } 


    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDM_NEW:
            CreateBoard( &board );
            return 0;

        case IDM_MARKQ:
            hMenu = GetMenu( hWnd );
            board.IsMarkQ = !board.IsMarkQ;
            if( board.IsMarkQ )
                CheckMenuItem( hMenu, IDM_MARKQ, MF_CHECKED );
            else
                CheckMenuItem( hMenu, IDM_MARKQ, MF_UNCHECKED );
            return 0;

        case IDM_BEGINNER:
            SetDifficulty( &board, BEGINNER );
            CreateBoard( &board );
            return 0;

        case IDM_ADVANCED:
            SetDifficulty( &board, ADVANCED );
            CreateBoard( &board );
            return 0;

        case IDM_EXPERT:
            SetDifficulty( &board, EXPERT );
            CreateBoard( &board );
            return 0;

        case IDM_CUSTOM:
            SetDifficulty( &board, CUSTOM );
            CreateBoard( &board );
            return 0;

        case IDM_EXIT:
            SendMessage( hWnd, WM_CLOSE, 0, 0L);
            return 0;

        case IDM_TIMES:
            g_board = &board;
            DialogBox( board.hInst, "DLG_TIMES", hWnd,
                    board.lpTimesDlgProc );        
            return 0;

        case IDM_ABOUT:
        {
            if (DialogBox( board.hInst, "DLG_ABOUT", board.hWnd,
                  board.lpAboutDlgProc) != 0)     
            return 0;
        }
        default:
            /*WINE_TRACE("Unknown WM_COMMAND command message received\n"); */
            break;
        }
    }
    return( DefWindowProc( hWnd, msg, wParam, lParam ));
}

int FAR PASCAL WinMain( HANDLE hInst, HANDLE hPrevInst, LPSTR cmdline, int cmdshow )
{
    MSG msg;
    WNDCLASS wc;
    HWND hWnd;
    HANDLE haccel;
    char appname[20];

    LoadString( hInst, IDS_APPNAME, appname, sizeof(appname));

    /*wc.cbSize = sizeof(wc); */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon( hInst, "WINEMINE" );
    wc.hCursor = LoadCursor( 0, IDI_APPLICATION );
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0xFF, 0xFF)); 
    wc.lpszMenuName = "MENU_WINEMINE";
    wc.lpszClassName = appname;
    /*wc.hIconSm = LoadImage( hInst, "WINEMINE", IMAGE_ICON,
                            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED );*/

    if (!RegisterClass(&wc)) 
      return FALSE;
    hWnd = CreateWindow( appname, appname,
	wnd_style,
        0, 0, 0, 0,
        0, 0, hInst, NULL );

    if (!hWnd) 
      return FALSE;;

    ShowWindow( hWnd, cmdshow );
    UpdateWindow( hWnd );

    haccel = LoadAccelerators( hInst, MAKEINTRESOURCE(IDA_WINEMINE) );
    SetTimer( hWnd, ID_TIMER, 1000, NULL );

    while( GetMessage(&msg, 0, 0, 0) ) {
        if (!TranslateAccelerator( hWnd, haccel, &msg ))
            TranslateMessage( &msg );

        DispatchMessage( &msg );
    }
    return msg.wParam;
}
