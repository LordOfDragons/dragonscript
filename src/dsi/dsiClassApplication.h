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
// File: script class application

// include only once
#ifndef _DSICLASSAPPLICATION_H_
#define _DSICLASSAPPLICATION_H_

// includes
#include "../scriptengine/libdscript.h"

// predefinitions

// script class application
class dsiClassApplication : public dsClass{
private:
	int p_argc;
	char **p_args;
public:
	// constructors, destructor
	dsiClassApplication(int argc, char **args);
	~dsiClassApplication();
	// management
	void CreateClassMembers(dsEngine *engine);
	
private:
	struct sInitData{
		dsClass *clsApp;
		dsClass *clsVoid;
		dsClass *clsInt;
		dsClass *clsStr;
		int argc;
		char **args;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	
	DEF_NATFUNC(nfCreate);
	class nfGetArgumentCount : public dsFunction{
	private:
		int argc;
	public:
		nfGetArgumentCount(const sInitData &init);
		void RunFunction( dsRunTime *rt, dsValue *myself );
	};
	class nfGetArgument : public dsFunction{
	private:
		int argc;
		char **args;
	public:
		nfGetArgument(const sInitData &init);
		void RunFunction( dsRunTime *rt, dsValue *myself );
	};
	DEF_NATFUNC(nfRun);
#undef DEF_NATFUNC
};

// end of include only once
#endif

