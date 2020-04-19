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
#ifndef _DSCLASSDS_H_
#define _DSCLASSDS_H_

// includes
#include "../../../scriptengine/libdscript.h"

// predefinitions
class dsClassClass;
class dsClassPackage;


// class dsClassDS
class dsClassDS : public dsClass {
private:
	dsClassClass *pClsClass;
	dsClassPackage *pClsPak;
public:
	// constructor, destructor
	dsClassDS();
	~dsClassDS();
	// management
	void CreateClassMembers(dsEngine *engine);
	// internal functions
	inline dsClassClass *GetClassClass() const{ return pClsClass; }
	inline dsClassPackage *GetClassPackage() const{ return pClsPak; }

private:
	struct sInitData{
		dsClass *clsDS, *clsClass, *clsPak, *clsVoid, *clsInt, *clsStr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfGetClassCount);
	DEF_NATFUNC(nfGetClass);
	DEF_NATFUNC(nfGetPackageCount);
	DEF_NATFUNC(nfGetPackage);
#undef DEF_NATFUNC
};

// end of include only once
#endif

