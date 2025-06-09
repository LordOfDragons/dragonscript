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

#ifndef _DSENGINEPACKAGES_H_
#define _DSENGINEPACKAGES_H_

#include "../dragonscript_export.h"

class dsEngine;
class dsBaseEnginePackage;
class dsNativePackage;

/**
 * Class dsEnginePackages.
 */
class DS_DLL_EXPORT dsEnginePackages{
private:
	dsEngine *pEngine;
	dsBaseEnginePackage **pPackages;
	int pPkgCount, pPkgSize;
	dsNativePackage **pNatPaks;
	int pNatPakCount, pNatPakSize;
	
public:
	// constructor, destructor
	dsEnginePackages(dsEngine *Engine);
	~dsEnginePackages();
	
	// package management
	inline int GetPackageCount() const{ return pPkgCount; }
	dsBaseEnginePackage *GetPackage(int index) const;
	dsBaseEnginePackage *GetPackage(const char *name) const;
	
	// native package management
	inline int GetNativePackageCount() const{ return pNatPakCount; }
	dsNativePackage *GetNativePackage(int index) const;
	dsNativePackage *GetNativePackage(const char *filename) const;
	void AddNatPackage(dsNativePackage *package);
	
private:
	void pClear();
	void pAddPackage(dsBaseEnginePackage *package);
};

#endif
