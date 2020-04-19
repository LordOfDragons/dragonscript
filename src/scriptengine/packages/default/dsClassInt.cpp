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
#include "dsClassInt.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// Native Dunctions
/////////////////////

// public func String toString()
dsClassInt::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsInt,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassInt::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	char buffer[ 20 ];
	sprintf( buffer, "%i", myself->GetInt() );
	rt->PushString( buffer );
}

// public func int hashCode()
dsClassInt::nfHashCode::nfHashCode( const sInitData &init ) : dsFunction( init.clsInt,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassInt::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( myself->GetInt() );
}

// public func bool equals( Object obj )
dsClassInt::nfEquals::nfEquals( const sInitData &init ) : dsFunction( init.clsInt,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassInt::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	
	switch( val.GetType()->GetPrimitiveType() ){
	case DSPT_BYTE:
		rt->PushBool( myself->GetInt() == ( int )val.GetByte() );
		break;
		
	case DSPT_INT:
		rt->PushBool( myself->GetInt() == val.GetInt() );
		break;
		
	case DSPT_FLOAT:
		rt->PushBool( myself->GetInt() == ( int )val.GetFloat() );
		break;
		
	default:
		rt->PushBool( false );
	}
}

// public func int compare( Object obj )
dsClassInt::nfCompare::nfCompare( const sInitData &init ) : dsFunction( init.clsInt,
"compare", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassInt::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	int value1 = myself->GetInt();
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
		value2 = val.GetInt();
		break;
		
	case DSPT_FLOAT:
		value2 = ( int )val.GetFloat();
		break;
		
	default:
		DSTHROW( dueInvalidAction );
	}
	
	rt->PushInt( value1 - value2 );
}



// Class dsClassInt
/////////////////////

// Constructor, Destructor
////////////////////////////

dsClassInt::dsClassInt() : dsClass( "int", DSCT_CLASS, DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE, DSPT_INT ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( 0 );
}

dsClassInt::~dsClassInt(){
}



// Management
///////////////

void dsClassInt::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsInt = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsBool = engine->GetClassBool();
	init.clsObj = engine->GetClassObject();
	init.clsStr = engine->GetClassString();
	
	AddFunction( new nfToString( init ) );
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfCompare( init ) );
}
