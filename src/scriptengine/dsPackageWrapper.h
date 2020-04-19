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
#ifndef _DSPACKAGEWRAPPER_H_
#define _DSPACKAGEWRAPPER_H_


// predefinitions
class dsClass;

// package wrapper class
// used by native-packages during creation time
class dsPackageWrapper{
public:

	// virtual destructor
	virtual ~dsPackageWrapper();

	// add a native class to the package. the class has to be
	// only created and no further steps done. also the class
	// must be non-null obviously and must not be already added.
	virtual void AddNativeClass(dsClass *theClass) = 0;
	
	// scripted classes are not supported by native packages
	// as for including them just place them in the same directory
	// as the library file is.
};

// end of include only once
#endif
