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
#include "dsClassSet.h"
#include "dsClassObject.h"
#include "dsClassBlock.h"
#include "dsClassArray.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"


static const char * const errorModifyWhileLocked = "modify while locked";

// native data structure
class cSetNatDatLockModifyGuard{
	int &pLockModify;
	
public:
	cSetNatDatLockModifyGuard( int &lockModify ) : pLockModify( lockModify ){
		pLockModify++;
	}
	
	~cSetNatDatLockModifyGuard(){
		pLockModify--;
	}
};

struct sSetNatData{
	dsValue **values;
	int count;
	int size;
	int lockModify;
	
	// helper functions
	void SetSize(int newSize, dsRunTime *rt, dsClass *clsObject){
		int i, oldSize=size;
		dsValue **vNewSet;
		// only growing allowed
		if(newSize > oldSize){
			if(!(vNewSet = new dsValue*[newSize])) DSTHROW(dueOutOfMemory);
			for(i=0; i<oldSize; i++) vNewSet[i] = values[i];
			if(values) delete [] values;
			values = vNewSet;
			for(i=oldSize; i<newSize; i++){
				values[i] = rt->CreateValue(clsObject);
				if(!values[i]) DSTHROW(dueOutOfMemory);
				size++;
			}
		}
	}
	
	int IndexOf( dsRunTime &rt, dsValue &value ){
		int i;
		
		if( value.GetType()->GetPrimitiveType() == DSPT_OBJECT && ! value.GetRealObject() ){
			// test value is null, special compare to avoid null pointer exception
			for( i=0; i<count; i++ ){
				if( values[ i ]->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! values[ i ]->GetRealObject() ){
					return i;
				}
			}
			
		}else{
			const int funcIndexEquals = ( ( dsClassObject* )rt.GetEngine()->GetClassObject() )->GetFuncIndexEquals();
			for( i=0; i<count; i++ ){
				rt.PushValue( values[ i ] );
				rt.RunFunctionFast( &value, funcIndexEquals ); // bool equals( key )
				if( rt.GetReturnBool() ){
					return i;
				}
			}
		}
		
		return -1;
	}
	
	bool Has( dsRunTime &rt, dsValue &value ){
		return IndexOf( rt, value ) != -1;
	}
};


// Native functions
/////////////////////

// constructor new()
dsClassSet::nfNew::nfNew(const sInitData &init) : dsFunction(init.clsSet,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassSet::nfNew::RunFunction( dsRunTime*, dsValue *myself ){
	sSetNatData *nd = (sSetNatData*)p_GetNativeData(myself);
	nd->values = NULL;
	nd->count = 0;
	nd->size = 0;
	nd->lockModify = 0;
}

// constructor Copy(Set set)
dsClassSet::nfCopy::nfCopy(const sInitData &init) : dsFunction(init.clsSet,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsSet); // arr
}
void dsClassSet::nfCopy::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = (sSetNatData*)p_GetNativeData(myself);
	dsClass *clsObject = ((dsClassSet*)GetOwnerClass())->GetClassObject();
	nd->values = NULL;
	nd->count = 0;
	nd->size = 0;
	nd->lockModify = 0;
	dsValue *vObj = rt->GetValue(0);
	sSetNatData *nd2 = (sSetNatData*)p_GetNativeData(vObj);
	int i, vSize = nd2->count;
	if(vSize > 0){
		nd->values = new dsValue*[vSize];
		if(!nd->values) DSTHROW(dueOutOfMemory);
		for(i=0; i<vSize; i++){
			nd->values[i] = rt->CreateValue(clsObject);
			if(!nd->values[i]) DSTHROW(dueOutOfMemory);
			nd->size++;
			rt->CopyValue(nd2->values[i], nd->values[i]);
		}
		nd->count = nd->size;
	}
}

// destructor Destructor
dsClassSet::nfDestructor::nfDestructor(const sInitData &init) : dsFunction(init.clsSet,
"destructor", DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassSet::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	if( ! nd.values ){
		return;
	}
	
	int i;
	for( i=0; i<nd.size; i++ ){
		rt->FreeValue( nd.values[ i ] );
	}
	
	delete [] nd.values;
	
	nd.values = NULL;
	nd.count = 0;
	nd.size = 0;
	nd.lockModify = 0;
}



// public static func Set newWith( Object obj )
dsClassSet::nfNewWith1::nfNewWith1(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj
}
void dsClassSet::nfNewWith1::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass * const clsObject = clsSet->GetClassObject();
	
	dsValue *newSet = NULL;
	sSetNatData *nd = NULL;
	
	try{
		newSet = clsSet->CreateSet( rt );
		nd = ( sSetNatData* )p_GetNativeData( newSet );
		
		nd->SetSize( 1, rt, clsObject );
		rt->CopyValue( rt->GetValue( 0 ), nd->values[ 0 ] );
		nd->count = 1;
		
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2 )
dsClassSet::nfNewWith2::nfNewWith2(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
}
void dsClassSet::nfNewWith2::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass * const clsObject = clsSet->GetClassObject();
	
	dsValue *newSet = NULL;
	sSetNatData *nd = NULL;
	
	try{
		newSet = clsSet->CreateSet( rt );
		nd = ( sSetNatData* )p_GetNativeData( newSet );
		
		nd->SetSize( 2, rt, clsObject );
		rt->CopyValue( rt->GetValue( 0 ), nd->values[ 0 ] );
		nd->count = 1;
		
		if( ! nd->Has( *rt, *rt->GetValue( 1 ) ) ){
			rt->CopyValue( rt->GetValue( 1 ), nd->values[ 1 ] );
			nd->count++;
		}
		
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3 )
dsClassSet::nfNewWith3::nfNewWith3(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
}
void dsClassSet::nfNewWith3::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass * const clsObject = clsSet->GetClassObject();
	
	dsValue *newSet = NULL;
	sSetNatData *nd = NULL;
	
	try{
		newSet = clsSet->CreateSet( rt );
		nd = ( sSetNatData* )p_GetNativeData( newSet );
		
		nd->SetSize( 3, rt, clsObject );
		rt->CopyValue( rt->GetValue( 0 ), nd->values[ 0 ] );
		nd->count = 1;
		
		for( int i=1; i<=2; i++ ){
			if( ! nd->Has( *rt, *rt->GetValue( i ) ) ){
				rt->CopyValue( rt->GetValue( i ), nd->values[ nd->count ] );
				nd->count++;
			}
		}
		
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3, Object obj4 )
dsClassSet::nfNewWith4::nfNewWith4(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
	p_AddParameter( init.clsObject ); // obj4
}
void dsClassSet::nfNewWith4::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass * const clsObject = clsSet->GetClassObject();
	
	dsValue *newSet = NULL;
	sSetNatData *nd = NULL;
	
	try{
		newSet = clsSet->CreateSet( rt );
		nd = ( sSetNatData* )p_GetNativeData( newSet );
		
		nd->SetSize( 4, rt, clsObject );
		rt->CopyValue( rt->GetValue( 0 ), nd->values[ 0 ] );
		nd->count = 1;
		
		for( int i=1; i<=3; i++ ){
			if( ! nd->Has( *rt, *rt->GetValue( i ) ) ){
				rt->CopyValue( rt->GetValue( i ), nd->values[ nd->count ] );
				nd->count++;
			}
		}
		
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3, Object obj4, Object obj5 )
dsClassSet::nfNewWith5::nfNewWith5(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
	p_AddParameter( init.clsObject ); // obj4
	p_AddParameter( init.clsObject ); // obj5
}
void dsClassSet::nfNewWith5::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass * const clsObject = clsSet->GetClassObject();
	
	dsValue *newSet = NULL;
	sSetNatData *nd = NULL;
	
	try{
		newSet = clsSet->CreateSet( rt );
		nd = ( sSetNatData* )p_GetNativeData( newSet );
		
		nd->SetSize( 5, rt, clsObject );
		rt->CopyValue( rt->GetValue( 0 ), nd->values[ 0 ] );
		nd->count = 1;
		
		for( int i=1; i<=4; i++ ){
			if( ! nd->Has( *rt, *rt->GetValue( i ) ) ){
				rt->CopyValue( rt->GetValue( i ), nd->values[ nd->count ] );
				nd->count++;
			}
		}
		
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newFrom( Array array )
dsClassSet::nfNewFromArray::nfNewFromArray(const sInitData &init) : dsFunction( init.clsSet,
"newFrom", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsArray ); // Array
}
void dsClassSet::nfNewFromArray::RunFunction( dsRunTime *rt, dsValue* ){
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsClassArray * const clsArray = ( dsClassArray* )rt->GetEngine()->GetClassArray();
	dsClass * const clsObject = clsSet->GetClassObject();
	
	dsRealObject * const objArray = rt->GetValue( 0 )->GetRealObject();
	const int count = clsArray->GetObjectCount( rt, objArray );
	dsValue *newSet = NULL;
	sSetNatData *nd = NULL;
	
	try{
		newSet = clsSet->CreateSet( rt );
		nd = ( sSetNatData* )p_GetNativeData( newSet );
		
		nd->SetSize( count, rt, clsObject );
		
		for( int i=0; i<count; i++ ){
			dsValue &value = *clsArray->GetObjectAt( rt, objArray, i );
			if( ! nd->Has( *rt, value ) ){
				rt->CopyValue( &value, nd->values[ nd->count ] );
				nd->count++;
			}
		}
		
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}



// function const int getCount()
dsClassSet::nfGetCount::nfGetCount(const sInitData &init) : dsFunction(init.clsSet, "getCount",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger){
}
void dsClassSet::nfGetCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( ((sSetNatData*)p_GetNativeData(myself))->count );
}

// function void add(Object Obj)
dsClassSet::nfAdd::nfAdd(const sInitData &init) : dsFunction(init.clsSet, "add",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObject); // Obj
}
void dsClassSet::nfAdd::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = (sSetNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	if( nd->Has( *rt, *rt->GetValue(0) ) ){
		return;
	}
	
	dsClass *clsObject = ((dsClassSet*)GetOwnerClass())->GetClassObject();
	if(nd->count == nd->size){
		nd->SetSize(nd->size * 3 / 2 + 1, rt, clsObject);
	}
	rt->CopyValue(rt->GetValue(0), nd->values[nd->count]);
	nd->count++;
}

// const function bool has( Object obj )
dsClassSet::nfHas::nfHas( const sInitData &init ) : dsFunction( init.clsSet, "has",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObject ); // obj
}
void dsClassSet::nfHas::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	rt->PushBool( nd.Has( *rt, *rt->GetValue( 0 ) ) );
}

// function void remove(Object object)
dsClassSet::nfRemove::nfRemove(const sInitData &init) : dsFunction(init.clsSet, "remove",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObject); // object
}
void dsClassSet::nfRemove::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	const int index = nd.IndexOf( *rt, *rt->GetValue( 0 ) );
	if( index == -1 ){
		return;
	}
	
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	if( index < nd.count - 1 ){
		rt->MoveValue( nd.values[ nd.count - 1 ], nd.values[ index ] );
		
	}else{
		rt->ClearValue( nd.values[ index ] );
	}
	nd.count--;
}

// function void removeAll()
dsClassSet::nfRemoveAll::nfRemoveAll(const sInitData &init) : dsFunction(init.clsSet,
"removeAll", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassSet::nfRemoveAll::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = (sSetNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	cSetNatDatLockModifyGuard lock( nd->lockModify );
	for(int i=0; i<nd->count; i++){
		rt->ClearValue(nd->values[i]);
	}
	nd->count = 0;
}



// Enumeration
////////////////

// public func void forEach( Block ablock )
dsClassSet::nfForEach::nfForEach( const sInitData &init ) : dsFunction( init.clsSet,
"forEach", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfForEach::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = ( sSetNatData* )p_GetNativeData( myself );
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cSetNatDatLockModifyGuard lock( nd->lockModify );
	for( i=0; i<nd->count; i++ ){
		rt->PushValue( nd->values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
	}
}

// public func void forEachWhile( Block ablock )
dsClassSet::nfForEachWhile::nfForEachWhile( const sInitData &init ) : dsFunction( init.clsSet,
"forEachWhile", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfForEachWhile::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = ( sSetNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cSetNatDatLockModifyGuard lock( nd->lockModify );
	for( i=0; i<nd->count; i++ ){
		rt->PushValue( nd->values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
		
		if( rt->GetReturnValue()->GetType() != clsBool ){
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
		}
		if( ! rt->GetReturnBool() ){
			break;
		}
	}
}

// public func Set map( Block ablock )
dsClassSet::nfMap::nfMap( const sInitData &init ) : dsFunction( init.clsSet,
"map", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfMap::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = ( sSetNatData* )p_GetNativeData( myself );
	dsClassSet *clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	dsValue *newSet = NULL;
	sSetNatData *ndnew;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, errorModifyWhileLocked );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cSetNatDatLockModifyGuard lock( nd->lockModify );
	try{
		// create the new set of the same capacity as this set
		newSet = rt->CreateValue( clsSet );
		rt->CreateObjectNaked( newSet, clsSet );
		ndnew = ( sSetNatData* )p_GetNativeData( newSet );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		ndnew->SetSize( nd->count, rt, clsObject );
		
		// do the mapping
		for( i=0; i<nd->count; i++ ){
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			rt->CopyValue( rt->GetReturnValue(), ndnew->values[ i ] );
			ndnew->count++;
		}
		
		// push the result and clean up
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ) rt->FreeValue( newSet );
		throw;
	}
}

// public func Set collect( Block ablock )
dsClassSet::nfCollect::nfCollect( const sInitData &init ) : dsFunction( init.clsSet,
"collect", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfCollect::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = ( sSetNatData* )p_GetNativeData( myself );
	dsClassSet *clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	dsValue *newSet = NULL;
	sSetNatData *ndnew;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cSetNatDatLockModifyGuard lock( nd->lockModify );
	
	try{
		// create the new set
		newSet = rt->CreateValue( clsSet );
		rt->CreateObjectNaked( newSet, clsSet );
		ndnew = ( sSetNatData* )p_GetNativeData( newSet );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		// do the mapping
		for( i=0; i<nd->count; i++ ){
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			
			if( rt->GetReturnValue()->GetType() != clsBool ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
			}
			if( ! rt->GetReturnBool() ){
				continue;
			}
			
			if( ndnew->count == ndnew->size ){
				ndnew->SetSize( ndnew->size * 3 / 2 + 1, rt, clsObject );
			}
			rt->CopyValue( nd->values[ i ], ndnew->values[ ndnew->count ] );
			ndnew->count++;
		}
		
		// push the result and clean up
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ) rt->FreeValue( newSet );
		throw;
	}
}

// public func Object fold( Block ablock )
dsClassSet::nfFold::nfFold( const sInitData &init ) : dsFunction( init.clsSet,
"fold", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObject ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfFold::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = ( sSetNatData* )p_GetNativeData( myself );
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	if( nd->count == 0 ){
		rt->PushObject( NULL, clsObject );
		
	}else if( nd->count == 1 ){
		rt->PushValue( nd->values[ 0 ] );
		
	}else{
		const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
		cSetNatDatLockModifyGuard lock( nd->lockModify );
		for( i=1; i<nd->count; i++ ){
			rt->PushValue( nd->values[ i ] );
			
			if( i == 1 ){
				rt->PushValue( nd->values[ 0 ] );
				
			}else{
				rt->PushReturnValue();
			}
			
			rt->RunFunctionFast( block, funcIndexRun );
		}
		
		rt->PushReturnValue();
	}
}

// public func Object inject( Object injectValue, Block ablock )
dsClassSet::nfInject::nfInject( const sInitData &init ) : dsFunction( init.clsSet,
"inject", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObject ){
	p_AddParameter( init.clsObject ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfInject::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	
	dsValue * const injectValue = rt->GetValue( 0 );
	dsValue * const block = rt->GetValue( 1 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	if( nd.count == 0 ){
		rt->PushValue( injectValue );
		return;
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	int i;
	
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.count; i++ ){
		rt->PushValue( nd.values[ i ] );
		
		if( i == 0 ){
			rt->PushValue( injectValue );
			
		}else{
			rt->PushReturnValue();
		}
		
		rt->RunFunctionFast( block, funcIndexRun );
	}
	
	rt->PushReturnValue();
}

// public func Object find( Block ablock )
dsClassSet::nfFind::nfFind( const sInitData &init ) : dsFunction( init.clsSet,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObject ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfFind::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData *nd = ( sSetNatData* )p_GetNativeData( myself );
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cSetNatDatLockModifyGuard lock( nd->lockModify );
	for( i=0; i<nd->count; i++ ){
		rt->PushValue( nd->values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
		
		if( rt->GetReturnValue()->GetType() != clsBool ){
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
		}
		if( ! rt->GetReturnBool() ){
			continue;
		}
		
		if( i >= nd->count ){
			DSTHROW( dueInvalidAction );
		}
		rt->PushValue( nd->values[ i ] );
		return;
	}
	
	rt->PushObject( NULL, clsObject );
}

// public func void removeIf( Block ablock )
dsClassSet::nfRemoveIf::nfRemoveIf( const sInitData &init ) : dsFunction( init.clsSet,
"removeIf", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfRemoveIf::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i, last = 0;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.count; i++ ){
		rt->PushValue( nd.values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
		
		if( rt->GetReturnValue()->GetType() != clsBool ){
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
		}
		if( rt->GetReturnBool() ){
			continue;
		}
		
		if( i > last ){
			rt->MoveValue( nd.values[ i ], nd.values[ last ] );
		}
		last++;
	}
	
	for( i=last; i<nd.count; i++ ){
		rt->ClearValue( nd.values[ i ] );
	}
	
	nd.count = last;
}

// public func int count( Block ablock )
dsClassSet::nfCount::nfCount( const sInitData &init ) : dsFunction( init.clsSet,
"count", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i, count = 0;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.count; i++ ){
		rt->PushValue( nd.values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
		
		if( rt->GetReturnValue()->GetType() != clsBool ){
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
		}
		if( rt->GetReturnBool() ){
			count++;
		}
	}
	
	rt->PushInt( count );
}



// public func bool equals( Object object )
dsClassSet::nfEquals::nfEquals( const sInitData &init ) :
dsFunction(init.clsSet, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObject ); // object
}
void dsClassSet::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	dsClass * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( object->GetType()->GetPrimitiveType() != DSPT_OBJECT || ! object->GetRealObject()
	|| object->GetRealObject()->GetType() != clsSet ){
		rt->PushBool( false );
		return;
	}
	
	sSetNatData &nd2 = *( ( sSetNatData* )p_GetNativeData( object ) );
	if( nd2.count != nd.count ){
		rt->PushBool( false );
		return;
	}
	
	cSetNatDatLockModifyGuard lock( nd.lockModify ), lock2( nd2.lockModify );
	int i;
	for( i=0; i<nd.count; i++ ){
		if( ! nd2.Has( *rt, *nd.values[ i ] ) ){
			rt->PushBool( false );
			return;
		}
	}
	
	rt->PushBool( true );
}

// public func Object random()
dsClassSet::nfRandom::nfRandom( const sInitData &init ) : dsFunction( init.clsSet,
"random", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObject ){
}
void dsClassSet::nfRandom::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	
	if( nd.count == 0 ){
		DSTHROW_INFO( dueInvalidParam, "count == 0" );
	}
	
	const int index = ( int )( ( ( float )rand() / ( float )RAND_MAX ) * ( float )nd.count );
	
	if( index < nd.count ){
		rt->PushValue( nd.values[ index ] );
		
	}else{
		rt->PushValue( nd.values[ nd.count - 1 ] );
	}
}

// public func Array toArray()
dsClassSet::nfToArray::nfToArray( const sInitData &init ) : dsFunction( init.clsSet,
"toArray", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArray ){
}
void dsClassSet::nfToArray::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	dsClassArray * const clsArray = ( dsClassArray* )rt->GetEngine()->GetClassArray();
	
	dsValue * const array = clsArray->CreateArray( rt );
	try{
		for( int i=0; i<nd.count; i++ ){
			clsArray->AddObject( rt, array->GetRealObject(), nd.values[ i ] );
		}
		rt->PushValue( array );
		rt->FreeValue( array );
		
	}catch( ... ){
		rt->FreeValue( array );
		throw;
	}
}

// public func String toString()
dsClassSet::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsSet,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassSet::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	char *buffer = NULL;
	int maxLen = 5000;
	int curLen = 0;
	int i;
	
	const int funcIndexToString = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexToString();
	
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	try{
		buffer = new char[ maxLen + 5 ]; // ] + ... just in case
		buffer[ 0 ] = '[';
		buffer[ 1 ] = '\0';
		curLen = 1;
		
		for( i=0; i<nd.count; i++ ){
			const char *itemstr;
			
			if( nd.values[ i ]->GetRealType()->GetPrimitiveType() != DSPT_OBJECT
			|| nd.values[ i ]->GetRealObject() ){
				rt->RunFunctionFast( nd.values[ i ], funcIndexToString );
				itemstr = rt->GetReturnValue()->GetString();
				
			}else{
				itemstr = "null";
			}
			
			const int islen = strlen( itemstr );
			
			if( i > 0 ){
				buffer[ curLen++ ] = ',';
				buffer[ curLen ] = '\0';
			}
			
			if( curLen + islen > maxLen ){
				strcat( buffer, "..." );
				curLen += 3;
				break;
				
			}else{
				strcat( buffer, itemstr );
				curLen += islen;
			}
		}
		
		buffer[ curLen ] = ']';
		buffer[ curLen + 1 ] = '\0';
		rt->PushString( buffer );
		delete [] buffer;
		
	}catch( ... ){
		if( buffer ){
			delete [] buffer;
		}
		throw;
	}
}



// public func Set +( Set set )
dsClassSet::nfOperatorPlus::nfOperatorPlus( const sInitData &init ) :
dsFunction(init.clsSet, "+", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsSet ); // set
}
void dsClassSet::nfOperatorPlus::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	dsClassObject * clsObject = ( dsClassObject* )rt->GetEngine()->GetClassObject();
	const sSetNatData &nd2 = *( ( sSetNatData* )p_GetNativeData( object ) );
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	int i;
	
	dsValue * const newSet = clsSet->CreateSet( rt );
	
	sSetNatData &ndNew = *( ( sSetNatData* )p_GetNativeData( newSet ) );
	ndNew.SetSize( nd.count + nd2.count, rt, clsObject );
	
	for( i=0; i<nd.count; i++ ){
		rt->CopyValue( nd.values[ i ], ndNew.values[ i ] );
	}
	ndNew.count = nd.count;
	
	try{
		for( i=0; i<nd2.count; i++ ){
			if( ! ndNew.Has( *rt, *nd2.values[ i ] ) ){
				rt->CopyValue( nd2.values[ i ], ndNew.values[ ndNew.count ] );
				ndNew.count++;
			}
		}
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		rt->FreeValue( newSet );
		throw;
	}
}

// public func Set -( Set set )
dsClassSet::nfOperatorMinus::nfOperatorMinus( const sInitData &init ) :
dsFunction(init.clsSet, "-", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsSet ); // set
}
void dsClassSet::nfOperatorMinus::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	dsClassObject * clsObject = ( dsClassObject* )rt->GetEngine()->GetClassObject();
	sSetNatData &nd2 = *( ( sSetNatData* )p_GetNativeData( object ) );
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	int i;
	
	dsValue * const newSet = clsSet->CreateSet( rt );
	
	sSetNatData &ndNew = *( ( sSetNatData* )p_GetNativeData( newSet ) );
	ndNew.SetSize( nd.count, rt, clsObject );
	
	try{
		for( i=0; i<nd.count; i++ ){
			if( ! nd2.Has( *rt, *nd.values[ i ] ) ){
				rt->CopyValue( nd.values[ i ], ndNew.values[ ndNew.count ] );
				ndNew.count++;
			}
		}
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		rt->FreeValue( newSet );
		throw;
	}
}

// public func Set +=( Set set )
dsClassSet::nfOperatorPlusAssign::nfOperatorPlusAssign( const sInitData &init ) :
dsFunction(init.clsSet, "+=", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsSet ); // set
}
void dsClassSet::nfOperatorPlusAssign::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	dsClassObject * clsObject = ( dsClassObject* )rt->GetEngine()->GetClassObject();
	const sSetNatData &nd2 = *( ( sSetNatData* )p_GetNativeData( object ) );
	int i;
	
	nd.SetSize( nd.count + nd2.count, rt, clsObject );
	
	for( i=0; i<nd2.count; i++ ){
		if( ! nd.Has( *rt, *nd2.values[ i ] ) ){
			rt->CopyValue( nd2.values[ i ], nd.values[ nd.count ] );
			nd.count++;
		}
	}
	rt->PushValue( myself );
}

// public func Set -=( Set set )
dsClassSet::nfOperatorMinusAssign::nfOperatorMinusAssign( const sInitData &init ) :
dsFunction(init.clsSet, "-=", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsSet ); // set
}
void dsClassSet::nfOperatorMinusAssign::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	const sSetNatData &nd2 = *( ( sSetNatData* )p_GetNativeData( object ) );
	int i;
	
	for( i=0; i<nd2.count; i++ ){
		const int index = nd.IndexOf( *rt, *nd2.values[ i ] );
		if( index == -1 ){
			continue;
		}
		
		if( index < nd.count - 1 ){
			rt->MoveValue( nd.values[ nd.count - 1 ], nd.values[ index ] );
			
		}else{
			rt->ClearValue( nd.values[ index ] );
		}
		nd.count--;
	}
	
	rt->PushValue( myself );
}



// Class dsClassSet
/////////////////////

// Constructor, destructor
////////////////////////////

dsClassSet::dsClassSet() : dsClass( "Set", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sSetNatData ) );
}

dsClassSet::~dsClassSet(){
}


// Management
///////////////

void dsClassSet::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	// store classes
	init.clsSet = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInteger = engine->GetClassInt();
	init.clsObject = engine->GetClassObject();
	init.clsBool = engine->GetClassBool();
	init.clsBlock = engine->GetClassBlock();
	init.clsString = engine->GetClassString();
	init.clsArray = engine->GetClassArray();
	
	// get object class from engine
	p_ClsObj = init.clsObject;
	
	// functions
	AddFunction( new nfNew( init ) );
	AddFunction( new nfCopy( init ) );
	AddFunction( new nfDestructor( init ) );
	AddFunction( new nfNewWith1( init ) );
	AddFunction( new nfNewWith2( init ) );
	AddFunction( new nfNewWith3( init ) );
	AddFunction( new nfNewWith4( init ) );
	AddFunction( new nfNewWith5( init ) );
	AddFunction( new nfNewFromArray( init ) );
	
	AddFunction( new nfGetCount( init ) );
	AddFunction( new nfAdd( init ) );
	AddFunction( new nfHas( init ) );
	AddFunction( new nfRemove( init ) );
	AddFunction( new nfRemoveAll( init ) );
	AddFunction( new nfForEach( init ) );
	AddFunction( new nfForEachWhile( init ) );
	AddFunction( new nfMap( init ) );
	AddFunction( new nfCollect( init ) );
	AddFunction( new nfFold( init ) );
	AddFunction( new nfInject( init ) );
	AddFunction( new nfFind( init ) );
	AddFunction( new nfRemoveIf( init ) );
	AddFunction( new nfCount( init ) );
	
	AddFunction( new nfRandom( init ) );
	AddFunction( new nfToArray( init ) );
	
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfToString( init ) );
	
	AddFunction( new nfOperatorPlus( init ) );
	AddFunction( new nfOperatorMinus( init ) );
	AddFunction( new nfOperatorPlusAssign( init ) );
	AddFunction( new nfOperatorMinusAssign( init ) );
}



dsValue *dsClassSet::CreateSet( dsRunTime *rt ){
	dsValue *newSet = NULL;
	sSetNatData *nd;
	
	try{
		newSet = rt->CreateValue( this );
		rt->CreateObjectNaked( newSet, this );
		nd = ( sSetNatData* )p_GetNativeData( newSet->GetRealObject()->GetBuffer() );
		nd->values = NULL;
		nd->count = 0;
		nd->size = 0;
		nd->lockModify = 0;
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
	
	return newSet;
}

void dsClassSet::AddObject( dsRunTime *rt, dsRealObject *myself, dsValue *value ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself->GetBuffer() ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	if( nd.count == nd.size ){
		nd.SetSize( nd.size * 3 / 2 + 1, rt, p_ClsObj );
	}
	
	if( value ){
		rt->CopyValue( value, nd.values[ nd.count ] );
		
	}else{
		rt->SetNull( nd.values[ nd.count ], p_ClsObj );
	}
	
	nd.count++;
}

void dsClassSet::RemoveObject( dsRunTime* rt, dsRealObject* myself, int index ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself->GetBuffer() ) );
	
	if( index < 0 || index >= nd.count ){
		DSTHROW( dueInvalidParam );
	}
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	if( index < nd.count - 1 ){
		rt->MoveValue( nd.values[ nd.count - 1 ], nd.values[ index ] );
		
	}else{
		rt->ClearValue( nd.values[ index ] );
	}
	nd.count--;
}

int dsClassSet::GetObjectCount( dsRunTime *rt, dsRealObject *myself ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	const sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself->GetBuffer() ) );
	return nd.count;
}

dsValue *dsClassSet::GetObjectAt( dsRunTime *rt, dsRealObject *myself, int index ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	const sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself->GetBuffer() ) );
	if( index < 0 || index >= nd.count ){
		DSTHROW( dueInvalidParam );
	}
	
	return nd.values[ index ];
}

bool dsClassSet::HasObject( dsRunTime *rt, dsRealObject *myself, dsValue *value ){
	if( ! rt || ! myself || ! value ){
		DSTHROW( dueNullPointer );
	}
	
	sSetNatData &nd = *( ( sSetNatData* )p_GetNativeData( myself->GetBuffer() ) );
	return nd.Has( *rt, *value );
}
