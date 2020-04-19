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



// Project: DragonScript - Interpreter
// Version: 1.0
// File: script class system

// include only once
#ifndef _DSICLASSSYSTEM_H_
#define _DSICLASSSYSTEM_H_

// includes
#include "../scriptengine/libdscript.h"

// predefinitions

// script class system
class dsiClassSystem : public dsClass{
public:
	// constructors, destructor
	dsiClassSystem();
	~dsiClassSystem();
	// management
	void CreateClassMembers(dsEngine *engine);
	
private:
	struct sInitData{
		dsClass *clsSys;
		dsClass *clsVoid;
		dsClass *clsFlt;
		dsClass *clsStr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfPrint);
	DEF_NATFUNC(nfGetString);
	DEF_NATFUNC(nfGetTickTime);
#undef DEF_NATFUNC
};

// end of include only once
#endif

