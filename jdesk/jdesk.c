#include <windows.h>
#include <stdio.h>
#include "jdesk.h"
#include "../lsapi/lsapi.h"

#pragma comment(linker,"/merge:.text=.data")
#pragma comment(linker,"/merge:.reloc=.data")

const char szAppName[] = TEXT("DesktopBackgroundClass"); // Our window class, etc

HWND hMainWnd; // main window handle
HWND hwndParent; // LS window handle

struct jDeskSettings jds;

BOOL bIsFirst = TRUE;
BOOL bIsRezChange = FALSE;
int screenWidth, screenHeight;
int waLeft=0, waRight=0, waTop=0, waBottom=0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void SetWorkArea(void);
void ResetWorkArea(void);

struct jDeskSettings ReadSettings()
{
	struct jDeskSettings settings;
	char tmp[MAX_PATH];
	char *token;

	settings.WorkArea = GetRCString(TEXT("jDeskWorkArea"), tmp, TEXT("0,0,0,0"), MAX_PATH);
	token = strtok(tmp, TEXT(" ,"));
		if (token != NULL)
			waLeft = atoi(token);
	token = strtok(NULL, TEXT(" ,"));
		if (token != NULL)
			waRight = atoi(token);
	token = strtok(NULL, TEXT(" ,"));
		if (token != NULL)
			waTop = atoi(token);
	token = strtok(NULL, TEXT(" ,"));
		if (token != NULL)
			waBottom = atoi(token);

	return (settings);
}

int initModuleEx(HWND parentWnd, HINSTANCE dllInst, LPCSTR szPath)
{
	WNDCLASS wc;

	hwndParent = parentWnd;

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	jds = ReadSettings();

	// Set the work area for maximized applications
	SetWorkArea();
	bIsFirst = FALSE;

	{// Register the jDesk desktop window class
		memset(&wc,0,sizeof(wc));
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpfnWndProc = WndProc;				// our window procedure
		wc.hInstance = dllInst;					// hInstance of DLL
		wc.lpszClassName = szAppName;		// our window class name
		wc.style = 0;
		if (!RegisterClass(&wc))
		{
			MessageBox(hwndParent,TEXT("Error registering window class"),TEXT("jDesk"),MB_OK);
			return 1;
		}
	}
	{// Create jDesk desktop window
		hMainWnd = CreateWindowEx(
			WS_EX_TOOLWINDOW,				// exstyles 
			szAppName,										// our window class name
			TEXT(""),										// use description for a window title
			WS_POPUP|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
			0, 0,											// position 
			screenWidth,screenHeight,						// width & height of window
			hwndParent,											// parent window 
			NULL,											// no menu
			dllInst,											// hInstance of DLL
			NULL);											// no window creation data
		if (!hMainWnd)
		{
			MessageBox(hwndParent,TEXT("Error creating window"),TEXT("jDesk"),MB_OK);
			return 1;
		}
	}

	//Final desktop window settings and display
	SetWindowLong(hMainWnd, GWL_USERDATA, 0x49474541);
	SetWindowPos(hMainWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	ShowWindow(hMainWnd,SW_SHOWNORMAL);
//	SetActiveWindow(hMainWnd);

	return 0;
}

void quitModule(HINSTANCE dllInst/*, BOOL gRecycle*/)
{ 
	DestroyWindow(hMainWnd); // delete jDesk desktop window
	UnregisterClass(szAppName,dllInst); // unregister jDesk desktop window class

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DISPLAYCHANGE:
		bIsRezChange = TRUE;
		SetWindowPos(hMainWnd, HWND_BOTTOM, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOACTIVATE);
		screenWidth = LOWORD(lParam); 
		screenHeight = HIWORD(lParam);
		SetWorkArea();
	return 0;
	case WM_ENDSESSION:
	case WM_QUERYENDSESSION:
	return SendMessage(hwndParent,message,wParam,lParam);
	case WM_ERASEBKGND:
		PaintDesktop((HDC)wParam);
	return 1;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_CLOSE://For using the standard alt+F4 to shutdown windows
			ParseBangCommand(hMainWnd, TEXT("!ShutDown"), NULL); // could actually just forward on, since LiteStep checks for this message as well, but this is safer.
		return 0;
		}
	break;
	case WM_KEYDOWN:
	case WM_KEYUP: // forward
		PostMessage(hwndParent,message,wParam,lParam);
	return 0;
	case WM_HOTKEY:
		PostMessage(hwndParent,message,wParam,lParam);
	case WM_WINDOWPOSCHANGING:
	{
		if (!bIsRezChange){
			// Trap windowposchanging messages
			WINDOWPOS *c = (WINDOWPOS*)lParam;
			c->hwndInsertAfter = HWND_BOTTOM;
			c->flags |= SWP_NOMOVE|SWP_NOSIZE;
			return 0;
		}else{
			bIsRezChange = FALSE;
		}
		break;
	}
	}

	return DefWindowProc(hwnd,message,wParam,lParam);
}

void SetWorkArea(void)
{
	RECT r;
	if (!(waLeft || (waRight && waRight != screenWidth) || waTop || (waBottom && waBottom != screenHeight)))
		jds.WorkArea = FALSE;  // don't need this, but just cuts back on unnecessary code execution. (or adds extra, depends on how you look at it :)
	if (jds.WorkArea)
	{
		int minSize = 200;//Allowing a minimum of 200x200 work area. Anything less is even more rediculous then that, plus impossible to access a max'd window.

		r.top = 0;
		r.left = 0;
		r.bottom = screenHeight;
		r.right = screenWidth;
		r.left = waLeft<0 ? screenWidth+waLeft:waLeft;
			if (r.left >= screenWidth-minSize) r.left = 0;

		r.right = waRight<0 ? screenWidth+waRight:waRight;
			if (r.right <= r.left+minSize) r.right = screenWidth;

		r.top = waTop<0 ? screenHeight+waTop:waTop;
			if (r.top >= screenHeight-minSize) r.top = 0;

		r.bottom = waBottom<0 ? screenHeight+waBottom:waBottom;
			if (r.bottom <= r.top+minSize) r.bottom = screenHeight;
	}
	SystemParametersInfo(SPI_SETWORKAREA,(bIsFirst ? 0:1),(jds.WorkArea ? &r:NULL),SPIF_SENDCHANGE);
	return;
}

void ResetWorkArea(void)
{
	SystemParametersInfo(SPI_SETWORKAREA,0,NULL,SPIF_SENDCHANGE);
	return;
}
