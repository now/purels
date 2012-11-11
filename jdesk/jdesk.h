#ifndef __JDESK_H
#define __JDESK_H

struct jDeskSettings
{
	BOOL WorkArea;
};

__declspec( dllexport ) int initModuleEx(HWND, HINSTANCE, LPCSTR);
__declspec( dllexport ) void quitModule(HINSTANCE);

#endif
