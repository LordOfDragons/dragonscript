/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2020, Roland Plüss (roland@rptd.ch)
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
#include "dsClassEnumeration.h"
#include "dsClassSet.h"
#include "dsClassBlock.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"
#include "../../objects/dsVariable.h"
#include "../../utils/dsuString.h"


// Native data structure
//////////////////////////

struct sEnumNatData{
	dsuString *name;
	dsuString *toString;
	int order;
};



// Native Dunctions
/////////////////////

// private constructor new( String name, int order )
dsClassEnumeration::nfNew::nfNew( const sInitData &init ) :
dsFunction( init.clsEnumeration, "new", DSFT_CONSTRUCTOR,
DSTM_PRIVATE | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsString ); // name
	p_AddParameter( init.clsInteger ); // order
}
void dsClassEnumeration::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const name = rt->GetValue( 0 )->GetString();
	const int order = rt->GetValue( 1 )->GetInt();
	
	( ( dsClassEnumeration* )rt->GetEngine()->GetClassEnumeration() )->SubClassInitConstant(
		*GetOwnerClass(), *myself->GetRealObject(), name, order );
}

// destructor Destructor
dsClassEnumeration::nfDestructor::nfDestructor( const sInitData &init ) :
dsFunction( init.clsEnumeration, "destructor", DSFT_DESTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassEnumeration::nfDestructor::RunFunction( dsRunTime*, dsValue *myself ){
	sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( myself ) );
	
	if( nd.name ){
		delete nd.name;
		nd.name = NULL;
	}
	if( nd.toString ){
		delete nd.toString;
		nd.toString = NULL;
	}
}



// public func String name()
dsClassEnumeration::nfName::nfName( const sInitData &init ) :
dsFunction( init.clsEnumeration, "name", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassEnumeration::nfName::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( myself ) );
	rt->PushString( nd.name->Pointer() );
}

// public func int order()
dsClassEnumeration::nfOrder::nfOrder( const sInitData &init ) :
dsFunction( init.clsEnumeration, "order", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassEnumeration::nfOrder::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( myself ) );
	rt->PushInt( nd.order );
}



// public func EnumSet set()
dsClassEnumeration::nfSet::nfSet( const sInitData &init ) :
dsFunction( init.clsEnumeration, "set", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
}
void dsClassEnumeration::nfSet::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassSet &clsSet = *( ( dsClassSet* )rt->GetEngine()->GetClassSet() );
	dsValue * const set = clsSet.CreateSet( rt );
	
	try{
		clsSet.AddObject( rt, set->GetRealObject(), myself );
		rt->PushValue( set );
		rt->FreeValue( set );
		
	}catch( ... ){
		rt->FreeValue( set );
		throw;
	}
}

// public func EnumSet setWithout()
dsClassEnumeration::nfSetWithout::nfSetWithout( const sInitData &init ) :
dsFunction( init.clsEnumeration, "setWithout", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
}
void dsClassEnumeration::nfSetWithout::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassSet &clsSet = *( ( dsClassSet* )rt->GetEngine()->GetClassSet() );
	dsRealObject * const myselfObject = myself->GetRealObject();
	dsClass &clsObject = *myselfObject->GetType();
	dsValue * const set = clsSet.CreateSet( rt );
	const int count = clsObject.GetVariableCount();
	int i;
	
	try{
		for( i=0; i<count; i++ ){
			dsVariable &variable = *clsObject.GetVariable( i );
			if( variable.GetStaticValue()->GetRealObject() != myselfObject ){
				clsSet.AddObject( rt, set->GetRealObject(), variable.GetStaticValue() );
			}
		}
		rt->PushValue( set );
		rt->FreeValue( set );
		
	}catch( ... ){
		rt->FreeValue( set );
		throw;
	}
}



// public func String toString()
dsClassEnumeration::nfToString::nfToString( const sInitData &init ) :
dsFunction( init.clsEnumeration, "toString", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassEnumeration::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( myself ) );
	rt->PushString( nd.toString->Pointer() );
}

// public func int hashCode()
dsClassEnumeration::nfHashCode::nfHashCode( const sInitData &init ) :
dsFunction( init.clsEnumeration, "hashCode", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassEnumeration::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( myself ) );
	rt->PushInt( nd.order );
}

// public func int compare( Object object )
dsClassEnumeration::nfCompare::nfCompare( const sInitData &init ) :
dsFunction( init.clsEnumeration, "compare", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassEnumeration::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( myself ) );
	dsValue * const object = rt->GetValue( 0 );
	
	if( p_IsObjOfType( object, myself->GetRealObject()->GetType() ) ){
		const sEnumNatData &other = *( ( sEnumNatData* )p_GetNativeData( object ) );
		rt->PushInt( nd.order - other.order );
		
	}else{
		DSTHROW( dueInvalidAction );
	}
}



// static public func Set all()
dsClassEnumeration::nfAll::nfAll( const sInitData &init ) :
dsFunction( init.clsEnumeration, "all", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsSet ){
}
void dsClassEnumeration::nfAll::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet &clsSet = *( ( dsClassSet* )rt->GetEngine()->GetClassSet() );
	dsClass &clsOwner = *GetOwnerClass();
	const int count = clsOwner.GetVariableCount();
	dsValue * const set = clsSet.CreateSet( rt );
	int i;
	
	try{
		for( i=0; i<count; i++ ){
			clsSet.AddObject( rt, set->GetRealObject(), clsOwner.GetVariable( i )->GetStaticValue() );
		}
		
		rt->PushValue( set );
		rt->FreeValue( set );
		
	}catch( ... ){
		rt->FreeValue( set );
		throw;
	}
}

// static public func Set none()
dsClassEnumeration::nfNone::nfNone( const sInitData &init ) :
dsFunction( init.clsEnumeration, "none", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsSet ){
}
void dsClassEnumeration::nfNone::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet &clsSet = *( ( dsClassSet* )rt->GetEngine()->GetClassSet() );
	dsValue * const set = clsSet.CreateSet( rt );
	rt->PushValue( set );
	rt->FreeValue( set );
}

// static public func void forEach( Block block )
dsClassEnumeration::nfForEach::nfForEach( const sInitData &init ) :
dsFunction( init.clsEnumeration, "forEach", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // block
}
void dsClassEnumeration::nfForEach::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW( dueNullPointer );
	}
	
	dsClass &clsOwner = *GetOwnerClass();
	const int count = clsOwner.GetVariableCount();
	int i;
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	for( i=0; i<count; i++ ){
		rt->PushValue( clsOwner.GetVariable( i )->GetStaticValue() );
		rt->RunFunctionFast( block, funcIndexRun );
	}
}

// static public func Enumeration named( String name )
dsClassEnumeration::nfNamed::nfNamed( const sInitData &init ) :
dsFunction( init.clsEnumeration, "named", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsEnumeration ){
	p_AddParameter( init.clsString ); // name
}
void dsClassEnumeration::nfNamed::RunFunction( dsRunTime *rt, dsValue* ){
	dsClass &clsOwner = *GetOwnerClass();
	const char * const name = rt->GetValue( 0 )->GetString();
	const int count = clsOwner.GetVariableCount();
	int i;
	
	for( i=0; i<count; i++ ){
		dsVariable &variable = *clsOwner.GetVariable( i );
		if( strcmp( variable.GetName(), name ) == 0 ){
			rt->PushValue( variable.GetStaticValue() );
			return;
		}
	}
	
	rt->PushObject( NULL, rt->GetEngine()->GetClassEnumeration() );
}

// static public func Enumeration withOrder( int order )
dsClassEnumeration::nfWithOrder::nfWithOrder( const sInitData &init ) :
dsFunction( init.clsEnumeration, "withOrder", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsEnumeration ){
	p_AddParameter( init.clsInteger ); // order
}
void dsClassEnumeration::nfWithOrder::RunFunction( dsRunTime *rt, dsValue* ){
	const dsClassEnumeration &clsEnumeration = *( ( dsClassEnumeration* )rt->GetEngine()->GetClassEnumeration() );
	dsClass &clsOwner = *GetOwnerClass();
	const int order = rt->GetValue( 0 )->GetInt();
	const int count = clsOwner.GetVariableCount();
	int i;
	
	for( i=0; i<count; i++ ){
		dsVariable &variable = *clsOwner.GetVariable( i );
		if( clsEnumeration.GetConstantOrder( *variable.GetStaticValue()->GetRealObject() ) == order ){
			rt->PushValue( variable.GetStaticValue() );
			return;
		}
	}
	
	rt->PushObject( NULL, &clsOwner );
}

// public func bool <( Enumeration enumeration )
dsClassEnumeration::nfOpLess::nfOpLess( const sInitData &init ) :
dsFunction( init.clsEnumeration, "<", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsEnumeration ); // enumeration
}
void dsClassEnumeration::nfOpLess::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsRealObject * const object = rt->GetValue( 0 )->GetRealObject();
	if( ! object ){
		DSTHROW( dueNullPointer );
	}
	
	const dsClassEnumeration &clsEnumeration = *( ( dsClassEnumeration* )rt->GetEngine()->GetClassEnumeration() );
	rt->PushBool( clsEnumeration.GetConstantOrder( *myself->GetRealObject() )
		< clsEnumeration.GetConstantOrder( *object ) );
}

// public func bool <=( Enumeration enumeration )
dsClassEnumeration::nfOpLessEqual::nfOpLessEqual( const sInitData &init ) :
dsFunction( init.clsEnumeration, "<=", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsEnumeration ); // enumeration
}
void dsClassEnumeration::nfOpLessEqual::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsRealObject * const object = rt->GetValue( 0 )->GetRealObject();
	if( ! object ){
		DSTHROW( dueNullPointer );
	}
	
	const dsClassEnumeration &clsEnumeration = *( ( dsClassEnumeration* )rt->GetEngine()->GetClassEnumeration() );
	rt->PushBool( clsEnumeration.GetConstantOrder( *myself->GetRealObject() )
		<= clsEnumeration.GetConstantOrder( *object ) );
}

// public func bool >( Enumeration enumeration )
dsClassEnumeration::nfOpGreater::nfOpGreater( const sInitData &init ) :
dsFunction( init.clsEnumeration, ">", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsEnumeration ); // enumeration
}
void dsClassEnumeration::nfOpGreater::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsRealObject * const object = rt->GetValue( 0 )->GetRealObject();
	if( ! object ){
		DSTHROW( dueNullPointer );
	}
	
	const dsClassEnumeration &clsEnumeration = *( ( dsClassEnumeration* )rt->GetEngine()->GetClassEnumeration() );
	rt->PushBool( clsEnumeration.GetConstantOrder( *myself->GetRealObject() )
		> clsEnumeration.GetConstantOrder( *object ) );
}

// public func bool >=( Enumeration enumeration )
dsClassEnumeration::nfOpGreaterEqual::nfOpGreaterEqual( const sInitData &init ) :
dsFunction( init.clsEnumeration, ">=", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsEnumeration ); // enumeration
}
void dsClassEnumeration::nfOpGreaterEqual::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsRealObject * const object = rt->GetValue( 0 )->GetRealObject();
	if( ! object ){
		DSTHROW( dueNullPointer );
	}
	
	const dsClassEnumeration &clsEnumeration = *( ( dsClassEnumeration* )rt->GetEngine()->GetClassEnumeration() );
	rt->PushBool( clsEnumeration.GetConstantOrder( *myself->GetRealObject() )
		>= clsEnumeration.GetConstantOrder( *object ) );
}



// Class dsClassEnumeration
/////////////////////////////

// Constructor, Destructor
////////////////////////////

dsClassEnumeration::dsClassEnumeration() : dsClass( "Enumeration", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sEnumNatData ) );
}

dsClassEnumeration::~dsClassEnumeration(){
}



// Management
///////////////

void dsClassEnumeration::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsEnumeration = this;
	init.clsInteger = engine->GetClassInt();
	init.clsBoolean = engine->GetClassBool();
	init.clsString = engine->GetClassString();
	init.clsObject = engine->GetClassObject();
	init.clsVoid = engine->GetClassVoid();
	init.clsSet = engine->GetClassSet();
	init.clsBlock = engine->GetClassBlock();
	
	AddFunction( new nfDestructor( init ) );
	
	AddFunction( new nfName( init ) );
	AddFunction( new nfOrder( init ) );
	
	AddFunction( new nfSet( init ) );
	AddFunction( new nfSetWithout( init ) );
	
	AddFunction( new nfToString( init ) );
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfCompare( init ) );
}

const dsuString &dsClassEnumeration::GetConstantName( dsRealObject &object ) const{
	const sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( object.GetBuffer() ) );
	if( ! nd.name ){
		DSTHROW( dueInvalidParam );
	}
	return *nd.name;
}

int dsClassEnumeration::GetConstantOrder( dsRealObject &object ) const{
	const sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( object.GetBuffer() ) );
	return nd.order;
}

void dsClassEnumeration::CreateConstant( dsRunTime *rt, dsClass *enumClass, dsValue *value,
const char *name, int order ){
	if( ! rt || ! enumClass || ! value || ! name || order < 0 ){
		DSTHROW( dueInvalidParam );
	}
	
	rt->CreateObjectNaked( value, enumClass );
	SubClassInitConstant( *enumClass, *value->GetRealObject(), name, order );
}

void dsClassEnumeration::CreateConstants( dsRunTime *rt, dsClass *enumClass ){
	if( ! rt || ! enumClass ){
		DSTHROW( dueInvalidParam );
	}
	
	const int count = enumClass->GetVariableCount();
	int i;
	
	for( i=0; i<count; i++ ){
		dsVariable &variable = *enumClass->GetVariable( i );
		if( ( variable.GetTypeModifiers() & DSTM_STATIC ) != DSTM_STATIC 
		|| ( variable.GetTypeModifiers() & DSTM_FIXED ) != DSTM_FIXED
		|| variable.GetStaticValue()->GetRealObject() ){
			DSTHROW( dueInvalidParam );
		}
		
		rt->CreateObjectNaked( variable.GetStaticValue(), enumClass );
		SubClassInitConstant( *enumClass, *variable.GetStaticValue()->GetRealObject(), variable.GetName(), i );
	}
}

void dsClassEnumeration::GenerateMethods( const dsEngine &engine, dsClass &subClass ){
	if( subClass.GetBaseClass() != engine.GetClassEnumeration() ){
		DSTHROW( dueInvalidParam );
	}
	
	sInitData init;
	
	init.clsEnumeration = &subClass;
	init.clsInteger = engine.GetClassInt();
	init.clsBoolean = engine.GetClassBool();
	init.clsString = engine.GetClassString();
	init.clsObject = engine.GetClassObject();
	init.clsVoid = engine.GetClassVoid();
	init.clsSet = engine.GetClassSet();
	init.clsBlock = engine.GetClassBlock();
	
	subClass.AddFunction( new nfNew( init ) );
	
	subClass.AddFunction( new nfAll( init ) );
	subClass.AddFunction( new nfNone( init ) );
	subClass.AddFunction( new nfForEach( init ) );
	subClass.AddFunction( new nfNamed( init ) );
	subClass.AddFunction( new nfWithOrder( init ) );
	subClass.AddFunction( new nfOpLess( init ) );
	subClass.AddFunction( new nfOpLessEqual( init ) );
	subClass.AddFunction( new nfOpGreater( init ) );
	subClass.AddFunction( new nfOpGreaterEqual( init ) );
}

void dsClassEnumeration::SubClassInitConstant( dsClass &subClass, dsRealObject &myself,
const char *name, int order ) const{
	sEnumNatData &nd = *( ( sEnumNatData* )p_GetNativeData( myself.GetBuffer() ) );
	nd.name = NULL;
	nd.toString = NULL;
	nd.order = 0;
	
	const int lenClassName = strlen( subClass.GetName() );
	const int lenName = strlen( name );
	
	nd.name = new dsuString( name );
	nd.order = order;
	
	nd.toString = new dsuString;
	nd.toString->Resize( lenClassName + lenName + 1 );
	snprintf( nd.toString->Pointer(), lenClassName + lenName + 2, "%s.%s", subClass.GetName(), name );
}
