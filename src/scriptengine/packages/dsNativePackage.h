/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2015, Roland Plüss (roland@rptd.ch)
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



// include only once
#ifndef _DSNATIVEPACKAGE_H_
#define _DSNATIVEPACKAGE_H_

// includes
#include "../dsPackageWrapper.h"

#ifdef OS_BEOS
#	include <kernel/image.h>
#elif defined OS_W32
#	ifdef _WIN32_WINNT
#		undef _WIN32_WINNT
#	endif
#	define _WIN32_WINNT _WIN32_WINNT_WIN7
#	include <windows.h>
#elif defined HAVE_DLFCN_H
#	include <dlfcn.h>
#endif

// predefinitions
class dsEngine;


// native package
class DS_DLL_EXPORT dsNativePackage{
private:
	class cWrapper : public dsPackageWrapper{
	private:
		dsPackage *pPackage;
	public:
		cWrapper(dsPackage *pak);
		~cWrapper();
		void AddNativeClass(dsClass *cls);
	};
private:
	char *pFilename;
#if defined(HAVE_DLFCN_H)
	void *pHandle;
#elif defined(OS_BEOS)
	image_id pHandle;
#elif defined(OS_W32)
	HMODULE pHandle;
#endif
public:
	// destructor
	dsNativePackage(const char *filename);
	~dsNativePackage();
	// modules management
	inline const char *GetFilename() const{ return (const char*)pFilename; }
	void LoadIntoPackage(dsEngine *engine, dsPackage *package);
};

// end of include only once
#endif
