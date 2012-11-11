/*
	This is part of the PureLS Source code, based off of the
	LiteStep Shell Source code, available from www.litestep.net

	Copyright (C) 2000 The PureLS Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define _WIN32_IE 0x0000

#include <tchar.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h> // thread call
#include "../purels/plsdefs.h"
#include "../purels/purels.h"
//#include "../config/config.h" //needed for loading buffer into memory
#include "../purels/lsapi.h" //needed for loading module lines... temporary dependancy.


/*************** TEMPORARY USE OF OLD REVISION REPORTING ***********************/
const _TCHAR rcsRevision[] = _T("$Revision: 0.30beta $");
const _TCHAR rcsId[] = _T("$Id: purels.c, v0.30beta 2000/05/29 00:22:00 jugg $");
/*******************************************************************************/

LPCTSTR szAppClass = _T("PureLSClass");
LPCTSTR szAppName = _T("PureLS");
PTSTR pszAppPath = NULL;

MessageData* MSGCallbacks = NULL;
ModuleData* Modules = NULL;
pLSSettings pls;

_TINT nModuleCount = 0;
_TINT nMessageCount = 0;
HWND hPLSWnd = NULL;
HINSTANCE hPLSInstance = NULL;
BOOL bRecycle = FALSE;
BOOL bEndSession = FALSE;
BOOL bSkipEvents = FALSE;
BOOL bStartup = TRUE;

/************ TEMPORARY USE OF OLD LSAPI.DLL FOR READING CONFIG SETTINGS**********/
BOOL SetupRC(LPCTSTR szPath);
void CloseRC(void);
/*********************************************************************************/


/***************************** Function Prototypes *******************************/
// Functions are Static, because the only interface to us should happen through
// the Message Handler, not through exporting our functions!
/*********************************************************************************/

/********************/
/* Message Handlers */
/********************/
static LRESULT CALLBACK WndProcPLS ( HWND, _TINT, WPARAM, LPARAM );
static _TINT HandlerExists ( _TINT );
static HRESULT PassOnMessage ( _TINT, WPARAM, LPARAM, BOOL );
static void AddMessage ( HWND, _TINT );
static void AddWndMessages ( HWND, _TINT* ); // Wrapper to AddMessage
static void RemoveMessage ( HWND, _TINT );
static void RemoveWndMessages ( HWND, _TINT* ); // Wrapper to RemoveMessage
static void PurgeMessages ( void );

/*******************/
/* Module Handlers */
/*******************/
	// Starts specified module
static BOOL StartModule ( LPCTSTR );

	// Wrapper to StartModule()
	// Reads config file and loads
	// all specified modules.
static void AddModules ( void );

	// Stops specified module.
static BOOL StopModule ( LPCTSTR );

	// Wrapper to StopModule()
	// unloads all modules.
static void RemoveModules ( void ); // Wrapper to StopModule

	// Eventually will call a new function
	// that will be implmented in modules
//static BOOL ResetModule ( LPCTSTR ); /* NOT FULLY IMPLEMENTED */

	// Wrapper to Start/StopModule()
	// unloads and reloads specified
	// module, and returns a failure
	// value depending on its success.
	// 0-Not Stopped (FAIL)
	// 1-Stopped
	// 2-Stopped/Started (SUCCESS)
static _TINT StopStartModule ( LPCTSTR );

	// Checks to see whether or not specified
	// module is loaded. This function will be
	// expanded to actually verify if the module
	// is a valid pLS or LS module, with different
	// return mask values to be more detailed.
static _TINT IsModule ( LPCTSTR );

/*********/
/* Other */
/*********/
	// Process waiting events.
	// Will be moving this to the
	// API soon.
static void DoEvents( HWND, _TINT );

	// Loads Startup.dll if it exists
	// and not disabled.
static void RunStartupStuff ( void* );


/***************************** Function Definitions ******************************/
/*********************************************************************************/

pLSSettings LoadpLSSetup ( PTSTR pCmdLine )
{
	// at this point, we are not copying pCmdLine into a buffer,
	// as this is a one time execution, and doesn't matter if we
	// modify it, as it -should- never be used again.

	// More settings to be added. The custom configuration file (step.rc)
	// switch is actually going to be removed from here and put into
	// config.dll once we finish the code for config.dll.

	PTSTR pTmp;
	_TINT len;
	_TINT n;
	pLSSettings settings;

	settings.cStartup = 1;
	settings.cModules = 1;
	settings.rcFile = NULL;
#ifdef _DEBUG
	settings.cDebugMode = 0;
#endif

	if (pCmdLine != NULL && *pCmdLine && _tcschr(pCmdLine, _T('/')) != NULL){
		n=0;
		len = _tcslen(pCmdLine);
		pTmp = pCmdLine;
		// put 0's in place of the '/' delim
		// we use the stored 'len' value to navigate
		// though the command line data.
		while (n++ < len){
			if (*pTmp == _T('/'))
				*pTmp = 0;
			++pTmp;
		}
		n=0;
		pTmp = pCmdLine;
		do{
			// move to the next zero
			while (n<len && *pTmp){
				++n;
				++pTmp;
			}
			// if its not the end of the line
			// increment past the zero
			if (n<len){
				++n;
				++pTmp;
			}
			// skip any whitespace between the
			// '/' (now a zero) and the switch char.
			while (n<len && _istspace(*pTmp)){
				++n;
				++pTmp;
			}
			// if it looks like we got something
			// lets parse it.
			if (n != len){
				switch (*pTmp){
					case _T('f'):
						{ // configuration file. Typically "step.rc"
							_TCHAR tmpPath[PLS_MAX_FILE];
							++n;
							++pTmp;
							if (_tcschr(pTmp, _T('\\')) == NULL){
								_tcscpy(tmpPath, pszAppPath);
								_tcscat(tmpPath, pTmp);
							}else
								_tcscpy(tmpPath, pTmp);
							scStripWS(tmpPath);
							// If file does not exist, don't use it.
							if (GetFileAttributes(tmpPath) != 0xFFFFFFFF){
								// if it already exits, lets free it
								// This is a safety for users specifying
								// multiple "/f" switches, meaning the last valid
								// one specified is the one that will be used.
								if (settings.rcFile != NULL){
									free(settings.rcFile);
									settings.rcFile = NULL;
								}
								settings.rcFile = (PTSTR)malloc((_tcslen(tmpPath)+1)*sizeof(_TCHAR));
								if (settings.rcFile != NULL)
									_tcscpy(settings.rcFile, tmpPath);
							}
							tmpPath[0] = 0;
							break;
						}
					case _T('s'):
						{ // Run startup items.
							// Off (0)
							// On (1) (default)
							++n;
							++pTmp;
							if (_istdigit(*pTmp))
								settings.cStartup = (_TCHAR)(*pTmp - 48);
							break;
						}
					case _T('m'):
						{ // Options for loading modules
							// Don't load Modules (0)
							// Load Modules (1) (default)
							// Prompt to quit pLS if no modules loaded (2)
							// Prompt to load each individual module (3)
							++n;
							++pTmp;
							if (_istdigit(*pTmp))
								settings.cModules = (_TCHAR)(*pTmp - 48);
							break;
						}
#ifdef _DEBUG
					case _T('d'):
						{ // debug message levels (debug version only)
							// Compiler debug messages (0) (default)
							// MessageBox debug messages (1)
							++n;
							++pTmp;
							if (_istdigit(*pTmp))
								settings.cDebugMode = (_TCHAR)(*pTmp - 48);
							break;
						}
#endif
				}
			}
		}while (n != len);
	}

	// Lets setup the default configuration file "step.rc" located in our
	// home folder. There is no real reason to check to see if the
	// settings.rcFile is valid here, but in never hurts to be careful.
	if (settings.rcFile == NULL || GetFileAttributes(settings.rcFile) == 0xFFFFFFFF){
		if (settings.rcFile != NULL){
			free(settings.rcFile);
			settings.rcFile = NULL;
		}
		settings.rcFile = (PTSTR)malloc((_tcslen(pszAppPath)+8)*sizeof(_TCHAR));
		if (settings.rcFile != NULL){
			_tcscpy(settings.rcFile, pszAppPath);
			_tcscat(settings.rcFile, _T("step.rc"));
		}
		// we don't really care if the default "step.rc" file exists or not.
	}

	// If the user inputed bogus data, lets fix their mistakes.
	if (settings.cModules > 3)
		settings.cModules = 1;
	if (settings.cStartup > 1)
		settings.cStartup = 1;
#ifdef _DEBUG
	if (settings.cDebugMode > 1)
		settings.cDebugMode = 0;
#endif

	return settings;
}

INT WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hNULL, PSTR pCmdLine, INT nCmdShow )
{
	ATOM aClass;
	WNDCLASS wc;

	// their's a compiler "function" to say not to produce a warning
	// for unreferenced params, but I forget what it is.
	nCmdShow=nCmdShow;
	pCmdLine=pCmdLine;
	hNULL=hNULL; // normally shown as hPrevInstance, but for win32 apps, this is always NULL.

	hPLSInstance = hInstance;

	// If SHIFT key is being held down, disable Startup Items flag.
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		bStartup = FALSE;

	{ // lets get our home directory
		_TCHAR tmpAppPath[PLS_MAX_FILE];
		// GetModuleFileName() puts the returned path all in CAPS, instead
		// of the path name's true case. Looking into a solution.
		DWORD len = GetModuleFileName(hPLSInstance, tmpAppPath, PLS_MAX_FILE);
		while (len && tmpAppPath[len-1] != _T('\\'))
			--len;
		if (len && (pszAppPath = (PTSTR)malloc((len+1)*sizeof(_TCHAR))) != NULL){
			_tcsncpy(pszAppPath,tmpAppPath,len);
			pszAppPath[len] = 0;
		}else{
			MessageBox(NULL, _T("Could not retrieve a valid path."), _T("Error running PureLS!"), MB_OK|MB_ICONERROR|MB_DEFBUTTON1|MB_SYSTEMMODAL);
			// its not the greatest thing to have more
			// then one return statement, but for memory
			// usage, this is the best way at this point.
			return 0;
		}
	} 

// I feel like putting the #ifdef _DEBUG check inside of the DebugPrintf()
// function, just to make our code look cleaner, however it would then
// cause extra processing to be done in the Release version, as it would
// have to enter the function in each call. So for now our code just looks
// a little ugly. (like it doesn't anyway :)
#ifdef _DEBUG
	DebugPrintf(FALSE, _T("PureLS Dir"), pszAppPath);
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000){
		free(pszAppPath);
		return 0;
	}
#endif

	memset(&wc,0,sizeof(wc));
	wc.lpfnWndProc = (WNDPROC)WndProcPLS;
	wc.hInstance = hPLSInstance;
	wc.lpszClassName = szAppClass;
	aClass = RegisterClass(&wc);
	if (aClass){
		hPLSWnd = CreateWindowEx(WS_EX_TOOLWINDOW,szAppClass,szAppName,WS_POPUP,0,0,0,0,NULL,NULL,hPLSInstance,NULL);
		if (hPLSWnd){
			// Initialize nChkMsgErr and message, to get rid of compiler
			// warning, even though we could never get to
			// the 2nd message loop without entering the first.
			MSG message;
			INT nChkMsgErr=1;
			// no real reason to do this, 'cept to get rid
			// of compiler warning, as stated above.
			memset(&message, 0, sizeof(message));

			pls = LoadpLSSetup(pCmdLine);

#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("PureLS Settings"), _T("Config File: %s\n\rModules: %1u\n\rStartup: %1u\n\r"), pls.rcFile, pls.cModules, pls.cStartup);
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000){
		free(pszAppPath);
		free(pls.rcFile);
		return 0;
	}
#endif

			do{
//				{ // load up the config
//					_TCHAR tmpPath[PLS_MAX_FILE];
//					_tcscpy(tmpPath, pszAppPath);
//					_tcscat(tmpPath, _T("step.rc"));
//					scFillBuffer(tmpPath);
//					SetupRC(pszCfgFile);
//				}
				//scFillBuffer(pls.rcFile);
				// temporary LSAPI use
				SetupRC(pls.rcFile);
				AddModules();

				/**********************************/
				if (bRecycle && HandlerExists(9211)) // temporary compatibility for Maduin's Systray module.
					PassOnMessage(9211,0,0,TRUE); // lets tell systray.dll to restore data
				/**********************************/

				// Running Startup procedure is loaded into a seperate
				// thread to keep pLS safe from crashes that would
				// otherwise kill pLS as well.
				//
				// Even though we already checked for the Shift Key,
				// lets check again, as maybe they were slow to press it.
				// Plus verify that the command line param allows startup items.
				if (!bRecycle && pls.cStartup == 1 && bStartup && !(GetAsyncKeyState(VK_SHIFT) & 0x8000))
					_beginthread(RunStartupStuff, 0, NULL); // thread is exited automatically.

				// Really need to look into this further. It appears that if
				// we don't use this, Explorer File Manger under NT works nicer.
				if (!bRecycle)
					SendMessage(GetDesktopWindow(),0x400,0,0); //tells Windows the shell has finished loading.
				else
					bRecycle = FALSE;

//				nChkMsgErr = GetMessage(&message,NULL,0,0);
//				while (nChkMsgErr && nChkMsgErr != -1 && !bRecycle && !bEndSession){
//					TranslateMessage(&message);
//					DispatchMessage(&message);
//					if (!bRecycle && !bEndSession)
//						nChkMsgErr = GetMessage(&message,NULL,0,0);
//				}

				// if we get a bRecycle or bEndSession, then GetMessage() is never called
				// with this logic. Previously it was possible for GetMessage to be called
				// but the message never processed, because of our logic. Now the message
				// is always processed if it is retrieved.
				while (!bRecycle && !bEndSession && (nChkMsgErr = GetMessage(&message,NULL,0,0)) != -1 && nChkMsgErr){
					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				/**********************************/
				if (bRecycle && HandlerExists(9210)) // temporary compatibility for Maduin's Systray module.
					PassOnMessage(9210,0,0,TRUE); // lets tell systray.dll to save data
				/**********************************/

				RemoveModules();
				// Lets be sure to clean up all the messages
				// since some modules forget to unregister
				// the messages they registered.
				PurgeMessages();
//				scFreeBuffer();
				// temporary LSAPI use
				CloseRC();
			}while (bRecycle && !bEndSession);

			// free our memory.
			free(pszAppPath);
			pszAppPath = NULL;
			free(pls.rcFile);
			pls.rcFile = NULL;

			// we are quiting manually
			// no because of logoff/windows exit
			if (bSkipEvents){
				DestroyWindow(hPLSWnd);
				hPLSWnd=NULL;
			}

			// gotta do a slightly different message loop,
			// since if we exited the last one because of a
			// PostQuitMessage() or an error -1, we don't
			// want to go into this one.
			while (nChkMsgErr && nChkMsgErr != -1){
				nChkMsgErr = GetMessage(&message,NULL,0,0);
				if (nChkMsgErr && nChkMsgErr != -1){
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}

			// This shouldn't be necessary, but there is no
			// reason not to have this security, so lets do it.
			if (hPLSWnd != NULL){
				DestroyWindow(hPLSWnd);
				hPLSWnd = NULL;
			}

			// don't have to unregister windowclass for an
			// application, because it happens automatically.
			return message.wParam;
		}
		// unregistering it manually since the window creation
		// didn't succeed and we never got into the Message loop.
		UnregisterClass(szAppClass,hPLSInstance);
	}

	// We never got going, lets tell the user
	// instead of leaving them hanging in the dark.
	MessageBox(NULL, pszAppPath, _T("Error Loading:"), MB_OK|MB_ICONERROR|MB_DEFBUTTON1|MB_SYSTEMMODAL);

	// free our memory.
	free(pszAppPath);
	pszAppPath = NULL;
	return 0;
}


static LRESULT CALLBACK WndProcPLS ( HWND hwnd, _TINT uMsg, WPARAM wParam, LPARAM lParam ){
	switch (uMsg){
		case WM_SYSCOMMAND:
		{
			switch (wParam){
				case SC_CLOSE:
				{
					PostMessage(hPLSWnd,LM_QUIT,QC_SHUTDOWN,0);
					return 0;
				}
			}
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case LM_OLDREGISTERMESSAGE:
		case LM_REGISTERMESSAGE:
		{
			AddWndMessages((HWND)wParam,(_TINT*)lParam);
			return 1;  // going to add a real return message to indicate success or not
		}
		case LM_OLDUNREGISTERMESSAGE:
		case LM_UNREGISTERMESSAGE:
		{
			RemoveWndMessages((HWND)wParam,(_TINT*)lParam);
			return 1;  // going to add a real return message to indicate success or not
		}
		case LM_QUIT: // handles events that will cause pLS to quit.
		{
			if (wParam == QC_LOGOFF){ // log out of current user session
				if (ExitWindowsEx(EWX_LOGOFF,0))
					bEndSession = TRUE;
			}
			else if (wParam == QC_REBOOT) // reboot windows... ummm this must not be complete!?
				bEndSession = TRUE;
			else if (wParam == QC_SHUTDOWN){ // shutdown windows/machine
				FARPROC (__stdcall *MSWinShutdown)(HWND) = NULL;
				MSWinShutdown = (FARPROC (__stdcall *)(HWND))GetProcAddress(GetModuleHandle("SHELL32.DLL"), (PTSTR)((long)0x3C));
				MSWinShutdown(hPLSWnd);
			}else{ // quit PureLS
				bEndSession = TRUE;
				bSkipEvents = TRUE;
			}
			break;
		}
		case LM_RECYCLE: // handles shell recycle; now passes on a Recycle message to any module that wants to know.
		{
			HRESULT i=0;
			if (HandlerExists(uMsg))
				i = PassOnMessage(uMsg,wParam,lParam,TRUE);
			bRecycle = TRUE;
			return i;
		}
		case LM_RESTART:
		{
			return 0; // Need to implement!
		}
		case LM_OLDUNLOADMODULE:
		case LM_STOPMODULE:
		{
			return wParam && StopModule((LPCTSTR)wParam) ? 1:0;
		}
		case LM_OLDRELOADMODULE:
		case LM_STARTMODULE:
		{
			if (wParam){
				if (StartModule((LPCTSTR)wParam)){
					DoEvents(NULL, 5);
					return 1;
				}
			}
			return 0;
		}
		case LM_STOPSTARTMODULE:
		{
			return wParam ? StopStartModule((LPCTSTR)wParam):0;
		}
		case LM_ISMODULE:
		{
			return wParam && IsModule((LPCTSTR)wParam) ? 1:0;
		}
		case LM_OLDRECYCLE:
		{
			if (wParam == 0)
				bRecycle = TRUE;
			else if (wParam == 1){
				if (ExitWindowsEx(EWX_LOGOFF, 0))
					bEndSession = TRUE;
			}
			else if (wParam == 2){
				bEndSession = TRUE;
				bSkipEvents = TRUE;
			}else
				PostMessage(hPLSWnd,LM_QUIT,QC_SHUTDOWN,0);
			break;
		}
		case LM_OLDGETREVID:
		{
			_TINT i=0;
			_TINT j=0;
			PTSTR buf = (PTSTR)lParam;
			INT bufLen = wParam >> 4;
			INT RetType = wParam & 0x0000f;

			if (RetType == 0){
				_tcscpy(buf, _T("purels.exe: "));
				_tcscat(buf, &rcsRevision[11]);
				buf[_tcslen(buf)-1] = 0;
			}
			else if (RetType == 1){
				_tcscpy(buf, &rcsId[5]);
				buf[_tcslen(buf)-1] = 0;
			}else
				_tcscpy(buf, "");

			if ((i=HandlerExists(uMsg)) > 0){
				_TCHAR buffer[1024];
				INT p=0;
				i -= 1;
				*buffer = 0;
				for (j=0;j<MSGCallbacks[i].nWndCount;j++){
					p = SendMessage(MSGCallbacks[i].hwnd[j], uMsg, (WPARAM)(wParam & 0x0F), (LPARAM)buffer);
					if (p && *buffer){
						_tcscat(buf, "\n");
						_tcsncat(buf, buffer, bufLen - _tcslen(buf));
					}
					buf[bufLen-1] = 0;
				}
			}
			return (HRESULT)_tcslen(buf);
		}
		default:
		{
			if (HandlerExists(uMsg))
				return PassOnMessage(uMsg,wParam,lParam,TRUE);
			else
				return DefWindowProc(hwnd,uMsg,wParam,lParam);
		}
	}
	return 0;
}


static _TINT HandlerExists ( _TINT Msg )
{
	_TINT i=0;
	while (i<nMessageCount){
		if (MSGCallbacks[i].Message && MSGCallbacks[i].Message == Msg)
			return i+1; // if you want to go into message struct array with this return value, subtract 1.
		++i;
	}
	return 0; // zero indicates no message was found.
}

static HRESULT PassOnMessage ( _TINT Msg, WPARAM wParam, LPARAM lParam, BOOL wait )
{
	_TINT i=0,j;
	BOOL found = FALSE;
	HRESULT retVal = 0;
	if (nMessageCount){
		while (!found && i<nMessageCount && MSGCallbacks[i].Message){
			if (MSGCallbacks[i].Message == Msg)
				found = TRUE;
			else
				++i;
		}
		if (found){
			for (j=0;j<MSGCallbacks[i].nWndCount;j++){
				if (MSGCallbacks[i].hwnd[j] != NULL){
					if (wait)
						retVal |= SendMessage(MSGCallbacks[i].hwnd[j],Msg,wParam,lParam);
					else
						PostMessage(MSGCallbacks[i].hwnd[j], Msg, wParam, lParam);
				}
			}
		}
	}
	return retVal;
}

static void AddMessage ( HWND hwnd, _TINT Msg )
{ // should probably make a return value to indicate success/failure
	_TINT i=0;
	_TINT j=0;
	while (i<nMessageCount && MSGCallbacks[i].Message != Msg)
		i++;
	if (i == nMessageCount){
		if (nMessageCount == 0 && MSGCallbacks != NULL){
			// should take the sizeof(MSGCallbacks) / MSGCallbacks[0]
			// to figure out how many are allocated, and then free
			// the sub allocations as well. Really should just exit the
			// function as well, 'cause any time memory allocation
			// is screwed, not much chance of recovering.
			free(MSGCallbacks);
			MSGCallbacks = NULL;
		}
		if (MSGCallbacks == NULL){
			nMessageCount = 0;
			MSGCallbacks = (MessageData *)malloc(sizeof(MessageData));
		}else{
			MessageData* tmpMsgData = MSGCallbacks;
			MSGCallbacks = (MessageData *)realloc(MSGCallbacks, (nMessageCount+1)*sizeof(MessageData));
			if (MSGCallbacks == NULL){
				MSGCallbacks = tmpMsgData;
				return;
			}
		}
		if (MSGCallbacks == NULL){
			nMessageCount = 0;
			return;
		}
		MSGCallbacks[i].hwnd = NULL;
		MSGCallbacks[i].Message = Msg;
		MSGCallbacks[i].nWndCount = 0;
		++nMessageCount;
	}

	while (j<MSGCallbacks[i].nWndCount && MSGCallbacks[i].hwnd[j] != hwnd)
		j++;
	if (j == MSGCallbacks[i].nWndCount){
		if (MSGCallbacks[i].nWndCount == 0 && MSGCallbacks[i].hwnd != NULL){
			free(MSGCallbacks[i].hwnd);
			MSGCallbacks[i].hwnd = NULL;
		}
		if (!MSGCallbacks[i].hwnd){
			MSGCallbacks[i].nWndCount = 0;
			MSGCallbacks[i].hwnd = (HWND *)malloc(sizeof(HWND));
		}else{
			HWND* tmpHwnd = MSGCallbacks[i].hwnd;
			MSGCallbacks[i].hwnd = (HWND *)realloc(MSGCallbacks[i].hwnd, ((MSGCallbacks[i].nWndCount+1))*sizeof(HWND));
			if (MSGCallbacks[i].hwnd == NULL){
				MSGCallbacks[i].hwnd = tmpHwnd;
				return;
			}
		}
		if (MSGCallbacks[i].hwnd == NULL){
			MSGCallbacks[i].nWndCount = 0;
			return;
		}
		MSGCallbacks[i].hwnd[j] = hwnd;
		++(MSGCallbacks[i].nWndCount);
	}
	return;
}

static void AddWndMessages ( HWND hwnd, _TINT *Msg )
{
	_TINT i=0;
	while (Msg[i])
		AddMessage(hwnd, Msg[i++]);
	return;
}

static void RemoveMessage ( HWND hwnd, _TINT Msg )
{// will want to clean this up some, putting in checks for allocated space when no messages/handles exist
	_TINT i=0;
	if (nMessageCount == 0 && MSGCallbacks != NULL) // just a little fail safe trap, doubt it could ever happen tho.
		PurgeMessages();
	while (i<nMessageCount && MSGCallbacks[i].Message != Msg)
		i++;

	if (i<nMessageCount){
		_TINT j=0;
		while (j<MSGCallbacks[i].nWndCount && MSGCallbacks[i].hwnd[j] != hwnd)
			j++;

		if (j<MSGCallbacks[i].nWndCount){
			MSGCallbacks[i].hwnd[j] = NULL;
			--(MSGCallbacks[i].nWndCount);

			if (MSGCallbacks[i].nWndCount>0){
				if (j<MSGCallbacks[i].nWndCount)
					memmove(&MSGCallbacks[i].hwnd[j], &MSGCallbacks[i].hwnd[j+1], (MSGCallbacks[i].nWndCount-j)*sizeof(HWND));
				MSGCallbacks[i].hwnd = (HWND *)realloc(MSGCallbacks[i].hwnd, (MSGCallbacks[i].nWndCount)*sizeof(HWND));
			}else{ // could put a check to see if MSGCallbacks[i].hwnd != NULL, but if it is, we are already screwed.
				free(MSGCallbacks[i].hwnd);
				MSGCallbacks[i].hwnd = NULL;
				MSGCallbacks[i].Message = 0;
				--nMessageCount;
				if (nMessageCount>0){
					if (i<nMessageCount)
						memmove(&MSGCallbacks[i], &MSGCallbacks[i+1], (nMessageCount-i)*sizeof(MessageData));
					MSGCallbacks = (MessageData *)realloc(MSGCallbacks, (nMessageCount)*sizeof(MessageData));
				}else{ // could put a check to see if MSGCallbacks != NULL, but if it is, we are already screwed.
					free(MSGCallbacks);
					MSGCallbacks = NULL;
				}
			}
		}
	}
	return;
}

static void RemoveWndMessages ( HWND hwnd, _TINT *Msg )
{ // requires a terminating "0" zero value message
	_TINT i=0;
	while (Msg[i])
		RemoveMessage(hwnd, Msg[i++]);
	return;
}

static void PurgeMessages ( void )
{
	_TINT i,j;

	for (i=0;i<nMessageCount;i++){
		MSGCallbacks[i].Message = 0;
		for (j=0;j<MSGCallbacks[i].nWndCount;j++)
			MSGCallbacks[i].hwnd[j] = NULL;
		MSGCallbacks[i].nWndCount = 0;
		if (MSGCallbacks[i].hwnd != NULL){
			free(MSGCallbacks[i].hwnd);
			MSGCallbacks[i].hwnd = NULL;
		}
	}
	nMessageCount=0;
	if (MSGCallbacks != NULL){
		free(MSGCallbacks);
		MSGCallbacks = NULL;
	}
	return;
}

static _TINT IsModule ( LPCTSTR pszModulePath )
{
	if (pszModulePath != NULL && *pszModulePath != 0)
		return GetModuleHandle(pszModulePath) != NULL ? 1:0;
	return 0;
}

// pretty darn huge function... gotta look into slimming it down
// but can't really think of anything at this point.
static BOOL StartModule ( LPCTSTR pszModulePath )
{
	ModuleData md;
	_TCHAR tmpModPath[PLS_MAX_FILE];
	BOOL bIsLS = FALSE; // Compatibility for LS style LoadModules
	_TINT modRetVal = 0;

	if (pszModulePath == NULL || *pszModulePath == 0)
		return FALSE;

	{
		HINSTANCE hTmpModInst = NULL;
		if ((hTmpModInst = GetModuleHandle(pszModulePath)) != NULL){
			// Give them the path and name of the module we think is already loaded.
			// It may be different then from what they think.
			DWORD len = GetModuleFileName(hTmpModInst, tmpModPath, PLS_MAX_FILE);
			if (!len)
				_tcscpy(tmpModPath, pszModulePath);
			MessageBox(hPLSWnd, tmpModPath, _T("Module is already loaded."), MB_OK|MB_ICONWARNING|MB_DEFBUTTON1|MB_SYSTEMMODAL);
			hTmpModInst = NULL;
			return FALSE;
		}
	}

#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Starting Module"), pszModulePath);
#endif

	memset(&md, 0, sizeof(ModuleData));

	md.hInstance = LoadLibrary(pszModulePath);
	if (md.hInstance != NULL){
		md.pStart = (ModuleStartFunc)GetProcAddress(md.hInstance,_T("StartModule"));
		if (md.pStart == NULL){ // Compatibility for LS style LoadModules
			md.pInit = (ModuleOldStartFunc)GetProcAddress(md.hInstance, _T("initModuleEx"));
			bIsLS = TRUE;
		}
		if (md.pStart != NULL || (bIsLS && md.pInit != NULL)){
			__try
			{
				if (bIsLS) // Compatibility for LS style LoadModules
					modRetVal = md.pInit(hPLSWnd,md.hInstance,pszAppPath);
				else
					modRetVal = md.pStart(hPLSWnd,md.hInstance);
			}
			__except(1)
			{
				FreeLibrary(md.hInstance);
				md.hInstance = NULL;
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Error Starting Module"), pszModulePath);
#endif
			}
		}

		// no reason to do this extra work if the
		// module is not going to be loaded.
		if (md.hInstance != NULL){
			// I have no clue why the original LiteStep has
			// modules returning '1' to indicate failure?!
			// And besides, the return value was never looked at!!
			if ((modRetVal==1 && bIsLS) || (modRetVal==0 && !bIsLS)){
				FreeLibrary(md.hInstance);
				md.hInstance = NULL;
			}else{
				if (bIsLS) // Compatibility for LS style LoadModules
					md.pStop = (ModuleStopFunc)GetProcAddress(md.hInstance, _T("quitModule"));
				else
					md.pStop = (ModuleStopFunc)GetProcAddress(md.hInstance, _T("StopModule"));
				// alowing either Module type to have this function call,
				// even though most LiteStep style modules will not.
				md.pReset = (ModuleResetFunc)GetProcAddress(md.hInstance, _T("ResetModule"));
			}
		}
	}

	if (md.hInstance == NULL){
		// probably should move the messagebox outside of this function...
		MessageBox(hPLSWnd, pszModulePath, _T("Error Loading:"), MB_OK|MB_ICONERROR|MB_DEFBUTTON1|MB_SYSTEMMODAL);
		//free (md.szFileName);
		//md.szFileName = NULL;
		return FALSE;
	}else{
		DWORD len = GetModuleFileName(md.hInstance, tmpModPath, PLS_MAX_FILE);
		if (!len)
			_tcscpy(tmpModPath, pszModulePath);
		md.szFileName = NULL;
		md.szFileName = (PTSTR)malloc((_tcslen(tmpModPath)+1)*sizeof(_TCHAR));
		if (md.szFileName != NULL)
			_tcscpy(md.szFileName, tmpModPath);
		// not to sure what to do is the malloc failed.
		// Guess there will just have to be no record
		// kept of the module's name.
	}

	if(Modules == NULL){
		nModuleCount = 0;
		Modules = (ModuleData *)malloc(sizeof(ModuleData));
	}else{
		ModuleData* tmpModData = Modules;
		Modules = (ModuleData *)realloc(Modules,(nModuleCount+1)*sizeof(ModuleData));
		if (Modules == NULL){
			Modules = tmpModData;
			// should unload the module or something here
			// and free the module filename allocation.
			return FALSE;
		}
	}
	memset(Modules+(nModuleCount), 0, sizeof(ModuleData));
	// should check the return value of this memcpy routine to make sure it was successful. Or something...
	memcpy(Modules+(nModuleCount++), &md, sizeof(ModuleData));
	return TRUE;
}

static BOOL StopModule ( LPCTSTR pszModulePath )
{
	if (pszModulePath != NULL && *pszModulePath != 0){
		_TINT i;
		HINSTANCE hTmpModInst = NULL;

		if ((hTmpModInst = GetModuleHandle(pszModulePath)) == NULL){
			MessageBox(hPLSWnd, pszModulePath, _T("Module is not loaded."), MB_OK|MB_ICONWARNING|MB_DEFBUTTON1|MB_SYSTEMMODAL);
			hTmpModInst = NULL;
			return FALSE;
		}

		for (i=0;i<nModuleCount && Modules[i].hInstance != hTmpModInst;i++);
		if (i != nModuleCount){
			if (Modules[i].pStop != NULL){
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Stopping Module"), pszModulePath);
#endif
				__try
				{
					Modules[i].pStop(Modules[i].hInstance);
				}
				__except(1) {
					// perhaps make this function an _TINT, and set a return fail level here.
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Error Stopping Module"), pszModulePath);
#endif
				}
			}
			if (Modules[i].szFileName != NULL){
				free(Modules[i].szFileName);
				Modules[i].szFileName = NULL;
			}
			FreeLibrary(Modules[i].hInstance);
			Modules[i].hInstance = NULL;

			if (i < --nModuleCount)
				memmove(&Modules[i], &Modules[i+1], sizeof(ModuleData)*(nModuleCount-i));
			if (nModuleCount > 0) // no reason to do it if the module count is now none.
				Modules = (ModuleData*)realloc(Modules,nModuleCount*sizeof(ModuleData));
			// not sure what to do about realloc failing here. So, going to ignore for now.
			if (!nModuleCount && Modules != NULL)
				free(Modules); // unloaded the last module
			return TRUE;
		}
	}
	return FALSE;
}


/*static BOOL ResetModule ( LPCTSTR ) // Need to finish.. not very hard.
{
	return FALSE;
}*/

static _TINT StopStartModule ( LPCTSTR pszModulePath )
{
	if (StopModule(pszModulePath)){
		if (StartModule(pszModulePath)){
			DoEvents(NULL, 5); // for some reason, this doesn't seem to be working
			return 2;
		}
		return 1;
	}
	return 0;
}

static void AddModules ( void )
{
	// always have to do a 'DoEvents(NULL, 5);'
	// after loading a module, however we do not
	// want to do it inside of this function.
	_TCHAR tmpBuf[PLS_MAX_FILE];
	FILE* tmp = NULL;

	if ((tmp = scfOpen (pls.rcFile)) != NULL){
		PTSTR ptrTmp = NULL;
		while (scfGetNextLine(tmp, tmpBuf, PLS_MAX_FILE*sizeof(_TCHAR)) == TRUE){
			ptrTmp = NULL;
			if ((ptrTmp = _tcsstr(tmpBuf, _T("LoadModule"))) == tmpBuf){
				ptrTmp += 10; // move pointer to first position past "LoadModule"
				if (scStripWS(ptrTmp)){
					StartModule(ptrTmp);
					DoEvents(NULL, 5);
				}
			}else
				continue;
		}
		scfClose(tmp);
	}
	return;
}

static void RemoveModules ( void )
{
	while (nModuleCount>0){
		if (Modules[--nModuleCount].pStop){
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Stopping Module"), Modules[nModuleCount].szFileName);
#endif
			__try
			{
				Modules[nModuleCount].pStop(Modules[nModuleCount].hInstance);
			}
			__except(1) {
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Error Stopping Module"), Modules[nModuleCount].szFileName);
#endif
			}
		}
		if (Modules[nModuleCount].szFileName != NULL){
			free(Modules[nModuleCount].szFileName);
			Modules[nModuleCount].szFileName = NULL;
		}
		FreeLibrary(Modules[nModuleCount].hInstance);
		Modules[nModuleCount].hInstance = NULL;
	}
	if (Modules != NULL){
		free(Modules);
		Modules = NULL;
	}
	return;
}


static void DoEvents ( HWND hwnd, _TINT n )
{
	MSG msg;
	_TINT i;
	INT b;

	b = PeekMessage(&msg,hwnd,0,0,PM_REMOVE);
	for (i=0;i<n&&b&&b!=-1;i++){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		b = PeekMessage(&msg,hwnd,0,0,PM_REMOVE);
	}
	if (msg.message == WM_QUIT)
		PostQuitMessage((int)msg.lParam);
	return;
}


static void RunStartupStuff ( void* dummy )
{
	#define RunPath _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
	#define RunOncePath _T("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce")
	BOOL bWasLoaded = TRUE;
	HINSTANCE hStartup = NULL;
	_TCHAR tmpPath[PLS_MAX_FILE];

	typedef void (*StartupRunEntriesIn) ( HKEY, LPCTSTR );
	typedef void (*StartupDeleteEntriesIn) ( HKEY, LPCTSTR );
	typedef void (*StartupRunStartupMenu) ( void );

	dummy=dummy;

	// lets specifically load the startup.dll in the PureLS directory
	// as there may be another startup.dll somewhere in the system.
	// Because if the user has removed our startup.dll, then this may
	// go and find an invalid startup.dll.  Doing this fixes that.
	_tcscpy(tmpPath, pszAppPath);
	_tcscat(tmpPath, _T("startup.dll"));

	// who knows, maybe for some odd reason its already loaded by something else.
	hStartup = GetModuleHandle(tmpPath);
	// but if not, lets load it ourselves.
	if (hStartup == NULL){
		hStartup = LoadLibrary(tmpPath);
		bWasLoaded = FALSE; // we loaded it.
	}

	if (hStartup != NULL){
		StartupRunEntriesIn pRunEntriesIn;
		StartupDeleteEntriesIn pDeleteEntriesIn;
		StartupRunStartupMenu pRunStartupMenu;

		pRunEntriesIn = (StartupRunEntriesIn)GetProcAddress(hStartup, _T("RunEntriesIn"));
		pDeleteEntriesIn = (StartupDeleteEntriesIn)GetProcAddress(hStartup, _T("DeleteEntriesIn"));
		pRunStartupMenu = (StartupRunStartupMenu)GetProcAddress(hStartup, _T("RunStartupMenu"));

		if (pRunEntriesIn != NULL && pDeleteEntriesIn != NULL){
			__try {
				// Run 'RunOnce' applications, and delete from registry [eg, setup stuff]
				pRunEntriesIn (HKEY_LOCAL_MACHINE, RunOncePath);
				pDeleteEntriesIn (HKEY_LOCAL_MACHINE, RunOncePath);
				pRunEntriesIn (HKEY_CURRENT_USER, RunOncePath);
				pDeleteEntriesIn (HKEY_CURRENT_USER, RunOncePath);
			}
			__except(1){
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Startup.dll pRun-pDel Entries"), _T("We got an exception during the RunOnce routines!"));
#endif
			}

			/* Just seperating them out so we can get more detailed user Debug messages. */

			__try {
				// Run regular permanent applications
				pRunEntriesIn (HKEY_LOCAL_MACHINE, RunPath);
				pRunEntriesIn (HKEY_CURRENT_USER, RunPath);
			}
			__except(1){
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Startup.dll pRun Entries"), _T("We got an exception during the Run regular routines!"));
#endif
			}
		}
#ifdef _DEBUG
		else
			DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Startup.dll"), _T("We didn't run registry entries, Startup.dll entry points not found!"));
#endif

		if (pRunStartupMenu != NULL){
			__try {
				// Lastly run items in the Startup folder of the start menu
				pRunStartupMenu();
			}
			__except(1){
#ifdef _DEBUG
	DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Startup.dll pRun Menu"), _T("We got an exception while running the Startup Menu!"));
#endif
			}
		}
#ifdef _DEBUG
		else
			DebugPrintf(pls.cDebugMode ? TRUE:FALSE, _T("Startup.dll"), _T("We didn't run start menu entries, Startup.dll entry points not found!"));
#endif

		// I guess if it was already loaded, then something else is using it
		// and since we didn't load it explicitly ourselves, lets not try to
		// unload it.
		if (bWasLoaded == FALSE)
			FreeLibrary(hStartup);
		hStartup = NULL;
	}
	return;
}
