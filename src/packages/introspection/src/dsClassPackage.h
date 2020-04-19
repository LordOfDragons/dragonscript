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
#ifndef _DSCLASSPACKAGE_H_
#define _DSCLASSPACKAGE_H_

// includes
#include "../../../scriptengine/libdscript.h"

// predefinitions
class dsClassClass;

// class dsClassPackage
class dsClassPackage : public dsClass {
private:
	dsClassClass *pClsClass;
public:
	// constructor, destructor
	dsClassPackage();
	~dsClassPackage();
	// management
	void CreateClassMembers(dsEngine *engine);
	// internal functions
	void PushPackage(dsRunTime *RT, dsPackage *pak);
	dsPackage *GetPackageFromObj(dsRealObject *obj);
	inline dsClassClass *GetClassClass() const{ return pClsClass; }

private:
	struct sInitData{
		dsClass *clsPak, *clsClass;
		dsClass *clsVoid, *clsInt, *clsStr, *clsBool, *clsObj;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfNew);
	DEF_NATFUNC(nfGetName);
	DEF_NATFUNC(nfGetClassCount);
	DEF_NATFUNC(nfGetClass);
	DEF_NATFUNC(nfGetClass2);
	DEF_NATFUNC(nfContainsClass);
	DEF_NATFUNC(nfHashCode);
	DEF_NATFUNC(nfEquals);
	DEF_NATFUNC(nfToString);
#undef DEF_NATFUNC
};

// end of include only once
#endif

