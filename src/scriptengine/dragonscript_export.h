/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2023, Roland Plüss (roland@rptd.ch)
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef _DRAGONSCRIPT_EXPORT_H_
#define _DRAGONSCRIPT_EXPORT_H_

#include "dragonscript_config.h"

#ifdef OS_W32_VS
	#ifdef DS_COMPILE_DLL
		#define DS_DLL_EXPORT __declspec(dllexport)
	#else
		#define DS_DLL_EXPORT __declspec(dllimport)
	#endif
	
	// for use by package entry points
	#define PACKAGE_ENTRY_POINT_ATTR __declspec(dllexport)
	
	// disable warning about "class X needs to have dll-interface to be used by clients of class Y"
	#pragma warning( disable: 4251 )
	
	// disable warning about "nonstandard extension used: zero-sized array in stuct/union"
	#pragma warning( disable: 4200 )

#else
	#define DS_DLL_EXPORT

	// for use by package entry points
	#define PACKAGE_ENTRY_POINT_ATTR
#endif

#endif
