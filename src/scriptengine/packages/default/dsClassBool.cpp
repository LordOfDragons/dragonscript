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
#include "../../dragonscript_config.h"
#include "dsClassBool.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// Native functions
/////////////////////

// public func String toString()
dsClassBool::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsBool,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassBool::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	if( myself->GetBool() ){
		rt->PushString( "True" );
		
	}else{
		rt->PushString( "False" );
	}
}

// public func int hashCode()
dsClassBool::nfHashCode::nfHashCode( const sInitData &init ) : dsFunction( init.clsBool,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassBool::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	if( myself->GetBool() ){
		rt->PushInt( 1 );
		
	}else{
		rt->PushInt( 0 );
	}
}

// public func bool equals( Object obj )
dsClassBool::nfEquals::nfEquals( const sInitData &init ) : dsFunction( init.clsBool,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassBool::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	
	rt->PushBool( val.GetType()->GetPrimitiveType() == DSPT_BOOL && myself->GetBool() == val.GetBool() );
}

// public func int compare( Object obj )
dsClassBool::nfCompare::nfCompare( const sInitData &init ) : dsFunction( init.clsBool,
"compare", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassBool::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	
	if( val.GetType()->GetPrimitiveType() != DSPT_BOOL ){
		DSTHROW( dueInvalidAction );
	}
	
	if( ! myself->GetBool() && val.GetBool() ){
		rt->PushInt( -1 );
		
	}else if( myself->GetBool() && ! val.GetBool() ){
		rt->PushInt( 1 );
		
	}else{
		rt->PushInt( 0 );
	}
}



// Class dsClassBool
///////////////////////

// Constructor, destructor
////////////////////////////

dsClassBool::dsClassBool() : dsClass( "bool", DSCT_CLASS, DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE, DSPT_BOOL) {
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( 0 );
}

dsClassBool::~dsClassBool(){
}



// Management
///////////////

void dsClassBool::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsBool = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsObj = engine->GetClassObject();
	init.clsStr = engine->GetClassString();
	
	AddFunction( new nfToString( init ) );
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfCompare( init ) );
}
