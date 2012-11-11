
////
/// system tray config functions
//

#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>

#ifndef OUTSIDER99
#include <stdio.h>
#include "lsapi.h"
#endif

#include "systray.h"
#include "registry.h"

MAP mapDirection[] = {
	TEXT("left"),   DIR_LEFT,
	TEXT("up"),     DIR_UP,
	TEXT("right"),  DIR_RIGHT,
	TEXT("down"),   DIR_DOWN,
	NULL,           0
};

MAP mapLR[] = {
	TEXT("left"),   DIR_LEFT,
	TEXT("right"),  DIR_RIGHT,
	NULL,           0
};

MAP mapUD[] = {
	TEXT("up"),     DIR_UP,
	TEXT("down"),   DIR_DOWN,
	NULL,           0
};

//
// variables
//

TCHAR szOutsiderINI[MAX_PATH];
TCHAR szThemePath[MAX_PATH];

//
// NextToken
//

LPCTSTR NextToken( LPCTSTR s, LPTSTR d, UINT m )
{
	// TODO: handle single quotes (')
	
	while( *s && *s <= 32 )
		s++;
	
	if( *s == 34 )
	{
		s++;
		
		while( *s && *s != 34 && --m )
			*d++ = *s++;
		
		s++;
	}
	else
	{
		while( *s && *s > 32 && --m )
			*d++ = *s++;
	}
	
	*d = 0;
	return s;
}

//
// ParseInteger
//

int ParseInteger( LPCTSTR pszSource )
{
	int nValue = 0;
	BOOL fNegative = FALSE;
	
	while( *pszSource && *pszSource <= 32 )
		pszSource++;
	
	if( *pszSource && *pszSource == '-' )
	{
		fNegative = TRUE;
		pszSource++;
	}
	
	while( *pszSource && *pszSource >= '0' && *pszSource <= '9' )
		nValue = (nValue * 10) + (*pszSource++ - '0');
	
	return fNegative ? -nValue : nValue;
}

//
// MapName
//

UINT MapName( PMAP mapTable, LPCTSTR pszName, UINT uDefaultValue )
{
	PMAP map;
	
	for( map = mapTable; map->pszName; map++ )
	{
		if( !lstrcmpi( map->pszName, pszName ) )
			return map->uValue;
	}
	
	return uDefaultValue;
}

//
// GetConfigBoolean
//

BOOL GetConfigBoolean( LPCTSTR pszName )
{
#ifdef OUTSIDER99
	TCHAR szValue[16];
	GetPrivateProfileString( CFG_NAME, pszName, TEXT("FALSE"), szValue, 16, szOutsiderINI );
	
	if( !lstrcmpi( szValue, TEXT("TRUE") ) || szValue[0] == '1' )
		return TRUE;
	
	return FALSE;
#else
	TCHAR szFullName[64];
	
	lstrcpy( szFullName, CFG_NAME );
	lstrcat( szFullName, pszName );
	
	return GetRCBool( szFullName, TRUE );
#endif
}

//
// GetConfigColor
//

COLORREF GetConfigColor( LPCTSTR pszName, COLORREF crDefault )
{
#ifdef OUTSIDER99
	return crDefault;
#else
	TCHAR szFullName[64];
	
	lstrcpy( szFullName, CFG_NAME );
	lstrcat( szFullName, pszName );
	
	return GetRCColor( szFullName, crDefault );
#endif
}

//
// GetConfigInteger
//

int GetConfigInteger( LPCTSTR pszName, int nDefault, int nMin, int nMax )
{
	int nValue;
	
#ifdef OUTSIDER99
	nValue = (int) GetPrivateProfileInt( CFG_NAME, pszName, nDefault, szOutsiderINI );
#else
	TCHAR szFullName[64];
	
	lstrcpy( szFullName, CFG_NAME );
	lstrcat( szFullName, pszName );
	
	nValue = GetRCInt( szFullName, nDefault );
#endif

	nValue = max( nMin, min( nMax, nValue ) );
	return nValue;
}

//
// GetConfigString
//

void GetConfigString( LPCTSTR pszName, LPTSTR pszBuffer, UINT nBufLen, LPCTSTR pszDefault )
{
#ifdef OUTSIDER99
	GetPrivateProfileString( CFG_NAME, pszName, pszDefault, pszBuffer, nBufLen, szOutsiderINI );
#else
	TCHAR szFullName[64];
	
	lstrcpy( szFullName, CFG_NAME );
	lstrcat( szFullName, pszName );
	
	GetRCString( szFullName, pszBuffer, pszDefault, nBufLen );
#endif
}

//
// SetConfigInteger
//

void SetConfigInteger( LPCTSTR pszName, int nValue )
{
#ifdef OUTSIDER99
	TCHAR szValue[16];
	
	wsprintf( szValue, TEXT("%d"), nValue );
	WritePrivateProfileString( CFG_NAME, pszName, szValue, szOutsiderINI );
#else
	// not supported
#endif
}

//
// ReadConfig
//

void ReadConfig()
{
	int nDir1;
	int nDir2;
	TCHAR szLine[MAX_PATH];
	TCHAR szBuffer[MAX_PATH];
	LPCTSTR p;
	BITMAP bm;
	COLORREF crFrom;
	COLORREF crTo;
	
#ifdef OUTSIDER99
	TCHAR szOSPath[MAX_PATH];
	TCHAR szThemeName[32];
	int nLen;
	
	RegQueryStringValue( HKEY_CURRENT_USER,
		TEXT("Software\\GrizzlySoftware\\Outsider99\\GUI"),
		TEXT("OSPath"),
		szOSPath,
		MAX_PATH,
		TEXT("") );
	
	nLen = lstrlen( szOSPath );
	
	if( szOSPath[nLen - 1] != '\\' )
		lstrcat( szOSPath, TEXT("\\") );
	
	lstrcpy( szOutsiderINI, szOSPath );
	lstrcat( szOutsiderINI, TEXT("Outsider99.ini") );
	lstrcpy( szThemePath, szOSPath );
	
	RegQueryStringValue( HKEY_CURRENT_USER,
		TEXT("Software\\GrizzlySoftware\\Outsider99\\GUI"),
		TEXT("ThemeName"),
		szThemeName,
		32,
		TEXT("") );
	
	if( szThemeName[0] )
	{
		lstrcpy( szThemePath, szOSPath );
		lstrcat( szThemePath, TEXT("Themes\\") );
		lstrcat( szThemePath, szThemeName );
		lstrcat( szThemePath, TEXT("\\") );
		
		lstrcpy( szOutsiderINI, szThemePath );
		lstrcat( szOutsiderINI, TEXT("Theme.ini") );
	}
#endif

	fAlwaysOnTop = GetConfigBoolean( CFG_ALWAYSONTOP );
	fBorderDrag = GetConfigBoolean( CFG_BORDERDRAG );
	fHidden = GetConfigBoolean( CFG_HIDDEN );
	fNoTransparency = GetConfigBoolean( CFG_NOTRANSPARENCY );
	fHideIfEmpty = GetConfigBoolean( CFG_HIDEIFEMPTY );
	fColorize = GetConfigBoolean( CFG_COLORIZE );
	
	if( fColorize )
	{
		crFrom = GetConfigColor( CFG_COLORFROM, RGB( 0, 0, 0 ) );
		
		bFromR = GetRValue( crFrom );
		bFromG = GetGValue( crFrom );
		bFromB = GetBValue( crFrom );
		
		crTo = GetConfigColor( CFG_COLORTO, RGB( 255, 255, 255 ) );
		
		bToR = GetRValue( crTo );
		bToG = GetGValue( crTo );
		bToB = GetBValue( crTo );
	}
	
	GetConfigString( CFG_BITMAP, szLine, MAX_PATH, TEXT("") );
	
	if( szLine[0] )
	{
		p = NextToken( szLine, szBuffer, MAX_PATH );
		hbmSkin = LoadBitmapFile( szBuffer );
		
		if( hbmSkin )
		{
			GetObject( hbmSkin, sizeof(BITMAP), &bm );
			nBitmapX = bm.bmWidth;
			nBitmapY = bm.bmHeight;
		}
		
		p = NextToken( p, szBuffer, MAX_PATH );
		nBorderLeft = ParseInteger( szBuffer );
		
		p = NextToken( p, szBuffer, MAX_PATH );
		nBorderTop = ParseInteger( szBuffer );
		
		p = NextToken( p, szBuffer, MAX_PATH );
		nBorderRight = ParseInteger( szBuffer );
		
		p = NextToken( p, szBuffer, MAX_PATH );
		nBorderBottom = ParseInteger( szBuffer );
		
		p = NextToken( p, szBuffer, MAX_PATH );
		fSkinTiled = (szBuffer[0] && !lstrcmpi( szBuffer, TEXT("tiled") )) ? TRUE : FALSE;
	}
	
	nBorderX = GetConfigInteger( CFG_BORDERX, 0, 0, MAXLONG );
	nBorderY = GetConfigInteger( CFG_BORDERY, 0, 0, MAXLONG );
	
	nDir1 = DIR_RIGHT;
	nDir2 = DIR_UP;
	
	GetConfigString( CFG_DIRECTION, szLine, MAX_PATH, TEXT("") );
	
	if( szLine[0] )
	{
		p = NextToken( szLine, szBuffer, MAX_PATH );
		nDir1 = MapName( mapDirection, szBuffer, DIR_RIGHT );
		
		p = NextToken( p, szBuffer, MAX_PATH );
		nDir2 = MapName( mapDirection, szBuffer, DIR_UP );
	}
	
	nIconSize = GetConfigInteger( CFG_ICONSIZE, 16, 1, MAXLONG );
	
	nX = GetConfigInteger( CFG_X, -1, MINLONG, MAXLONG );
	nY = GetConfigInteger( CFG_Y, -1, MINLONG, MAXLONG );
	
	nResizeH = DIR_RIGHT;
	nResizeV = DIR_DOWN;
	
	GetConfigString( CFG_AUTOSIZE, szLine, MAX_PATH, TEXT("") );
	
	if( szLine[0] )
	{
		p = NextToken( szLine, szBuffer, MAX_PATH );
		nResizeH = MapName( mapLR, szBuffer, DIR_RIGHT );
		
		p = NextToken( p, szBuffer, MAX_PATH );
		nResizeV = MapName( mapUD, szBuffer, DIR_DOWN );
	}
	
	nSnapDistance = GetConfigInteger( CFG_SNAPDISTANCE, 4, 0, MAXLONG );
	
	nSpacingX = GetConfigInteger( CFG_SPACINGX, 2, 0, MAXLONG );
	nSpacingY = GetConfigInteger( CFG_SPACINGY, 2, 0, MAXLONG );	
	
	nWrapCount = GetConfigInteger( CFG_WRAPCOUNT, -1, -1, MAXLONG );
	
	nMinWidth = GetConfigInteger( CFG_MINWIDTH, -1, -1, MAXLONG );
	nMinHeight = GetConfigInteger( CFG_MINHEIGHT, -1, -1, MAXLONG );
	nMaxWidth = GetConfigInteger( CFG_MAXWIDTH, -1, -1, MAXLONG );
	nMaxHeight = GetConfigInteger( CFG_MAXHEIGHT, -1, -1, MAXLONG );
	
#ifndef OUTSIDER99
	nShortcutGroup = GetConfigInteger( CFG_DOCKTOSHORTCUTGROUP, -1, -1, MAXLONG );
#endif
	
	fHorizontal = ((nDir1 == DIR_LEFT) || (nDir1 == DIR_RIGHT)) ? TRUE : FALSE;
	
	if( fHorizontal )
	{
		nDeltaX = (nDir1 == DIR_LEFT) ? -1 : 1;
		nDeltaY = (nDir2 == DIR_DOWN) ? 1 : -1;
	}
	else
	{
		nDeltaX = (nDir2 == DIR_LEFT) ? -1 : 1;
		nDeltaY = (nDir1 == DIR_DOWN) ? 1 : -1;
	}
	
	nDeltaX *= nIconSize + nSpacingX;
	nDeltaY *= nIconSize + nSpacingY;
	
	nX = (nX < 0) ? (nScreenX + nX + 1) : nX;
	nY = (nY < 0) ? (nScreenY + nY + 1) : nY;
	
	nWrapCount = (nWrapCount < 0) ? -1 : ((nWrapCount == 0) ? 1 : nWrapCount);
	
	if( fDocked )
	{
		RECT r;
		GetClientRect( hWharfParent, &r );
		
		fAlwaysOnTop = FALSE;
		fBorderDrag = FALSE;
		
		nX = 0;
		nY = 0;
		
		nSnapDistance = 0;
		
		nMinWidth = r.right - r.left;
		nMinHeight = r.bottom - r.top;
		nMaxWidth = nMinWidth;
		nMaxHeight = nMinHeight;
	}
}
