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

#include <stdio.h>
#include <string.h>
#include "../../../config.h"
#include "dsClassFloat.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// Native Functions
/////////////////////

// public func String toString()
dsClassFloat::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsFlt,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassFloat::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	char buffer[ 20 ];
	sprintf( buffer, "%g", myself->GetFloat() );
	rt->PushString( buffer );
}

// public func int hashCode()
dsClassFloat::nfHashCode::nfHashCode( const sInitData &init ) : dsFunction( init.clsFlt,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassFloat::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const float floatValue = myself->GetFloat();
	const int * const intValue = ( int* )&floatValue;
	rt->PushInt( *intValue >> 10 ); // cut off 10 bits for a bit of uncertainty to be allowed
}

// public func bool equals( Object obj )
dsClassFloat::nfEquals::nfEquals( const sInitData &init ) : dsFunction( init.clsFlt,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassFloat::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	
	switch( val.GetType()->GetPrimitiveType() ){
	case DSPT_BYTE:
		rt->PushBool( myself->GetFloat() == ( float )val.GetByte() );
		break;
		
	case DSPT_INT:
		rt->PushBool( myself->GetFloat() == ( float )val.GetInt() );
		break;
		
	case DSPT_FLOAT:
		rt->PushBool( myself->GetFloat() == val.GetFloat() );
		break;
		
	default:
		rt->PushBool( false );
	}
}

// public func int compare( Object obj )
dsClassFloat::nfCompare::nfCompare( const sInitData &init ) : dsFunction( init.clsFlt,
"compare", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassFloat::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	float value1 = myself->GetFloat();
	float value2;
	
	switch( val.GetType()->GetPrimitiveType() ){
	case DSPT_BOOL:
		if( val.GetBool() ){
			value2 = 1.0f;
			
		}else{
			value2 = 0.0f;
		}
		break;
		
	case DSPT_BYTE:
		value2 = ( float )val.GetByte();
		break;
		
	case DSPT_INT:
		value2 = ( float )val.GetInt();
		break;
		
	case DSPT_FLOAT:
		value2 = val.GetFloat();
		break;
		
	default:
		DSTHROW( dueInvalidAction );
	}
	
	const int * const intValue1 = ( int* )&value1;
	const int * const intValue2 = ( int* )&value2;
	rt->PushInt( ( *intValue1 >> 10 ) - ( *intValue2 >> 10 ) ); // see hashCode
}



// Class dsClassFloat
///////////////////////

// Cconstructor, Destructor
/////////////////////////////

dsClassFloat::dsClassFloat() : dsClass( "float", DSCT_CLASS, DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE, DSPT_FLOAT ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( 0 );
}

dsClassFloat::~dsClassFloat(){
}



// Management
///////////////

void dsClassFloat::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsFlt = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsBool = engine->GetClassBool();
	init.clsInt = engine->GetClassInt();
	init.clsObj = engine->GetClassObject();
	init.clsStr = engine->GetClassString();
	
	AddFunction( new nfToString( init ) );
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfCompare( init ) );
}
