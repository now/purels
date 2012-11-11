
////
/// system tray bang commands
//

#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include "systray.h"

void SystrayHide( HWND hCaller, LPCTSTR pszArgs )
{
	ShowWindow( hSystray, SW_HIDE );
	fVisible = FALSE;
}

void SystrayMove( HWND hCaller, LPCTSTR pszArgs )
{
	LPCTSTR p = pszArgs;
	TCHAR szValue[16];
	
	p = NextToken( p, szValue, 16 );
	nX = ParseInteger( szValue );
	nX = (nX < 0) ? (nScreenX + nX + 1) : nX;
	
	p = NextToken( p, szValue, 16 );
	nY = ParseInteger( szValue );
	nY = (nY < 0) ? (nScreenY + nY + 1) : nY;
	
	SetWindowPos( hSystray, NULL, nX, nY, 0, 0, SWP_NOSIZE |
		SWP_NOZORDER | SWP_NOACTIVATE );
}
	
void SystrayShow( HWND hCaller, LPCTSTR pszArgs )
{
	ShowWindow( hSystray, SW_SHOWNOACTIVATE );
	fVisible = TRUE;
}

void SystrayToggle( HWND hCaller, LPCTSTR pszArgs )
{
	fVisible = !fVisible;
	ShowWindow( hSystray, fVisible ? SW_SHOWNOACTIVATE : SW_HIDE );
}
