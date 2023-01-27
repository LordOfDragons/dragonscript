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
#ifndef _DSCLASSSTRINGBUFFER_H_
#define _DSCLASSSTRINGBUFFER_H_

// includes
#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

// predefinitions
class dsEngine;
class dsRealObject;

// class dsClassStringBuffer
class DS_DLL_EXPORT dsClassStringBuffer : public dsClass {
friend class dsRunTime;
public:
	// constructor, destructor
	dsClassStringBuffer();
	~dsClassStringBuffer();
	// management
	void CreateClassMembers(dsEngine *engine);

private:
	struct sInitData{
		dsClass *clsSB, *clsStr, *clsVoid, *clsByte, *clsBool, *clsInt, *clsFlt;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfNew);
	DEF_NATFUNC(nfNew2);
	DEF_NATFUNC(nfNew3);
	DEF_NATFUNC(nfNew4);
	DEF_NATFUNC(nfDestructor);
	DEF_NATFUNC(nfGetLength);
	DEF_NATFUNC(nfGetChar);
	DEF_NATFUNC(nfSetChar);
	DEF_NATFUNC(nfFindChar);
	DEF_NATFUNC(nfToString);
	DEF_NATFUNC(nfOpAdd);
	DEF_NATFUNC(nfOpAddByte);
	DEF_NATFUNC(nfOpAddBool);
	DEF_NATFUNC(nfOpAddInt);
	DEF_NATFUNC(nfOpAddFloat);
	DEF_NATFUNC(nfOpAddString);
#undef DEF_NATFUNC
};

// end of include only once
#endif

