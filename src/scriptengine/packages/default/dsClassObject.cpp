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
#include <stdint.h>
#include <string.h>
#include "../../dragonscript_config.h"
#include "dsClassObject.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsFuncList.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// Native Functions
/////////////////////

// public func new()
dsClassObject::nfNew::nfNew( const sInitData &init ) : dsFunction( init.clsObj,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassObject::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
}



// public func String className()
dsClassObject::nfClassName::nfClassName( const sInitData &init ) : dsFunction( init.clsObj,
"className", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassObject::nfClassName::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClass * const type = myself->GetRealType();
	
	// determine the length of the needed string first
	dsClass *curClass = type;
	int fullNameLen = 0;
	
	while( curClass ){
		fullNameLen += ( int )strlen( curClass->GetName() ) + 1;
		curClass = curClass->GetParent();
	}
	fullNameLen--;
	
	// create and push string containing full name
	char * const fullName = new char[ fullNameLen + 1 ];
	if( ! fullName ){
		DSTHROW( dueOutOfMemory );
	}
	
	try{
		fullName[ fullNameLen ] = '\0';
		curClass = type;
		
		while( curClass ){
			const int nameLen = ( int )strlen( curClass->GetName() );
			fullNameLen -= nameLen;
			// new -Wstringop-truncation check is fail. fix is to use memcpy instead of
			// strncpy. seriously... how brain dead is this?!
			memcpy( fullName + fullNameLen, curClass->GetName(), nameLen );
			curClass = curClass->GetParent();
			if( curClass ){
				fullNameLen--;
				fullName[ fullNameLen ] = '.';
			}
		}
		
		rt->PushString( fullName );
		delete [] fullName;
		
	}catch( ... ){
		if( fullName ){
			delete [] fullName;
		}
		throw;
	}
}



// public func String toString()
dsClassObject::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsObj,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassObject::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	// determine the name of the class
	dsClass *type = myself->GetType();
	
	if( type->GetPrimitiveType() == DSPT_OBJECT ){
		dsRealObject * const obj = myself->GetRealObject();
		if( obj ){
			type = obj->GetType();
		}
	}
	
	const char * const className = type->GetName();
	
	// build string from the classname
	char *buffer = NULL;
	
	try{
		const int size = 10 + ( int )strlen( className );
		buffer = new char[ size ];
		if( ! buffer ){
			DSTHROW( dueOutOfMemory );
		}
		
		snprintf( buffer, size, "Class=%s", className );
		rt->PushString( buffer );
		delete [] buffer;
		
	}catch( ... ){
		if( buffer ){
			delete [] buffer;
		}
		throw;
	}
}

// public func int hashCode()
dsClassObject::nfHashCode::nfHashCode( const sInitData &init ) : dsFunction( init.clsObj,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassObject::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClass * const type = myself->GetType();
	
	if( type->GetPrimitiveType() != DSPT_OBJECT ){
		DSTHROW_INFO_FMT( dueInvalidParam, "object is primitive type: %s", ErrorObjectType( myself ).Pointer() );
	}
	
	rt->PushInt( ( int )( intptr_t )myself->GetRealObject() );
}

// public func bool equals( Object obj )
dsClassObject::nfEquals::nfEquals( const sInitData &init ) : dsFunction( init.clsObj,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassObject::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsValue &val = *rt->GetValue( 0 );
	
	if( val.GetType()->GetPrimitiveType() == DSPT_OBJECT ){
		rt->PushBool( myself->GetRealObject() == val.GetRealObject() );
		
	}else{
		rt->PushBool( false );
	}
}

// public func int compare( Object obj )
dsClassObject::nfCompare::nfCompare( const sInitData &init ) : dsFunction( init.clsObj,
"compare", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassObject::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClass *clsInt = rt->GetEngine()->GetClassInt();
	dsValue * const val = rt->GetValue( 0 );
	
	if( val->GetType()->GetPrimitiveType() != DSPT_OBJECT ){
		DSTHROW_INFO_FMT( dueInvalidParam, "object is primitive type: %s", ErrorObjectType( val ).Pointer() );
	}
	
	const int funcIndexHashCode = ( ( dsClassObject* )GetOwnerClass() )->GetFuncIndexHashCode();
	rt->RunFunctionFast( myself, funcIndexHashCode ); // int hashCode()
	if( rt->GetReturnValue()->GetType() != clsInt ){
		DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsInt ) );
	}
	const int hash1 = rt->GetReturnInt();
	
	rt->RunFunctionFast( val, funcIndexHashCode ); // int hashCode()
	if( rt->GetReturnValue()->GetType() != clsInt ){
		DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsInt ) );
	}
	const int hash2 = rt->GetReturnInt();
	
	rt->PushInt( hash1 - hash2 );
}


// Class dsClassObject
////////////////////////

// Constructor, Destructor
////////////////////////////

dsClassObject::dsClassObject() : dsClass( "Object", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE ){
	p_SetNativeDataSize( 0 );
}

dsClassObject::~dsClassObject(){
}



// Management
///////////////

void dsClassObject::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsObj = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsBool = engine->GetClassBool();
	init.clsInt = engine->GetClassInt();
	init.clsStr = engine->GetClassString();
	
	AddFunction( new nfNew( init ) ); // function 0
	
	AddFunction( new nfClassName( init ) ); // function 1
	AddFunction( new nfToString( init ) ); // function 2
	AddFunction( new nfHashCode( init ) ); // function 3
	AddFunction( new nfEquals( init ) ); // function 4
	AddFunction( new nfCompare( init ) ); // function 5
	
	// store function indices for fast function calling in native code
	const dsFuncList &funcList = *GetFuncList();
	pFuncIndexToString = funcList.GetIndexOf( GetFunction( 2 ) );
	pFuncIndexHashCode = funcList.GetIndexOf( GetFunction( 3 ) );
	pFuncIndexEquals = funcList.GetIndexOf( GetFunction( 4 ) );
	pFuncIndexCompare = funcList.GetIndexOf( GetFunction( 5 ) );
}
