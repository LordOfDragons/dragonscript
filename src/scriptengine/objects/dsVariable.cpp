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



// includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../dragonscript_config.h"
#include "dsClass.h"
#include "dsVariable.h"
#include "dsValue.h"
#include "../dsRunTime.h"
#include "../exceptions.h"

// class dsVariable
/////////////////////
// constructor, destructor
dsVariable::dsVariable(dsClass *OwnerClass, const char *Name, dsClass *Type, int TypeModifiers){
	if(!OwnerClass || !Name || !Type) DSTHROW(dueInvalidParam);
	// check type modifiers
	if(TypeModifiers & DSTM_ABSTRACT) DSTHROW(dseInvalidType);
	if(TypeModifiers & DSTM_NATIVE) DSTHROW(dseInvalidType);
	if(TypeModifiers & DSTM_PUBLIC){
		if(TypeModifiers & (DSTM_PROTECTED | DSTM_PRIVATE)) DSTHROW(dseInvalidTypeModifier);
	}else if(TypeModifiers & DSTM_PROTECTED){
		if(TypeModifiers & DSTM_PRIVATE) DSTHROW(dseInvalidTypeModifier);
	}else if(!(TypeModifiers & DSTM_PRIVATE)){
		DSTHROW(dseInvalidTypeModifier);
	}
	if((TypeModifiers & DSTM_FIXED) && !(TypeModifiers & DSTM_STATIC)) DSTHROW(dseInvalidType);
	// check type
	if(Type->GetPrimitiveType() == DSPT_NULL) DSTHROW(dueInvalidParam);
	// init rest
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size + 1);
	#endif
	p_Name[ size ] = 0;
	p_OwnerClass = OwnerClass;
	p_Type = Type;
	p_Offset = 0;
	p_TypeModifiers = TypeModifiers;
	p_StaVal = NULL;
	if(TypeModifiers & DSTM_STATIC){
		p_StaVal = new dsValue(Type);
		if(!p_StaVal) DSTHROW(dueOutOfMemory);
	}
}
dsVariable::~dsVariable(){
	if(p_StaVal) delete p_StaVal;
	delete [] p_Name;
}
// management
void dsVariable::InitVariable(void *ParentData){
	if(p_TypeModifiers & DSTM_STATIC) DSTHROW(dueInvalidAction);
	if(!ParentData) DSTHROW(dueInvalidParam);
	dsValue *vData = (dsValue*)((byte*)(ParentData) + p_Offset);
	switch(p_Type->GetPrimitiveType()){
	case DSPT_BYTE: vData->SetByte(p_Type, 0); break;
	case DSPT_BOOL: vData->SetBool(p_Type, false); break;
	case DSPT_INT: vData->SetInt(p_Type, 0); break;
	case DSPT_FLOAT: vData->SetFloat(p_Type, 0); break;
	default: vData->SetRealObject(p_Type, NULL); break;
	}
}
void dsVariable::FreeVariable(dsRunTime *RT, void *ParentData){
	if(p_TypeModifiers & DSTM_STATIC) DSTHROW(dueInvalidAction);
	if(!RT || !ParentData) DSTHROW(dueInvalidParam);
	dsValue *vData = (dsValue*)((byte*)(ParentData) + p_Offset);
	RT->ClearValue(vData);
}
void dsVariable::SetOffset(int Offset){
	if(p_TypeModifiers & DSTM_STATIC) DSTHROW(dueInvalidAction);
	if(Offset < 0) DSTHROW(dueInvalidParam);
	p_Offset = Offset;
}
int dsVariable::SizeOf() const{
	return sizeof(dsValue);
}
dsValue *dsVariable::GetValue(void *ParentData){
	if(p_TypeModifiers & DSTM_STATIC) return p_StaVal;
	if(!ParentData) DSTHROW(dueInvalidParam);
	return (dsValue*)((byte*)(ParentData) + p_Offset);
}
// static content
void dsVariable::FreeStaticValue(dsRunTime *RT){
	if(!(p_TypeModifiers & DSTM_STATIC)) DSTHROW(dueInvalidAction);
	if(!RT) DSTHROW(dueInvalidParam);
	RT->ClearValue(p_StaVal);
}
