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
#ifndef _DSENGINEPACKAGESOURCE_H_
#define _DSENGINEPACKAGESOURCE_H_

// includes
#include "../dsPackageSource.h"

// predefinitions
class dsEngine;

// class dsEnginePackageSource
class DS_DLL_EXPORT dsEnginePackageSource : public dsPackageSource{
private:
	dsEngine *pEngine;
public:
	// constructor, destructor
	dsEnginePackageSource(dsEngine *engine);
	~dsEnginePackageSource();
	// determine if the given name matches an engine
	// package, either native or scripted
	bool CanHandle(const char *name);
	// loads the given engine package
	dsPackage *LoadPackage(const char *name);
private:
	const char *p_FindPackage(const char *name);
	void p_AddScripts(dsPackage *pak, const char *path);
	void p_AddNativeLib(dsPackage *pak, const char *path);
};

// end of include only once
#endif

