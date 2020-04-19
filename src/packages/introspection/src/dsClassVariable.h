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
#ifndef _DSCLASSVARIABLE_H_
#define _DSCLASSVARIABLE_H_

// includes
#include "../../../scriptengine/libdscript.h"

// predefinitions
class dsClassClass;

// class dsClassVariable
class dsClassVariable : public dsClass {
private:
	dsClassClass *pClsClass;
public:
	// constructor, destructor
	dsClassVariable();
	~dsClassVariable();
	// management
	void CreateClassMembers(dsEngine *engine);
	// internal functions
	void PushConstant(dsRunTime *RT, dsClass *parent, dsConstant *constant);
	void PushVariable(dsRunTime *RT, dsVariable *variable);
	void PushFullName(dsRunTime *RT, dsClass *parent, dsConstant *constant, dsVariable *variable);
	inline dsClassClass *GetClassClass() const{ return pClsClass; }

private:
	struct sInitData{
		dsClass *clsVar, *clsClass, *clsVoid, *clsBool, *clsInt, *clsStr, *clsObj;
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
	DEF_NATFUNC(nfGetValue);
	DEF_NATFUNC(nfIsSet);
	DEF_NATFUNC(nfHashCode);
	DEF_NATFUNC(nfEquals);
	DEF_NATFUNC(nfToString);
#undef DEF_NATFUNC
};

// end of include only once
#endif

