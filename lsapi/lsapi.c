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


#define _WIN32_IE 0x0000
#undef WIN32_LEAN_AND_MEAN
#define RPC_NO_WINDOWS_H


#include <tchar.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <shellapi.h>

#define LSAPI_INTERNAL
#include "lsapi.h"

typedef struct {
	char *name;
	void (*func)(HWND caller, char* args, ...);
	BOOL ex; // Killarny
} tBangCommand;

typedef struct {
	char program[255];
	char bitmap[255];
} tThemePic;

const char rcsRevision[] = "$Revision: pls - 0.30beta $"; // Our Version 
const char rcsId[] = "$Id: lsapi.c, pls - v0.30beta 2000/05/29 00:22:00 jugg $"; // The Full RCS ID.
static char **RCBuffer = NULL;
static char **ThemeBuffer = NULL;
static tThemePic *PicBuffer = NULL;
static int ThemePicLen = 0;
static int ThemeLen = 0;
static int RCLen = 0;
static int RCOpened = 0;
static BOOL ThemeInUse = FALSE;
static BOOL ColorBGR = FALSE;
static BOOL AboutDialog = FALSE;
char vbuffer[2048];

int matche_after_star (register char *pattern, register char *text);
int fast_match_after_star (register char *pattern, register char *text);

LRESULT CALLBACK About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void BangRecycle (HWND caller,char* args);
void BangRun (HWND caller,char* args);
void BangShutdown(HWND caller,char* args);
void BangLogoff(HWND caller,char* args);
void BangQuit(HWND caller,char* param);
void BangToggleWharf(HWND caller,char* args);
void BangGather(HWND caller,char* args);
void BangVWMDesk(HWND caller,char* args);
void BangVWMUp(HWND caller,char* args);
void BangVWMDown(HWND caller,char* args);
void BangVWMLeft(HWND caller,char* args);
void BangVWMRight(HWND caller,char* args);
void BangVWMNav(HWND caller,char* args);
void BangPopup(HWND caller,char* args);
void BangTileWindowsH(HWND caller, char *args);
void BangTileWindowsV(HWND caller, char *args);
void BangCascadeWindows(HWND caller, char *args);
void BangEasterEgg(HWND caller, char* args);
void BangMinimizeWindows(HWND caller, char *args);
void BangRestoreWindows(HWND caller, char *args);
void BangAbout(HWND caller, char *args);
void BangUnloadModule (HWND caller,char* args);
void BangReloadModule (HWND caller,char* args);
void BangExecute(HWND caller, char *args);

static tBangCommand BangCommands[] = {
	{ "!GATHER", BangGather, FALSE },
	{ "!RECYCLE", BangRecycle, FALSE },
	{ "!RUN", BangRun, FALSE },
	{ "!SHUTDOWN", BangShutdown, FALSE },
	{ "!TOGGLEWHARF", BangToggleWharf, FALSE },
	{ "!LOGOFF", BangLogoff, FALSE },
	{ "!QUIT", BangQuit, FALSE },
	{ "!VWMDESK", BangVWMDesk, FALSE },
	{ "!VWMDOWN", BangVWMDown, FALSE },
	{ "!VWMLEFT", BangVWMLeft, FALSE },
	{ "!VWMNAV", BangVWMNav, FALSE },
	{ "!VWMRIGHT", BangVWMRight, FALSE },
	{ "!VWMUP", BangVWMUp, FALSE },
	{ "!POPUP", BangPopup, FALSE },
	{ "!EASTEREGG",	BangEasterEgg, FALSE },
	{ "!TILEWINDOWSH", BangTileWindowsH, FALSE },
	{ "!TILEWINDOWSV", BangTileWindowsV, FALSE },
	{ "!CASCADEWINDOWS", BangCascadeWindows, FALSE },
	{ "!MINIMIZEWINDOWS", BangMinimizeWindows, FALSE },
	{ "!RESTOREWINDOWS", BangRestoreWindows, FALSE },
	{ "!ABOUT",	BangAbout, FALSE },
	{ "!UNLOADMODULE", BangUnloadModule, FALSE },
	{ "!RELOADMODULE", BangReloadModule, FALSE },
  { "!EXECUTE", BangExecute, FALSE }
};

static tBangCommand * BangAdded = NULL;
static int Bangs = 0;



#define MAX_BANG_COMMANDS       ((sizeof(BangCommands) / sizeof(tBangCommand)) - 0)
#define MAX_LINE_LENGTH 4096

TCHAR szFileName[MAX_PATH];

static BOOL LCReadNextLine (FILE *f, LPSTR szBuffer, DWORD dwLength);

FILE *LCOpen (LPCTSTR szPath)
{
	FILE *f = NULL;

	if (szPath)
		lstrcpy (szFileName, szPath);
	else{
		LSGetLitestepPath(szFileName, MAX_PATH);
		lstrcat(szFileName, "step.rc");
	}
	f = fopen (szFileName, "r");
	
	if (f)
		fseek (f, 0, SEEK_SET);
	return f;
}




BOOL LCClose (FILE *f)
{
	if (f)
		fclose (f);
	return TRUE;
}

BOOL LCReadNextCommand (FILE *f, LPSTR szBuffer, DWORD dwLength)
{
	char szTempBuffer[MAX_LINE_LENGTH];

	if (!f)
	{
		return FALSE;
	}

	while (LCReadNextLine (f, szTempBuffer, sizeof (szTempBuffer)))
	{
		if (!ispunct (szTempBuffer[0]))
		{
			strncpy (szBuffer, szTempBuffer, dwLength);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL LCReadNextConfig (FILE *f, LPCSTR szPrefix, LPSTR szBuffer, DWORD dwLength)
{
	char szTempPrefix[MAX_LINE_LENGTH], szTempBuffer[MAX_LINE_LENGTH];
	int prefix_length = 0;

	if (!f)
	{
		return FALSE;
	}

	szTempPrefix[0] = 0;

	if (szPrefix[0] != '*')
	{
		strcpy (szTempPrefix, "*");
	}

	strcat (szTempPrefix, szPrefix);
	strcat (szTempPrefix, " ");

	prefix_length = strlen (szTempPrefix);

	while (LCReadNextLine (f, szTempBuffer, sizeof (szTempBuffer)))
	{
		if (!_strnicmp (szTempBuffer, szTempPrefix, prefix_length))
		{
			strncpy (szBuffer, szTempBuffer, dwLength);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL LCReadNextLine (FILE *f, LPSTR szBuffer, DWORD dwLength)
{
	char szTempBuffer[MAX_LINE_LENGTH];

	while (f && !feof (f))
	{
		int length;

		strcpy(szBuffer, "");

		if (!fgets (szTempBuffer, dwLength, f))
		{
			break;
		}

		length = strlen (szTempBuffer);

		// Skip any leading and trailing whitespace
		while (length && isspace (szTempBuffer[0]))
		{
			length--;
			strcpy (szTempBuffer, szTempBuffer + 1);
		}

		while (length && isspace (szTempBuffer[length-1]))
		{
			szTempBuffer[--length] = '\0';
		}

		if (length && szTempBuffer[0] != ';')
		{
			if (RCOpened)
			{
				VarExpansion(szBuffer, szTempBuffer);
//				MessageBox(0, szBuffer, "test", MB_OK|MB_TOPMOST);
			}
			else
			{
				strcpy(szBuffer, szTempBuffer);
			}
			return TRUE;
		}
	}

	return FALSE;
}



// Parses szString, splitting it up into tokens.  Pays attention to single and
// double quotes.

int LCTokenize (LPCSTR szString, LPSTR *lpszBuffers, DWORD dwNumBuffers, LPSTR szExtraParameters)
{
	int		index = 0;
	char	quoteChar = 0;

	char	buffer[MAX_LINE_LENGTH];
	char	output[MAX_LINE_LENGTH];
	char	*pOutput = NULL;
	DWORD	dwBufferCount = 0;

	strcpy (buffer, szString);

	pOutput = output;

	while (buffer[index] && dwBufferCount < dwNumBuffers)
	{
		BOOL skipWhitespace = FALSE;

		switch (buffer[index])
		{
		case '"':
		case '\'':
			{
				if (!quoteChar)
				{
					quoteChar = buffer[index];
					break;
				}
				else
				{
					if (quoteChar == buffer[index])
					{
						quoteChar = 0;
						strcpy (*lpszBuffers, output);
						lpszBuffers++;
						dwBufferCount++;

						if (dwBufferCount < dwNumBuffers)
						{
							(*lpszBuffers)[0] = '\0';
						}
						pOutput = output;
						*pOutput = '\0';
						skipWhitespace = TRUE;
						break;
					}
					else
					{
						*pOutput++ = buffer[index];
						*pOutput = '\0';
						break;
					}
				}
			}
		case ' ':
		case '\t':
			{
				if (!quoteChar)
				{
					if (strlen (output))
					{
						strcpy (*lpszBuffers, output);
						lpszBuffers++;
						dwBufferCount++;
						if (dwBufferCount < dwNumBuffers)
						{
							(*lpszBuffers)[0] = '\0';
						}
						pOutput = output;
						*pOutput = '\0';
						skipWhitespace = TRUE;
					}
				}
				else
				{
					*pOutput++ = buffer[index];
					*pOutput = '\0';
				}
				break;
			}
		default:
			{
				*pOutput++ = buffer[index];
				*pOutput = '\0';
				break;
			}
		}

		index++;

		if (skipWhitespace)
		{
			while (isspace (buffer[index]))
			{
				index++;
			}
		}
	}

	if (strlen (output))
	{
		if (dwBufferCount < dwNumBuffers)
		{
			dwBufferCount++;

			strcat (*lpszBuffers, output);
		}
	}

	if (szExtraParameters && dwBufferCount == dwNumBuffers)
	{
		strcpy (szExtraParameters, buffer + index);
	}

	return dwBufferCount;
}



char* FindLine(LPCTSTR lpKeyName)
{
	TCHAR szCommand[64];
	LPTSTR pszCommand;
	LPTSTR pszBuffer;
	int i;
	int nLen;

	if (ThemeInUse)
	{
		for (i = 0; i < ThemeLen; i++)
		{
			pszBuffer = ThemeBuffer[i];
			pszCommand = szCommand;
			nLen = 63;

			while( *pszBuffer && *pszBuffer != ' ' && *pszBuffer != '\t' )
			{
				if( nLen )
				{
					*pszCommand++ = *pszBuffer;
					nLen--;
				}

				pszBuffer++;
			}

			*pszCommand = 0;

			if( !_stricmp( szCommand, lpKeyName ) )
				return ThemeBuffer[i];
		}
	}

	for (i = 0; i < RCLen; i++)
	{
		pszBuffer = RCBuffer[i];
		pszCommand = szCommand;
		nLen = 63;

		while( *pszBuffer && *pszBuffer != ' ' && *pszBuffer != '\t' )
		{
			if( nLen )
			{
				*pszCommand++ = *pszBuffer;
				nLen--;
			}

			pszBuffer++;
		}

		*pszCommand = 0;

		if( !_stricmp( szCommand, lpKeyName ) )
			return RCBuffer[i];
	}

	return NULL;
}




int GetRCInt(LPCTSTR lpKeyName, int nDefault)
{
	int val = nDefault;
	char *line = NULL;
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH];
	char *tokens[2];
//	int i;
//	int l = strlen(lpKeyName);

	tokens[0] = token1;
	tokens[1] = token2;

	line = FindLine(lpKeyName);
	if (line)
	{
		int count = 0;
		token1[0] = token2[0] = '\0';
		
		count = LCTokenize (line, tokens, 2, NULL);
		if (count < 2) return val;
		val = atoi(token2);
	}
	return val;
}

BOOL GetRCBool(LPCTSTR lpKeyName, BOOL ifFound)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH];
	char *tokens[2];
	char *line = NULL;
//	int i;
//	int l = strlen(lpKeyName);

	tokens[0] = token1;
	tokens[1] = token2;

	line = FindLine(lpKeyName);
	if (line)
	{
		int count = 0;
		token1[0] = token2[0] = '\0';
		
		count = LCTokenize (line, tokens, 2, NULL);
		if (count < 2) return ifFound;

		if (!_strnicmp (token2, "OFF", 3))
		{
  			return (!ifFound);
		}
		else
		{
			return (ifFound);
		}

	}
	return (!ifFound);
}

BOOL GetRCString(LPCTSTR lpKeyName, LPSTR value, LPCTSTR defStr, int maxLen)
{
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH];
	char *tokens[2];
	char *line = NULL;
//	int i;
//	int l = strlen(lpKeyName);

	tokens[0] = token1;
	tokens[1] = token2;

	strncpy (value, defStr, maxLen);

	line = FindLine(lpKeyName);
	if (line)
	{
		int count = 0;
		token1[0] = token2[0] = '\0';
		count = LCTokenize (line, tokens, 2, NULL);
		if (count < 2) return FALSE;
		VarExpansion(value, token2);
/*		else
		  strncpy (value, token2, maxLen);
*/		return TRUE;
	}
	return FALSE;

}

COLORREF GetRCColor(LPCTSTR lpKeyName, COLORREF colDef)
{
	COLORREF val = colDef;
	char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH], token3[16], token4[16];
	char * strtocvt, *endstr;
	char *tokens[4];
	char *line = NULL;

	tokens[0] = token1;
	tokens[1] = token2;
	tokens[2] = token3;
	tokens[3] = token4;

	line = FindLine(lpKeyName);

	if (line){
		int count = 0;
		token1[0] = token2[0] = token3[0] = token4[0] = '\0';

		count = LCTokenize (line, tokens, 4, NULL);

		if( count == 4 && isdigit( token2[0] ) && isdigit( token3[0] ) && isdigit( token4[0] ) ){
			int r, g, b;

			r = atoi( token2 );
			g = atoi( token3 );
			b = atoi( token4 );

			val = RGB( r, g, b );
		}
		else if( count >= 2 ){
			strtocvt = token2;

			if( !_strnicmp( token2, "0x", 2 ) ){
				strtocvt += 2;
			}

			val = strtol( strtocvt, &endstr, 16 );

			if( !ColorBGR ){
				val = RGB( GetBValue( val ), GetGValue( val ), GetRValue( val ) );
			}
		}
	}
	return val;
}

BOOL GetRCLine( LPCTSTR pszCommand, LPTSTR pszBuffer, UINT nBufLen, LPCTSTR pszDefault )
{
	LPTSTR pszLine = NULL;

	// verify parameters
	if( !pszBuffer || !nBufLen )
		return FALSE;

	// find it
	if( pszCommand )
		pszLine = FindLine( pszCommand );

	if( !pszLine )
	{
		if( pszDefault )
			strncpy( pszBuffer, pszDefault, nBufLen );
		else
			*pszBuffer = 0;

		return FALSE;
	}

	// read past command
	while( *pszLine && *pszLine != ' ' && *pszLine != '\t' )
		pszLine++;

	// read past whitespace
	while( *pszLine && (*pszLine == ' ' || *pszLine == '\t') )
		pszLine++;

	nBufLen--;

	// read in data
	while( *pszLine )
	{
		if( nBufLen-- )
			*pszBuffer++ = *pszLine;

		pszLine++;
	}

	*pszBuffer = 0;
	return TRUE;
}

void VarExpansion(char *buffer, const char * value)
{
	int i=0, j=0;
	char * startDollar, * endDollar;
	char * string1;
	char buf[MAX_LINE_LENGTH];
	char buf2[MAX_LINE_LENGTH];
	char bvalue[MAX_LINE_LENGTH];
	int StillVars = 1;
	char *envvar;

	strcpy(bvalue, value);
	strcpy(buffer, value);
	while (StillVars)
	{
		if ((startDollar = strchr(bvalue, '$')) != NULL)
		{
			if ((endDollar = strchr(&startDollar[1], '$')) != NULL)
			{
	//			MessageBox(0,value, "hrm", MB_OK|MB_TOPMOST);
				i = strlen(startDollar) - strlen(endDollar) - 1;
				j = strlen(bvalue) - strlen(startDollar);
				string1 = startDollar;
				string1++;
				strncpy(buf, string1, i);
				buf[i]='\0';

				envvar=NULL;
				if (!GetRCString(buf, buf2, buf, 255))
					envvar = getenv(buf);
				if (bvalue != startDollar)
				{
					strncpy(buffer, bvalue, j);
					buffer[j] = '\0';
				}
				else
				{
					strcpy(buffer, "");
				}
				
				if (envvar) {
					strcat(buffer,envvar);
				} else {
					strcat(buffer, buf2);
				}
				strcat(buffer, ++endDollar);
				strcpy(bvalue, buffer);
			}
			else
			{
				StillVars = 0;
			}
		}
		else
		{
			StillVars = 0;
		}
	}
}


BOOL SetupRC(LPCTSTR szPath)
{
	FILE*	rc;
	FILE*	thm;
	char	buffer[MAX_LINE_LENGTH];

	if (RCOpened) return FALSE;
	
	rc = LCOpen(szPath);
	if (!(rc))
	{
		return FALSE;
	}
	
	while (LCReadNextLine(rc, buffer, sizeof (buffer)))
	{
		char *temp;

		if (buffer[0] == '*')
		{
			continue;
		}

		temp = malloc (strlen (buffer) + 1);
		strcpy (temp, buffer);

		if (!RCBuffer)
		{
			RCBuffer = malloc ((RCLen + 1) * sizeof (char *));
		}
		else
		{
			RCBuffer = realloc (RCBuffer, (RCLen + 1) * sizeof (char *));
		}

		RCBuffer[RCLen++] = temp;
	}
	
	buffer[0] = 0;
	GetRCString("LSThemeFile", buffer, "", MAX_LINE_LENGTH);

	// Modified - Maduin, 10-20-1999
	//   Accept both ThemeFile and LSThemeFile to facilitate
	//   greater backwards compatibility.
	
	if( !buffer[0] )
		GetRCString( TEXT("ThemeFile"), buffer, TEXT(""), MAX_LINE_LENGTH );

	if ((buffer[0]) && !(ThemeLen))
	{
		thm = LCOpen(buffer);
		if (thm)
		{
			while (LCReadNextLine(thm, buffer, sizeof (buffer)))
			{
				char *temp;
				
				if (buffer[0] == '*')
				{
					if (!_strnicmp("*ThemePic ", buffer, 10))
					{
						char token1[MAX_LINE_LENGTH], token2[MAX_LINE_LENGTH],
							token3[MAX_LINE_LENGTH];
						char *tokens[3];
						int blah = 0, count = 0;


						tokens[0] = token1;
						tokens[1] = token2;
						tokens[2] = token3;

						token1[0] = token2[0] = token3[0] = '\0';
						count = LCTokenize (buffer, tokens, 3, NULL);
						if (count < 3) continue;
						if (is_valid_pattern(token2, &blah))
						{
							if (!PicBuffer)
							{
								PicBuffer = malloc((ThemePicLen+1)* sizeof(tThemePic));
							}
							else
							{
								PicBuffer = realloc(PicBuffer, (ThemePicLen+1)*sizeof(tThemePic));
							}
							strncpy(PicBuffer[ThemePicLen].program, token2, 255);
							strncpy(PicBuffer[ThemePicLen].bitmap, token3, 255);
							ThemePicLen++;
						}
						else
							continue;

					}
					else
					{
						continue;
					}
				}

				temp = malloc (strlen (buffer) + 1);
				strcpy (temp, buffer);

				if (!ThemeBuffer)
				{
					ThemeBuffer = malloc ((ThemeLen + 1) * sizeof (char *));
				}
				else
				{
					ThemeBuffer = realloc (ThemeBuffer, (ThemeLen + 1) * sizeof (char *));
				}
				ThemeBuffer[ThemeLen++] = temp;
			}
			ThemeInUse = TRUE;
		}
		LCClose(thm);
	}

	ColorBGR = GetRCBool("LSColorBGR", TRUE);
	RCOpened = 1;

	return LCClose(rc);
}

void CloseRC(void)
{
	int loop;

    if (RCOpened) {
		for (loop = 0; loop < RCLen; ++loop) {
			free (RCBuffer[loop]);
		}
		if (RCBuffer) {
			free (RCBuffer);
			RCBuffer = NULL;
		}
		if (ThemeInUse) {
			for (loop = 0; loop < ThemeLen; ++loop) {
				free(ThemeBuffer[loop]);
			}
			if (ThemeBuffer) {
				free (ThemeBuffer);
				ThemeBuffer = NULL;
			}
			ThemeInUse = FALSE;
			if (PicBuffer) {
				free(PicBuffer);
				PicBuffer = NULL;
			}
		}
		RCLen = 0;
		ThemeLen = 0;
		ThemePicLen = 0;
		RCOpened = 0;
		ColorBGR = FALSE;
	}
	for (loop = 0; loop < Bangs; loop++) {
		free(BangAdded[loop].name);
	}
	if (BangAdded) {
		free(BangAdded);
		BangAdded = NULL;
	}
	Bangs = 0;
}



HRGN BitmapToRegion (HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance, int xoffset, int yoffset)
{
	HRGN hRgn = NULL;

	if (hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC)
		{
			// Get bitmap size
			HBITMAP hbm32;
			BITMAP bm;
			BITMAPINFOHEADER RGB32BITSBITMAPINFO;
			VOID * pbits32; 
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC 
			RGB32BITSBITMAPINFO.biSize = sizeof(BITMAPINFOHEADER);
			RGB32BITSBITMAPINFO.biWidth = bm.bmWidth;
			RGB32BITSBITMAPINFO.biHeight = bm.bmHeight;
			RGB32BITSBITMAPINFO.biPlanes = 1;
			RGB32BITSBITMAPINFO.biBitCount = 32;
			RGB32BITSBITMAPINFO.biCompression = BI_RGB;
			RGB32BITSBITMAPINFO.biSizeImage = 0;
			RGB32BITSBITMAPINFO.biXPelsPerMeter = 0;
			RGB32BITSBITMAPINFO.biYPelsPerMeter = 0;
			RGB32BITSBITMAPINFO.biClrUsed = 0;
			RGB32BITSBITMAPINFO.biClrImportant = 0;

			hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if (hbm32) {
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory DC
				HDC hDC = CreateCompatibleDC(hMemDC);
				if (hDC) {
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits)
					HBITMAP holdBmp;
					BITMAP bm32;
					DWORD maxRects;
					HANDLE hData;
					RGNDATA *pData;
					HRGN h;
					BYTE *p32;
					int y, x;

					// Keep on hand highest and lowest values for the "transparent" pixels
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = (BYTE) min(0xff, lr + GetRValue(cTolerance));
					BYTE hg = (BYTE) min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = (BYTE) min(0xff, lb + GetBValue(cTolerance));

					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy the bitmap into the memory DC
					holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles by
					// amount of ALLOC_UNIT number in this structure.
					#define ALLOC_UNIT	100
					maxRects = ALLOC_UNIT;
					hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to right
						for (x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if (b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								RECT *pr;

								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0+xoffset, y+yoffset, x+xoffset, y+1+yoffset);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0+xoffset;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y+yoffset;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x+xoffset;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1+yoffset;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
								// large (ie: > 4000). Therefore, we have to create the region by multiple steps.
								if (pData->rdh.nCount == 2000)
								{
									if (hRgn) {
										HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									} else
										hRgn = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangles
					if (hRgn) {
						h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					} else
						hRgn = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					// Clean up
					GlobalUnlock(hData);
					GlobalFree(hData);
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}
				DeleteObject(SelectObject(hMemDC, holdBmp));
			}
			DeleteDC(hMemDC);
		}	
	}
	return hRgn;
}

BOOL AddBangCommand(char *command, void *f) {
	int i =  0;
	char * temp;

	for (i = 0; i < MAX_BANG_COMMANDS; i++) {
		if (!_strcmpi(BangCommands[i].name, command)) {
			BangCommands[i].func = f;
			BangCommands[i].ex = FALSE;
			return TRUE;
		}
	}
	if(BangAdded) {
		for (i = 0; i < Bangs; i++) {
			if (!_strcmpi(BangAdded[i].name, command)) {
				BangAdded[i].func = f;
				BangAdded[i].ex = FALSE;
				return TRUE;
			}
		}
	}
	if (!BangAdded)
		BangAdded = malloc ((Bangs + 1) * sizeof(tBangCommand));
	else
		BangAdded = realloc(BangAdded, (Bangs + 1) * sizeof(tBangCommand));
	temp = malloc (strlen (command) + 1);
	strcpy (temp, command);
	BangAdded[Bangs].name = temp;
	BangAdded[Bangs].func = f;
	BangAdded[Bangs].ex = FALSE;
	Bangs++;
//	free(temp);
	return FALSE;
}

BOOL AddBangCommandEx(char *command, void * f) { // Killarny
	int i =  0;
	char * temp;

	for (i = 0; i < MAX_BANG_COMMANDS; i++) {
		if (!_strcmpi(BangCommands[i].name, command)) {
			BangCommands[i].func = f;
			BangCommands[i].ex = TRUE;
			return TRUE;
		}
	}
	if(BangAdded) {
		for (i = 0; i < Bangs; i++) {
			if (!_strcmpi(BangAdded[i].name, command)) {
				BangAdded[i].func = f;
				BangAdded[i].ex = TRUE;
				return TRUE;
			}
		}
	}
	if (!BangAdded)
		BangAdded = malloc ((Bangs + 1) * sizeof(tBangCommand));
	else
		BangAdded = realloc(BangAdded, (Bangs + 1) * sizeof(tBangCommand));
	temp = malloc (strlen (command) + 1);
	strcpy (temp, command);
	BangAdded[Bangs].name = temp;
	BangAdded[Bangs].func = f;
	BangAdded[Bangs].ex = TRUE;
	Bangs++;
	return FALSE;
}

BOOL RemoveBangCommand(char *command)
{
  int i =  0;

  for (i = 0; i < MAX_BANG_COMMANDS; i++)
	{
		if (!_strcmpi(BangCommands[i].name, command))
		{
//			BangCommands[i].func = f;
			return TRUE;
		}
	}
  if (BangAdded)
  {
		BOOL found = FALSE;
		for (i = 0; i < Bangs; i++)
		{
			if(!found)	// Keep going till we find command
			{
				if (!_strcmpi(BangAdded[i].name, command))
				{
					free(BangAdded[i].name);
					found =  TRUE;
				}
			}
			else	//Found so move remaining commands up the array
			{
				memcpy(&BangAdded[i - 1], &BangAdded[i], sizeof(tBangCommand));
//				BangAdded[i - 1].name = BangAdded[i].name;
//				BangAdded[i - 1].func = BangAdded[i].func;
			}
		}
		if(found)
			BangAdded = realloc(BangAdded, (--Bangs) * sizeof(tBangCommand));
	}
	return FALSE;
}

BOOL ParseBangCommand(HWND caller, char *command, char *args) {
	int i;
	int length;
	int length2;
	char newArgs[4096];
	char newCmd[1024];

	length2 = command ? strlen (command) : 0;
	if (length2){
		strcpy(newCmd, command);
		// strip leading and trailing white space from command
		while (length2 && isspace ((int)newCmd[0])){
			length2--;
			strcpy (newCmd, newCmd + 1);
		}
		while (length2 && isspace (newCmd[length2-1]))
			newCmd[--length2] = '\0';
		if (!length2){
			*newCmd = '\0';
			return FALSE;
		}
	}
	if (!length2){
		*newCmd = '\0';
		return FALSE;
	}

  length = args ? strlen (args) : 0;
	if (length){
		strcpy(newArgs, args);

		// strip leading and trailing white space from arguments
		while (length && isspace ((int)newArgs[0])){
			length--;
			strcpy (newArgs, newArgs + 1);
		}
		while (length && isspace (newArgs[length-1]))
			newArgs[--length] = '\0';
		if (!length)
			*newArgs = '\0';
	}
	if (!length)
		*newArgs = '\0';

	if(newArgs == NULL || newArgs[0] == '/0' || !strchr(newArgs, '!')) {
		for (i = 0; i < MAX_BANG_COMMANDS; i++) {
			if (!_strcmpi(BangCommands[i].name, newCmd)) {
				if( BangCommands[i].ex )
					BangCommands[i].func( caller, newCmd, newArgs );
				else
					BangCommands[i].func(caller, newArgs);
				return TRUE;
			}
		}
		for (i = 0; i < Bangs; i++) {
			if (!_strcmpi(BangAdded[i].name, newCmd)) {
				if( BangAdded[i].ex )
					BangAdded[i].func( caller, newCmd, newArgs );
				else
					BangAdded[i].func(caller, newArgs);
				return TRUE;
			}
		}
	} else {
		char tmpCommand[1024];
		char tmpArgs[4096];
		char* cmdPtr;   //Points to next byte of temp command buffer
		char* argsPtr;	//Points to next byte of args to process
		int finished;
		int argLen;

		// Process first command differently due to command being in command not args
		argLen = (int)(strchr(newArgs, '!') - newArgs);

		// Copy upto next bang into tmpArgs
		if(argLen) {
			memcpy(tmpArgs, newArgs, argLen);
			tmpArgs[argLen] = 0;
		} else
			tmpArgs[0] = 0;

		//Call ourselves to run the first command
		ParseBangCommand(caller, newCmd, tmpArgs);
		// Set argsPtr to next byte of args to use
		argsPtr = newArgs + argLen;

		finished = 0;
		while (!finished) {
			cmdPtr = tmpCommand;
			// Copy next command until white space
			while( argsPtr[0] != ' ' && argsPtr[0] != '\t' && argsPtr[0] != 0)
				*cmdPtr++ = *argsPtr++;
			*cmdPtr = 0;	// Null terminate command
			// Skip whitespace chars
			while( argsPtr[0] == ' ' || argsPtr[0] == '\t')
				*argsPtr++;
			if(argsPtr[0] == '/0') {
				finished = 1;		// No more to go
				tmpArgs[0] = 0;
				//Call ourselves to run the last command
				ParseBangCommand(caller, tmpCommand, tmpArgs);
			} else if(!strchr(argsPtr, '!')) {
				finished = 1;		// No more to go
				//Call ourselves to run the last command
				ParseBangCommand(caller, tmpCommand, argsPtr);
			} else {
				argLen = (int)(strchr(argsPtr, '!') - argsPtr);
				// Copy upto next bang into tmpArgs
				memcpy(tmpArgs, argsPtr, argLen);
				tmpArgs[argLen] = 0;
				// Set argsPtr to next byte of args to use
				argsPtr = argsPtr + argLen;
				//Call ourselves to run the first command
				ParseBangCommand(caller, tmpCommand, tmpArgs);
			}
		}
	}
	return FALSE;
}

int CommandTokenize (LPCSTR szString, LPSTR *lpszBuffers, DWORD dwNumBuffers, LPSTR szExtraParameters)
{
	int		index = 0;
	char	quoteChar = 0;

	char	buffer[MAX_LINE_LENGTH];
	char	output[MAX_LINE_LENGTH];
	char	*pOutput = NULL;
	DWORD	dwBufferCount = 0;

	strcpy (buffer, szString);

	pOutput = output;
	*pOutput = '\0';

	while (isspace (buffer[index]))
		index++;
	if (buffer[index] != '"' && buffer[index] != '[' && buffer[index] != ']'){
		dwBufferCount = 1;
		if (lpszBuffers){
			strcpy(*lpszBuffers, szString);
		}
		return dwBufferCount;
	}

	while (buffer[index] && (dwBufferCount < dwNumBuffers || !lpszBuffers))
	{
		BOOL skipWhitespace = FALSE;

		switch (buffer[index])
		{
			case '"':
			case '\'':
			case '[':
			case ']':
				{
					if (!quoteChar)
					{
						if (buffer[index] == '['){
							if (*pOutput){
								if (lpszBuffers){
									strcpy(*lpszBuffers, output);
									lpszBuffers++;
									dwBufferCount++;
									if (dwBufferCount < dwNumBuffers){
										(*lpszBuffers)[0] = '\0';
									}
								}else{
									dwBufferCount++;
								}
								pOutput = output;
								*pOutput = '\0';
								skipWhitespace = TRUE;
							}
							quoteChar = ']';
						}
						else if (buffer[index] == ']'){
							return -1;
						}else{
							quoteChar = buffer[index];
						}
						break;
					}else{
						if (quoteChar == buffer[index]){
							quoteChar = 0;
							if (lpszBuffers){
								strcpy (*lpszBuffers, output);
								lpszBuffers++;
								dwBufferCount++;
								if (dwBufferCount < dwNumBuffers){
									(*lpszBuffers)[0] = '\0';
								}
							}else{
								dwBufferCount++;
							}
							pOutput = output;
							*pOutput = '\0';
							skipWhitespace = TRUE;
							break;
						}else{
							*pOutput++ = buffer[index];
							*pOutput = '\0';
							break;
						}
					}
				}
			case ' ':
			case '\t':
				{
					if (!quoteChar){
						if (strlen (output)){
							if (lpszBuffers){
								strcpy (*lpszBuffers, output);
								lpszBuffers++;
								dwBufferCount++;
								if (dwBufferCount < dwNumBuffers){
									(*lpszBuffers)[0] = '\0';
								}
							}else{
								dwBufferCount++;
							}
							pOutput = output;
							*pOutput = '\0';
							skipWhitespace = TRUE;
						}
					}else{
						*pOutput++ = buffer[index];
						*pOutput = '\0';
					}
					break;
				}
			default:
				{
					*pOutput++ = buffer[index];
					*pOutput = '\0';
					break;
				}
		}
		index++;

		if (skipWhitespace){
			while (isspace (buffer[index]))
				index++;
		}
	}

	if (strlen (output)){
		if (dwBufferCount < dwNumBuffers || !lpszBuffers){
			dwBufferCount++;
			if (lpszBuffers){
				strcat (*lpszBuffers, output);
			}
		}
	}
	if (szExtraParameters && dwBufferCount == dwNumBuffers){
		strcpy (szExtraParameters, buffer + index);
	}

	return dwBufferCount;
}

void CommandParse(LPCSTR cmd, LPSTR cmdbuf, LPSTR argsbuf, DWORD dwCmdBufSize, DWORD dwArgsBufSize)
{
  char *newcmd, *args, *p, *command = cmd ? (char *)malloc(strlen(cmd) + 1) : NULL;

  if(!command)
    return;

  strcpy(command, cmd);

  if(*command == '\"') {
    newcmd = command + sizeof(char);
    newcmd = strtok(newcmd, "\"");
    args = strtok(NULL, "\0");
    
    if(!newcmd)
      return;
  } else {
    newcmd = strtok(command, "\t ");
    if(!newcmd) {
      newcmd = command;
      args = NULL;
    } else {
      args = strtok(NULL, "\0");
    }
  }

  p = newcmd + (strlen(newcmd) - 1)*sizeof(char);
  while(*newcmd == ' ' || *newcmd == '\t')
    ++newcmd;
  while(*p == ' ' || *p == '\t') {
    *p = '\0';
    --p;
  }

  if(*newcmd != '?' && *newcmd != '!') {
    if(args) {
      p = args + (strlen(args) - 1)*sizeof(char);
      while(*args == ' ' || *args == '\t' || *args == '\"')
        ++args;
      while(*p == ' ' || *p == '\t' || *p == '\"') {
        *p = '\0';
        --p;
      }
    }
  } else {
    if(args) {
      p = args + (strlen(args) - 1)*sizeof(char);
      while(*args == ' ' || *args == '\t')
        ++args;
      while(*p == ' ' || *p == '\t') {
        *p = '\0';
        --p;
      }
    }
  }

  // Finally, we have our command token, and its arguments
  if(newcmd && cmdbuf)
    strncpy(cmdbuf, newcmd, dwCmdBufSize);
  if(args && argsbuf)
    strncpy(argsbuf, args, dwArgsBufSize);
}

HINSTANCE LSExecuteEx(HWND Owner, LPCSTR szOperation, LPCSTR szCommand, LPCSTR szArgs, LPCSTR szDirectory, int nShowCmd)
{
  DWORD type;
  SHELLEXECUTEINFO si;
  char *cmd, *args;

  if(*szCommand == '!') {
		BOOL retVal;
    cmd = (char *)malloc(strlen(szCommand) + 1);
    args = (char *)malloc(strlen(szArgs) + 1);
    strcpy(cmd, szCommand);
    strcpy(args, szArgs);
		retVal = ParseBangCommand(Owner, cmd, args);
    free(cmd);
    free(args);
    return  retVal ? (HINSTANCE)1 : (HINSTANCE)0;
  } else {
    type = GetFileAttributes(szCommand);
    if(type & FILE_ATTRIBUTE_DIRECTORY && type != 0xFFFFFFFF)
      return ShellExecute(Owner, szOperation, szCommand, szArgs, NULL, nShowCmd ? nShowCmd : SW_SHOWNORMAL);
    else {
      memset(&si, 0, sizeof(si));
      si.cbSize = sizeof(SHELLEXECUTEINFO);
      si.hwnd = Owner;
      si.lpVerb = szOperation;
      si.lpFile = szCommand;
      si.lpParameters = szArgs;
      si.lpDirectory = szDirectory;
      si.nShow = nShowCmd;
      si.fMask = SEE_MASK_DOENVSUBST;
      ShellExecuteEx(&si);
      return (HINSTANCE)GetLastError();
    }
  }
}

HINSTANCE LSExecute(HWND Owner, LPCSTR szCommand, int nShowCmd)
{
  int numCmds = 0, i, total;
	char command[4096], **tokens, newcmd[4096], args[4096], dir[_MAX_DIR], full_directory[_MAX_DIR + _MAX_DRIVE + 1]; // Can't rely on MAX_PATH for command
  HINSTANCE val = 0;

	if(!szCommand || strlen(szCommand) < 1)
		return 0;

  VarExpansion(command, szCommand);

  // Parse []
  numCmds = CommandTokenize(command, NULL, 0, NULL);

  tokens = (char **)malloc(sizeof(char*) * numCmds);
  for(i=0; i<numCmds; ++i)
    tokens[i] = (char *)malloc(4096);
  if(!CommandTokenize(command, tokens, numCmds, NULL)) {
    for(i=0; i < numCmds; ++i)
      free(tokens[i]);
    free(tokens);
    return (HINSTANCE)-1;
  }

  total = numCmds;

  for(i=0; i < total; ++i) {
    *newcmd = '\0';
    *args = '\0';
    CommandParse(tokens[i], newcmd, args, sizeof(newcmd), sizeof(args));

    if(*newcmd == '!')
      val = LSExecuteEx(Owner, NULL, newcmd, args, NULL, 0);
    else if(*newcmd) {
      _splitpath(newcmd, full_directory, dir, NULL, NULL);
      strcat(full_directory, dir);
      val = LSExecuteEx(Owner, "open", newcmd, args, full_directory, nShowCmd ? nShowCmd : SW_SHOWNORMAL);
    }
  }

  for(i=0; i < numCmds; ++i)
    free(tokens[i]);

  free(tokens);
  return val;
}

void BangExecute(HWND caller, char* args)
{
  LSExecute(caller, args, 0 /*default*/);
}

void BangRecycle(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls, LM_RECYCLE, 0, 0);
	}

	return;
}

void BangUnloadModule(HWND caller,char* args) {
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls && args)
		SendMessage(ls, LM_UNLOADMODULE, (WPARAM)args, 0);
	return;
}

void BangReloadModule(HWND caller,char* args) {
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls && args)
		SendMessage(ls, LM_RELOADMODULE, (WPARAM)args, 0);
	return;
}

void BangEasterEgg(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	int * Operator;
	// The magic Number
	int Key = 31337;
	void (*func)(HWND caller, char* args);

	char ThisOp[] = "DoEasterEggRoutines";
	args=args;
	caller=caller;

	if (ls)
	{
		// Initialise.
		Operator = &Key;

		// Execute code.
		if (((*Operator&0x03)? (*Operator): (0) )? 1 : Key)
		{
			 func = (void *) &Key;
			 func ( (HWND) Key, ThisOp);
		}
	}

	return;
}

void BangGather(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls, LM_BRINGTOFRONT, 1, 0);
	}

	return;
}

void BangRun(HWND caller,char* args)
{
	FARPROC (__stdcall *MSRun)(HWND, int, int, char*, char*, int) = NULL;

	MSRun = (FARPROC (__stdcall *) (HWND, int, int, char*, char*, int))GetProcAddress(GetModuleHandle("SHELL32.DLL"), (char*)((long)0x3D));
	MSRun(NULL, 0, 0, NULL, NULL, 0);

	args=args;
	caller=caller;
	return;
}

void BangShutdown(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd ();
	FARPROC (__stdcall *MSWinShutdown)(HWND) = NULL;

	MSWinShutdown = (FARPROC (__stdcall *)(HWND))GetProcAddress(GetModuleHandle("SHELL32.DLL"), (char*)((long)0x3C));
	MSWinShutdown(ls);

	args=args;
	caller=caller;
	return;

/*	HWND ls = GetLitestepWnd ();

	if (ls)
	{
		PostMessage (ls, 9260, 3, 0);
	}
*/
}

void BangLogoff(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd ();
	args=args;
	caller=caller;

	if (ls)
	{
		PostMessage (ls, LM_RECYCLE, 1, 0);
	}
}

void BangQuit(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd ();
	args=args;
	caller=caller;

	if (ls)
	{
		PostMessage (ls, LM_RECYCLE, 2, 0);
	}
}

void BangToggleWharf(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls, LM_SHADETOGGLE, 0, 0);
	}

    return;
}

void BangVWMDesk(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls && args != NULL)
	{
		SendMessage(ls,LM_SWITCHTON,atoi(args),0);
	}

	return;
}

void BangVWMUp(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls,LM_VWMUP,0,0);
	}

	return;
}

void BangVWMDown(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls,LM_VWMDOWN,1,0);
	}

	return;
}

void BangVWMLeft(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls,LM_VWMLEFT,1,0);
	}

	return;
}

void BangVWMRight(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls,LM_VWMRIGHT,1,0);
	}

	return;
}

void BangVWMNav(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		SendMessage(ls,LM_VWMNAV,1,0);
	}

	return;
}

void BangPopup(HWND caller,char* args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		POINT p;
		if (GetCursorPos((LPPOINT)&p)) {
			SendMessage(ls,LM_HIDEPOPUP,0,0);
			SendMessage(ls,LM_POPUP,p.x,p.y);
		}
	}
	
	return;
}

void BangTileWindowsH(HWND caller, char *args)
{
	args=args;
	caller=caller;
	TileWindows(NULL, MDITILE_HORIZONTAL, NULL, 0, NULL);
	return;
}

void BangTileWindowsV(HWND caller, char *args)
{
	args=args;
	caller=caller;
	TileWindows(NULL, MDITILE_VERTICAL, NULL, 0, NULL);
	return;
}

void BangCascadeWindows(HWND caller, char *args)
{
	args=args;
	caller=caller;
	CascadeWindows(NULL, MDITILE_SKIPDISABLED, NULL, 0, NULL);
	return;
}

void BangMinimizeWindows(HWND caller, char *args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		int maxWin, i;
		windowType *winList;;

		maxWin = (int) SendMessage(ls, LM_WINLIST, 1, 0);
		winList = (windowType *)SendMessage(ls, LM_WINLIST, 0, 0);
		for (i = 0; i < maxWin && winList[i].Handle; i++)
		{
			HWND parent;
			if (GetWindowLong(winList[i].Handle, GWL_USERDATA) == magicDWord) 
				continue;
			if (GetWindowLong(winList[i].Handle, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) 
				continue;
			parent = GetParent(winList[i].Handle);
			if (GetWindowLong(parent, GWL_USERDATA) == magicDWord) 
				continue;
			if (!IsWindowVisible(winList[i].Handle)) 
				continue;
			ShowWindow(winList[i].Handle, SW_MINIMIZE);
		}
	}
	return;
}

void BangRestoreWindows(HWND caller, char *args)
{
	HWND ls = GetLitestepWnd();
	args=args;
	caller=caller;

	if (ls)
	{
		int maxWin, i;
		windowType *winList;;

		maxWin = (int) SendMessage(ls, LM_WINLIST, 1, 0);
		winList = (windowType *)SendMessage(ls, LM_WINLIST, 0, 0);
		for (i = 0; i < maxWin && winList[i].Handle; i++)
		{
			HWND parent;

			if (GetWindowLong(winList[i].Handle, GWL_USERDATA) == magicDWord) 
				continue;
			if (GetWindowLong(winList[i].Handle, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) 
				continue;
			parent = GetParent(winList[i].Handle);
			if (GetWindowLong(parent, GWL_USERDATA) == magicDWord) 
				continue;
			if (!IsIconic(winList[i].Handle)) 
				continue;
			ShowWindow(winList[i].Handle, SW_RESTORE);
		}
	}
	return;
}

void BangAbout(HWND caller, char *args)
{
	char buffer[2048];
	int tipe = 0;
	int RevType = 0;
	HWND ls = GetLitestepWnd();
	caller=caller;

	if (args){
		if (!_stricmp(args, "DETAILED")){
			tipe = 1;
		}
	}
	RevType = tipe;
	strcpy(buffer, "");
	tipe |= (2048 << 4);

	if (SendMessage(ls, LM_GETREVID, tipe, (LPARAM) buffer))
	{
		strcat(buffer, "\n");
		if (RevType)
		{
			strcat(buffer, &rcsId[5]);
			buffer[strlen(buffer)-1] = '\0';
		}
		else
		{
			strcat(buffer, "lsapi.dll: ");
			strcat(buffer, &rcsRevision[11]);
			buffer[strlen(buffer)-1] = '\0';
		}
		strcpy(vbuffer, buffer);
		MessageBox(ls, vbuffer, _T("About Information:"), MB_OK|MB_ICONINFORMATION|MB_DEFBUTTON1|MB_SYSTEMMODAL);
//		if (!AboutDialog) {
//			CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), ls, (DLGPROC) About);
//			AboutDialog = TRUE;
//		}
	}
	return;
}

LRESULT CALLBACK About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char text[256];
	lParam=lParam;
//	static hwndEdit = NULL;

	switch (uMsg){
	case WM_INITDIALOG:
		//hwndEdit = CreateWindow (_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 0,0,0,0, ls, (HMENU)ID_EDIT, GetModuleHandle(NULL), NULL);
		GetDlgItemText(hDlg, IDC_VINFO, (LPTSTR) &text, 256);
		SetDlgItemText(hDlg, IDC_VINFO, (LPTSTR) &vbuffer);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, TRUE);
			DestroyWindow(hDlg);
			AboutDialog = FALSE;
			return TRUE;
		}
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE) {
			EndDialog(hDlg, TRUE);
			DestroyWindow(hDlg);
			AboutDialog = FALSE;
			return TRUE;
		}
	case WM_CTLCOLORSTATIC:
/*		if (GetDlgItem(hDlg, IDC_VINFO) == (HWND) lParam) {
			GetWindowRect(control, &area);
			SetTextColor((HDC)wParam, 0x00FF00);
			DrawText((HDC)wParam, (LPCSTR)&vbuffer, 2048, &area, DT_LEFT|DT_VCENTER|DT_WORDBREAK);
		}
		if (GetDlgItem(hDlg, IDC_LS) == (HWND) lParam) {
			GetWindowRect(control, &area);
			DrawText((HDC)wParam, (LPCSTR)&vbuffer, 2048, &area, DT_LEFT|DT_VCENTER);
		} */
		break;
	case STN_CLICKED:
		break;
	}
	return FALSE;
}

void CheckTheme(char *szImage, LPCSTR szFile)
{
	int i;
	char filename[MAX_PATH];

	if (ThemePicLen && szFile)
	{
		strcpy(filename, szFile);

		for (i=0; i<ThemePicLen; i++)
		{
			if (match(PicBuffer[i].program, filename))
			{
				strcpy(szImage, PicBuffer[i].bitmap);
				return;
			}
		}
	}
}

// Takes strings of the form:
//   File.bmp
//   .extract
//   .extract=file.exe[,3]

HBITMAP LoadLSImage (LPCSTR szImage, LPCSTR szFile) {
	static char szImagesFolder[MAX_PATH] = { '\0' };
	char szImageBuf[MAX_PATH];
	char szImageFinal[MAX_PATH];
	static BOOL bCheckedImagesFolder = FALSE;
	HINSTANCE	hInstance;
	HBITMAP 	hBitmap;
	HWND		hWnd;

	strcpy(szImageBuf, szImage);
//	if (ThemeInUse && szFile)
	if (ThemeInUse)
		CheckTheme(szImageBuf, szFile);

	
	if (!bCheckedImagesFolder)
	{
		// Modified - Maduin, 10-20-1999
		//   Changed to use new API LSGetImagePath rather than
		//   access the step.rc directly.

		LSGetImagePath( szImagesFolder, MAX_PATH );
		bCheckedImagesFolder = TRUE;
	}
	
	if (!_strcmpi (szImageBuf, ".none"))
	{
		return NULL;
	}
	
	hWnd = GetLitestepWnd ();
	if (!hWnd)
	{
		return NULL;
	}
	
	hInstance = (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE);
	if (!hInstance)
	{
		return NULL;
	}
// Bitmap merging by Thedd
//  Thedd - pic1.bmp|pic2.bmp merges the images. Works recursively,
//  so pic1.bmp|.extract=whatever.dll,3|pic2.bmp also works etc...
//  pic1.bmp is put "on top of" pic2.bmp in pic1.bmp|pic2.bmp. 
//  Images should be of the same size, or wierd things will happen.
	if (strrchr (szImageBuf, '|')) {
		HDC tmpDC2, tmpDC1, hMemDC;
		HBITMAP tmpbitmap1, tmpbitmap2, finalBMP, hOldBitmap;
		HBITMAP oldBMP1, oldBMP2;
		int xs,ys,xs2,ys2;
		char szBuffer1[MAX_PATH+10] = "\0", szBuffer2[1024] = "\0";      
		char *pipepos = NULL;

		pipepos = strchr(szImageBuf, '|');
		pipepos++;
		strcpy(szBuffer2, pipepos);
		strncpy(szBuffer1, szImageBuf, strlen(szImageBuf)-strlen(szBuffer2)-1);
		tmpbitmap1 = LoadLSImage(szBuffer1, szFile);
		tmpbitmap2 = LoadLSImage(szBuffer2, szFile);
		if(tmpbitmap2 == NULL) 
			return tmpbitmap1;
		GetLSBitmapSize(tmpbitmap2, &xs, &ys);
		GetLSBitmapSize(tmpbitmap1, &xs2, &ys2);
		tmpDC2=CreateCompatibleDC(NULL);
		tmpDC1=CreateCompatibleDC(NULL);
		oldBMP1 = SelectObject(tmpDC1, tmpbitmap1);
		oldBMP2 = SelectObject(tmpDC2, tmpbitmap2);
		TransparentBltLS(tmpDC2, (xs-xs2)/2, (ys-ys2)/2, xs2, ys2, tmpDC1, 0, 0, RGB(255,0,255));
		hMemDC = CreateCompatibleDC(tmpDC2);
		finalBMP=CreateCompatibleBitmap(tmpDC2, xs, ys);
		hOldBitmap = SelectObject(hMemDC, finalBMP);
		BitBlt(hMemDC, 0, 0, ys, xs, tmpDC2, 0, 0, SRCCOPY);
		finalBMP = SelectObject(hMemDC, hOldBitmap);
		DeleteObject(SelectObject(tmpDC1, oldBMP1));
		DeleteDC(tmpDC1);
		DeleteObject(SelectObject(tmpDC2, oldBMP2));
		DeleteDC(tmpDC2);
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
		DeleteObject(hOldBitmap);
		return finalBMP;
	}
// End bitmap merging by Thedd
	if (!_strnicmp (szImageBuf, ".extract", strlen (".extract")))
	{
		char		szBuffer[MAX_PATH+10];	// Leave some overhang for the icon number, if present
		char*		szTemp = NULL;
		int 		iIndex = 0;
		HICON		hIcon;
		
		if (szImageBuf[strlen (".extract")] == '=')
		{
			strcpy (szBuffer, szImageBuf + strlen (".extract="));
			szTemp = strrchr (szBuffer, ',');
		}
		else
		{
			if (!szFile)
			{
				return NULL;
			}
			
			strcpy (szBuffer, szFile);
		}
		
		if (szTemp) // .extract=c:\file.dll,3
		{
			*szTemp = '\0';
			
			szTemp++;
			iIndex = atoi (szTemp);
		}
		
		// Now szBuffer is the filename, and iIndex is the index of the icon (zero by default)
		if (iIndex < 0) 	// Keep the user from being stupid.
		{
			return NULL;
		}
		
		if (iIndex > 0)
		{
			int iCount;
			
			iCount = (int) ExtractIcon (hInstance, szBuffer, 0xffffffff);
			
			if (iIndex >= iCount)	// Is the index out of range?
			{
				return NULL;
			}
		}
		
		hIcon = ExtractIcon (hInstance, szBuffer, iIndex);
		
		if (hIcon)
		{
			hBitmap = BitmapFromIcon (hIcon);
			DestroyIcon (hIcon);
			
			return hBitmap;
		}
	}
	else	// For now, we only support .BMP files
	{
		char szFullPath[MAX_PATH];
		strcpy (szFullPath, szImagesFolder);

		VarExpansion(szImageFinal, szImageBuf);
//		MessageBox(0, szFile, szImageFinal, MB_OK|MB_TOPMOST);

		strcat (szFullPath, szImageFinal);

		hBitmap = (HBITMAP) LoadImage
			(
			hInstance,
			szFullPath,
			IMAGE_BITMAP,
			0,
			0,
			LR_DEFAULTCOLOR | LR_LOADFROMFILE
			);
		
		if (!hBitmap)
		{
			hBitmap = (HBITMAP) LoadImage
				(
				hInstance,
				szImageFinal,
				IMAGE_BITMAP,
				0,
				0,
				LR_DEFAULTCOLOR | LR_LOADFROMFILE
				);
		}
		return hBitmap;
	}
	
	return NULL;
}

// Creates a 64x64 bitmap of an icon, with pink in the transparent regions.
HBITMAP BitmapFromIcon (HICON hIcon)
{
	ICONINFO info;

	if (GetIconInfo (hIcon, &info))
	{
		HDC		dc;
		HBITMAP	hBitmap, oldBMP;
		HBRUSH	hBrush;
		BITMAP	bitmap;

		dc = CreateCompatibleDC (NULL);

		GetObject (info.hbmColor, sizeof (BITMAP), &bitmap);
//		hBitmap = CopyImage (info.hbmColor, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);

		hBitmap = CreateBitmapIndirect (&bitmap);
		oldBMP = SelectObject(dc, hBitmap);
		hBrush = CreateSolidBrush (RGB (255,0,255));
		DrawIconEx (dc, 0, 0, hIcon, 0, 0, 0, hBrush, DI_NORMAL);
		DeleteObject (hBrush);
		DeleteObject (info.hbmMask);
		DeleteObject (info.hbmColor);
		SelectObject(dc, oldBMP);
		DeleteDC (dc);
		return hBitmap;
	}
	return NULL;
}

// Takes strings of the form:
//   File.ico
//   libary.icl,3 <- libary extraction in imagesfolder
//   c:\path\     <- icon extraction for path out of desktop.ini
//   .extract
//   .extract=file.exe[,3]  ... and returns an icon

HICON LoadLSIcon (LPCSTR szImage, LPCSTR szFile)
{
	static char szImagesFolder[MAX_PATH] = { '\0' };
	char szImageBuf[MAX_PATH];
	char szImageFinal[MAX_PATH];
	char*	szTemp = NULL;
	int iIndex = 0;

	static BOOL bCheckedImagesFolder = FALSE;
	HINSTANCE	hInstance;
	HICON 	hIcon;
	HWND		hWnd;

	strcpy(szImageBuf, szImage);

	if (ThemeInUse)
	{
		CheckTheme(szImageBuf, szFile);
	}

	
	if (!bCheckedImagesFolder)
	{
		// Modified - Maduin, 10-20-1999
		//   Changed to use new API LSGetImagePath rather than
		//   access the step.rc directly.

		LSGetImagePath( szImagesFolder, MAX_PATH );
	}
	
	if (!_strcmpi (szImageBuf, ".none"))
	{
		return NULL;
	}
	
	hWnd = GetLitestepWnd ();
	if (!hWnd)
	{
		return NULL;
	}
	
	hInstance = (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE);
	if (!hInstance)
	{
		return NULL;
	}

	szTemp=strrchr(szImageBuf,'\\');
	if ((szTemp)&&(strlen(szTemp)==1)) { // c:\path\ -> desktop.ini
		char szFullPath[MAX_PATH];

		strcat(szImageBuf,"desktop.ini");
		
		GetPrivateProfileString(".ShellClassInfo","IconIndex","0",szFullPath,MAX_PATH,szImageBuf);
		iIndex=atoi(szFullPath);
		GetPrivateProfileString(".ShellClassInfo","IconFile","",szFullPath,MAX_PATH,szImageBuf);
			
		hIcon=ExtractIcon (hInstance, szFullPath, iIndex);
		return hIcon;
	}

	if (!_strnicmp (szImageBuf, ".extract", strlen (".extract")))
	{
		char		szBuffer[MAX_PATH+10];	// Leave some overhang for the icon number, if present
		
		if (szImageBuf[strlen (".extract")] == '=')
		{
			strcpy (szBuffer, szImageBuf + strlen (".extract="));
			szTemp = strrchr (szBuffer, ',');
		}
		else
		{
			if (!szFile)
			{
				return NULL;
			}
			
			strcpy (szBuffer, szFile);
		}
		
		if (szTemp) // .extract=c:\file.dll,3
		{
			*szTemp = '\0';
			
			szTemp++;
			iIndex = atoi (szTemp);
		}
		
		// Now szBuffer is the filename, and iIndex is the index of the icon (zero by default)
		if (iIndex < 0) 	// Keep the user from being stupid.
		{
			return NULL;
		}
		
		if (iIndex > 0)
		{
			int iCount;
			
			iCount = (int) ExtractIcon (hInstance, szBuffer, 0xffffffff);
			
			if (iIndex >= iCount)	// Is the index out of range?
			{
				return NULL;
			}
		}
		
		hIcon = ExtractIcon (hInstance, szBuffer, iIndex);
		if (hIcon)
		{
			return hIcon;
		}
	}
	else	
	{
		char szFullPath[MAX_PATH];

		strcpy (szFullPath, szImagesFolder);
		VarExpansion(szImageFinal, szImageBuf);
		strcat (szFullPath, szImageFinal);

		szTemp = strrchr (szFullPath, ','); // libary.icl,1
		if (szTemp) {
			*szTemp = '\0';
			
			szTemp++;
			iIndex = atoi (szTemp);

			hIcon=ExtractIcon (hInstance, szFullPath, iIndex);
			return hIcon;
		}


		hIcon = (HICON) LoadImage
			(
			hInstance,
			szFullPath,
			IMAGE_ICON,
			0,
			0,
			LR_DEFAULTCOLOR | LR_LOADFROMFILE
			);
		
		if (!hIcon)
		{
			hIcon = (HICON) LoadImage
				(
				hInstance,
				szImageFinal,
				IMAGE_ICON,
				0,
				0,
				LR_DEFAULTCOLOR | LR_LOADFROMFILE
				);
		}
		return hIcon;
	}
	
	return NULL;
}

void SetDesktopArea(int left, int top, int right, int bottom) {
	RECT r;

	r.left = left;
	r.top = top;
	r.right = right;
	r.bottom = bottom;
	SystemParametersInfo(SPI_SETWORKAREA,0,(PVOID)&r,SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)&r,SPIF_SENDCHANGE);
}

void GetLSBitmapSize(HBITMAP hBitmap, int *x, int *y)
{
	BITMAP bm;
	if (!GetObject(hBitmap, sizeof(bm), (LPSTR)&bm))
    {
		*x=0;
		*y=0;
    }
	else
    {
		*x = bm.bmWidth;
		*y = bm.bmHeight;
    }
}

void TransparentBltLS( HDC dc, int nXDest, int nYDest, int nWidth, 
	int nHeight, HDC tempDC, int nXSrc, int nYSrc,
	COLORREF colorTransparent ) {

	HDC locMemDC, maskDC;
	HBITMAP maskBitmap, bmpImage;
	HBITMAP hOldMemBmp, hOldMaskBmp;

	maskDC = CreateCompatibleDC(tempDC);
	locMemDC = CreateCompatibleDC(tempDC);
	bmpImage = CreateCompatibleBitmap(dc, nWidth, nHeight);
	hOldMemBmp = SelectObject(locMemDC, bmpImage);
	BitBlt(locMemDC, 0,0,nWidth, nHeight, tempDC, nXSrc, nYSrc, SRCCOPY);
	// Create monochrome bitmap for the mask
	maskBitmap = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
	hOldMaskBmp = SelectObject(maskDC, maskBitmap);
	SetBkColor(locMemDC, colorTransparent);
	// Create the mask from the memory DC
	BitBlt(maskDC, 0, 0, nWidth, nHeight, locMemDC, 0, 0, SRCCOPY);
	// Set the background in locMemDC to black. Using SRCPAINT with black 
	// and any other color results in the other color, thus making 
	// black the transparent color
	SetBkColor(locMemDC, RGB(0,0,0));
	SetTextColor(locMemDC, RGB(255,255,255));
	BitBlt(locMemDC, 0, 0, nWidth, nHeight, maskDC, 0, 0, SRCAND);
	// Set the foreground to black. See comment above.
	SetBkColor(dc, RGB(255,255,255));
	SetTextColor(dc, RGB(0,0,0));
	BitBlt(dc, nXDest, nYDest, nWidth, nHeight, maskDC, 0, 0, SRCAND);
	// Combine the foreground with the background
	BitBlt(dc, nXDest, nYDest, nWidth, nHeight, locMemDC, 0, 0, SRCPAINT);
	SelectObject(maskDC, hOldMaskBmp);
	SelectObject(locMemDC, hOldMemBmp);
	DeleteDC(maskDC);
	DeleteDC(locMemDC);
	DeleteObject(maskBitmap);
	DeleteObject(bmpImage);
	DeleteObject(hOldMemBmp);
	DeleteObject(hOldMaskBmp);
}

HWND GetLitestepWnd()
{
	return FindWindow("PureLSClass", "PureLS");
}

void Frame3D(HDC dc, RECT rect, COLORREF TopColor, COLORREF BottomColor, int Width)
{
	HPEN hPen1 = CreatePen(PS_SOLID, 1, TopColor), 
		hPen2 = CreatePen(PS_SOLID, 1, BottomColor),
		OldPen;
	POINT points[3];

	rect.bottom--;
	rect.right--;

	OldPen = SelectObject(dc, hPen1);

	while (Width > 0)
	{
		POINT p;
		Width--;

		points[0].x = rect.left;
		points[0].y = rect.bottom;
		points[1].x = rect.left;
		points[1].y = rect.top;
		points[2].x = rect.right;
		points[2].y = rect.top;

		Polyline(dc, points, 3);

		SelectObject(dc, hPen2);

		p = points[0];
		points[0] = points[2];
		points[1].x = rect.bottom;
		points[1].y = rect.right;
		points[2] = p;
		points[2].x--;

		Polyline(dc, points, 3);

		SelectObject(dc, hPen1);

		InflateRect(&rect, -1, -1);
	}
	SelectObject(dc, OldPen);
	DeleteObject(hPen1);
	DeleteObject(hPen2);
}

/*----------------------------------------------------------------------------
*
* Return TRUE if PATTERN has is a well formed regular expression according
* to the above syntax
*
* error_type is a return code based on the type of pattern error.  Zero is
* returned in error_type if the pattern is a valid one.  error_type return
* values are as follows:
*
*   PATTERN_VALID - pattern is well formed
*   PATTERN_ESC   - pattern has invalid escape ('\' at end of pattern)
*   PATTERN_RANGE - [..] construct has a no end range in a '-' pair (ie [a-])
*   PATTERN_CLOSE - [..] construct has no end bracket (ie [abc-g )
*   PATTERN_EMPTY - [..] construct is empty (ie [])
*
----------------------------------------------------------------------------*/

BOOL is_valid_pattern (char *p, int *error_type)
{
      /* init error_type */
      *error_type = PATTERN_VALID;

      /* loop through pattern to EOS */
      while (*p)
      {
            /* determine pattern type */
            switch (*p)
            {
            /* check literal escape, it cannot be at end of pattern */
            case '\\':
				  if (!*++p)
                  {
                        *error_type = PATTERN_ESC;
                        return FALSE;
                  }
                  p++;
                  break;
                  
                  /* the [..] construct must be well formed */
            case '[':
                  p++;

                  /* if the next character is ']' then bad pattern */
                  if (*p == ']')
                  {
						*error_type = PATTERN_EMPTY;
                        return FALSE;
                  }
                
                  /* if end of pattern here then bad pattern */
                  if (!*p)
                  {
                        *error_type = PATTERN_CLOSE;
                        return FALSE;
                  }

                  /* loop to end of [..] construct */
				  while (*p != ']')
                  {
                        /* check for literal escape */
                        if (*p == '\\')
                        {
                              p++;

                              /* if end of pattern here then bad pattern */
                              if (!*p++)
                              {
                                    *error_type = PATTERN_ESC;
                                    return FALSE;
                              }
                        }
                        else  p++;

                        /* if end of pattern here then bad pattern */
                        if (!*p)
                        {
                              *error_type = PATTERN_CLOSE;
                              return FALSE;
                        }

                        /* if this a range */
                        if (*p == '-')
                        {
                              /* we must have an end of range */
							  if (!*++p || *p == ']')
                              {
                                    *error_type = PATTERN_RANGE;
                                    return FALSE;
                              }
                              else
                              {

                                    /* check for literal escape */
                                    if (*p == '\\')
                                          p++;

                                    /* if end of pattern here
                                       then bad pattern           */
                                    if (!*p++)
									{
                                          *error_type = PATTERN_ESC;
                                          return FALSE;
                                    }
                              }
                        }
                  }
                  break;

                  /* all other characters are valid pattern elements */
            case '*':
            case '?':
			default:
                  p++;                              /* "normal" character */
                  break;
            }
      }
      return TRUE;
}

/*----------------------------------------------------------------------------
*
*  Match the pattern PATTERN against the string TEXT;
*
*  returns MATCH_VALID if pattern matches, or an errorcode as follows
*  otherwise:
*
*            MATCH_PATTERN  - bad pattern
*            MATCH_LITERAL  - match failure on literal mismatch
*            MATCH_RANGE    - match failure on [..] construct
*            MATCH_ABORT    - premature end of text string
*            MATCH_END      - premature end of pattern string
*            MATCH_VALID    - valid match
*
*
*  A match means the entire string TEXT is used up in matching.
*
*  In the pattern string:
*       `*' matches any sequence of characters (zero or more)
*       `?' matches any character
*       [SET] matches any character in the specified set,
*       [!SET] or [^SET] matches any character not in the specified set.
*
*  A set is composed of characters or ranges; a range looks like
*  character hyphen character (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the
*  minimal set of characters allowed in the [..] pattern construct.
*  Other characters are allowed (ie. 8 bit characters) if your system
*  will support them.
*
*  To suppress the special syntactic significance of any of `[]*?!^-\',
*  and match the character exactly, precede it with a `\'.
*
----------------------------------------------------------------------------*/

int matche (char *p, char *t)
{
      char range_start, range_end;  /* start and end in range */

      BOOL invert;             /* is this [..] or [!..] */
      BOOL member_match;       /* have I matched the [..] construct? */
      BOOL loop;               /* should I terminate? */

      for ( ; *p; p++, t++)
      {
            /* if this is the end of the text
			   then this is the end of the match */

            if (!*t)
            {
                  return ( *p == '*' && *++p == '\0' ) ?
                        MATCH_VALID : MATCH_ABORT;
            }

            /* determine and react to pattern type */

            switch (*p)
            {
            case '?':                     /* single any character match */
                  break;

			case '*':                     /* multiple any character match */
                  return matche_after_star (p, t);

            /* [..] construct, single member/exclusion character match */
            case '[':
            {
                  /* move to beginning of range */

                  p++;

                  /* check if this is a member match or exclusion match */

				  invert = FALSE;
                  if (*p == '!' || *p == '^')
                  {
                        invert = TRUE;
                        p++;
                  }

                  /* if closing bracket here or at range start then we have a
                        malformed pattern */

                  if (*p == ']')
                  {
                        return MATCH_PATTERN;
                  }

				  member_match = FALSE;
                  loop = TRUE;

                  while (loop)
                  {
                        /* if end of construct then loop is done */

                        if (*p == ']')
                        {
                              loop = FALSE;
                              continue;
                        }

                        /* matching a '!', '^', '-', '\' or a ']' */

                        if (*p == '\\')
                        {
                              range_start = range_end = *++p;
                        }
                        else  range_start = range_end = *p;

                        /* if end of pattern then bad pattern (Missing ']') */

                        if (!*p)
                              return MATCH_PATTERN;

                        /* check for range bar */
						if (*++p == '-')
                        {
                              /* get the range end */

                              range_end = *++p;
                              
                              /* if end of pattern or construct
                                 then bad pattern */

                              if (range_end == '\0' || range_end == ']')
                                    return MATCH_PATTERN;

							  /* special character range end */
                              if (range_end == '\\')
                              {
                                    range_end = *++p;

                                    /* if end of text then
                                       we have a bad pattern */
                                    if (!range_end)
                                          return MATCH_PATTERN;
                              }

                              /* move just beyond this range */
                              p++;
                        }

						/* if the text character is in range then match found.
                           make sure the range letters have the proper
                           relationship to one another before comparison */

                        if (range_start < range_end)
                        {
                              if (*t >= range_start && *t <= range_end)
                              {
                                    member_match = TRUE;
                                    loop = FALSE;
                              }
                        }
						else
                        {
                              if (*t >= range_end && *t <= range_start)
                              {
                                    member_match = TRUE;
                                    loop = FALSE;
                              }
                        }
                  }

                  /* if there was a match in an exclusion set then no match */
                  /* if there was no match in a member set then no match */

                  if ((invert && member_match) || !(invert || member_match))
                        return MATCH_RANGE;

				  /* if this is not an exclusion then skip the rest of
					 the [...] construct that already matched. */

				  if (member_match)
				  {
						while (*p != ']')
						{
							  /* bad pattern (Missing ']') */
							  if (!*p)
									return MATCH_PATTERN;

							  /* skip exact match */
							  if (*p == '\\')
							  {
									p++;

									/* if end of text then
									   we have a bad pattern */

									if (!*p)
										  return MATCH_PATTERN;
							  }

							  /* move to next pattern char */

							  p++;
						}
				  }
				  break;
			}
			case '\\':  /* next character is quoted and must match exactly */

				  /* move pattern pointer to quoted char and fall through */

				  p++;

				  /* if end of text then we have a bad pattern */

				  if (!*p)
						return MATCH_PATTERN;

				  /* must match this character exactly */

			default:
				  if (toupper(*p) != toupper(*t))
						return MATCH_LITERAL;
			}
	  }
	  /* if end of text not reached then the pattern fails */

	  if (*t)
			return MATCH_END;
	  else  return MATCH_VALID;
}


/*----------------------------------------------------------------------------
*
* recursively call matche() with final segment of PATTERN and of TEXT.
*
----------------------------------------------------------------------------*/

int matche_after_star (char *p, char *t)
{
	  int match = 0;
	  char nextp;

	  /* pass over existing ? and * in pattern */

	  while ( *p == '?' || *p == '*' )
	  {
			/* take one char for each ? and + */

			if (*p == '?')
			{
				  /* if end of text then no match */
				  if (!*t++)
						return MATCH_ABORT;
			}

			/* move to next char in pattern */

			p++;
	  }

	  /* if end of pattern we have matched regardless of text left */

	  if (!*p)
			return MATCH_VALID;

	  /* get the next character to match which must be a literal or '[' */

	  nextp = *p;
	  if (nextp == '\\')
	  {
			nextp = p[1];

			/* if end of text then we have a bad pattern */

			if (!nextp)
				  return MATCH_PATTERN;
	  }

	  /* Continue until we run out of text or definite result seen */

	  do
	  {
			/* a precondition for matching is that the next character
			   in the pattern match the next character in the text or that
			   the next pattern char is the beginning of a range.  Increment
			   text pointer as we go here */

			if (tolower(nextp) == tolower(*t) || nextp == '[')
				  match = matche(p, t);

			/* if the end of text is reached then no match */

			if (!*t++)
				  match = MATCH_ABORT;

	  } while ( match != MATCH_VALID &&
				match != MATCH_ABORT &&
				match != MATCH_PATTERN);

	  /* return result */

	  return match;
}

/*----------------------------------------------------------------------------
*
* match() is a shell to matche() to return only BOOL values.
*
----------------------------------------------------------------------------*/

BOOL match( char *p, char *t )
{
	  int error_type;

	  error_type = matche(p,t);
	  return (error_type == MATCH_VALID ) ? TRUE : FALSE;
}

// Added - Maduin, 10-20-1999
//   Helper function to retrieve the directory in which
//   LITESTEP.EXE resides.

BOOL WINAPI LSGetLitestepPath( LPTSTR pszPath, UINT nMaxLen )
{
	static TCHAR szPath[MAX_PATH] = { 0 };

	if( !szPath[0] )
	{
		HINSTANCE hInstance;
		int nLen;

		hInstance = (HINSTANCE) GetWindowLong( GetLitestepWnd(), GWL_HINSTANCE );
		GetModuleFileName( hInstance, szPath, MAX_PATH );
		nLen = lstrlen( szPath ) - 1;

		while( nLen > 0 && szPath[nLen] != '\\' )
			nLen--;

		szPath[nLen + 1] = 0;
	}

	lstrcpyn( pszPath, szPath, nMaxLen );
	return TRUE;
}

// Added - Maduin, 10-20-1999
//   Function added to prevent modules from querying
//   for the LSImageFolder (PixmapPath) command using
//   GetRCString directly.

BOOL WINAPI LSGetImagePath( LPTSTR pszPath, UINT nMaxLen )
{
	TCHAR szPath[MAX_PATH] = { 0 };

	if( !szPath[0] )
	{
		UINT nLen;

		GetRCString( TEXT("LSImageFolder"), szPath, TEXT(""), MAX_PATH );

		if( !szPath[0] )
		{
			// backwards compatibility (will be removed eventually)
			GetRCString( TEXT("PixmapPath"), szPath, TEXT(""), MAX_PATH );

			if( !szPath[0] )
			{
				LSGetLitestepPath( szPath, MAX_PATH );

				if( szPath[0] )
					lstrcat( szPath, TEXT("IMAGES\\") );
			}
		}

		nLen = lstrlen( szPath );

		if( nLen && szPath[nLen - 1] != '\\' )
			lstrcat( szPath, TEXT("\\") );
	}
	lstrcpyn( pszPath, szPath, nMaxLen );
	return TRUE;
}

/*
	$Log: lsapi.c,v $
	Revision 1.43  1999/01/16 02:37:16  bryan
	Variable Expansion extended to the whole of LS.
	
	Revision 1.42  1999/01/08 21:01:17  cyberian
	
	Fixed crash on recycle and loading of startup items
	
	Revision 1.39  1998/11/11 19:55:20  cyberian
	*** empty log message ***
	
 */

