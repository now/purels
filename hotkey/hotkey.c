/*

  This is a part of the LiteStep Shell Source code.

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

/****************************************************************************
 19/02/99 - D. Monk
 			Added virtual keys Num0 - Num 9 suggested by Ben Gruver<JesusFreke@usa.net>
 09/11/98 - Fahim Farook
			Added support for the WIN key to run popup menu
 16/09/98 - J. Vaughn
            Added support for F keys, cursor keys, etc
			broken tho, cant seem to get it to work?
 17/07/98 - D. Hodgkiss
            This file contains the source code for the hotkeys module

****************************************************************************/


#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include <malloc.h>

#include "lsapi.h"

const char rcsRevision[] = "$Revision: 1.2 $"; // Our Version 
const char rcsId[] = "$Id: hotkey.c,v 1.2 2000/02/14 06:02:15 message Exp $"; // The Full RCS ID.

static const tVKTable VKTable[] = {
		{ "HOME",	VK_HOME },
		{ "END",    VK_END},
		{ "PAGEUP", VK_PRIOR},
		{ "PAGEDOWN",  VK_NEXT},
		{ "INSERT", VK_INSERT},
		{ "DELETE",	VK_DELETE},
		{ "F1",		VK_F1},
		{ "F2",		VK_F2},
		{ "F3",		VK_F3},
		{ "F4",		VK_F4},
		{ "F5",		VK_F5},
		{ "F6",		VK_F6},
		{ "F7",		VK_F7},
		{ "F8",		VK_F8},
		{ "F9",		VK_F9},
		{ "F10",	VK_F10},
		{ "F11",	VK_F11},
		{ "F12",	VK_F12},
		{ "ESCAPE", VK_ESCAPE},
		{ "LEFT",   VK_LEFT},
		{ "RIGHT",  VK_RIGHT},
		{ "UP",		VK_UP},
		{ "DOWN",	VK_DOWN},
		{ "SPACEBAR", VK_SPACE},
		{ "BACKSPACE", VK_BACK},
		{ "NUM0",   VK_NUMPAD0},
		{ "NUM1",   VK_NUMPAD1},
		{ "NUM2",   VK_NUMPAD2},
		{ "NUM3",   VK_NUMPAD3},
		{ "NUM4",   VK_NUMPAD4},
		{ "NUM5",   VK_NUMPAD5},
		{ "NUM6",   VK_NUMPAD6},
		{ "NUM7",   VK_NUMPAD7},
		{ "NUM8",   VK_NUMPAD8},
		{ "NUM9",   VK_NUMPAD9}
};

#define MAX_VKEYS       (sizeof(VKTable) / sizeof(tVKTable))

const char szAppName[] = "HotkeyWindow";

char szLitestepPath[256];

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT KeyHook(int code, WPARAM wParam, LPARAM lParam);
void loadHotkeys();
void freeHotkeys();

hotkeyType *hotkeys = NULL;
HWND hMainWnd = NULL; // main window handle
HWND parent = NULL;
HINSTANCE dll = NULL;
HHOOK hSystemHook;  // Handle to keyboard hook
int numHotkeys = 0;
DWORD desktopThread;

// -------------------------------------------------------------------------------------------------------

int initModule(HWND ParentWnd, HINSTANCE dllInst, wharfDataType* wd)
{
	return initModuleEx (ParentWnd, dllInst, wd->lsPath);
}

int initModuleEx (HWND ParentWnd, HINSTANCE dllInst, LPCSTR szPath)
{
	int Msgs[10];
	strcpy (szLitestepPath, szPath);
	dll = dllInst;
	parent = ParentWnd;

    {
		WNDCLASS wc;

		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = WndProc;
		wc.hInstance = dllInst;
		wc.lpszClassName = szAppName;
		wc.style = 0;

		if (!RegisterClass(&wc))
		{  
			MessageBox(parent, "Error registering window class", szAppName, MB_OK);
			return 1;
		}
	}

	loadHotkeys();

	hMainWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		szAppName,
		"LSHotKey",
		WS_CHILD,
		0,
		0,
		0,
		0,
		parent,
		NULL,
		dllInst,
		NULL);
	
	Msgs[0] = LM_GETREVID;
	Msgs[1] = 0;
	SendMessage(parent, LM_REGISTERMESSAGE, (WPARAM) hMainWnd, (LPARAM) Msgs);
	if (GetRCBool("HotkeyLoadExplorerKeys", TRUE)){
		drizzt_InitHotKeys ();
	}
	return 0;
}

void quitModule(HINSTANCE dllInst)
{
	int Msgs[10];
	
	Msgs[0] = LM_GETREVID;
	Msgs[1] = 0;
	SendMessage(parent, LM_UNREGISTERMESSAGE, (WPARAM) hMainWnd, (LPARAM) Msgs);
	freeHotkeys();
	if (GetRCBool("HotkeyLoadExplorerKeys", TRUE)){
		drizzt_FreeHotKeys ();
	}
	UnhookWindowsHookEx(hSystemHook);
	DestroyWindow(hMainWnd);
	UnregisterClass(szAppName, dllInst);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case LM_GETREVID:
		{
			char *buf = (char *) lParam;

			if (wParam == 0)
			{
				strcpy(buf, "hotkey.dll: ");
				strcat(buf, &rcsRevision[11]);
				buf[strlen(buf)-1] = '\0';
			}
			else if (wParam == 1)
			{
				strcpy(buf, &rcsId[1]);
				buf[strlen(buf)-1] = '\0';
			} else
			{
				strcpy(buf, "");
			}
			return strlen(buf);

		}
	case WM_ENDSESSION:
	case WM_QUERYENDSESSION:
		return SendMessage(parent,message,wParam,lParam);
	case WM_SYSCOMMAND:
		{
		switch (wParam)
			{
			case SC_CLOSE:
				PostMessage(parent,WM_KEYDOWN,LM_SHUTDOWN,0);
				return 0;
			default:
				return DefWindowProc(hwnd,message,wParam,lParam);
			}
		}

	case WM_CREATE:
		{
			int i;

			for (i=0;i<numHotkeys;i++)
			{
				RegisterHotKey(hwnd,i,hotkeys[i].sub,hotkeys[i].ch);
			}
			if (GetRCBool("HotkeyNoWinKeyPopup",FALSE)){
				if (!RegisterHotKey(hwnd, GlobalAddAtom("LWIN_KEY"),MOD_WIN, VK_LWIN)){
					MessageBox(NULL,"Error registering Win Key",szAppName,MB_OK);
				}
				if (!RegisterHotKey(hwnd, GlobalAddAtom("RWIN_KEY"),MOD_WIN, VK_RWIN)){
					MessageBox(NULL,"Error registering Win Key",szAppName,MB_OK);
				}
			}
			if (GetRCBool("LSNoShellWarning", FALSE) && GetRCBool("ExplorerNoWarn", FALSE)){
				if (!RegisterHotKey(hwnd, GlobalAddAtom("CTL_ESC"),MOD_CONTROL,VK_ESCAPE)){
					MessageBox(NULL,"Error registering Ctrl+Esc",szAppName,MB_OK);
				}
			} 
		}
		return 0;

	case WM_DESTROY:
		UnregisterHotKey(hwnd, GlobalFindAtom("LWIN_KEY"));
		UnregisterHotKey(hwnd, GlobalFindAtom("RWIN_KEY"));
		UnregisterHotKey(hwnd, GlobalFindAtom("CTL_ESC"));
		GlobalDeleteAtom(GlobalFindAtom("LWIN_KEY"));
		GlobalDeleteAtom(GlobalFindAtom("RWIN_KEY"));
		GlobalDeleteAtom(GlobalFindAtom("CTL_ESC"));
		return 0;

	case WM_ERASEBKGND: 
		return 0;

	case WM_PAINT:  
		return 0;

	case WM_HOTKEY:
		{
			if (wParam <  (WPARAM) numHotkeys)
			{
				int num = wParam;

				if (lstrlen(hotkeys[num].szCommand))
				{
					if (hotkeys[num].szCommand[0] == '!') {
						KillTimer(hwnd, 1);
						ParseBangCommand(hwnd, hotkeys[num].szCommand, hotkeys[num].szParameters);
					} else {
			            char workDirectory[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR];

						_splitpath(hotkeys[num].szCommand, drive, dir, NULL, NULL);
						strcpy(workDirectory, drive);
						strcat(workDirectory, dir);

						LSExecuteEx(GetDesktopWindow(), NULL, hotkeys[num].szCommand, hotkeys[num].szParameters, workDirectory, SW_SHOWNORMAL);

						KillTimer(hwnd, 1);
						/*
						SHELLEXECUTEINFO si;
						char workDirectory[MAX_PATH];
						char drive[_MAX_DRIVE];
						char dir[_MAX_DIR];
		
						_splitpath(hotkeys[num].szCommand, drive, dir, NULL, NULL);
						strcpy(workDirectory, drive);
						strcat(workDirectory, dir);
						memset(&si, 0, sizeof(si));
						si.cbSize = sizeof(SHELLEXECUTEINFO);
						si.lpDirectory = workDirectory;
						si.lpVerb = NULL;
						si.nShow = 1;
						si.fMask = SEE_MASK_DOENVSUBST;
						si.lpFile = hotkeys[num].szCommand;
						si.lpParameters = hotkeys[num].szParameters;
						ShellExecuteEx(&si);
						*/
					}
				}
			}
			else
			{
				char szCommand[1024];

				if (GlobalGetAtomName((ATOM)wParam, szCommand, 1024) > 0)
				{
					if (!strcmp(szCommand, "CTL_ESC"))
						SendMessage(parent, LM_POPUP, 0, 0);
					else if (!strcmp(szCommand, "REINIT") && GetRCBool("HotkeyLoadExplorerKeys", TRUE)) {
						drizzt_FreeHotKeys();
						drizzt_InitHotKeys();
					} else if (!strcmp(szCommand, "LWIN_KEY")) {
						SetTimer(hwnd, 1, 750, NULL);
					} else if (!strcmp(szCommand, "RWIN_KEY")) {
						SetTimer(hwnd, 1, 750, NULL);
					} else {
						KillTimer(hwnd, 1);
						if (GetRCBool("HotkeyLoadExplorerKeys", TRUE)){
							drizzt_execute(szCommand, NULL, NULL);
						}
					}
				}
			}

			break;
		}
	case WM_TIMER:
		if (wParam == 1) {
			ParseBangCommand(hwnd, "!Popup", NULL);
			KillTimer(hwnd, 1);
		}
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

void freeHotkeys()
{
	int i;

	for (i=0;i<numHotkeys;i++)
	{
		UnregisterHotKey(hMainWnd,i);
	}

	if (hotkeys != NULL)
		free(hotkeys);
}

void loadHotkeys()
{
    FILE *f;
	int i;

	f = LCOpen(NULL);
	if (f)
	{
		char	buffer[4096];
		char	token1[4096], token2[4096], token3[4096], token4[4096], extra_text[4096];
		char*	tokens[4];
		
		tokens[0] = token1;
		tokens[1] = token2;
		tokens[2] = token3;
		tokens[3] = token4;

		buffer[0] = 0;

		while (LCReadNextConfig (f, "*Hotkey", buffer, sizeof (buffer)))
		{
			int count;

			token1[0] = token2[0] = token3[0] = token4[0] = extra_text[0] = '\0';

			count = LCTokenize (buffer, tokens, 4, extra_text);
			
			switch(count)
			{
			case 4:
				{
					char *tmp;

					if (!hotkeys)
						hotkeys = (hotkeyType *)malloc(sizeof(hotkeyType));
					else
						hotkeys = realloc(hotkeys, (numHotkeys+1)*sizeof(hotkeyType));

					hotkeys[numHotkeys].sub = 0;
					tmp = strtok(token2, "+");
					while (tmp)
					{
						if (!strcmpi(tmp, "Win"))
							hotkeys[numHotkeys].sub |= MOD_WIN;
						if (!strcmpi(tmp, "Alt"))
							hotkeys[numHotkeys].sub |= MOD_ALT;
						if (!strcmpi(tmp, "Ctrl"))
							hotkeys[numHotkeys].sub |= MOD_CONTROL;
						if (!strcmpi(tmp, "Shift"))
							hotkeys[numHotkeys].sub |= MOD_SHIFT;
						tmp = strtok(NULL, "+");
					}
					if (lstrlen(token3) == 1)
					{
						hotkeys[numHotkeys].ch = token3[0];
					} else {
						for (i = 0; i < MAX_VKEYS; i++)
						{
							if (!strcmpi(VKTable[i].key, token3))
							{
								hotkeys[numHotkeys].ch = VKTable[i].vkey;
							}
						}
					}

					strcpy(hotkeys[numHotkeys].szCommand, token4);
					strcpy(hotkeys[numHotkeys].szParameters, extra_text);
					numHotkeys++;
				}
			}
		}
		LCClose(f);
	}
}

/*
	$Log: hotkey.c,v $
	Revision 1.2  2000/02/14 06:02:15  message
	*** empty log message ***
	
	Revision 1.26  2000/01/06 05:26:12  message
	Changed source files back to dos format, somehow had become unix format, updated
	dependencies of desktop to include lsutil, wasn't linking properly because lsutil
	was not being built first.
	
	Revision 1.14  1998/12/10 15:22:52  cyberian
	
	Changed all instances using SHELLEXECUTEINFO to set the working directory to the application directory unless otherwise specified
	
 */