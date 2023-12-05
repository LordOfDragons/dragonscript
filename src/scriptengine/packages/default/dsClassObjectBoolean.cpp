/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2018, Roland Plüss (roland@rptd.ch)
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
#include <string.h>

#include "../../dragonscript_config.h"
#include "dsClassObjectBoolean.h"

#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"


// Native data structure
//////////////////////////

struct sObjBoolNatData{
	bool value;
};



// Native Dunctions
/////////////////////

// public func new( bool value )
dsClassObjectBoolean::nfNew::nfNew( const sInitData &init ) :
dsFunction( init.clsObjBoolean, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBoolean ); // value
}
void dsClassObjectBoolean::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjBoolNatData &nd = *( ( sObjBoolNatData* )p_GetNativeData( myself ) );
	nd.value = rt->GetValue( 0 )->GetBool();
}



// public func bool value()
dsClassObjectBoolean::nfValue::nfValue( const sInitData &init ) :
dsFunction( init.clsObjBoolean, "value", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
}
void dsClassObjectBoolean::nfValue::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjBoolNatData &nd = *( ( sObjBoolNatData* )p_GetNativeData( myself ) );
	rt->PushBool( nd.value );
}



// public func String toString()
dsClassObjectBoolean::nfToString::nfToString( const sInitData &init ) :
dsFunction( init.clsObjBoolean, "toString", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassObjectBoolean::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjBoolNatData &nd = *( ( sObjBoolNatData* )p_GetNativeData( myself ) );
	
	if( nd.value ){
		rt->PushString( "True" );
		
	}else{
		rt->PushString( "False" );
	}
}

// public func int hashCode()
dsClassObjectBoolean::nfHashCode::nfHashCode( const sInitData &init ) :
dsFunction( init.clsObjBoolean, "hashCode", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassObjectBoolean::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjBoolNatData &nd = *( ( sObjBoolNatData* )p_GetNativeData( myself ) );
	
	if( nd.value ){
		rt->PushInt( 1 );
		
	}else{
		rt->PushInt( 0 );
	}
}

// public func bool equals( Object object )
dsClassObjectBoolean::nfEquals::nfEquals( const sInitData &init ) :
dsFunction( init.clsObjBoolean, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectBoolean::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjBoolNatData &nd = *( ( sObjBoolNatData* )p_GetNativeData( myself ) );
	dsClass * const clsObjBoolean = ( dsClassObjectBoolean* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( p_IsObjOfType( object, clsObjBoolean ) ){
		const sObjBoolNatData &other = *( ( sObjBoolNatData* )p_GetNativeData( object ) );
		rt->PushBool( nd.value == other.value );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BOOL ){
		rt->PushBool( nd.value == object->GetBool() );
		
	}else{
		rt->PushBool( false );
	}
}

// public func int compare( Object object )
dsClassObjectBoolean::nfCompare::nfCompare( const sInitData &init ) :
dsFunction( init.clsObjBoolean, "compare", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectBoolean::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjBoolNatData &nd = *( ( sObjBoolNatData* )p_GetNativeData( myself ) );
	dsClass * const clsObjBoolean = ( dsClassObjectBoolean* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	bool boolOther;
	
	if( p_IsObjOfType( object, clsObjBoolean ) ){
		const sObjBoolNatData &other = *( ( sObjBoolNatData* )p_GetNativeData( object ) );
		boolOther = other.value;
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BOOL ){
		boolOther = object->GetBool();
		
	}else{
		DSTHROW( dueInvalidAction );
	}
	
	if( ! nd.value && boolOther ){
		rt->PushInt( -1 );
		
	}else if( nd.value && ! boolOther ){
		rt->PushInt( 1 );
		
	}else{
		rt->PushInt( 0 );
	}
}



// Class dsClassObjectBoolean
////////////////////////////////

// Constructor, Destructor
////////////////////////////

dsClassObjectBoolean::dsClassObjectBoolean() : dsClass( "Boolean", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sObjBoolNatData ) );
}

dsClassObjectBoolean::~dsClassObjectBoolean(){
}



// Management
///////////////

void dsClassObjectBoolean::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsObjBoolean = this;
	init.clsInteger = engine->GetClassInt();
	init.clsBoolean = engine->GetClassBool();
	init.clsVoid = engine->GetClassVoid();
	init.clsBoolean = engine->GetClassBool();
	init.clsObject = engine->GetClassObject();
	init.clsString = engine->GetClassString();
	
	AddFunction( new nfNew( init ) );
	
	AddFunction( new nfValue( init ) );

	AddFunction( new nfToString( init ) );
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfCompare( init ) );
}
