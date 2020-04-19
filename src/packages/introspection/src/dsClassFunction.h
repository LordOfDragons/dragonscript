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
#ifndef _DSCLASSFUNCTION_H_
#define _DSCLASSFUNCTION_H_

// includes
#include "../../../scriptengine/libdscript.h"

// predefinitions
class dsClassClass;

// class dsClassFunction
class dsClassFunction : public dsClass {
private:
	dsClassClass *pClsClass;
	dsClass *pClsArr;
	dsClass *pClsObj;
public:
	// constructor, destructor
	dsClassFunction();
	~dsClassFunction();
	// management
	void CreateClassMembers(dsEngine *engine);
	// internal functions
	void PushFunction(dsRunTime *RT, dsFunction *function);
	void PushSignature(dsRunTime *RT, dsSignature *sig );
	void PushFullName(dsRunTime *RT, dsFunction *function);
	inline dsClassClass *GetClassClass() const{ return pClsClass; }
	inline dsClass *GetClassObject() const{ return pClsObj; }
	void CallFunction(dsRunTime *RT, dsFunction *func, bool direct=false);

private:
	struct sInitData{
		dsClass *clsFunc, *clsClass, *clsVoid, *clsInt, *clsBool, *clsStr, *clsObj, *clsArr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfGetName);
	DEF_NATFUNC(nfGetFullName);
	DEF_NATFUNC(nfGetType);
	DEF_NATFUNC(nfGetTypeModifiers);
	DEF_NATFUNC(nfGetParent);
	DEF_NATFUNC(nfGetSignature);
	DEF_NATFUNC(nfCallDirect);
	DEF_NATFUNC(nfCall);
	DEF_NATFUNC(nfIsSet);
	DEF_NATFUNC(nfHashCode);
	DEF_NATFUNC(nfEquals);
	DEF_NATFUNC(nfToString);
#undef DEF_NATFUNC
};

// end of include only once
#endif

