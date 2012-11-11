###
## systray
#

# this is actually defined so systray could be compiled
# for a variety of shells just by changing the SHELL=...
# parameter to nmake. currently it only builds for LS or
# Outsider.

# to build systray.dll for Litestep:
# nmake SHELL=LITESTEP

# to build systray.exe for Outsider99:
# nmake SHELL=OUTSIDER99

# NOTE: you must rebuild all (/A) if you change want to
# change the SHELL. I really should add a "clean" target.

!IFNDEF SHELL
SHELL    = LITESTEP
!ENDIF

BASENAME = systray

!IF "$(SHELL)" == "LITESTEP"
BINTYPE  = DLL
SHLOBJS  = bang.obj
SHLLIBS  = lsapi.lib
!ELSEIF "$(SHELL)" == "OUTSIDER99"
BINTYPE  = EXE
!ENDIF

!IF "$(BINTYPE)" == "DLL"
FINAL    = $(BASENAME).dll
LFLAGS   = /dll /entry:DllMain /out:$(FINAL) /nologo /subsystem:windows
!ELSE
FINAL    = $(BASENAME).exe
LFLAGS   = /entry:main /out:$(FINAL) /nologo /subsystem:windows
!ENDIF

CFLAGS   = /Ox /nologo /D$(SHELL)
OBJS     = $(SHLOBJS) config.obj image.obj main.obj registry.obj shell.obj systray.obj
LIBS     = $(SHLLIBS) advapi32.lib comctl32.lib gdi32.lib kernel32.lib shell32.lib user32.lib

$(FINAL): $(OBJS)
 link $(LFLAGS) $(OBJS) $(LIBS)

bang.obj: bang.c systray.h
config.obj: config.c systray.h
image.obj: image.c systray.h
main.obj: main.c systray.h
registry.obj: registry.c registry.h
shell.obj: shell.c systray.h
systray.obj: systray.c systray.h
