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


#ifndef __PURELSDEFS_H
	#define __PURELSDEFS_H

	#define PLS_MAX_SIZE 4096
	#define PLS_MAX_FILE _MAX_PATH + _MAX_FNAME + _MAX_EXT

	/*system message define block 9000-9199*/
	#define LM_QUIT 9000 // pass 0 as params just to quit
	 #define QC_LOGOFF 1
	 #define QC_REBOOT 2
	 #define QC_SHUTDOWN 3
	#define LM_RECYCLE 9001
	#define LM_RESTART 9002

	/*module message define block 9200-9399*/
	#define LM_STARTMODULE 9200
	#define LM_STOPMODULE 9201
	#define LM_STOPSTARTMODULE 9202
	#define LM_RESETMODULE 9203
	#define LM_ISMODULE 9204

	#define LM_REGISTERMESSAGE 9220
	#define LM_UNREGISTERMESSAGE 9221

	/*open module define block 9400-9999*/

	//old stuff defines
	#define LM_OLDRECYCLE 9260
	#define LM_OLDREGISTERMESSAGE 9263
	#define LM_OLDUNREGISTERMESSAGE 9264
	#define LM_OLDGETREVID 9265
	#define LM_OLDUNLOADMODULE 9266
	#define LM_OLDRELOADMODULE 9267

#endif /*__PURELSDEFS_H*/