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
#include "dsClassObjectByte.h"

#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"


// Native data structure
//////////////////////////

struct sObjByteNatData{
	byte value;
};



// Native Dunctions
/////////////////////

// public func new( byte value )
dsClassObjectByte::nfNew::nfNew( const sInitData &init ) :
dsFunction( init.clsObjByte, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsByte ); // value
}
void dsClassObjectByte::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sObjByteNatData &nd = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(myself));
	nd.value = rt->GetValue( 0 )->GetByte();
}



// public func byte value()
dsClassObjectByte::nfValue::nfValue( const sInitData &init ) :
dsFunction( init.clsObjByte, "value", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsByte ){
}
void dsClassObjectByte::nfValue::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjByteNatData &nd = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(myself));
	rt->PushByte( nd.value );
}



// public func String toString()
dsClassObjectByte::nfToString::nfToString( const sInitData &init ) :
dsFunction( init.clsObjByte, "toString", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassObjectByte::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjByteNatData &nd = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(myself));
	char buffer[ 2 ];
	
	snprintf( buffer, sizeof( buffer ), "%c", nd.value );
	rt->PushString( buffer );
}

// public func int hashCode()
dsClassObjectByte::nfHashCode::nfHashCode( const sInitData &init ) :
dsFunction( init.clsObjByte, "hashCode", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassObjectByte::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjByteNatData &nd = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(myself));
	rt->PushInt( nd.value );
}

// public func bool equals( Object object )
dsClassObjectByte::nfEquals::nfEquals( const sInitData &init ) :
dsFunction( init.clsObjByte, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectByte::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjByteNatData &nd = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(myself));
	dsClass * const clsObjByte = ( dsClassObjectByte* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( p_IsObjOfType( object, clsObjByte ) ){
		const sObjByteNatData &other = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(object));
		rt->PushBool( nd.value == other.value );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BOOL ){
		if( object->GetBool() ){
			rt->PushBool( nd.value == 1 );
			
		}else{
			rt->PushBool( nd.value == 0 );
		}
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BYTE ){
		rt->PushBool( nd.value == object->GetByte() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_INT ){
		rt->PushBool( nd.value == ( byte )object->GetInt() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_FLOAT ){
		rt->PushBool( nd.value == ( byte )object->GetFloat() );
		
	}else{
		rt->PushBool( false );
	}
}

// public func int compare( Object object )
dsClassObjectByte::nfCompare::nfCompare( const sInitData &init ) :
dsFunction( init.clsObjByte, "compare", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassObjectByte::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sObjByteNatData &nd = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(myself));
	dsClass * const clsObjByte = ( dsClassObjectByte* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( p_IsObjOfType( object, clsObjByte ) ){
		const sObjByteNatData &other = dsNativeDataGet<sObjByteNatData>(p_GetNativeData(object));
		rt->PushInt( ( int )nd.value - ( int )other.value );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BOOL ){
		if( object->GetBool() ){
			rt->PushInt( ( int )nd.value - 1 );
			
		}else{
			rt->PushInt( ( int )nd.value - 0 );
		}
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_BYTE ){
		rt->PushInt( ( int )nd.value - ( int )object->GetByte() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_INT ){
		rt->PushInt( ( int )nd.value - object->GetInt() );
		
	}else if( object->GetType()->GetPrimitiveType() == DSPT_FLOAT ){
		rt->PushInt( ( int )nd.value - ( int )object->GetFloat() );
		
	}else{
		DSTHROW( dueInvalidAction );
	}
}



// Class dsClassObjectByte
////////////////////////////

// Constructor, Destructor
////////////////////////////

dsClassObjectByte::dsClassObjectByte() : dsClass( "Byte", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize(dsNativeDataSize<sObjByteNatData>());
}

dsClassObjectByte::~dsClassObjectByte(){
}



// Management
///////////////

void dsClassObjectByte::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsObjByte = this;
	init.clsInteger = engine->GetClassInt();
	init.clsByte = engine->GetClassByte();
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
