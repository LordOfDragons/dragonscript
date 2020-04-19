/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2019, Roland Plüss (roland@rptd.ch)
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



#include <stdio.h>
#include <stdint.h>

#include "dsClassObjectRef.h"
#include "../../../config.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

struct sObjectRefNatData{
	dsValue *obj;
};

// Native functions
/////////////////////

// func new()
dsClassObjectRef::nfNew::nfNew( const sInitData &init ) :
dsFunction( init.clsObjRef, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassObjectRef::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjectRefNatData &nd = *( ( sObjectRefNatData* )p_GetNativeData( myself ) );
	nd.obj = NULL;
	
	nd.obj = rt->CreateValue();
}

// func new(object obj)
dsClassObjectRef::nfNew2::nfNew2( const sInitData &init ) :
dsFunction( init.clsObjRef, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassObjectRef::nfNew2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjectRefNatData &nd = *( ( sObjectRefNatData* )p_GetNativeData( myself ) );
	nd.obj = NULL;
	
	nd.obj = rt->CreateValue();
	rt->CopyValue( rt->GetValue( 0 ), nd.obj );
}

// func destructor
dsClassObjectRef::nfDestructor::nfDestructor( const sInitData &init ) :
dsFunction( init.clsObjRef, DSFUNC_DESTRUCTOR, DSFT_DESTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassObjectRef::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjectRefNatData &nd = *( ( sObjectRefNatData* )p_GetNativeData( myself ) );
	if( ! nd.obj ){
		return;
	}
	
	dsValue * const value = nd.obj;
	nd.obj = NULL;
	
	rt->FreeValue(value);
}

// func object get()
dsClassObjectRef::nfGet::nfGet( const sInitData &init ) : dsFunction(init.clsObjRef,
"get", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj){
}
void dsClassObjectRef::nfGet::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjectRefNatData &nd = *( ( sObjectRefNatData* )p_GetNativeData( myself ) );
	rt->PushValue( nd.obj );
}

// func void set(object obj)
dsClassObjectRef::nfSet::nfSet( const sInitData &init ) : dsFunction(init.clsObjRef,
"set", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassObjectRef::nfSet::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjectRefNatData &nd = *( ( sObjectRefNatData* )p_GetNativeData( myself ) );
	rt->CopyValue( rt->GetValue( 0 ), nd.obj );
}

// public func int hashCode()
dsClassObjectRef::nfHashCode::nfHashCode( const sInitData &init ) : dsFunction(init.clsObjRef,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassObjectRef::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjectRefNatData &nd = *( ( sObjectRefNatData* )p_GetNativeData( myself ) );
	
	switch( nd.obj->GetType()->GetPrimitiveType() ){
	case DSPT_NULL:
		rt->PushInt( 0 );
		return;
		
	case DSPT_OBJECT:
		if( ! nd.obj->GetRealObject() ){
			rt->PushInt( 0 );
			return;
		}
		break;
		
	default:
		break;
	}
	
	rt->RunFunction( nd.obj, "hashCode", 0 ); // return value is also our return value
}

// public func bool equals(Object obj)
dsClassObjectRef::nfEquals::nfEquals( const sInitData &init ) :
dsFunction( init.clsObjRef, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassObjectRef::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClass * const clsObjRefRef = ( dsClassObjectRef* )GetOwnerClass();
	dsValue * const obj = rt->GetValue( 0 );
	
	if( ! p_IsObjOfType( obj, clsObjRefRef ) ){
		rt->PushBool( false );
		return;
	}
	
	sObjectRefNatData &nd = *( ( sObjectRefNatData* )p_GetNativeData( myself ) );
	sObjectRefNatData &ndOther = *( (sObjectRefNatData* )p_GetNativeData( obj ) );
	
	const bool isNull = nd.obj->GetType()->GetPrimitiveType() == DSPT_NULL
		|| ( nd.obj->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! nd.obj->GetRealObject() );
	const bool isNullOther = ndOther.obj->GetType()->GetPrimitiveType() == DSPT_NULL
		|| ( ndOther.obj->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! ndOther.obj->GetRealObject() );
	
	if( isNull && isNullOther ){
		rt->PushBool( true );
		
	}else if( ( isNull && ! isNullOther ) || ( ! isNull && isNullOther ) ){
		rt->PushBool( false );
		
	}else{
		rt->PushValue( ndOther.obj );
		rt->RunFunction( nd.obj, "equals", 1 ); // return value is also our return value
	}
}



// Class deClassObjectRef
///////////////////////////

// Constructor, Destructor
////////////////////////////

dsClassObjectRef::dsClassObjectRef() : dsClass( "ObjectReference",
DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE) {
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sObjectRefNatData ) );
}

dsClassObjectRef::~dsClassObjectRef(){
}



// Management
///////////////

void dsClassObjectRef::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsObjRef = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsBool = engine->GetClassBool();
	init.clsObj = engine->GetClassObject();
	
	AddFunction( new nfNew( init ) );
	AddFunction( new nfNew2( init ) );
	AddFunction( new nfDestructor( init ) );
	AddFunction( new nfGet( init ) );
	AddFunction( new nfSet( init ) );
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfEquals( init ) );
}
