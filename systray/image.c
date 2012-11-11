
////
/// image routines
//

#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include "systray.h"

#ifndef OUTSIDER99
#include <stdio.h>
#include "lsapi.h"
#endif

//
// LoadBitmapFile
//

HBITMAP LoadBitmapFile( LPCTSTR pszFile )
{
#ifdef OUTSIDER99
	HBITMAP hBitmap;
	
	hBitmap = (HBITMAP) LoadImage( NULL,
		pszFile,
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE );
	
	if( !hBitmap )
	{
		TCHAR szPath[MAX_PATH];
		
		lstrcpy( szPath, szThemePath );
		lstrcat( szPath, pszFile );
		
		hBitmap = (HBITMAP) LoadImage( NULL,
			szPath,
			IMAGE_BITMAP,
			0,
			0,
			LR_LOADFROMFILE );
	}
	
	return hBitmap;
#else
	return LoadLSImage( pszFile, NULL );
#endif
}

//
// TransBlt
//

BOOL TransBlt( HDC hdcDest, int xDest, int yDest, int cx, int cy, HDC hdcSrc, int xSrc, int ySrc, COLORREF clrTrans )
{
	HDC hdcMask;
	HBITMAP hbmMask;
	HDC hdcMem;
	HBITMAP hbmMem;
	COLORREF clrBack;
	COLORREF clrText;
	
	hdcMask = CreateCompatibleDC( hdcDest );
	hbmMask = CreateBitmap( cx, cy, 1, 1, NULL );
	hbmMask = (HBITMAP) SelectObject( hdcMask, hbmMask );
	
	hdcMem = CreateCompatibleDC( hdcDest );
	hbmMem = CreateCompatibleBitmap( hdcDest, cx, cy );
	hbmMem = (HBITMAP) SelectObject( hdcMem, hbmMem );
	
	clrBack = SetBkColor( hdcSrc, clrTrans );
	BitBlt( hdcMask, 0, 0, cx, cy, hdcSrc, xSrc, ySrc, SRCCOPY );
	SetBkColor( hdcSrc, clrBack );
	
	clrBack = SetBkColor( hdcDest, 0xFFFFFF );
	clrText = SetTextColor( hdcDest, 0x000000 );
	
	BitBlt( hdcMem, 0, 0, cx, cy, hdcDest, xDest, yDest, SRCCOPY );
	BitBlt( hdcMem, 0, 0, cx, cy, hdcSrc, xSrc, ySrc, SRCINVERT );
	BitBlt( hdcMem, 0, 0, cx, cy, hdcMask, 0, 0, SRCAND );
	BitBlt( hdcMem, 0, 0, cx, cy, hdcSrc, xSrc, ySrc, SRCINVERT );
	BitBlt( hdcDest, xDest, yDest, cx, cy, hdcMem, 0, 0, SRCCOPY );
	
	SetBkColor( hdcDest, clrBack );
	SetTextColor( hdcDest, clrText );
	
	hbmMask = (HBITMAP) SelectObject( hdcMask, hbmMask );
	DeleteObject( hbmMask );
	DeleteDC( hdcMask );
	
	hbmMem = (HBITMAP) SelectObject( hdcMem, hbmMem );
	DeleteObject( hbmMem );
	DeleteDC( hdcMem );
	
	return TRUE;
}
