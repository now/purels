
////
/// system tray
//

#ifndef __COMMON_H
#define __COMMON_H

// #define DEBUG

#ifndef EXTERN_C
  #ifdef __cplusplus
    #define EXTERN_C extern "C"
  #else
    #define EXTERN_C extern
  #endif
#endif

#ifdef OUTSIDER99
#define EXPORT
#else
#define EXPORT __declspec(dllexport)
#endif

//
// structures
//

typedef struct MAP {
	LPCTSTR pszName;
	UINT uValue;
} MAP, *PMAP, FAR *LPMAP;

//
// constants
//

#define LM_DOCKTRAY        9900	// Easycuts 2.5+ IPC
#define LM_HIDETRAY        9901	// Outsider IPC
#define LM_SHOWTRAY        9902	// "
#define LM_RECYCLETRAY     9903	// "

#ifdef OUTSIDER99
#define LM_SYSTRAY         9214	// system tray message (defined in LSAPI.H in LS)
#endif

#ifdef OUTSIDER99
#define WC_SHELLDESKTOP    TEXT("Desktop Class")
#else
#define WC_SHELLDESKTOP    TEXT("DesktopBackgroundClass")
#endif

#define WC_SHELLTRAYWND    TEXT("Shell_TrayWnd")
#define WC_SYSTRAY         TEXT("SystemTray")

#define DIR_LEFT           0
#define DIR_UP             1
#define DIR_RIGHT	          2
#define DIR_DOWN           3

// I need a better way to store the icon data
// between recycles, but for now registry works
// fine.

#define REGKEY_SYSTRAY     TEXT("Software\\Litestep\\Systray")
#define REGKEY_SAVEDDATA   REGKEY_SYSTRAY TEXT("\\SavedData")

#define CFG_NAME                 TEXT("Systray")
#define CFG_ALWAYSONTOP          TEXT("AlwaysOnTop")
#define CFG_AUTOSIZE             TEXT("AutoSize")
#define CFG_BITMAP               TEXT("Bitmap")
#define CFG_BORDERDRAG           TEXT("BorderDrag")
#define CFG_BORDERX              TEXT("BorderX")
#define CFG_BORDERY              TEXT("BorderY")
#define CFG_COLORIZE             TEXT("Colorize")
#define CFG_COLORFROM            TEXT("ColorFrom")
#define CFG_COLORTO              TEXT("ColorTo")
#define CFG_DIRECTION            TEXT("Direction")
#define CFG_DOCKTOSHORTCUTGROUP  TEXT("DockToShortcutGroup")
#define CFG_HIDDEN               TEXT("Hidden")
#define CFG_HIDEIFEMPTY          TEXT("HideIfEmpty")
#define CFG_ICONSIZE             TEXT("IconSize")
#define CFG_MAXHEIGHT            TEXT("MaxHeight")
#define CFG_MAXWIDTH             TEXT("MaxWidth")
#define CFG_MINHEIGHT            TEXT("MinHeight")
#define CFG_MINWIDTH             TEXT("MinWidth")
#define CFG_NOTRANSPARENCY       TEXT("NoTransparency")
#define CFG_SNAPDISTANCE         TEXT("SnapDistance")
#define CFG_SPACINGX             TEXT("SpacingX")
#define CFG_SPACINGY             TEXT("SpacingY")
#define CFG_WRAPCOUNT            TEXT("WrapCount")
#define CFG_X                    TEXT("X")
#define CFG_Y                    TEXT("Y")

//
// variables
//

// way too many globals =)
// my excuse is that in the beginning there were
// only a few and when more were I added I didn't
// wanna change all the existing code. maybe one
// day I should move these into a struct.

extern HWND hLitestep;
extern HWND hSystray;

extern BOOL fDocked;
extern HWND hWharfParent;

extern BOOL fAlwaysOnTop;
extern BOOL fBorderDrag;
extern int nBorderX;
extern int nBorderY;
extern BOOL fColorize;
extern int nIconSize;
extern int nX;
extern int nY;
extern int nSnapDistance;
extern int nSpacingX;
extern int nSpacingY;
extern int nWrapCount;
extern int nShortcutGroup;
extern BOOL fHidden;
extern BOOL fNoTransparency;
extern BOOL fHideIfEmpty;
extern BOOL fVisible;

extern int nMinWidth;
extern int nMinHeight;
extern int nMaxWidth;
extern int nMaxHeight;

extern BYTE bFromR, bToR;
extern BYTE bFromG, bToG;
extern BYTE bFromB, bToB;

extern HBITMAP hbmSkin;
extern BOOL fSkinTiled;
extern int nBitmapX;
extern int nBitmapY;
extern int nBorderLeft;
extern int nBorderTop;
extern int nBorderRight;
extern int nBorderBottom;

extern BOOL fHorizontal;
extern int nDeltaX;
extern int nDeltaY;
extern int nResizeH;
extern int nResizeV;

extern int nScreenX;
extern int nScreenY;

extern MAP mapDirection[];
extern MAP mapLR[];
extern MAP mapUD[];

extern TCHAR szThemePath[];

extern BOOL fWinNT;

//
// functions
//

extern LPCTSTR NextToken( LPCTSTR, LPTSTR, UINT );
extern int ParseInteger( LPCTSTR );
extern UINT MapName( PMAP, LPCTSTR, UINT );
extern BOOL GetConfigBoolean( LPCTSTR );
extern int GetConfigInteger( LPCTSTR, int, int, int );
extern void GetConfigString( LPCTSTR, LPTSTR, UINT, LPCTSTR );
extern void ReadConfig();

extern LRESULT WINAPI SystrayProc( HWND, UINT, WPARAM, LPARAM );
extern LRESULT WINAPI ShellTrayWndProc( HWND, UINT, WPARAM, LPARAM );

extern void AdjustLayout();
extern void Recycle();

#ifndef OUTSIDER99
extern void SystrayHide( HWND, LPCTSTR );
extern void SystrayMove( HWND, LPCTSTR );
extern void SystrayShow( HWND, LPCTSTR );
extern void SystrayToggle( HWND, LPCTSTR );
#endif

extern HBITMAP LoadBitmapFile( LPCTSTR );
extern BOOL TransBlt( HDC, int, int, int, int, HDC, int, int, COLORREF );

//
// API
//

#undef RtlFillMemory
#undef RtlMoveMemory
#undef RtlZeroMemory

EXTERN_C NTSYSAPI VOID NTAPI RtlFillMemory( VOID UNALIGNED *Destination, DWORD Length, BYTE Fill );
EXTERN_C NTSYSAPI VOID NTAPI RtlMoveMemory( VOID UNALIGNED *Destination, CONST VOID UNALIGNED *Source, DWORD Length );
EXTERN_C NTSYSAPI VOID NTAPI RtlZeroMemory( VOID UNALIGNED *Destination, DWORD Length );

#endif
