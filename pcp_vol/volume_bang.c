#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lsapi/lsapi.h"

#include "volume.h"
#include "volume_bang.h"

#pragma comment(linker,"/merge:.text=.data")
#pragma comment(linker,"/merge:.reloc=.data")

/****************************************************************
 * Type Definitions
 ****************************************************************/

#define CHECK_BOUNDS(x)		if (x > 65535) { x = 65535; } else if (x < 0) { x = 0; }

/****************************************************************
 * Function Definitions
 ****************************************************************/

void Volume_BangUp(HWND hwndCaller, char *szArgs);
void Volume_BangDown(HWND hwndCaller, char *szArgs);
void Volume_BangMute(HWND hwndCaller, char *szArgs);
void Volume_BangSoundOn(HWND hwndCaller, char *szArgs);
void Volume_BangSoundOff(HWND hwndCaller, char *szArgs);

void volume_adjust_volume(char *args, BOOL bIncrease);
int volume_getchannel(char *args);

/****************************************************************
 * Global Variables
 ****************************************************************/

int s_nVolumeStepWidth	= 5;
BOOL s_bIgnoreBalance	= FALSE;

/****************************************************************
 * Function Implementations
 ****************************************************************/

int initModuleEx(HWND parent, HINSTANCE dll, LPCTSTR path)
{
    if (!mixer_init(0))
	return (1); // return error

    s_nVolumeStepWidth	= GetRCInt(_T("VolumeStepWidth"), 6);
    s_bIgnoreBalance	= GetRCBool(_T("VolumeIgnoreBalance"), TRUE);

    if (s_nVolumeStepWidth < 1)
	s_nVolumeStepWidth = 1;

    if (s_nVolumeStepWidth > 16)
	s_nVolumeStepWidth = 16;

    AddBangCommand(_T("!Volume_Up"), Volume_BangUp);
    AddBangCommand(_T("!Volume_Down"), Volume_BangDown);
    AddBangCommand(_T("!Volume_Mute"), Volume_BangMute);
    AddBangCommand(_T("!Volume_Sound_On"), Volume_BangSoundOn);
    AddBangCommand(_T("!Volume_Sound_Off"), Volume_BangSoundOff);

    return (0);
}

void quitModule(HINSTANCE dllInst)
{
    RemoveBangCommand(_T("!Volume_Up"));
    RemoveBangCommand(_T("!Volume_Down"));
    RemoveBangCommand(_T("!Volume_Mute"));
    RemoveBangCommand(_T("!Volume_Sound_On"));
    RemoveBangCommand(_T("!Volume_Sound_Off"));

    mixer_deinit();
}

void Volume_BangUp(HWND hwndCaller, char *args)
{
    volume_adjust_volume(args, TRUE);
}

void Volume_BangDown(HWND caller, char *args)
{
    volume_adjust_volume(args, FALSE);
}

void Volume_BangMute(HWND caller, char *args)
{
    int channel;
    BOOL muted;

    channel = volume_getchannel(args);

    if (!mixer_getmute(channel, &muted))
	return;

    mixer_setmute(channel, !muted);
}

void Volume_BangSoundOn(HWND caller, char *args)
{
    mixer_setmute(volume_getchannel(args), FALSE);
}

void Volume_BangSoundOff(HWND caller, char *args)
{
    mixer_setmute(volume_getchannel(args), TRUE);
}

void volume_adjust_volume(char *args, BOOL incr)
{
    static double balance = 1;
    static BOOL swapped = FALSE;
    int channel;
    double left;
    double right;
    DWORD tmp_left;
    DWORD tmp_right;

    channel = volume_getchannel(args);

    if (!mixer_getvolume(channel, &tmp_left, &tmp_right))
	return;

    left = (double)tmp_left;
    right = (double)tmp_right;

    if (left > right)
    {
	double tmp = right;

	right = left;
	left = tmp;
	swapped = TRUE;
    }
    else if (left > 0)
    {
	swapped = FALSE;
    }

    if (left > 0 || right > 0)
	balance = (left / max((right / 100), 1));

    right += ((incr) ? 1 : -1) * (655.35 * s_nVolumeStepWidth);

    CHECK_BOUNDS(right);

    if (s_bIgnoreBalance)
	left = right;
    else
	left = (balance * (right / 100));

    CHECK_BOUNDS(left);

    if (swapped)
    {
	double tmp = right;

	right = left;
	left = tmp;
    }

    mixer_setvolume(channel, (DWORD)left, (DWORD)right);
}

int volume_getchannel(char *args)
{
    char *p;

    while (isspace(*args))
	args++;

    if (*args == '\0')
	return (0);

    p = args;

    while (isdigit(*p))
	p++;

    *p = '\0';

    return (atoi(args));
}