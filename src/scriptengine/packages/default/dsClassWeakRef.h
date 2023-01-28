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
#ifndef _DSWEAKREF_H_
#define _DSWEAKREF_H_

// includes
#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

// predefinitions
class dsEngine;
class dsValue;

// class dsClassWeakRef
class DS_DLL_EXPORT dsClassWeakRef : public dsClass {
public:
	// constructor, destructor
	dsClassWeakRef();
	~dsClassWeakRef();
	// management
	void CreateClassMembers(dsEngine *engine);

private:
	struct sInitData{
		dsClass *clsWeak, *clsVoid, *clsInt, *clsBool, *clsObj;
		dsClass *clsBlock;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfNew);
	DEF_NATFUNC(nfNew2);
	DEF_NATFUNC(nfDestructor);
	DEF_NATFUNC(nfSet);
	DEF_NATFUNC(nfHashCode);
	DEF_NATFUNC(nfEquals);
	DEF_NATFUNC(nfGet);
	DEF_NATFUNC(nfRun);
	DEF_NATFUNC(nfRunCastable);
#undef DEF_NATFUNC
};

// end of include only once
#endif

