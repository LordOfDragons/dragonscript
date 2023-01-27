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
#include "dsClassObjectFloat.h"

#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"


// Native data structure
//////////////////////////

struct sObjFloatNatData{
	float value;
};



// Native Dunctions
/////////////////////

// public func new( float value )
dsClassObjectFloat::nfNew::nfNew( const sInitData &init ) :
dsFunction( init.clsObjFloat, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsFloat ); // value
}
void dsClassObjectFloat::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjFloatNatData &nd = *( ( sObjFloatNatData* )p_GetNativeData( myself ) );
	nd.value = rt->GetValue( 0 )->GetFloat();
}



// public func float value()
dsClassObjectFloat::nfValue::nfValue( const sInitData &init ) :
dsFunction( init.clsObjFloat, "value", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsFloat ){
}
void dsClassObjectFloat::nfValue::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjFloatNatData &nd = *( ( sObjFloatNatData* )p_GetNativeData( myself ) );
	rt->PushFloat( nd.value );
}



// public func String toString()
dsClassObjectFloat::nfToString::nfToString( const sInitData &init ) :
dsFunction( init.clsObjFloat, "toString", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassObjectFloat::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjFloatNatData &nd = *( ( sObjFloatNatData* )p_GetNativeData( myself ) );
	char buffer[ 20 ];
	
	snprintf( buffer, sizeof( buffer ), "%g", nd.value );
	rt->PushString( buffer );
}

// public func int hashCode()
dsClassObjectFloat::nfHashCode::nfHashCode( const sInitData &init ) :
dsFunction( init.clsObjFloat, "hashCode", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassObjectFloat::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjFloatNatData &nd = *( ( sObjFloatNatData* )p_GetNativeData( myself ) );
	const int * const intValue = ( int* )&nd.value;
	rt->PushInt( *intValue >> 10 ); // cut off 10 bits for a bit of uncertainty to be allowed
}

// public func bool equals( Object object )
dsClassObjectFloat::nfEquals::nfEquals( const sInitData &init ) :
dsFunction( init.clsObjFloat, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectFloat::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjFloatNatData &nd = *( ( sObjFloatNatData* )p_GetNativeData( myself ) );
	dsClass * const clsObjFloat = ( dsClassObjectFloat* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( p_IsObjOfType( object, clsObjFloat ) ){
		const sObjFloatNatData &other = *( ( sObjFloatNatData* )p_GetNativeData( object ) );
		rt->PushBool( nd.value == other.value );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BOOL ){
		if( object->GetBool() ){
			rt->PushBool( nd.value == 1.0f );
			
		}else{
			rt->PushBool( nd.value == 0.0f );
		}
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BYTE ){
		rt->PushBool( nd.value == ( float )object->GetByte() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_INT ){
		rt->PushBool( nd.value == ( float )object->GetInt() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_FLOAT ){
		rt->PushBool( nd.value == object->GetFloat() );
		
	}else{
		rt->PushBool( false );
	}
}

// public func int compare( Object object )
dsClassObjectFloat::nfCompare::nfCompare( const sInitData &init ) :
dsFunction( init.clsObjFloat, "compare", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectFloat::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjFloatNatData &nd = *( ( sObjFloatNatData* )p_GetNativeData( myself ) );
	dsClass * const clsObjFloat = ( dsClassObjectFloat* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	const float float1 = nd.value;
	float float2 = float1;
	
	if( p_IsObjOfType( object, clsObjFloat ) ){
		const sObjFloatNatData &other = *( ( sObjFloatNatData* )p_GetNativeData( object ) );
		float2 = other.value;
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BOOL ){
		if( object->GetBool() ){
			float2 = 1.0f;
			
		}else{
			float2 = 0.0f;
		}
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BYTE ){
		float2 = ( float )object->GetByte();
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_INT ){
		float2 = ( float )object->GetInt();
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_FLOAT ){
		float2 = object->GetFloat();
		
	}else{
		DSTHROW( dueInvalidAction );
	}
	
	const int * const intValue1 = ( int* )&float1;
	const int * const intValue2 = ( int* )&float2;
	rt->PushInt( ( *intValue1 >> 10 ) - ( *intValue2 >> 10 ) ); // see hashCode
}



// Class dsClassObjectFloat
/////////////////////////////

// Constructor, Destructor
////////////////////////////

dsClassObjectFloat::dsClassObjectFloat() : dsClass( "Float", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sObjFloatNatData ) );
}

dsClassObjectFloat::~dsClassObjectFloat(){
}



// Management
///////////////

void dsClassObjectFloat::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsObjFloat = this;
	init.clsInteger = engine->GetClassInt();
	init.clsFloat = engine->GetClassFloat();
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
