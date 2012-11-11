#ifndef  __LSAPI_H
#define __LSAPI_H

// #include <windows.h>
// #include "../litestep/wharfdata.h"
// #include "../litestep/resource.h"

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


#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

__declspec( dllexport ) FILE* LCOpen (LPCTSTR szPath);

__declspec( dllexport ) BOOL LCClose (FILE *f);

__declspec( dllexport ) BOOL LCReadNextCommand (FILE *f, LPSTR szBuffer, DWORD dwLength);

__declspec( dllexport ) BOOL LCReadNextConfig (FILE *f, LPCSTR szPrefix, LPSTR szBuffer, DWORD dwLength);

__declspec( dllexport ) int LCTokenize (LPCSTR szString, char **lpszBuffers, DWORD dwNumBuffers, LPSTR szExtraParameters);

__declspec( dllexport ) HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance, int xoffset, int yoffset);

__declspec( dllexport ) BOOL ParseBangCommand (HWND caller, char *command, char *args);

__declspec( dllexport ) BOOL AddBangCommand(char *command, void * f);

__declspec( dllexport ) BOOL AddBangCommandEx(char *command, void * f);

__declspec( dllexport ) BOOL RemoveBangCommand(char *command);

__declspec( dllexport ) HBITMAP LoadLSImage(LPCSTR szImage, LPCSTR szFile);

__declspec( dllexport ) HBITMAP BitmapFromIcon (HICON hIcon);

__declspec( dllexport ) HICON LoadLSIcon (LPCSTR szImage, LPCSTR szFile);

__declspec( dllexport) void SetDesktopArea(int left, int top, int right, int bottom);

__declspec( dllexport) void GetLSBitmapSize(HBITMAP hBitmap, int *x, int *y);

__declspec( dllexport) void TransparentBltLS (HDC dc, int nXDest, int nYDest, int nWidth, int nHeight, HDC tempDC, int nXSrc, int nYSrc, COLORREF colorTransparent);

__declspec( dllexport ) HWND GetLitestepWnd();

__declspec( dllexport ) void VarExpansion(char *buffer, const char * value);

__declspec( dllexport ) BOOL SetupRC(LPCTSTR szPath);

__declspec( dllexport ) int GetRCInt(LPCTSTR lpKeyName, int nDefault);

__declspec( dllexport ) BOOL GetRCString(LPCTSTR lpKeyName, LPSTR value, LPCTSTR defStr, int maxLen);

__declspec( dllexport ) BOOL GetRCBool(LPCTSTR lpKeyName, BOOL ifFound);

__declspec( dllexport ) COLORREF GetRCColor(LPCTSTR lpKeyName, COLORREF colDef);

__declspec( dllexport ) void CloseRC(void);

__declspec( dllexport ) void Frame3D(HDC dc, RECT rect, COLORREF TopColor, COLORREF BottomColor, int Width);

__declspec( dllexport ) BOOL match (char *pattern, char *text);

__declspec( dllexport ) int matche(char *pattern, char *text);

__declspec( dllexport ) BOOL is_pattern (char *pattern);

__declspec( dllexport ) BOOL is_valid_pattern (char *pattern, int *error_type);

#ifdef	__cplusplus
};
#endif	/* __cplusplus */

#endif  /* __LSAPI_H */
