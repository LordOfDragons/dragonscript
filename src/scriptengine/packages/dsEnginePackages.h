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
#ifndef _DSENGINEPACKAGES_H_
#define _DSENGINEPACKAGES_H_

// includes

// predefinitions
class dsEngine;
class dsBaseEnginePackage;
class dsNativePackage;

// class dsEnginePackages
class dsEnginePackages{
private:
	dsEngine *p_Engine;
	dsBaseEnginePackage **p_Packages;
	int p_PkgCount, p_PkgSize;
	dsNativePackage **p_NatPaks;
	short p_NatPakCount, p_NatPakSize;
public:
	// constructor, destructor
	dsEnginePackages(dsEngine *Engine);
	~dsEnginePackages();
	// package management
	inline int GetPackageCount() const{ return p_PkgCount; }
	dsBaseEnginePackage *GetPackage(int Index) const;
	dsBaseEnginePackage *GetPackage(const char *Name) const;
	// native package management
	inline int GetNativePackageCount() const{ return p_NatPakCount; }
	dsNativePackage *GetNativePackage(int Index) const;
	dsNativePackage *GetNativePackage(const char *filename) const;
	void AddNatPackage(dsNativePackage *Package);
private:
	void p_Clear();
	void p_AddPackage(dsBaseEnginePackage *Package);
};

// end of include only once
#endif

