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
#ifndef _DSCLASSEXCEPTION_H_
#define _DSCLASSEXCEPTION_H_

// includes
#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

// predefinitions
class dsEngine;
class dsValue;
class dsExceptionTrace;


// class dsClassException
class dsClassException : public dsClass {
public:
	// constructor, destructor
	dsClassException();
	~dsClassException();
	// management
	void CreateClassMembers(dsEngine *engine);
	// internal functions
	dsExceptionTrace *GetTrace( dsRealObject *myself );
	void PushException(dsRunTime *RT, const char *reason);
	void AddTrace(dsValue *obj, dsClass *callClass, dsFunction *function, int codeOffset);
	void PrintTrace(dsRunTime *RT, dsValue *obj);
private:
	char *BuildFullName(dsClass *theClass);

private:
	struct sInitData{
		dsClass *clsExcep, *clsObj, *clsVoid, *clsByte, *clsBool, *clsInt, *clsStr;
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
	DEF_NATFUNC(nfGetReason);
	DEF_NATFUNC(nfPrintTrace);
	DEF_NATFUNC(nfGetTraceCount);
	DEF_NATFUNC(nfGetTraceClass);
	DEF_NATFUNC(nfGetTraceFunction);
	DEF_NATFUNC(nfGetTraceLine);
	DEF_NATFUNC(nfHashCode);
	DEF_NATFUNC(nfEquals);
	DEF_NATFUNC(nfToString);
#undef DEF_NATFUNC
};

// end of include only once
#endif

