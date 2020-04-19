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
#include "../../../config.h"
#include "dsClassNull.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// Native Functions
/////////////////////

// public func bool equals( Object obj )
dsClassNull::nfEquals::nfEquals( const sInitData &init ) : dsFunction( init.clsNull,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassNull::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	
	switch( val.GetType()->GetPrimitiveType() ){
	case DSPT_NULL:
		rt->PushBool( true );
		break;
		
	case DSPT_OBJECT:
		rt->PushBool( val.GetRealObject() == NULL );
		break;
		
	default:
		rt->PushBool( false );
	}
}



// Class dsClassNull
///////////////////////

// Constructor, Destructor
////////////////////////////

dsClassNull::dsClassNull() : dsClass( "null", DSCT_CLASS, DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE, DSPT_NULL ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( 0 );
}

dsClassNull::~dsClassNull(){
}



// Management
///////////////

void dsClassNull::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsNull = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsBool = engine->GetClassBool();
	init.clsObj = engine->GetClassObject();
	
	AddFunction( new nfEquals( init ) );
}
