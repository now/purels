#ifndef  __LSAPI_H
#define __LSAPI_H

#define LSAPI

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

LSAPI FILE* LCOpen (LPCTSTR szPath);
LSAPI BOOL LCClose (FILE *f);
LSAPI BOOL LCReadNextCommand (FILE *f, LPSTR szBuffer, DWORD dwLength);
LSAPI BOOL LCReadNextConfig (FILE *f, LPCSTR szPrefix, LPSTR szBuffer, DWORD dwLength);
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
