
////
/// system tray module interface implementation
//

#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <commctrl.h>

#ifndef OUTSIDER99
#include <stdio.h>
#include "lsapi.h"
#include "wharfdata.h"
#endif

#include "systray.h"
#include "registry.h"
// #include "debug.h"

#pragma comment(linker,"/merge:.text=.data")
#pragma comment(linker,"/merge:.reloc=.data")

HWND hLitestep;
HWND hSystray;
HWND hShellTrayWnd;

BOOL fDocked = FALSE;
HWND hWharfParent = NULL;

BOOL fAlwaysOnTop;
BOOL fBorderDrag;
int nBorderX;
int nBorderY;
BOOL fColorize;
BOOL fHidden;
int nIconSize;
int nX;
int nY;
int nSnapDistance;
int nSpacingX;
int nSpacingY;
int nWrapCount;
int nShortcutGroup;
BOOL fHidden;
BOOL fNoTransparency;
BOOL fHideIfEmpty;
BOOL fVisible;

int nMinWidth;
int nMinHeight;
int nMaxWidth;
int nMaxHeight;

BYTE bFromR, bToR;
BYTE bFromG, bToG;
BYTE bFromB, bToB;

HBITMAP hbmSkin = NULL;
BOOL fSkinTiled;
int nBitmapX;
int nBitmapY;
int nBorderLeft;
int nBorderTop;
int nBorderRight;
int nBorderBottom;

BOOL fHorizontal;
int nDeltaX;
int nDeltaY;
int nResizeH;
int nResizeV;

int nScreenX;
int nScreenY;

BOOL fWinNT;

#ifndef OUTSIDER99
UINT nMessages[] = {
//	LM_GETREVID,
	LM_RESTORESYSTRAY,
	LM_SAVESYSTRAY,
	LM_SENDSYSTRAY,
	LM_SYSTRAY,
	0
};
#endif

//
// GetShellDesktop
//

HWND GetShellDesktop()
{
	HWND hDesktop = FindWindow( WC_SHELLDESKTOP, NULL );
	
	if( !hDesktop )
		hDesktop = GetDesktopWindow();
	
	return hDesktop;
}

//
// initModuleEx
//

EXTERN_C EXPORT int initModuleEx( HWND hParent, HINSTANCE hInstance, LPCTSTR pszPath )
{
	WNDCLASSEX wc;
	OSVERSIONINFO osvi;
	
#ifndef OUTSIDER99
	hLitestep = GetLitestepWnd();
#else
	hLitestep = NULL;
#endif

	hSystray = NULL;
	hShellTrayWnd = NULL;
	
	nScreenX = GetSystemMetrics( SM_CXSCREEN );
	nScreenY = GetSystemMetrics( SM_CYSCREEN );
	
	InitCommonControls();
	
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &osvi );
	fWinNT = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) ? TRUE : FALSE;
	
	ReadConfig();
	
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc = SystrayProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hIcon = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WC_SYSTRAY;
	wc.hIconSm = NULL;
	
	RegisterClassEx( &wc );
	
	hSystray = CreateWindowEx( WS_EX_TOOLWINDOW,
		WC_SYSTRAY,
		NULL,
		fDocked ? WS_CHILD : WS_POPUP,
		nX,
		nY,
		0,
		0,
		fDocked ? hParent : hLitestep,
		NULL,
		hInstance,
		NULL );
	
	if( fAlwaysOnTop )
	{
		SetWindowPos( hSystray,
			HWND_TOPMOST,
			0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
	}
	else if( !fDocked )
	{
		SetParent( hSystray, GetShellDesktop() );
	}
	
	fVisible = !fHidden;
	AdjustLayout();
	
	SetWindowLong( hSystray, GWL_USERDATA, 0x49474541 );
	ShowWindow( hSystray, (fVisible && !fHideIfEmpty) ? SW_SHOWNOACTIVATE : SW_HIDE );
	
#ifndef OUTSIDER99
	SendMessage( hLitestep, LM_REGISTERMESSAGE, (WPARAM) hSystray, (LPARAM) nMessages );
	
	if( nShortcutGroup >= 0 )
		SendMessage( hLitestep, LM_DOCKTRAY, (WPARAM) hSystray, (LPARAM) nShortcutGroup );
	
	AddBangCommand( TEXT("!SystrayHide"), SystrayHide );
	AddBangCommand( TEXT("!SystrayMove"), SystrayMove );
	AddBangCommand( TEXT("!SystrayShow"), SystrayShow );
	AddBangCommand( TEXT("!SystrayToggle"), SystrayToggle );
#endif

	if( !FindWindow( WC_SHELLTRAYWND, NULL ) )
	{
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_GLOBALCLASS;
		wc.lpfnWndProc = ShellTrayWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hCursor = NULL;
		wc.hIcon = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = WC_SHELLTRAYWND;
		wc.hIconSm = NULL;
		
		RegisterClassEx( &wc );
		
		hShellTrayWnd = CreateWindowEx( WS_EX_TOOLWINDOW,
			WC_SHELLTRAYWND,
			NULL,
			WS_POPUP,
			0,
			0,
			0,
			0,
			NULL,
			NULL,
			hInstance,
			NULL );
	}
	
	return 0;
}

//
// initModule
//

#ifndef OUTSIDER99
EXTERN_C EXPORT int initModule( HWND hParent, HINSTANCE hInstance, wharfDataType *wd )
{
	return initModuleEx( hParent, hInstance, wd->lsPath );
}
#endif

//
// quitModule
//

EXTERN_C EXPORT void quitModule( HINSTANCE hInstance )
{
#ifndef OUTSIDER99
	BOOL (*pRemoveBangCommand)( LPCSTR );
	pRemoveBangCommand = (BOOL (*)( LPCSTR )) GetProcAddress( GetModuleHandle( TEXT("LSAPI.DLL") ), TEXT("RemoveBangCommand") );
	
	if( pRemoveBangCommand )
	{
		(*pRemoveBangCommand)( TEXT("!SystrayHide") );
		(*pRemoveBangCommand)( TEXT("!SystrayMove") );
		(*pRemoveBangCommand)( TEXT("!SystrayShow") );
		(*pRemoveBangCommand)( TEXT("!SystrayToggle") );
	}
#endif

	if( hShellTrayWnd )
	{
		DestroyWindow( hShellTrayWnd );
		UnregisterClass( WC_SHELLTRAYWND, hInstance );
	}
	
#ifndef OUTSIDER99
	SendMessage( hLitestep, LM_UNREGISTERMESSAGE, (WPARAM) hSystray, (LPARAM) nMessages );
#endif

	DestroyWindow( hSystray );
	UnregisterClass( WC_SYSTRAY, hInstance );
	
	hSystray = NULL;
}

//
// initWharfModule
//

#ifndef OUTSIDER99
EXTERN_C EXPORT int initWharfModule( HWND hParent, HINSTANCE hInstance, wharfDataType *wd )
{
	hWharfParent = hParent;
	fDocked = TRUE;
	
	return initModuleEx( hParent, hInstance, wd->lsPath );
}
#endif

//
// quitWharfModule
//

#ifndef OUTSIDER99
EXTERN_C EXPORT void quitWharfModule( HINSTANCE hInstance )
{
	quitModule( hInstance );
}
#endif

//
// DllEntryPoint
//

#ifndef OUTSIDER99
EXTERN_C int APIENTRY DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved )
{
	return 1;
}
#endif

//
// main
//

#ifdef OUTSIDER99
void main()
{
	HINSTANCE hInstance;
	MSG msg;
	
	hInstance = GetModuleHandle( NULL );
	InitCommonControls();
	
	initModuleEx( NULL, hInstance, NULL );
	
	while( GetMessage( &msg, 0, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	
	quitModule( hInstance );
	ExitProcess( GetLastError() );
}
#endif

//
// Recycle
//

void Recycle()
{
	DeleteObject( hbmSkin );
	hbmSkin = NULL;
	
	ReadConfig();
	
	if( fAlwaysOnTop || fDocked )
		SetParent( hSystray, NULL );
	else
		SetParent( hSystray, GetShellDesktop() );
	
	SetWindowPos( hSystray, fAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
		nX, nY, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
	
	ShowWindow( hSystray, fHidden ? SW_HIDE : SW_SHOWNOACTIVATE );
	fVisible = !fHidden;
	
	AdjustLayout();
}
