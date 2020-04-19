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
#include "dsClassByte.h"
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
dsClassByte::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsByte,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassByte::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	char buffer[ 2 ];
	sprintf( buffer, "%c", myself->GetByte() );
	rt->PushString( buffer );
}

// public func int hashCode()
dsClassByte::nfHashCode::nfHashCode( const sInitData &init ) : dsFunction( init.clsByte,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassByte::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( ( int )myself->GetByte() );
}

// public func bool equals( Object obj )
dsClassByte::nfEquals::nfEquals( const sInitData &init ) : dsFunction( init.clsByte,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassByte::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	
	switch( val.GetType()->GetPrimitiveType() ){
	case DSPT_BYTE:
		rt->PushBool( myself->GetByte() == val.GetByte() );
		break;
		
	case DSPT_INT:
		rt->PushBool( myself->GetByte() == ( byte )val.GetInt() );
		break;
		
	case DSPT_FLOAT:
		rt->PushBool( myself->GetByte() == ( byte )val.GetFloat() );
		break;
		
	default:
		rt->PushBool( false );
	}
}

// public func int compare( Object obj )
dsClassByte::nfCompare::nfCompare( const sInitData &init ) : dsFunction( init.clsByte,
"compare", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassByte::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	int value1 = ( int )myself->GetByte();
	int value2;
	
	switch( val.GetType()->GetPrimitiveType() ){
	case DSPT_BOOL:
		if( val.GetBool() ){
			value2 = 1;
			
		}else{
			value2 = 0;
		}
		break;
		
	case DSPT_BYTE:
		value2 = ( int )val.GetByte();
		break;
		
	case DSPT_INT:
		value2 = ( int )( ( byte )val.GetInt() );
		break;
		
	case DSPT_FLOAT:
		value2 = ( int )( ( byte )val.GetFloat() );
		break;
		
	default:
		DSTHROW( dueInvalidAction );
	}
	
	rt->PushInt( value1 - value2 );
}



// Class dsClassByte
///////////////////////

// Constructor, Destructor
////////////////////////////

dsClassByte::dsClassByte() : dsClass( "byte", DSCT_CLASS, DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE, DSPT_BYTE) {
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( 0 );
}

dsClassByte::~dsClassByte(){
}



// Management
///////////////

void dsClassByte::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsByte = this;
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
