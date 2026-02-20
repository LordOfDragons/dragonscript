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
#ifndef _DSVARIABLE_H_
#define _DSVARIABLE_H_

// includes
#include "../utils/dsuArrayable.h"

// prefinitions
class dsClass;
class dsFunction;
class dsValue;
class dsRunTime;

// class dsVariable
class DS_DLL_EXPORT dsVariable : public dsuArrayable {
private:
	dsClass *p_OwnerClass;
	char *p_Name;
	dsClass *p_Type;
	int p_Offset;
	int p_TypeModifiers;
	dsValue *p_StaVal;
public:
	// constructor, destructor
	dsVariable(dsClass *OwnerClass, const char *Name, dsClass *Type, int TypeModifiers = DSTM_PUBLIC);
	~dsVariable();
	// management
	inline dsClass *GetOwnerClass() const{ return p_OwnerClass; }
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dsClass *GetType() const{ return p_Type; }
	inline int GetOffset() const{ return p_Offset; }
	void InitVariable(void *ParentData);
	void FreeVariable(dsRunTime *RT, void *ParentData);
	void SetOffset(int Offset);
	dsValue *GetValue(void *ParentData);
	inline int GetTypeModifiers() const{ return p_TypeModifiers; }
	// static content
	inline dsValue *GetStaticValue(){ return p_StaVal; }
	void FreeStaticValue(dsRunTime *RT);
};

// end of include only once
#endif

