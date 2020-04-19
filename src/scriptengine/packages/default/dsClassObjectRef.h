/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2019, Roland Plüss (roland@rptd.ch)
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



#ifndef _DSOBJECTREF_H_
#define _DSOBJECTREF_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;

// class deClassObjectRef
class dsClassObjectRef : public dsClass {
public:
	// constructor, destructor
	   dsClassObjectRef();
	   ~dsClassObjectRef();
	// management
	void CreateClassMembers(dsEngine *engine);

private:
	struct sInitData{
		dsClass *clsObjRef, *clsVoid, *clsInt, *clsBool, *clsObj;
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
#undef DEF_NATFUNC
};

// end of include only once
#endif

