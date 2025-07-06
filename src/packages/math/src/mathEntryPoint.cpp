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

#include "../../../scriptengine/libdscript.h"
#include "dsClassMath.h"

#ifdef WITH_INTERNAL_PACKAGES

#include "../../../scriptengine/packages/dsBaseEnginePackage.h"

class dsMathInternalModule : public dsBaseEnginePackage{
public:
	dsMathInternalModule() = default;
	~dsMathInternalModule() override = default;
	
	const char *GetName() const override{
		return "Math";
	}
	
	dsPackage *CreatePackage() override{
		dsPackage * const package = new dsPackage(GetName());
		try{
			package->AddHostClass(new dsClassMath);
		}catch(...){
			delete package;
			throw;
		}
		return package;
	}
};

dsBaseEnginePackage *dsMathRegisterInternalModule(){
	return new dsMathInternalModule;
}

#else

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
		// create math class
		cls = new dsClassMath;
		if(!cls) return false;
		wrapper->AddNativeClass(cls);
		cls = NULL;
	}catch( ... ){
		if(cls) delete cls;
		return false;
	}
	return true;
}

#endif
