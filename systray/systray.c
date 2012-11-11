
////
/// system tray
//

#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <mmsystem.h>

#ifndef OUTSIDER99
#include <stdio.h>
#include "lsapi.h"
#endif

#include "registry.h"
#include "systray.h"

// not in my shellapi.h?
// EXTERN_C HICON WINAPI DuplicateIcon( HINSTANCE, HICON );

HICON MyCopyIcon( HICON );
void ApplyFilter( HBITMAP );

#define CLR_TRANSPARENT  RGB( 255, 0, 255 )

/*
// NID struct for shell version 4 (9x, NT4)
typedef struct _NID4 {
	DWORD cbSize;
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
	CHAR szTip[64];
} NID4, *PNID4;

#define NID4_SIZE sizeof(NID4)

// NID struct for shell version 5 (2K)
typedef struct _NID5 {
	DWORD cbSize;
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
	CHAR szTip[128];
	DWORD dwState;
	DWORD dwStateMask;
	CHAR szInfo[256];
	union {
		UINT  uTimeout;
		UINT  uVersion;
	};
	CHAR   szInfoTitle[64];
	DWORD dwInfoFlags;
} NID5, *PNID5;

#define NID5_SIZE sizeof(NID5)
*/

// internal NID struct
#define LEN_TOOLTIP    64
// #define LEN_TOOLTIP    128
#define LEN_INFO       256
#define LEN_INFOTITLE  64

#define NIM_SETFOCUS		3
#define NIM_SETVERSION	4

typedef struct _SYSTRAYICON SYSTRAYICON;
typedef SYSTRAYICON *PSYSTRAYICON;

struct _SYSTRAYICON {
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
	TCHAR szTip[LEN_TOOLTIP];
//	DWORD dwState;
//	DWORD dwStateMask;
//	TCHAR szInfo[LEN_INFO];
//	UINT uTimeout;
//	UINT uVersion;
//	TCHAR szInfoTitle[LEN_INFOTITLE];
//	DWORD dwInfoFlags;
	RECT rc;
	UINT uToolTip;
	HICON hOriginalIcon;
	PSYSTRAYICON pNext;
};

PSYSTRAYICON pIconList;
int cIcons;

HWND hToolTip;
UINT uLastID = 1;

int nSizeX;
int nSizeY;

HBITMAP hbmSaved = NULL;
HBITMAP hbmBuffer = NULL;
HDC hdcBuffer = NULL;
BOOL fMoveTemp;

//
// AdjustLayout
//

void AdjustLayout()
{
	// boy is this function a mess =)
	// ... but, it works!
	
	PSYSTRAYICON psti = pIconList;
	int nOriginX, nOriginY;
	int nCurrentX, nCurrentY;
	int nActualX, nActualY;
	int nCount = 0;
	
	if( fHorizontal )
	{
		nSizeX = (nWrapCount > 0) ? min( cIcons, nWrapCount ) : cIcons;
		nSizeX = (nSizeX * (nIconSize + nSpacingX)) - nSpacingX;
		
		nSizeY = (nWrapCount > 0) ? ((((cIcons > 1 ? cIcons : 1) - 1) / nWrapCount) + 1) : 1;
		nSizeY = (nSizeY * (nIconSize + nSpacingY)) - nSpacingY;
	}
	else
	{
		nSizeX = (nWrapCount > 0) ? ((((cIcons > 1 ? cIcons : 1) - 1) / nWrapCount) + 1) : 1;
		nSizeX = (nSizeX * (nIconSize + nSpacingX)) - nSpacingX;
		
		nSizeY = (nWrapCount > 0) ? min( cIcons, nWrapCount ) : cIcons;
		nSizeY = (nSizeY * (nIconSize + nSpacingY)) - nSpacingY;
	}
	
	if( nSizeX )
		nSizeX += nBorderLeft + nBorderRight + (nBorderX * 2);
	
	if( nSizeY )
		nSizeY += nBorderTop + nBorderBottom + (nBorderY * 2);
	
	nSizeX = (nMinWidth > 0) ? max( nMinWidth, nSizeX ) : nSizeX;
	nSizeX = (nMaxWidth > 0) ? min( nMaxWidth, nSizeX ) : nSizeX;
	nSizeY = (nMinHeight > 0) ? max( nMinHeight, nSizeY ) : nSizeY;
	nSizeY = (nMaxHeight > 0) ? min( nMaxHeight, nSizeY ) : nSizeY;
	
	if( nDeltaX < 0 )
	{
		if( nDeltaY < 0 )
		{
			nOriginX = nSizeX - nIconSize - nBorderRight - nBorderX;
			nOriginY = nSizeY - nIconSize - nBorderBottom - nBorderY;
		}
		else
		{
			nOriginX = nSizeX - nIconSize - nBorderRight - nBorderX;
			nOriginY = nBorderTop + nBorderY;
		}
	}
	else
	{
		if( nDeltaY < 0 )
		{
			nOriginX = nBorderLeft + nBorderX;
			nOriginY = nSizeY - nIconSize - nBorderBottom - nBorderY;
		}
		else
		{
			nOriginX = nBorderLeft + nBorderX;
			nOriginY = nBorderTop + nBorderY;
		}
	}
	
	nCurrentX = nOriginX;
	nCurrentY = nOriginY;
	
	while( psti )
	{
		psti->rc.left = nCurrentX;
		psti->rc.top = nCurrentY;
		psti->rc.right = nCurrentX + nIconSize;
		psti->rc.bottom = nCurrentY + nIconSize;
		
		if( psti->uToolTip )
		{
			TOOLINFO ti;
			
			ti.cbSize = sizeof(TOOLINFO);
			ti.hwnd = hSystray;
			ti.uId = psti->uToolTip;
			ti.rect = psti->rc;
			
			SendMessage( hToolTip, TTM_NEWTOOLRECT, 0, (LPARAM) &ti );
		}
		
		nCount++;
		
		if( (nWrapCount > 0) && (nCount >= nWrapCount) )
		{
			if( fHorizontal )
				nOriginY += nDeltaY;
			else
				nOriginX += nDeltaX;
			
			nCurrentX = nOriginX;
			nCurrentY = nOriginY;
			
			nCount = 0;
		}
		else
		{
			if( fHorizontal )
				nCurrentX += nDeltaX;
			else
				nCurrentY += nDeltaY;
		}
		
		psti = psti->pNext;
	}
	
	nActualX = (nResizeH == DIR_LEFT) ? (nX - nSizeX) : nX;
	nActualY = (nResizeV == DIR_UP) ? (nY - nSizeY) : nY;
	
	if( nActualX < 0 )
		nActualX = 0;
	else if( (nActualX + nSizeX) > nScreenX )
		nActualX = nScreenX - nSizeX;

	if( nActualY < 0 )
		nActualY = 0;
	else if( (nActualY + nSizeY) > nScreenY )
		nActualY = nScreenY - nSizeY;
	
	fMoveTemp = TRUE;
	
	SetWindowPos( hSystray,
		NULL,
		nActualX,
		nActualY,
		nSizeX,
		nSizeY,
		SWP_NOZORDER | SWP_NOACTIVATE );
	
	if( fHideIfEmpty )
	{
		if( !cIcons )
		{
			if( fVisible )
				ShowWindow( hSystray, SW_HIDE );
		}
		else
		{
			if( fVisible && !IsWindowVisible( hSystray ) )
				ShowWindow( hSystray, SW_SHOWNOACTIVATE );
		}
	}
	
	fMoveTemp = FALSE;
	InvalidateRect( hSystray, NULL, FALSE );
}

//
// AddIcon
//

PSYSTRAYICON AddIcon( BOOL fRedraw )
{
	PSYSTRAYICON pIcon;
	PSYSTRAYICON pLast = pIconList;
	
	pIcon = (PSYSTRAYICON) LocalAlloc( LPTR, sizeof(SYSTRAYICON) );
	pIcon->pNext = NULL;
	
	if( !pIconList )
		pIconList = pIcon;
	else
	{
		while( pLast->pNext )
			pLast = pLast->pNext;
		
		pLast->pNext = pIcon;
	}
	
	cIcons++;
	
	if( fRedraw )
		AdjustLayout();
	
	return pIcon;
}

//
// SearchForIcon
//

PSYSTRAYICON SearchForIcon( HWND hWnd, UINT uID, BOOL fAdd )
{
	PSYSTRAYICON psti = pIconList;
	
	while( psti )
	{
		if( psti->hWnd == hWnd && psti->uID == uID )
			return psti;
		
		psti = psti->pNext;
	}
	
	if( !fAdd )
		return NULL;
	
	return AddIcon( TRUE );
}

//
// RemoveIcon
//

BOOL RemoveIcon( HWND hWnd, UINT uID )
{
	PSYSTRAYICON pIcon = pIconList;
	PSYSTRAYICON pPrev = NULL;
	
	while( pIcon )
	{
		if( pIcon->hWnd == hWnd && pIcon->uID == uID )
		{
			if( pPrev )
				pPrev->pNext = pIcon->pNext;
			else
				pIconList = pIcon->pNext;
			
			LocalFree( (LPVOID) pIcon );
			cIcons--;
			
			AdjustLayout();
			return TRUE;
		}
		
		pPrev = pIcon;
		pIcon = pIcon->pNext;
	}
	
	return FALSE;
}

//
// FreeIconList
//

void FreeIconList()
{
	PSYSTRAYICON pIcon = pIconList;
	PSYSTRAYICON pNext;
	
	while( pIcon )
	{
		pNext = pIcon->pNext;
		LocalFree( (LPVOID) pIcon );
		pIcon = pNext;
	}
	
	pIconList = NULL;
	cIcons = 0;
}

//
// OmniBlt (all-in-one blt function)
//

BOOL OmniBlt( HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, BOOL fTile, COLORREF crTrans )
{
	// this function selects the appropriate blt
	// method (tile or stretch). makes SkinTray
	// simpler.
	
	BOOL fResult = TRUE;
	
	if( fTile )
	{
		int nWidth = cxDest;
		int nHeight = cyDest;
		int x, y;
		
		for( y = 0; y < nHeight; y += cySrc )
		{
			cxDest = nWidth;
			
			for( x = 0; x < nWidth; x += cxSrc )
			{
				if( crTrans != CLR_NONE )
					TransBlt( hdcDest, xDest + x, yDest + y, min( cxDest, cxSrc ), min( cyDest, cySrc ), hdcSrc, xSrc, ySrc, crTrans );
				else
					fResult &= BitBlt( hdcDest, xDest + x, yDest + y, min( cxDest, cxSrc ), min( cyDest, cySrc ), hdcSrc, xSrc, ySrc, SRCCOPY );
				
				cxDest -= cxSrc;
			}
			
			cyDest -= cySrc;
		}
	}
	else
	{
		fResult = StretchBlt( hdcDest, xDest, yDest, cxDest, cyDest, hdcSrc, xSrc, ySrc, cxSrc, cySrc, SRCCOPY );
	}
	
	return fResult;
}

//
// SkinTray
//

BOOL SkinTray( HDC hDC )
{
	// very ugly, but it works...
	
	HDC hdcMem;
	HBITMAP hbmOld;
	
	if( fDocked )
	{
		hdcMem = CreateCompatibleDC( hDC );
		
		if( !hbmSaved )
		{
			hbmSaved = CreateCompatibleBitmap( hDC, nSizeX, nSizeY );
			hbmOld = (HBITMAP) SelectObject( hdcMem, hbmSaved );
			
			BitBlt( hdcMem, 0, 0, nSizeX, nSizeY, hDC, 0, 0, SRCCOPY );
			SelectObject( hdcMem, hbmOld );
		}
		
		hbmOld = (HBITMAP) SelectObject( hdcMem, hbmSaved );
		BitBlt( hDC, 0, 0, nSizeX, nSizeY, hdcMem, 0, 0, SRCCOPY );
		
		SelectObject( hdcMem, hbmOld );
		DeleteDC( hdcMem );
		
		return TRUE;
	}
		
	if( hbmSkin )
	{
		hdcMem = CreateCompatibleDC( hDC );
		hbmOld = (HBITMAP) SelectObject( hdcMem, hbmSkin );
		
		SetStretchBltMode( hDC, STRETCH_DELETESCANS );
		
		if( nBorderTop )
		{
			// topleft corner
			TransBlt( hDC, 0, 0, nBorderLeft, nBorderTop, hdcMem, 0, 0,
				CLR_TRANSPARENT );
			
			// top edge
			OmniBlt( hDC, nBorderLeft, 0, nSizeX - nBorderLeft - nBorderRight,
				nBorderTop, hdcMem, nBorderLeft, 0, nBitmapX - nBorderLeft -
				nBorderRight, nBorderTop, fSkinTiled, CLR_TRANSPARENT );
			
			// topright corner
			TransBlt( hDC, nSizeX - nBorderRight, 0, nBorderRight, nBorderTop,
				hdcMem, nBitmapX - nBorderRight, 0, CLR_TRANSPARENT );
		}
		
		if( nBorderLeft )
		{
			// left edge
			OmniBlt( hDC, 0, nBorderTop, nBorderLeft, nSizeY - nBorderTop -
				nBorderBottom, hdcMem, 0, nBorderTop, nBorderLeft, nBitmapY -
				nBorderTop - nBorderBottom, fSkinTiled, CLR_TRANSPARENT );
		}
		
		// background
		OmniBlt( hDC, nBorderLeft, nBorderTop, nSizeX - nBorderLeft -
			nBorderRight, nSizeY - nBorderTop - nBorderBottom, hdcMem,
			nBorderLeft, nBorderTop, nBitmapX - nBorderLeft - nBorderRight,
			nBitmapY - nBorderTop - nBorderBottom, fSkinTiled, CLR_TRANSPARENT );
		
		if( nBorderRight )
		{
			// right edge
			OmniBlt( hDC, nSizeX - nBorderRight, nBorderTop, nBorderRight,
				nSizeY - nBorderTop - nBorderBottom, hdcMem, nBitmapX -
				nBorderRight, nBorderTop, nBorderRight, nBitmapY - nBorderTop -
				nBorderBottom, fSkinTiled, CLR_TRANSPARENT );
		}
		
		if( nBorderBottom )
		{
			// bottomleft corner
			TransBlt( hDC, 0, nSizeY - nBorderBottom, nBorderLeft, nBorderBottom,
				hdcMem, 0, nBitmapY - nBorderBottom, CLR_TRANSPARENT );
			
			// bottom edge
			OmniBlt( hDC, nBorderLeft, nSizeY - nBorderBottom, nSizeX -
				nBorderLeft - nBorderRight, nBorderBottom, hdcMem, nBorderLeft,
				nBitmapY - nBorderBottom, nBitmapX - nBorderLeft - nBorderRight,
				nBorderBottom, fSkinTiled, CLR_TRANSPARENT );
			
			// bottomright corner
			TransBlt( hDC, nSizeX - nBorderRight, nSizeY - nBorderBottom,
				nBorderRight, nBorderBottom, hdcMem, nBitmapX - nBorderRight,
				nBitmapY - nBorderBottom, CLR_TRANSPARENT );
		}
		
		SelectObject( hdcMem, hbmOld );
		DeleteDC( hdcMem );
	}
	
	return TRUE;
}

//
// SystrayProc
//

LRESULT WINAPI SystrayProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam )
{
	switch( nMessage )
	{
#ifndef OUTSIDER99
		case LM_RESTORESYSTRAY:
		{
			PNOTIFYICONDATA pnid;
			UINT uLength;
			int n;
			int nCount;
			
			nCount = RegQueryLongValue( HKEY_CURRENT_USER, REGKEY_SAVEDDATA,
				TEXT("IconCount"), 0 );
			
			if( !nCount )
				return 0;
			
			uLength = nCount * sizeof(NOTIFYICONDATA);
			pnid = (PNOTIFYICONDATA) LocalAlloc( LPTR, uLength );
			
			if( !pnid )
				return 0;
			
			RegQueryBinaryValue( HKEY_CURRENT_USER, REGKEY_SAVEDDATA,
				TEXT("Icons"), (LPBYTE) pnid, uLength );
			
			for( n = 0; n < nCount; n++ )
			{
				if( IsWindow( pnid[n].hWnd ) )
					SendMessage( hWnd, LM_SYSTRAY, NIM_ADD, (LPARAM) &pnid[n] );
				
				if( pnid[n].uFlags & NIF_ICON )
					DestroyIcon( pnid[n].hIcon );
			}
			
			RegDeleteKey( HKEY_CURRENT_USER, REGKEY_SAVEDDATA );
			
			LocalFree( (HLOCAL) pnid );
			return 0;
		}
		
		case LM_SENDSYSTRAY:
		{
			// tray saver defines this function, but it looks as
			// if it sends the message directly to the desktop
			// window rather than through LS's message routing.
			// hence this is probably never called, oh well...
			
			PSYSTRAYICON pIcon = pIconList;
			NOTIFYICONDATA nid;
			COPYDATASTRUCT cds;
			int nCount = 0;
			
			cds.dwData = wParam;
			cds.cbData = sizeof(NOTIFYICONDATA);
			
			while( pIcon )
			{
				nid.cbSize = sizeof(NOTIFYICONDATA);
				nid.uID = pIcon->uID;
				nid.uFlags = pIcon->uFlags;
				nid.uCallbackMessage = pIcon->uCallbackMessage;
				nid.hIcon = pIcon->hOriginalIcon;
				nid.szTip[0] = 0;
				
				if( nid.uFlags & NIF_TIP )
					lstrcpyn( nid.szTip, pIcon->szTip, 64 );
				
				cds.lpData = (LPVOID) &nid;
				SendMessage( (HWND) lParam, WM_COPYDATA, (WPARAM) hWnd, (LPARAM) &cds );
				
				pIcon = pIcon->pNext;
				nCount++;
			}
			
			return nCount;
		}
		
		case LM_SAVESYSTRAY:
		{
			PSYSTRAYICON psti = pIconList;
			PNOTIFYICONDATA pnid;
			UINT uLength;
			int n = 0;
			
			if( !cIcons )
				return 0;
			
			RegSetLongValue( HKEY_CURRENT_USER, REGKEY_SAVEDDATA,
				TEXT("IconCount"), cIcons );
			
			uLength = cIcons * sizeof(NOTIFYICONDATA);
			pnid = (PNOTIFYICONDATA) LocalAlloc( LPTR, uLength );
			
			if( !pnid )
				return 0;
			
			while( psti )
			{
				pnid[n].cbSize = sizeof(NOTIFYICONDATA);
				pnid[n].hWnd = psti->hWnd;
				pnid[n].uID = psti->uID;
				pnid[n].uFlags = psti->uFlags;
				pnid[n].uCallbackMessage = psti->uCallbackMessage;
				pnid[n].hIcon = psti->hIcon;
				pnid[n].szTip[0] = 0;
				
				if( pnid[n].uFlags & NIF_TIP )
					lstrcpyn( pnid[n].szTip, psti->szTip, 64 );
				
				n++;
				psti = psti->pNext;
			}
			
			RegSetBinaryValue( HKEY_CURRENT_USER, REGKEY_SAVEDDATA,
				TEXT("Icons"), (CONST BYTE *) pnid, uLength );
			
			LocalFree( (HLOCAL) pnid );
			return 0;
		}
#endif
		case LM_SYSTRAY:
		{
			PNOTIFYICONDATA pnid = (PNOTIFYICONDATA) lParam;
			PSYSTRAYICON psti;
			TOOLINFO ti;
			
			switch( wParam )
			{
				case NIM_ADD:
				{
					if( !IsWindow( pnid->hWnd ) )
						return FALSE;
					
					if( SearchForIcon( pnid->hWnd, pnid->uID, FALSE ) )
						return SendMessage( hWnd, LM_SYSTRAY, NIM_MODIFY, lParam );
					
					psti = SearchForIcon( pnid->hWnd, pnid->uID, TRUE );
					
					if( !psti )
						return FALSE;
					
					// by far the most annoying thing I ran into
					// in this project was trying to find a good
					// way to copy icons.
					//
					// first I tried CopyImage(LR_COPYFROMRESOURCE)
					// figuring it would do better when using
					// different icon sizes, but there are a few
					// apps that it fails on.
					//
					// CopyIcon seems to work in the majority of
					// apps (so far I've only found one that it
					// fails on). the problem is if I store the
					// original hIcon the app gave us when recycled
					// trying CopyIcon again fails. the workaround
					// I devised is to store the copy, then call
					// CopyIcon on the copy (hey, it works).
					//
					// I also tried to directly get the bits of
					// the icon with GetIconInfo and then create a
					// new icon with CreateIconIndirect. but oddly
					// enough some icons cause GetIconInfo to fail.
					//
					// in the end I chose CopyIcon, as it works in
					// most cases (at least that I've tried) and it
					// is also the method LS uses.
					
					// update (08-01-1999)
					//
					// while digging through some API dox I stumbled
					// accros yet another function: DuplicateIcon.
					// this one is exported by shell32.dll. my SDK
					// dox (that came w/ VC5) didn't have an entry
					// for it, tho according the dox on MSDN online,
					// the function has existed since 95. I checked
					// and this appears to be true.
					//
					// so I am going to try it, but not now. at the
					// time I found this I was already finishing up
					// for 1.0 final and I don't want to introduce
					// such a change now. so, I'll try DuplicateIcon
					// when I begin work on 2.0.
					
					psti->hWnd = pnid->hWnd;
					psti->uID = pnid->uID;
					psti->uFlags = pnid->uFlags;
					psti->uCallbackMessage = pnid->uCallbackMessage;
					psti->hOriginalIcon = pnid->hIcon;
					psti->hIcon = MyCopyIcon( pnid->hIcon );
					psti->szTip[0] = 0;
					psti->uToolTip = uLastID++;
					
					if( psti->uFlags & NIF_TIP )
						lstrcpyn( psti->szTip, pnid->szTip, 64 );
					
					ti.cbSize = sizeof(TOOLINFO);
					ti.uFlags = 0;
					ti.hwnd = hWnd;
					ti.uId = psti->uToolTip;
					ti.rect = psti->rc;
					ti.hinst = NULL;
					ti.lpszText = (psti->uFlags & NIF_TIP) ? psti->szTip : NULL;
					
					SendMessage( hToolTip, TTM_ADDTOOL, 0, (LPARAM) &ti );
					InvalidateRect( hWnd, NULL, FALSE );
					
					return TRUE;
				}
				
				case NIM_MODIFY:
				{
					psti = SearchForIcon( pnid->hWnd, pnid->uID, FALSE );
					
					if( !psti )
						return SendMessage( hWnd, LM_SYSTRAY, NIM_ADD, lParam );
					
					if( pnid->uFlags & NIF_MESSAGE )
					{
						psti->uFlags |= NIF_MESSAGE;
						psti->uCallbackMessage = pnid->uCallbackMessage;
					}
					
					if( pnid->uFlags & NIF_TIP )
					{
						psti->uFlags |= NIF_TIP;
						lstrcpyn( psti->szTip, pnid->szTip, 64 );
						
						ti.cbSize = sizeof(TOOLINFO);
						ti.hwnd = hWnd;
						ti.uId = psti->uToolTip;
						ti.hinst = NULL;
						ti.lpszText = psti->szTip;
						
						SendMessage( hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM) &ti );
					}
					
					if( pnid->uFlags & NIF_ICON )
					{
						if( psti->uFlags & NIF_ICON )
							DestroyIcon( psti->hIcon );
						
						psti->uFlags |= NIF_ICON;
						psti->hOriginalIcon = pnid->hIcon;
						psti->hIcon = MyCopyIcon( pnid->hIcon );
						
						InvalidateRect( hWnd, &psti->rc, FALSE );
					}
					
					return TRUE;
				}
				
				case NIM_DELETE:
				{
					psti = SearchForIcon( pnid->hWnd, pnid->uID, FALSE );
					
					if( !psti )
						return FALSE;
					
					// the Win32 SDK documentation for CopyIcon never
					// says if you should free the returned icon and
					// the DestroyIcon dox say it's only necessary to
					// call it for icons created w/ CreateIconIndirect.
					// however, when I don't use DestroyIcon I get mem
					// leaks... oh well.
					
					if( psti->uFlags & NIF_ICON )
						DestroyIcon( psti->hIcon );
					
					ti.cbSize = sizeof(TOOLINFO);
					ti.hwnd = hWnd;
					ti.uId = psti->uToolTip;
					
					SendMessage( hToolTip, TTM_DELTOOL, 0, (LPARAM) &ti );
					return RemoveIcon( pnid->hWnd, pnid->uID );
				}
				
				case NIM_SETVERSION:
				{
					return TRUE;
				}
				
				case NIM_SETFOCUS:
				{
					return TRUE;
				}
			}
			
			return FALSE;
		}
		
		case WM_CLOSE:
		{
			return 0;
		}
		
		case WM_CREATE:
		{
			UINT nTaskbarMessage;
			
			pIconList = NULL;
			cIcons = 0;
			
			nSizeX = 0;
			nSizeY = 0;
			
			fMoveTemp = FALSE;
			
			hToolTip = CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				TOOLTIPS_CLASS,
				NULL,
				TTS_ALWAYSTIP | WS_POPUP,
				0,
				0,
				0,
				0,
				hWnd,
				NULL,
				NULL,
				NULL );
			
			// Win98/IE4 stuff
			nTaskbarMessage = RegisterWindowMessage( TEXT("TaskbarCreated") );
			PostMessage( HWND_BROADCAST, nTaskbarMessage, 0, 0 );
			
			return 0;
		}
		
		case WM_DESTROY:
		{
			DestroyWindow( hToolTip );
			FreeIconList();
			
			if( hbmSaved )
				DeleteObject( hbmSaved );
			
		#ifdef OUTSIDER99
			PostQuitMessage( FALSE );
		#endif
		
			return 0;
		}
		
		case WM_ENDSESSION:
		{
		#ifndef OUTSIDER99
			if( wParam )
				RegDeleteKey( HKEY_CURRENT_USER, REGKEY_SAVEDDATA );
		#endif
		
			return 0;
		}
		
		case WM_ERASEBKGND:
		{
			if( !fDocked && !fNoTransparency )
				PaintDesktop( (HDC) wParam );
			
			return SkinTray( (HDC) wParam );
		}
		
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:		// whew!
		{
			PSYSTRAYICON psti = pIconList;
			POINT pt;
			MSG msg;
			
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			
			msg.hwnd = hWnd;
			msg.message = nMessage;
			msg.wParam = wParam;
			msg.lParam = lParam;
			msg.time = GetTickCount();
			msg.pt = pt;
			
			SendMessage( hToolTip, TTM_RELAYEVENT, 0, (LPARAM) &msg );
			
			while( psti )
			{
				if( PtInRect( &psti->rc, pt ) )
				{
					if( !IsWindow( psti->hWnd ) )
					{
						NOTIFYICONDATA nid;
						
						nid.cbSize = sizeof(NOTIFYICONDATA);
						nid.hWnd = psti->hWnd;
						nid.uID = psti->uID;
						nid.uFlags = 0;
						
						SendMessage( hWnd, LM_SYSTRAY, NIM_DELETE, (LPARAM) &nid );
					}
					else if( psti->uFlags & NIF_MESSAGE )
					{
						PostMessage( psti->hWnd, psti->uCallbackMessage,
							(WPARAM) psti->uID, (LPARAM) nMessage );
					}
					
					return 0;
				}
				
				psti = psti->pNext;
			}
			
			if( fBorderDrag && nMessage == WM_LBUTTONDOWN )
				SendMessage( hWnd, WM_SYSCOMMAND, SC_MOVE | 2, lParam );
			
		#ifndef OUTSIDER99
			if( nMessage == WM_RBUTTONUP )
			{
				POINT pt;
				
				GetCursorPos( &pt );
				PostMessage( hLitestep, LM_POPUP, (WPARAM) pt.x, (LPARAM) pt.y );
			}
		#endif
		
			return 0;
		}
		
		case WM_MOUSEACTIVATE:
		{
			return MA_NOACTIVATE;
		}
		
		case WM_MOVE:
		{
			if( !fMoveTemp )
			{
				nX = LOWORD(lParam);
				nY = HIWORD(lParam);
				
				if( nResizeH == DIR_LEFT )
					nX = nX + nSizeX;
				
				if( nResizeV == DIR_UP )
					nY = nY + nSizeY;
				
				if( nSnapDistance )
				{
					if( nResizeH == DIR_RIGHT )
					{
						if( nX > (nScreenX - nSizeX - nSnapDistance) )
							nX = nScreenX;
					}
					
					if( nResizeV == DIR_DOWN )
					{
						if( nY > (nScreenY - nSizeY - nSnapDistance) )
							nY = nScreenY;
					}
				}				
			}
			
			if( !hbmSkin )
				InvalidateRect( hWnd, NULL, FALSE );
			
			return 0;
		}
		
		case WM_EXITSIZEMOVE:
		{
			if( hbmSkin )
				InvalidateRect( hWnd, NULL, FALSE );
			
			return 0;
		}
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDC;
			PSYSTRAYICON psti = pIconList;
			
			if( !wParam )
				hDC = BeginPaint( hWnd, &ps );
			else
				hDC = (HDC) wParam;
			
			hbmBuffer = (HBITMAP) SelectObject( hdcBuffer, hbmBuffer );
			
			// PaintDesktop is needed in order to support pseudo
			// transparency, but it is quite limited. it uses
			// internal data stored in the DC (which cannot be
			// set by APIs) to determine which part of the desktop
			// to paint and cannot draw a piece into a memory DC.
			// in deskfolders I tried to workaround it by copying
			// entire desktop image to a memory DC and then BitBlt
			// the part I needed. tho it wastes memory, it worked
			// fine for me and for lots of people. but for some
			// people it generated garbage. from what I can tell
			// on some computers PaintDesktop simply fails if the
			// given DC is not a display DC.
			//
			// because of this I am forced to always PaintDesktop
			// to the paint DC, then copy it into the double buffer
			// DC. the result is flicker whenever transparency is
			// enabled. I've had some ideas for ways around it, but
			// I'm not sure if they're worth it... we'll see.
			
			if( !fDocked && !fNoTransparency )
			{
				PaintDesktop( hDC );
				BitBlt( hdcBuffer, 0, 0, nSizeX, nSizeY, hDC, 0, 0, SRCCOPY );
			}
			
			SkinTray( hdcBuffer );
			
			while( psti )
			{
				if( !IsWindow( psti->hWnd ) )
				{
					NOTIFYICONDATA nid;
					
					nid.cbSize = sizeof(NOTIFYICONDATA);
					nid.hWnd = psti->hWnd;
					nid.uID = psti->uID;
					nid.uFlags = 0;
					
					SendMessage( hWnd, LM_SYSTRAY, NIM_DELETE, (LPARAM) &nid );
					break;
				}
				else if( psti->uFlags & NIF_ICON );
				{
					DrawIconEx( hdcBuffer, psti->rc.left, psti->rc.top, psti->hIcon,
						nIconSize, nIconSize, 0, NULL, DI_NORMAL );
				}
				
				psti = psti->pNext;
			}
				
			BitBlt( hDC, 0, 0, nSizeX, nSizeY, hdcBuffer, 0, 0, SRCCOPY );
			hbmBuffer = (HBITMAP) SelectObject( hdcBuffer, hbmBuffer );
			
			if( !wParam )
				EndPaint( hWnd, &ps );
			
			return 0;
		}
		
		case WM_SIZE:
		{
			HDC hDC = GetDC( NULL );
			
			if( !hdcBuffer )
				hdcBuffer = CreateCompatibleDC( hDC );
			
			if( hbmBuffer )
				DeleteObject( hbmBuffer );
			
			nSizeX = LOWORD( lParam );
			nSizeY = HIWORD( lParam );
			
			hbmBuffer = CreateCompatibleBitmap( hDC, nSizeX, nSizeY );
			ReleaseDC( NULL, hDC );
			
			return 0;
		}
		
		case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS pwp = (LPWINDOWPOS) lParam;
			
			if( !fAlwaysOnTop )
				pwp->flags |= SWP_NOZORDER;
			
			if( nSnapDistance )
			{
				if( pwp->x < nSnapDistance )
					pwp->x = 0;
				else if( pwp->x > (nScreenX - nSizeX - nSnapDistance) )
					pwp->x = nScreenX - nSizeX;
				
				if( pwp->y < nSnapDistance )
					pwp->y = 0;
				else if( pwp->y > (nScreenY - nSizeY - nSnapDistance) )
					pwp->y = nScreenY - nSizeY;
			}
			
			return 0;
		}
	}
	
	return DefWindowProc( hWnd, nMessage, wParam, lParam );
}

HICON MyCopyIcon( HICON hIcon )
{
	ICONINFO iconInfo;
	HICON hCopiedIcon;
	
	if( !hIcon )
		return NULL;
	
	SetLastError( 0 );
	
	if( !GetIconInfo( hIcon, &iconInfo ) )
	{
		char message[64];
		
		wsprintf( message, "GetIconInfo( %08X ): %d", hIcon, GetLastError() );
		// MessageBox( NULL, message, "System Tray", MB_SETFOREGROUND );
		
		return NULL;
	}
	
//	if( fColorize )
//		ApplyFilter( iconInfo.hbmColor );
	
	hCopiedIcon = CreateIconIndirect( &iconInfo );
	
	DeleteObject( iconInfo.hbmColor );
	DeleteObject( iconInfo.hbmMask );
	
	return hCopiedIcon;
}

