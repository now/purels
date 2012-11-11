#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include <windows.h>
#include <mmsystem.h>
#include <malloc.h>

/****************************************************************
 * Type Definitions
 ****************************************************************/

typedef struct MixerArray
{
    int		    count;
    LPMIXERCONTROL  mixers;
} MixerArray;

#define CHECK_MMRETURN(code) \
if ((code) != MMSYSERR_NOERROR) \
{ \
    return (FALSE); \
}

 /****************************************************************
 * Function Definitions
 ****************************************************************/

BOOL my_realloc(void **p, size_t size);
BOOL mixer_getlinecontrols(const LPMIXERLINE line);
BOOL mixer_manipulate_volume(int channel, BOOL get, LPDWORD left, LPDWORD right);
BOOL mixer_manipulate_mute(int channel, BOOL get, LPBOOL mute);

/****************************************************************
 * Global Variables
 ****************************************************************/

static HMIXER s_hMixer;
static MixerArray s_lines;
static MixerArray s_mutes;

/****************************************************************
 * Function Implementations
 ****************************************************************/

BOOL my_realloc(void **p, size_t size)
{
    void *np;

    np = realloc(*p, size);

    if (np == NULL)
    {
	MessageBox(NULL, _T("Couldn't realloc()"), NULL, MB_SETFOREGROUND);

	return (FALSE);
    }

    *p = np;

    return (TRUE);
}

BOOL mixer_init(DWORD dwCallback)
{
    MIXERLINE line;
    int i;
    int j;

    CHECK_MMRETURN(mixerOpen(&s_hMixer, 0, dwCallback, 0,
			     CALLBACK_WINDOW | MIXER_OBJECTF_MIXER));

    line.cbStruct	= sizeof(MIXERLINE);
    line.dwDestination	= 0;
    line.dwSource	= 0;

    CHECK_MMRETURN(mixerGetLineInfo((HMIXEROBJ)s_hMixer, &line,
				    MIXER_GETLINEINFOF_DESTINATION));

    if (!mixer_getlinecontrols(&line))
	return (FALSE);

    for (i = 0, j = line.cConnections; i < j; i++)
    {
	line.dwSource	= i;

	CHECK_MMRETURN(mixerGetLineInfo((HMIXEROBJ)s_hMixer, &line,
					MIXER_GETLINEINFOF_SOURCE));

	if (!mixer_getlinecontrols(&line))
	    return (FALSE);
    }

    return (TRUE);
}

BOOL mixer_deinit(void)
{
    if (s_hMixer != NULL)
    	mixerClose(s_hMixer);

    if (s_lines.mixers != NULL)
	free(s_lines.mixers);

    if (s_mutes.mixers != NULL)
	free(s_lines.mixers);

    return (TRUE);
}

BOOL mixer_getlinecontrols(const LPMIXERLINE line)
{
    int i;
    int lines;
    int mutes;
    MIXERLINECONTROLS controls;

    controls.cbStruct	= sizeof(MIXERLINECONTROLS);
    controls.dwLineID	= line->dwLineID;
    controls.cControls	= line->cControls;
    controls.cbmxctrl	= sizeof(MIXERCONTROL);
    controls.pamxctrl	= NULL;

    if (!my_realloc(&controls.pamxctrl, sizeof(MIXERCONTROL) * controls.cControls))
	return (FALSE);

    CHECK_MMRETURN(mixerGetLineControls((HMIXEROBJ)s_hMixer, &controls,
					MIXER_GETLINECONTROLSF_ALL));

    for (i = 0, lines = s_lines.count, mutes = s_mutes.count; i < (int)controls.cControls; i++)
    {
	switch (controls.pamxctrl[i].dwControlType)
	{
	case MIXERCONTROL_CONTROLTYPE_VOLUME:	lines++; break;
	case MIXERCONTROL_CONTROLTYPE_MUTE:	mutes++; break;
	}
    }

    if (!my_realloc((void **)&s_lines.mixers, sizeof(MIXERCONTROL) * lines))
	return (FALSE);

    if (!my_realloc((void **)&s_mutes.mixers, sizeof(MIXERCONTROL) * mutes))
	return (FALSE);

    for (i = 0; i < (int)controls.cControls; i++)
    {
	switch (controls.pamxctrl[i].dwControlType)
	{
	case MIXERCONTROL_CONTROLTYPE_VOLUME:
		s_lines.mixers[s_lines.count++] = controls.pamxctrl[i];
	break;
	case MIXERCONTROL_CONTROLTYPE_MUTE:
		s_mutes.mixers[s_mutes.count++] = controls.pamxctrl[i];
	break;
	}
    }

    free(controls.pamxctrl);

    return (TRUE);
}


BOOL mixer_setvolume(int channel, DWORD left, DWORD right)
{
    return (mixer_manipulate_volume(channel, FALSE, &left, &right));
}

BOOL mixer_getvolume(int channel, LPDWORD left, LPDWORD right)
{
    return (mixer_manipulate_volume(channel, TRUE, left, right));
}

BOOL mixer_setmute(int channel, BOOL mute)
{
    return (mixer_manipulate_mute(channel, FALSE, &mute));
}

BOOL mixer_getmute(int channel, LPBOOL mute)
{
    return (mixer_manipulate_mute(channel, TRUE, mute));
}

BOOL mixer_manipulate_volume(int channel, BOOL get, LPDWORD left, LPDWORD right)
{
    MIXERCONTROLDETAILS_UNSIGNED details[2];
    MIXERCONTROLDETAILS mcd;

    details[0].dwValue	= *left;
    details[1].dwValue	= *right;

    if (s_lines.count == 0)
	return (FALSE);

    if (channel < 0 || channel >= s_lines.count)
	return (FALSE);

    mcd.cbStruct	= sizeof(MIXERCONTROLDETAILS);
    mcd.dwControlID 	= s_lines.mixers[channel].dwControlID;
    mcd.cChannels	= 2;
    mcd.cMultipleItems	= 0;
    mcd.cbDetails	= sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    mcd.paDetails	= &details;

    if (get)
    {
	CHECK_MMRETURN(mixerGetControlDetails((HMIXEROBJ)s_hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE));
    }
    else
    {
	CHECK_MMRETURN(mixerSetControlDetails((HMIXEROBJ)s_hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE));
    }

    *left	= details[0].dwValue;
    *right	= details[1].dwValue;
	
    return (TRUE);
}

BOOL mixer_manipulate_mute(int channel, BOOL get, LPBOOL mute)
{
    MIXERCONTROLDETAILS_BOOLEAN details;
    MIXERCONTROLDETAILS mcd;

    details.fValue	= *mute;

    if (s_mutes.count == 0)
	return (FALSE);

    if (channel < 0 || channel >= s_mutes.count)
	return (FALSE);

    mcd.cbStruct	= sizeof(MIXERCONTROLDETAILS);
    mcd.dwControlID 	= s_mutes.mixers[channel].dwControlID;
    mcd.cChannels	= 1;
    mcd.cMultipleItems	= 0;
    mcd.cbDetails	= sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mcd.paDetails	= &details;

    if (get)
    {
	CHECK_MMRETURN(mixerGetControlDetails((HMIXEROBJ)s_hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE));
    }
    else
    {
	CHECK_MMRETURN(mixerSetControlDetails((HMIXEROBJ)s_hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE));
    }

    *mute = details.fValue;

    return (TRUE);
}