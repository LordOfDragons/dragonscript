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
#include "dsFunction.h"
#include "dsSignature.h"
#include "dsByteCode.h"
#include "dsRealObject.h"
#include "dsValue.h"
#include "optimized/dsFunctionOptimized.h"
#include "../dsRunTime.h"
#include "../exceptions.h"

// class dsFunction
//////////////////////////
// constructor, destructor
dsFunction::dsFunction(dsClass *OwnerClass, const char *Name, int FuncType, int TypeModifiers, dsClass *Type) :
pOptimized( NULL )
{
	if(!OwnerClass || !Type || !Name){
		printf("[ERROR!] function %s.%s\n", OwnerClass ? OwnerClass->GetName() : "?",
			Name ? Name : "?");
		DSTHROW(dueInvalidParam);
	}
	if(!dsiIsValidFuncType(FuncType)) DSTHROW(dseInvalidPrimitiveType);
	
	if( ! AreTypeModsValid( FuncType, TypeModifiers ) ){
		DSTHROW( dseInvalidTypeModifier );
	}
	
	// init the rest
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size);
	#endif
	p_Name[ size ] = 0;
	p_OwnerClass = OwnerClass;
	p_FuncType = FuncType;
	p_Type = Type;
	if(!(p_Signature = new dsSignature)) DSTHROW(dueOutOfMemory);
	p_ByteCode = NULL;
	p_TypeModifiers = TypeModifiers & DSTM_ALLTYPES;
	if(!(p_TypeModifiers & DSTM_NATIVE)){
		if(!(p_ByteCode = new dsByteCode)) DSTHROW(dueOutOfMemory);
	}
	p_LocVarSize = 0;
	// determine return value
	p_HasRetVal = Type->GetPrimitiveType() != DSPT_VOID;
}
dsFunction::~dsFunction(){
	delete p_Signature;
	delete [] p_Name;
	if(p_ByteCode) delete p_ByteCode;
	if( pOptimized ){
		delete pOptimized;
	}
}
// management
bool dsFunction::IsEqualTo(dsFunction *Function) const{
	if(strcmp(p_Name, Function->p_Name) != 0) return false;
//	if(!p_Type->IsEqualTo(Function->p_Type)) return false;
	if(!p_Signature->IsEqualTo(Function->p_Signature)) return false;
	return true;
}
void dsFunction::SetLocVarSize(int Size){
	if(Size < 0) DSTHROW(dueInvalidParam);
	p_LocVarSize = Size;
}

void dsFunction::SetOptimized( dsFunctionOptimized *optimized ){
	if( optimized == pOptimized ){
		return;
	}
	
	if( pOptimized ){
		delete pOptimized;
	}
	
	pOptimized = optimized;
}

// run function
void dsFunction::RunFunction( dsRunTime *rt, dsValue *myself ){
#ifdef __DEBUG__
	if( ! p_ByteCode ) DSTHROW( dueInvalidAction );
#endif
	
#ifdef WITH_OPTIMIZATIONS
	if( ! pOptimized ){
		rt->p_ExecFunc( myself->GetRealObject(), p_OwnerClass, this, NULL );
		
	}else{
		pOptimized->RunFunction( rt, myself );
	}
#else
	rt->p_ExecFunc( myself->GetRealObject(), p_OwnerClass, this, NULL );
#endif
}



bool dsFunction::AreTypeModsValid( int functionType, int typeModifiers ){
	const bool typeModPublic = ( ( typeModifiers & DSTM_PUBLIC ) == DSTM_PUBLIC );
	const bool typeModProtected = ( ( typeModifiers & DSTM_PROTECTED ) == DSTM_PROTECTED );
	const bool typeModPrivate = ( ( typeModifiers & DSTM_PRIVATE ) == DSTM_PRIVATE );
	const bool typeModFixed = ( ( typeModifiers & DSTM_FIXED ) == DSTM_FIXED );
	const bool typeModAbstract = ( ( typeModifiers & DSTM_ABSTRACT ) == DSTM_ABSTRACT );
	const bool typeModStatic = ( ( typeModifiers & DSTM_STATIC ) == DSTM_STATIC );
	
	if( typeModPublic ){
		if( typeModProtected || typeModPrivate ){
			return false;
		}
		
	}else if( typeModProtected ){
		if( typeModPrivate ){
			return false;
		}
		
	}else if( ! typeModPrivate ){
		return false;
	}
	
	if( typeModFixed ){
		return false;
	}
	
	if( functionType == DSFT_FUNCTION ){
		if( typeModAbstract && typeModStatic ){
			return false;
		}
		
	}else{
		if( functionType != DSFT_OPERATOR && typeModFixed ){
			return false;
		}
		
		if( typeModAbstract || typeModStatic ){
			return false;
		}
	}
	
	return true;
}

// protected functions
// helper functions for native implementators
void *dsFunction::p_GetNativeData(dsValue *This) const{
//	if(This->GetType()->GetPrimitiveType() != DSPT_OBJECT) DSTHROW(dueInvalidParam);
//	if(!This->GetRealObject()) DSTHROW(dueInvalidParam);
	return p_OwnerClass->p_GetNativeData( This->GetRealObject()->GetBuffer() );
}
void dsFunction::p_AddParameter(dsClass *type){
	p_Signature->AddParameter(type);
}
bool dsFunction::p_IsObjOfType(dsValue *obj, dsClass *type) const{
	if(!obj || !type) DSTHROW(dueInvalidParam);
	return obj->GetType()->GetPrimitiveType() == DSPT_OBJECT &&
		obj->GetRealObject() &&
		type->IsEqualTo( obj->GetRealObject()->GetType() );
}
