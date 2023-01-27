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
#ifndef _DSBASEENGINEPACKAGE_H_
#define _DSBASEENGINEPACKAGE_H_

// includes
#include "../dragonscript_export.h"

// predefinitions
class dsPackage;

// base engine package interface
class DS_DLL_EXPORT dsBaseEnginePackage{
public:
	// destructor
	virtual ~dsBaseEnginePackage();
	// creation
	virtual const char *GetName() const = 0;
	virtual dsPackage *CreatePackage() = 0;
};

// end of include only once
#endif
