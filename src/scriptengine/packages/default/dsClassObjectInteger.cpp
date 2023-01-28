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
#include "dsClassObjectInteger.h"

#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"


// Native data structure
//////////////////////////

struct sObjIntNatData{
	int value;
};



// Native Dunctions
/////////////////////

// public func new( int value )
dsClassObjectInteger::nfNew::nfNew( const sInitData &init ) :
dsFunction( init.clsObjInteger, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInteger ); // value
}
void dsClassObjectInteger::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjIntNatData &nd = *( ( sObjIntNatData* )p_GetNativeData( myself ) );
	nd.value = rt->GetValue( 0 )->GetInt();
}



// public func int value()
dsClassObjectInteger::nfValue::nfValue( const sInitData &init ) :
dsFunction( init.clsObjInteger, "value", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassObjectInteger::nfValue::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjIntNatData &nd = *( ( sObjIntNatData* )p_GetNativeData( myself ) );
	rt->PushInt( nd.value );
}



// public func String toString()
dsClassObjectInteger::nfToString::nfToString( const sInitData &init ) :
dsFunction( init.clsObjInteger, "toString", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassObjectInteger::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjIntNatData &nd = *( ( sObjIntNatData* )p_GetNativeData( myself ) );
	char buffer[ 20 ];
	
	snprintf( buffer, sizeof( buffer ), "%i", nd.value );
	rt->PushString( buffer );
}

// public func int hashCode()
dsClassObjectInteger::nfHashCode::nfHashCode( const sInitData &init ) :
dsFunction( init.clsObjInteger, "hashCode", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassObjectInteger::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjIntNatData &nd = *( ( sObjIntNatData* )p_GetNativeData( myself ) );
	rt->PushInt( nd.value );
}

// public func bool equals( Object object )
dsClassObjectInteger::nfEquals::nfEquals( const sInitData &init ) :
dsFunction( init.clsObjInteger, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectInteger::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjIntNatData &nd = *( ( sObjIntNatData* )p_GetNativeData( myself ) );
	dsClass * const clsObjInt = ( dsClassObjectInteger* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( p_IsObjOfType( object, clsObjInt ) ){
		const sObjIntNatData &other = *( ( sObjIntNatData* )p_GetNativeData( object ) );
		rt->PushBool( nd.value == other.value );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BYTE ){
		if( object->GetBool() ){
			rt->PushBool( nd.value == 1 );
			
		}else{
			rt->PushBool( nd.value == 0 );
		}
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BYTE ){
		rt->PushBool( nd.value == ( int )object->GetByte() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_INT ){
		rt->PushBool( nd.value == object->GetInt() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_FLOAT ){
		rt->PushBool( nd.value == ( int )object->GetFloat() );
		
	}else{
		rt->PushBool( false );
	}
}

// public func int compare( Object object )
dsClassObjectInteger::nfCompare::nfCompare( const sInitData &init ) :
dsFunction( init.clsObjInteger, "compare", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectInteger::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjIntNatData &nd = *( ( sObjIntNatData* )p_GetNativeData( myself ) );
	dsClass * const clsObjInt = ( dsClassObjectInteger* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( p_IsObjOfType( object, clsObjInt ) ){
		const sObjIntNatData &other = *( ( sObjIntNatData* )p_GetNativeData( object ) );
		rt->PushInt( nd.value - other.value );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BOOL ){
		if( object->GetBool() ){
			rt->PushInt( nd.value - 1 );
			
		}else{
			rt->PushInt( nd.value - 0 );
		}
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BYTE ){
		rt->PushInt( nd.value - ( int )object->GetByte() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_INT ){
		rt->PushInt( nd.value - object->GetInt() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_FLOAT ){
		rt->PushInt( nd.value - ( int )object->GetFloat() );
		
	}else{
		DSTHROW( dueInvalidAction );
	}
}



// Class dsClassObjectInteger
///////////////////////////////

// Constructor, Destructor
////////////////////////////

dsClassObjectInteger::dsClassObjectInteger() : dsClass( "Integer", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sObjIntNatData ) );
}

dsClassObjectInteger::~dsClassObjectInteger(){
}



// Management
///////////////

void dsClassObjectInteger::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsObjInteger = this;
	init.clsInteger = engine->GetClassInt();
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
