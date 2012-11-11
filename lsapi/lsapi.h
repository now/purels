 /*

  This is a part of the LiteStep Shell Source code modified for use with
	PureLS Source code.

  Copyright (C) 1997-98 The LiteStep Development Team

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#ifndef  __LSAPI_H
#define __LSAPI_H

#include "wharfdata.h"
#include "resource.h"

#ifdef LSAPI_INTERNAL
  #define LSAPI __declspec(dllexport)
#else
  #define LSAPI
#endif

/* match defines */
#define MATCH_PATTERN  6    /* bad pattern */
#define MATCH_LITERAL  5    /* match failure on literal match */
#define MATCH_RANGE    4    /* match failure on [..] construct */
#define MATCH_ABORT    3    /* premature end of text string */
#define MATCH_END      2    /* premature end of pattern string */
#define MATCH_VALID    1    /* valid match */

/* pattern defines */
#define PATTERN_VALID  0    /* valid pattern */
#define PATTERN_ESC   -1    /* literal escape at end of pattern */
#define PATTERN_RANGE -2    /* malformed range in [..] construct */
#define PATTERN_CLOSE -3    /* no end bracket in [..] construct */
#define PATTERN_EMPTY -4    /* [..] contstruct is empty */

/* message defines */
#define LM_SHUTDOWN          8889
#define LM_REPAINT           8890
#define LM_BRINGTOFRONT      8891
#define LM_SAVEDATA			 8892
#define LM_RESTOREDATA		 8893
#define LM_POPUP             9182
#define LM_HIDEPOPUP         9183
#define LM_FIRSTDESKTOPPAINT 9184
#define LM_LSSELECT          9185
#define LM_SETTASKBARONTOP   9186
#define LM_SAVESYSTRAY       9210
#define LM_RESTORESYSTRAY    9211
#define LM_CHECKFORAPPBAR    9212
#define	LM_SENDSYSTRAY		 9213
#define LM_SYSTRAY           9214
#define LM_ADDWINDOW         9220
#define LM_REMOVEWINDOW      9221
#define LM_MINMAXWIN         9222
#define LM_ACTIVEWIN         9223
#define LM_RECYCLE           9260
#define LM_REGISTERMESSAGE   9263
#define LM_UNREGISTERMESSAGE 9264
#define LM_GETREVID			 9265
#define LM_UNLOADMODULE		 9266
#define LM_RELOADMODULE		 9267
#define LM_SHADETOGGLE       9300
#define LM_VWMUP             9350
#define LM_VWMDOWN           9351
#define LM_VWMLEFT           9352
#define LM_VWMRIGHT          9353
#define LM_VWMNAV            9354
#define LM_SWITCHTON         9355
#define LM_WINLIST           9400
#define LM_BANGCOMMAND       9420

#define LMBC_MAXCOMMAND  64
#define LMBC_MAXARGS     256

typedef struct _LMBANGCOMMANDA
{
	UINT cbSize;
	HWND hWnd;
	CHAR szCommand[LMBC_MAXCOMMAND];
	CHAR szArgs[LMBC_MAXARGS];
} LMBANGCOMMANDA, *PLMBANGCOMMANDA;

#define LMBANGCOMMAND    LMBANGCOMMANDA
#define PLMBANGCOMMAND   PLMBANGCOMMANDA

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

LSAPI FILE* LCOpen (LPCTSTR szPath);
LSAPI BOOL LCClose (FILE *f);
LSAPI BOOL LCReadNextCommand (FILE *f, LPSTR szBuffer, DWORD dwLength);
LSAPI BOOL LCReadNextConfig (FILE *f, LPCSTR szPrefix, LPSTR szBuffer, DWORD dwLength);
//LSAPI BOOL LCReadNextLine (FILE *f, LPSTR szBuffer, DWORD dwLength);
LSAPI int LCTokenize (LPCSTR szString, char **lpszBuffers, DWORD dwNumBuffers, LPSTR szExtraParameters);

LSAPI BOOL SetupRC(LPCTSTR szPath);
LSAPI void CloseRC(void);

LSAPI int GetRCInt(LPCTSTR lpKeyName, int nDefault);
LSAPI BOOL GetRCString(LPCTSTR lpKeyName, LPSTR value, LPCTSTR defStr, int maxLen);
LSAPI BOOL GetRCBool(LPCTSTR lpKeyName, BOOL ifFound);
LSAPI BOOL GetRCLine( LPCTSTR, LPTSTR, UINT, LPCTSTR );
LSAPI COLORREF GetRCColor(LPCTSTR lpKeyName, COLORREF colDef);

LSAPI BOOL AddBangCommand(char *command, void * f);
LSAPI BOOL AddBangCommandEx(char *command, void * f);
LSAPI BOOL RemoveBangCommand(char *command);
LSAPI BOOL ParseBangCommand (HWND caller, char *command, char *args);

LSAPI HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance, int xoffset, int yoffset);
LSAPI HBITMAP BitmapFromIcon (HICON hIcon);
LSAPI HBITMAP LoadLSImage(LPCSTR szImage, LPCSTR szFile);
LSAPI HICON LoadLSIcon (LPCSTR szImage, LPCSTR szFile);
LSAPI void GetLSBitmapSize(HBITMAP hBitmap, int *x, int *y);
LSAPI void TransparentBltLS (HDC dc, int nXDest, int nYDest, int nWidth, int nHeight, HDC tempDC, int nXSrc, int nYSrc, COLORREF colorTransparent);

LSAPI int CommandTokenize (LPCSTR szString, LPSTR *lpszBuffers, DWORD dwNumBuffers, LPSTR szExtraParameters);
LSAPI void CommandParse(LPCSTR cmd, LPSTR cmdbuf, LPSTR argsbuf, DWORD dwCmdBufSize, DWORD dwArgsBufSize);

LSAPI HINSTANCE LSExecute(HWND Owner, LPCSTR szCommand, int nShowCmd);
LSAPI HINSTANCE LSExecuteEx(HWND Owner, LPCSTR szOperation, LPCSTR szCommand, LPCSTR szArgs, LPCSTR szDirectory, int nShowCmd);

LSAPI HWND GetLitestepWnd();
LSAPI BOOL WINAPI LSGetLitestepPath( LPTSTR, UINT );
LSAPI BOOL WINAPI LSGetImagePath( LPTSTR, UINT );
LSAPI void VarExpansion(char *buffer, const char * value);
LSAPI void Frame3D(HDC dc, RECT rect, COLORREF TopColor, COLORREF BottomColor, int Width);
LSAPI void SetDesktopArea(int left, int top, int right, int bottom);

LSAPI BOOL match (char *pattern, char *text);
LSAPI int matche(char *pattern, char *text);
LSAPI BOOL is_pattern (char *pattern);
LSAPI BOOL is_valid_pattern (char *pattern, int *error_type);

#ifdef	__cplusplus
};
#endif	/* __cplusplus */

#endif  /* __LSAPI_H */
