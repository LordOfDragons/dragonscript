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



// includes
#include "../../../scriptengine/libdscript.h"
#include "dsClassClass.h"
#include "dsClassVariable.h"
#include "dsClassFunction.h"
#include "dsClassDS.h"
#include "dsClassPackage.h"



// export definition
#ifdef __cplusplus
extern "C" {
#endif
PACKAGE_ENTRY_POINT_ATTR bool CreatePackage(dsPackageWrapper *wrapper);
#ifdef  __cplusplus
}
#endif



// entry function
// used to create the native classes and to add them
// to the provided package using the given wrapper.
// returns true on success or false otherwise.
//////////////////////////////////////////////////////
bool CreatePackage(dsPackageWrapper *wrapper){
	dsClass *cls=NULL;
	try{
		// create class class
		cls = new dsClassClass;
		if(!cls) return false;
		wrapper->AddNativeClass(cls);
		cls = NULL;
		// create class variable
		cls = new dsClassVariable;
		if(!cls) return false;
		wrapper->AddNativeClass(cls);
		cls = NULL;
		// create class function
		cls = new dsClassFunction;
		if(!cls) return false;
		wrapper->AddNativeClass(cls);
		cls = NULL;
		// create class ds
		cls = new dsClassDS;
		if(!cls) return false;
		wrapper->AddNativeClass(cls);
		cls = NULL;
		// create class package
		cls = new dsClassPackage;
		if(!cls) return false;
		wrapper->AddNativeClass(cls);
		cls = NULL;
	}catch( ... ){
		if(cls) delete cls;
		return false;
	}
	return true;
}
