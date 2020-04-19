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
#ifndef _DSCLASSCLASS_H_
#define _DSCLASSCLASS_H_

// includes
#include "../../../scriptengine/libdscript.h"

// predefinitions
class dsClassVariable;
class dsClassFunction;

// class dsClassClass
class dsClassClass : public dsClass {
private:
	dsClass *p_ClsObj;
	dsClassVariable *pClsVar;
	dsClassFunction *pClsFunc;
public:
	// constructor, destructor
	dsClassClass();
	~dsClassClass();
	// management
	void CreateClassMembers(dsEngine *engine);
	// internal functions
	void PushClass(dsRunTime *RT, dsClass *theClass);
	void PushFullName(dsRunTime *RT, dsClass *theClass);
	dsClass *GetClassFromObj(dsRealObject *obj);
	inline dsClassVariable *GetClassVariable() const{ return pClsVar; }
	inline dsClassFunction *GetClassFunction() const{ return pClsFunc; }

private:
	struct sInitData{
		dsClass *clsClass, *clsVar, *clsFunc;
		dsClass *clsVoid, *clsInt, *clsStr, *clsBool, *clsObj, *clsArr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfNew);
	DEF_NATFUNC(nfNew2);
	DEF_NATFUNC(nfGetName);
	DEF_NATFUNC(nfGetFullName);
	DEF_NATFUNC(nfGetClassType);
	DEF_NATFUNC(nfGetTypeModifiers);
	DEF_NATFUNC(nfGetBase);
	DEF_NATFUNC(nfGetParent);
	DEF_NATFUNC(nfIsSubClassOf);
	DEF_NATFUNC(nfGetInterfaceCount);
	DEF_NATFUNC(nfGetInterface);
	DEF_NATFUNC(nfGetClassCount);
	DEF_NATFUNC(nfGetClass);
	DEF_NATFUNC(nfGetVariableCount);
	DEF_NATFUNC(nfGetVariable);
	DEF_NATFUNC(nfGetVariable2);
	DEF_NATFUNC(nfGetFunctionCount);
	DEF_NATFUNC(nfGetFunction);
	DEF_NATFUNC(nfGetFunction2);
	DEF_NATFUNC(nfFindFunction);
	DEF_NATFUNC(nfIsSet);
	DEF_NATFUNC(nfHashCode);
	DEF_NATFUNC(nfEquals);
	DEF_NATFUNC(nfToString);
#undef DEF_NATFUNC
};

// end of include only once
#endif

