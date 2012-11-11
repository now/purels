#ifndef __VOLUME_H
#define __VOLUME_H

BOOL mixer_init(DWORD dwCallback);
BOOL mixer_deinit(void);
BOOL mixer_setvolume(int channel, DWORD left, DWORD right);
BOOL mixer_getvolume(int channel, LPDWORD left, LPDWORD right);
BOOL mixer_setmute(int channel, BOOL mute);
BOOL mixer_getmute(int channel, LPBOOL mute);

#endif /* __VOLUME_H */
