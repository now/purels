
////
/// system tray shell support code
//

// normally desktop.dll handles this, but if
// desktop.dll isn't loaded or we're running
// with Outsider, then we handle it ourselves

#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <shellapi.h>

#ifndef OUTSIDER99
#include <stdio.h>
#include "lsapi.h"
#endif

#include "systray.h"

typedef struct SHELLTRAYDATA {
	DWORD dwUnknown;
	DWORD dwMessage;
	NOTIFYICONDATA nid;
} SHELLTRAYDATA, *PSHELLTRAYDATA, FAR *LPSHELLTRAYDATA;

BOOL SystrayMessage( PSHELLTRAYDATA pstd )
{
	NOTIFYICONDATA nid;
	
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = pstd->nid.hWnd;
	nid.uID = pstd->nid.uID;
	nid.uFlags = pstd->nid.uFlags;
	nid.uCallbackMessage = pstd->nid.uCallbackMessage;
	nid.hIcon = pstd->nid.hIcon;
	nid.szTip[0] = 0;
	
	if( nid.uFlags & NIF_TIP )
	{
		if( fWinNT )
			WideCharToMultiByte( CP_ACP, 0, (LPWSTR) pstd->nid.szTip, -1, nid.szTip, 64, NULL, NULL );
		else
			lstrcpyn( nid.szTip, pstd->nid.szTip, 64 );
	}
	
#ifdef OUTSIDER99
	return (BOOL) SendMessage( hSystray, LM_SYSTRAY, pstd->dwMessage, (LPARAM) &nid );
#else
	return (BOOL) SendMessage( hLitestep, LM_SYSTRAY, pstd->dwMessage, (LPARAM) &nid );
#endif
}

LRESULT WINAPI ShellTrayWndProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{
	switch( nMessage )
	{
		case WM_COPYDATA:
		{
			PCOPYDATASTRUCT pcds;
			
			pcds = (PCOPYDATASTRUCT) lParam;
			
			if( pcds->dwData != 1 )
				return FALSE;
			
			return SystrayMessage( (PSHELLTRAYDATA) pcds->lpData );
		}
		
		case LM_HIDETRAY:
		{
			ShowWindow( hSystray, SW_HIDE );
			return 0;
		}
		
		case LM_SHOWTRAY:
		{
			ShowWindow( hSystray, SW_SHOWNOACTIVATE );
			return 0;
		}
		
		case LM_RECYCLETRAY:
		{
			Recycle();
			return 0;
		}
	}
	
	return DefWindowProc( hWnd, nMessage, wParam, lParam );
}
