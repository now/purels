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


#ifndef __STARTUP_H
	#define __STARTUP_H

	#ifdef STARTUP_OPENED
		#define STARTUP __declspec( dllexport )
	#else
		#define STARTUP
	#endif

	#ifdef __cplusplus
		extern "C" {
	#endif

		STARTUP void RunEntriesIn ( HKEY, LPCTSTR );
		STARTUP void DeleteEntriesIn ( HKEY, LPCTSTR );
		STARTUP void RunFolderContents( LPCTSTR );
		STARTUP void RunStartupMenu ( void );

	#ifdef __cplusplus
		};
	#endif

#endif /*__STARTUP_H*/