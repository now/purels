/*

  This is a part of the LiteStep Shell Source code modified for
	use with PureLS Source code.

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
#include <shlobj.h>
#include <io.h>

#define STARTUP_OPENED
#include "startup.h"


// from old LSAPI.DLL
HINSTANCE LSExecuteEx ( HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, INT );

void RunEntriesIn ( HKEY key, LPCTSTR path )
{
	HKEY hKey = NULL;
	LONG lResult = RegOpenKeyEx(key, path, 0, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS){
		_TCHAR szNameBuffer[1024], szValueBuffer[1024];
		DWORD dwLoop, dwNameSize, dwValueSize;

		for (dwLoop=0; lResult == ERROR_SUCCESS; ++dwLoop){
			dwNameSize = sizeof(szNameBuffer);
			dwValueSize = sizeof(szValueBuffer);
			lResult = RegEnumValue(hKey, dwLoop, szNameBuffer, &dwNameSize, NULL, NULL, (LPBYTE) szValueBuffer, &dwValueSize);
			if (lResult == ERROR_SUCCESS)
				WinExec(szValueBuffer, SW_SHOW);
		}
		RegCloseKey(hKey);
	}
	return;
}

void DeleteEntriesIn ( HKEY key, LPCTSTR path )
{
	HKEY hKey = NULL;
	LONG lResult = RegOpenKeyEx(key, path, 0, KEY_ALL_ACCESS, &hKey);

	if (lResult == ERROR_SUCCESS){
		TCHAR szNameBuffer[1024], szValueBuffer[1024];
		DWORD dwLoop, dwNameSize, dwValueSize;

		for (dwLoop=0; lResult == ERROR_SUCCESS; ){
			dwNameSize = sizeof(szNameBuffer);
			dwValueSize = sizeof(szValueBuffer);

			lResult = RegEnumValue(hKey, dwLoop, szNameBuffer, &dwNameSize, NULL, NULL, (LPBYTE) szValueBuffer, &dwValueSize);
			if (lResult == ERROR_SUCCESS)
				lResult = RegDeleteValue(hKey,szNameBuffer);
		}
		RegCloseKey (hKey);
	}
	return;
}

void RunFolderContents( LPCTSTR szParams )
{
	_TCHAR szDir[_MAX_PATH], szPath[_MAX_PATH];
	struct _finddata_t finddata;
	long search_handle;

	_tcscpy(szPath, szParams);
	_tcscpy(szDir, szPath);
	if (szPath[strlen(szPath)-1] != '\\')
		_tcscat(szPath, "\\");
	_tcscat(szPath, "*.*");

	if (strcmp(szPath,"\\*.*")!=0){
		search_handle = _findfirst(szPath, &finddata);
		while (search_handle != -1){
			if (strcmp(finddata.name, ".") && strcmp(finddata.name, ".."))
				LSExecuteEx(NULL, NULL, finddata.name, NULL, szDir, SW_SHOWNORMAL);
			if (_findnext(search_handle, &finddata) == -1){
				_findclose(search_handle);
				search_handle = -1;
			}
		}
	}
	return;
}

void RunStartupMenu ( void )
{
	_TCHAR szPath[_MAX_PATH];
	LPITEMIDLIST item;
	if (SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_STARTUP, &item) == NOERROR)
		if (SHGetPathFromIDList(item, szPath) == TRUE)
			RunFolderContents(szPath);

	if (SHGetSpecialFolderLocation(NULL,CSIDL_STARTUP,&item) == NOERROR)
		if (SHGetPathFromIDList(item, szPath) == TRUE)
			RunFolderContents(szPath);
	return;
}
