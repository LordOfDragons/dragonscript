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
#ifndef _DSPACKAGESOURCE_H_
#define _DSPACKAGESOURCE_H_

// includes
#include "utils/dsuArrayable.h"

// predefinitions
class dsPackage;

// class dsPackageSource
class DS_DLL_EXPORT dsPackageSource : public dsuArrayable{
public:
	// determine if the given package name can be handled
	// by this package source. if you return true then
	// you will be responsible for loading this package.
	virtual bool CanHandle(const char *name) = 0;
	// load the package by creating a package object and
	// fill it with host classes and/or script sources.
	// you have to return a valid package object. if this
	// is not possible throw an appropriate exceptions.
	virtual dsPackage *LoadPackage(const char *name) = 0;
};

// end of include only once
#endif

