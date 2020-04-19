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
#ifndef _DSCLASSMATH_H_
#define _DSCLASSMATH_H_

// includes
#include "../../../scriptengine/libdscript.h"

// predefinitions

// class dsClassMath
class dsClassMath : public dsClass {
public:
	// constructor, destructor
	dsClassMath();
	~dsClassMath();
	// management
	void CreateClassMembers(dsEngine *engine);

private:
	struct sInitData{
		dsClass *clsMath, *clsVoid, *clsInt, *clsFlt;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfabs);
	DEF_NATFUNC(nfacos);
	DEF_NATFUNC(nfasin);
	DEF_NATFUNC(nfatan);
	DEF_NATFUNC(nfatan2);
	DEF_NATFUNC(nfceil);
	DEF_NATFUNC(nfcos);
	DEF_NATFUNC(nfcosh);
	DEF_NATFUNC(nfexp);
	DEF_NATFUNC(nffabs);
	DEF_NATFUNC(nffloor);
	DEF_NATFUNC(nffmod);
	DEF_NATFUNC(nflog);
	DEF_NATFUNC(nflog10);
	DEF_NATFUNC(nfpow);
	DEF_NATFUNC(nfsin);
	DEF_NATFUNC(nfsinh);
	DEF_NATFUNC(nfsqrt);
	DEF_NATFUNC(nftan);
	DEF_NATFUNC(nftanh);
	DEF_NATFUNC( nfMinI );
	DEF_NATFUNC( nfMinF );
	DEF_NATFUNC( nfMaxI );
	DEF_NATFUNC( nfMaxF );
	DEF_NATFUNC( nfClampI );
	DEF_NATFUNC( nfClampF );
#undef DEF_NATFUNC
};

// end of include only once
#endif

