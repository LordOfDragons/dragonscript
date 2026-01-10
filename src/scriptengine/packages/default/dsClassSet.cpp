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
#include "../../dragonscript_config.h"
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
	dsValue **values = nullptr;
	int count = 0;
	int size = 0;
	int lockModify = 0;
	
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
	
	bool CastableTo( int index, dsClass *type ) const{
		const dsValue &value = *values[ index ];
		dsClass *valueType = value.GetType();
		if( valueType->GetPrimitiveType() == DSPT_OBJECT ){
			if( value.GetRealObject() ){
				valueType = value.GetRealObject()->GetType();
			}
		}
		return valueType->CastableTo( type );
	}
	
	void Add( dsRunTime *rt, dsValue *value, dsClass *clsObject ){
		if( count == size ){
			SetSize( size * 3 / 2 + 1, rt, clsObject );
		}
		rt->CopyValue( value, values[ count ] );
		count++;
	}
};

class dsClassSet_NewFinally{
	dsRunTime &pRT;
	dsValue *pValue;
	
public:
	dsClassSet_NewFinally( dsRunTime &rt, dsClass &cls, int argumentCount = 0 ) :
	pRT( rt ), pValue( NULL ){
		dsClassSet &clsSet = ( dsClassSet& )cls;
		if( argumentCount == 0 ){
			pValue = clsSet.CreateSet( &rt );
			
		}else{
			pValue = clsSet.CreateSet( &rt, argumentCount );
		}
	}
	
	inline dsValue *Value() const{ return pValue; }
	inline void Push() const{ pRT.PushValue( pValue ); }
	
	~dsClassSet_NewFinally(){
		if( pValue ){
			pRT.FreeValue( pValue );
		}
	}
};

class dsClassSet_BlockRunner{
public:
	dsRunTime &rt;
	sSetNatData &nd;
	dsValue * const block;
	const dsClassBlock &clsBlock;
	const dsSignature &signature;
	const bool useIndex;
	const int funcIndexRun;
	const cSetNatDatLockModifyGuard lock;
	
	dsClassSet_BlockRunner( dsRunTime &rt, sSetNatData &nd, dsValue *block ) :
	rt( rt ),
	nd( nd ),
	block( block ),
	clsBlock( *( ( dsClassBlock* )rt.GetEngine()->GetClassBlock() ) ),
	signature( clsBlock.GetSignature( block->GetRealObject() ) ),
	useIndex( signature.GetCount() == 2 ),
	funcIndexRun( clsBlock.GetFuncIndexRun1() ),
	lock( nd.lockModify ){
	}
	
	void Run( int index ) const{
		rt.PushValue( nd.values[ index ] );
		if( useIndex ){
			rt.PushInt( index );
		}
		rt.RunFunctionFast( block, funcIndexRun );
	}
	
	dsClass * const castType() const{
		return signature.GetParameter( signature.GetCount() - 1 );
	}
};

class dsClassSet_BlockRunnerBool : public dsClassSet_BlockRunner{
public:
	dsClass * const clsBool;
	
	dsClassSet_BlockRunnerBool( dsRunTime &rt, sSetNatData &nd, dsValue *block ) :
	dsClassSet_BlockRunner( rt, nd, block ),
	clsBool( rt.GetEngine()->GetClassBool() ){
	}
	
	bool RunBool( int index ) const{
		Run( index );
		if( rt.GetReturnValue()->GetType() != clsBool ){
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt.GetReturnValue(), clsBool ) );
		}
		return rt.GetReturnBool();
	}
};



// Native functions
/////////////////////

// constructor new()
dsClassSet::nfNew::nfNew(const sInitData &init) : dsFunction(init.clsSet,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassSet::nfNew::RunFunction( dsRunTime*, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	nd.values = NULL;
	nd.count = 0;
	nd.size = 0;
	nd.lockModify = 0;
}

// constructor Copy(Set set)
dsClassSet::nfCopy::nfCopy(const sInitData &init) : dsFunction(init.clsSet,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsSet); // arr
}
void dsClassSet::nfCopy::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	dsClass *clsObject = ((dsClassSet*)GetOwnerClass())->GetClassObject();
	nd.values = NULL;
	nd.count = 0;
	nd.size = 0;
	nd.lockModify = 0;
	dsValue *vObj = rt->GetValue(0);
	sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(vObj));
	int i, vSize = nd2.count;
	if(vSize > 0){
		nd.values = new dsValue*[vSize];
		if(!nd.values) DSTHROW(dueOutOfMemory);
		for(i=0; i<vSize; i++){
			nd.values[i] = rt->CreateValue(clsObject);
			if(!nd.values[i]) DSTHROW(dueOutOfMemory);
			nd.size++;
			rt->CopyValue(nd2.values[i], nd.values[i]);
		}
		nd.count = nd.size;
	}
}

// destructor Destructor
dsClassSet::nfDestructor::nfDestructor(const sInitData &init) : dsFunction(init.clsSet,
"destructor", DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassSet::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
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
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 1 );
	try{
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
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 2 );
	try{
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
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 3 );
	try{
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
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 4 );
	try{
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
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 5 );
	try{
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6 )
dsClassSet::nfNewWith6::nfNewWith6(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
	p_AddParameter( init.clsObject ); // obj4
	p_AddParameter( init.clsObject ); // obj5
	p_AddParameter( init.clsObject ); // obj6
}
void dsClassSet::nfNewWith6::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 6 );
	try{
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7 )
dsClassSet::nfNewWith7::nfNewWith7(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
	p_AddParameter( init.clsObject ); // obj4
	p_AddParameter( init.clsObject ); // obj5
	p_AddParameter( init.clsObject ); // obj6
	p_AddParameter( init.clsObject ); // obj7
}
void dsClassSet::nfNewWith7::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 7 );
	try{
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7, Object obj8 )
dsClassSet::nfNewWith8::nfNewWith8(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
	p_AddParameter( init.clsObject ); // obj4
	p_AddParameter( init.clsObject ); // obj5
	p_AddParameter( init.clsObject ); // obj6
	p_AddParameter( init.clsObject ); // obj7
	p_AddParameter( init.clsObject ); // obj8
}
void dsClassSet::nfNewWith8::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 8 );
	try{
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7, Object obj8, Object obj9 )
dsClassSet::nfNewWith9::nfNewWith9(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
	p_AddParameter( init.clsObject ); // obj4
	p_AddParameter( init.clsObject ); // obj5
	p_AddParameter( init.clsObject ); // obj6
	p_AddParameter( init.clsObject ); // obj7
	p_AddParameter( init.clsObject ); // obj8
	p_AddParameter( init.clsObject ); // obj9
}
void dsClassSet::nfNewWith9::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 9 );
	try{
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
}

// public static func Set newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7, Object obj8, Object obj9, Object obj10 )
dsClassSet::nfNewWith10::nfNewWith10(const sInitData &init) : dsFunction( init.clsSet,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsObject ); // obj1
	p_AddParameter( init.clsObject ); // obj2
	p_AddParameter( init.clsObject ); // obj3
	p_AddParameter( init.clsObject ); // obj4
	p_AddParameter( init.clsObject ); // obj5
	p_AddParameter( init.clsObject ); // obj6
	p_AddParameter( init.clsObject ); // obj7
	p_AddParameter( init.clsObject ); // obj8
	p_AddParameter( init.clsObject ); // obj9
	p_AddParameter( init.clsObject ); // obj10
}
void dsClassSet::nfNewWith10::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newSet = ( ( dsClassSet* )GetOwnerClass() )->CreateSet( rt, 10 );
	try{
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
	
	try{
		newSet = clsSet->CreateSet( rt );
		sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(newSet));
		
		nd.SetSize( count, rt, clsObject );
		
		for( int i=0; i<count; i++ ){
			dsValue &value = *clsArray->GetObjectAt( rt, objArray, i );
			if( ! nd.Has( *rt, value ) ){
				rt->CopyValue( &value, nd.values[ nd.count ] );
				nd.count++;
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
	rt->PushInt(dsNativeDataGet<sSetNatData>(p_GetNativeData(myself)).count);
}

// function void add(Object Obj)
dsClassSet::nfAdd::nfAdd(const sInitData &init) : dsFunction(init.clsSet, "add",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObject); // Obj
}
void dsClassSet::nfAdd::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	if( nd.Has( *rt, *rt->GetValue(0) ) ){
		return;
	}
	
	dsClass *clsObject = ((dsClassSet*)GetOwnerClass())->GetClassObject();
	if(nd.count == nd.size){
		nd.SetSize(nd.size * 3 / 2 + 1, rt, clsObject);
	}
	rt->CopyValue(rt->GetValue(0), nd.values[nd.count]);
	nd.count++;
}

// function void addAll( Set set )
dsClassSet::nfAddAll::nfAddAll( const sInitData &init ) :
dsFunction( init.clsSet, "addAll", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsSet ); // set
}
void dsClassSet::nfAddAll::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsClassSet &clsSet = *( ( dsClassSet* )GetOwnerClass() );
	dsValue * const set = rt->GetValue( 0 );
	if( ! set->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	if( set == myself ){
		return;
	}
	
	const sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(set));
	if( nd2.count == 0 ){
		return;
	}
	
	int i;
	for( i=0; i<nd2.count; i++ ){
		if( nd.Has( *rt, *nd2.values[ i ] ) ){
			continue;
		}
		
		if( nd.count == nd.size ){
			nd.SetSize( nd.size * 3 / 2 + 1, rt, clsSet.GetClassObject() );
		}
		rt->CopyValue( nd2.values[ i ], nd.values[ nd.count ] );
		nd.count++;
	}
}

// const function bool has( Object obj )
dsClassSet::nfHas::nfHas( const sInitData &init ) : dsFunction( init.clsSet, "has",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObject ); // obj
}
void dsClassSet::nfHas::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	rt->PushBool( nd.Has( *rt, *rt->GetValue( 0 ) ) );
}

// function void remove(Object object)
dsClassSet::nfRemove::nfRemove(const sInitData &init) : dsFunction(init.clsSet, "remove",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObject); // object
}
void dsClassSet::nfRemove::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	for(int i=0; i<nd.count; i++){
		rt->ClearValue(nd.values[i]);
	}
	nd.count = 0;
}

// function void removeAll( Set set )
dsClassSet::nfRemoveAll2::nfRemoveAll2( const sInitData &init ) :
dsFunction( init.clsSet, "removeAll", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsSet ); // set
}
void dsClassSet::nfRemoveAll2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const set = rt->GetValue( 0 );
	if( ! set->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	if( set == myself ){
		DSTHROW_INFO( dueNullPointer, "set == this" );
	}
	
	const sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(set));
	if( nd2.count == 0 ){
		return;
	}
	
	int i;
	for( i=0; i<nd2.count; i++ ){
		const int index = nd.IndexOf( *rt, *nd2.values[ i ] );
		if( index == -1 ){
			continue;
		}
		
		cSetNatDatLockModifyGuard lock( nd.lockModify );
		if( index < nd.count - 1 ){
			rt->MoveValue( nd.values[ nd.count - 1 ], nd.values[ index ] );
			
		}else{
			rt->ClearValue( nd.values[ index ] );
		}
		nd.count--;
	}
}



// Enumeration
////////////////

// public func void forEach( Block ablock )
dsClassSet::nfForEach::nfForEach( const sInitData &init ) : dsFunction( init.clsSet,
"forEach", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfForEach::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.count; i++ ){
		rt->PushValue( nd.values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
	}
}

// public func void forEachCastable( Block ablock )
dsClassSet::nfForEachCastable::nfForEachCastable( const sInitData &init ) : dsFunction( init.clsSet,
"forEachCastable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfForEachCastable::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const dsClassSet_BlockRunner blockRunner( *rt, nd, block );
	dsClass * const castType = blockRunner.castType();
	int i;
	for( i=0; i<nd.count; i++ ){
		if( nd.CastableTo( i, castType ) ){
			blockRunner.Run( i );
		}
	}
}

// public func void forEachWhile( Block ablock )
dsClassSet::nfForEachWhile::nfForEachWhile( const sInitData &init ) : dsFunction( init.clsSet,
"forEachWhile", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfForEachWhile::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
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
		if( ! rt->GetReturnBool() ){
			break;
		}
	}
}

// public func void forEachWhileCastable( Block ablock )
dsClassSet::nfForEachWhileCastable::nfForEachWhileCastable( const sInitData &init ) : dsFunction( init.clsSet,
"forEachWhileCastable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfForEachWhileCastable::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const dsClassSet_BlockRunnerBool blockRunner( *rt, nd, block );
	dsClass * const castType = blockRunner.castType();
	int i;
	for( i=0; i<nd.count; i++ ){
		if( nd.CastableTo( i, castType ) && ! blockRunner.RunBool( i ) ){
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	dsClassSet *clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	dsValue *newSet = NULL;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, errorModifyWhileLocked );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	try{
		// create the new set of the same capacity as this set
		newSet = rt->CreateValue( clsSet );
		rt->CreateObjectNaked( newSet, clsSet );
		sSetNatData &ndnew = dsNativeDataNew<sSetNatData>(p_GetNativeData(newSet));
		
		ndnew.SetSize( nd.count, rt, clsObject );
		
		// do the mapping
		for( i=0; i<nd.count; i++ ){
			rt->PushValue( nd.values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			rt->CopyValue( rt->GetReturnValue(), ndnew.values[ i ] );
			ndnew.count++;
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	dsClassSet *clsSet = ( dsClassSet* )GetOwnerClass();
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	dsValue *newSet = NULL;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cSetNatDatLockModifyGuard lock( nd.lockModify );
	
	try{
		// create the new set
		newSet = rt->CreateValue( clsSet );
		rt->CreateObjectNaked( newSet, clsSet );
		sSetNatData &ndnew = dsNativeDataNew<sSetNatData>(p_GetNativeData(newSet));
		
		// do the mapping
		for( i=0; i<nd.count; i++ ){
			rt->PushValue( nd.values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			
			if( rt->GetReturnValue()->GetType() != clsBool ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
			}
			if( ! rt->GetReturnBool() ){
				continue;
			}
			
			if( ndnew.count == ndnew.size ){
				ndnew.SetSize( ndnew.size * 3 / 2 + 1, rt, clsObject );
			}
			rt->CopyValue( nd.values[ i ], ndnew.values[ ndnew.count ] );
			ndnew.count++;
		}
		
		// push the result and clean up
		rt->PushValue( newSet );
		rt->FreeValue( newSet );
		
	}catch( ... ){
		if( newSet ) rt->FreeValue( newSet );
		throw;
	}
}

// public func Set collectCastable( Block ablock )
dsClassSet::nfCollectCastable::nfCollectCastable( const sInitData &init ) : dsFunction( init.clsSet,
"collectCastable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsSet ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfCollectCastable::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const dsClassSet_BlockRunnerBool blockRunner( *rt, nd, block );
	dsClass * const castType = blockRunner.castType();
	int i;
	
	const dsClassSet_NewFinally set( *rt, *GetOwnerClass() );
	sSetNatData &ndnew = dsNativeDataGet<sSetNatData>(p_GetNativeData(set.Value()));
	dsClass * const clsObject = rt->GetEngine()->GetClassObject();
	
	for( i=0; i<nd.count; i++ ){
		if( nd.CastableTo( i, castType ) && blockRunner.RunBool( i ) ){
			ndnew.Add( rt, nd.values[ i ], clsObject );
		}
	}
	
	set.Push();
}

// public func Object fold( Block ablock )
dsClassSet::nfFold::nfFold( const sInitData &init ) : dsFunction( init.clsSet,
"fold", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObject ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfFold::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	if( nd.count == 0 ){
		rt->PushObject( NULL, clsObject );
		
	}else if( nd.count == 1 ){
		rt->PushValue( nd.values[ 0 ] );
		
	}else{
		const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
		cSetNatDatLockModifyGuard lock( nd.lockModify );
		for( i=1; i<nd.count; i++ ){
			rt->PushValue( nd.values[ i ] );
			
			if( i == 1 ){
				rt->PushValue( nd.values[ 0 ] );
				
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	dsClass *clsObject = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
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
		if( ! rt->GetReturnBool() ){
			continue;
		}
		
		if( i >= nd.count ){
			DSTHROW( dueInvalidAction );
		}
		rt->PushValue( nd.values[ i ] );
		return;
	}
	
	rt->PushObject( NULL, clsObject );
}

// public func Object findCastable( Block ablock )
dsClassSet::nfFindCastable::nfFindCastable( const sInitData &init ) : dsFunction( init.clsSet,
"findCastable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObject ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfFindCastable::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const dsClassSet_BlockRunnerBool blockRunner( *rt, nd, block );
	dsClass * const castType = blockRunner.castType();
	int i;
	
	for( i=0; i<nd.count; i++ ){
		if( nd.CastableTo( i, castType ) && blockRunner.RunBool( i ) ){
			rt->PushValue( nd.values[ i ] );
			return;
		}
	}
	
	rt->PushObject( NULL, rt->GetEngine()->GetClassObject() );
}

// public func void removeIf( Block ablock )
dsClassSet::nfRemoveIf::nfRemoveIf( const sInitData &init ) : dsFunction( init.clsSet,
"removeIf", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfRemoveIf::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
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

// public func void removeIfCastable( Block ablock )
dsClassSet::nfRemoveIfCastable::nfRemoveIfCastable( const sInitData &init ) : dsFunction( init.clsSet,
"removeIfCastable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfRemoveIfCastable::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const dsClassSet_BlockRunnerBool blockRunner( *rt, nd, block );
	dsClass * const castType = blockRunner.castType();
	int i, last = 0;
	
	for( i=0; i<nd.count; i++ ){
		if( nd.CastableTo( i, castType ) && blockRunner.RunBool( i ) ){
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
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

// public func int countCastable( Block ablock )
dsClassSet::nfCountCastable::nfCountCastable( const sInitData &init ) : dsFunction( init.clsSet,
"countCastable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassSet::nfCountCastable::RunFunction( dsRunTime *rt, dsValue *myself ){
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const dsClassSet_BlockRunnerBool blockRunner( *rt, nd, block );
	dsClass * const castType = blockRunner.castType();
	int i, count = 0;
	
	for( i=0; i<nd.count; i++ ){
		if( nd.CastableTo( i, castType ) && blockRunner.RunBool( i ) ){
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	dsClass * const clsSet = ( dsClassSet* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( object->GetType()->GetPrimitiveType() != DSPT_OBJECT || ! object->GetRealObject()
	|| object->GetRealObject()->GetType() != clsSet ){
		rt->PushBool( false );
		return;
	}
	
	sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(object));
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
	const sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
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
	const sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
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
			
			const int islen = ( int )strlen( itemstr );
			
			if( i > 0 ){
				buffer[ curLen++ ] = ',';
				buffer[ curLen ] = '\0';
			}
			
			if( curLen + islen > maxLen ){
				#ifdef OS_W32_VS
					strcat_s( buffer, maxLen + 5, "..." );
				#else
					strcat( buffer, "..." );
				#endif
				curLen += 3;
				break;
				
			}else{
				#ifdef OS_W32_VS
					strcat_s( buffer, maxLen + 5, itemstr );
				#else
					strcat( buffer, itemstr );
				#endif
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	dsClassObject * clsObject = ( dsClassObject* )rt->GetEngine()->GetClassObject();
	const sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(object));
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	int i;
	
	dsValue * const newSet = clsSet->CreateSet( rt );
	
	sSetNatData &ndNew = dsNativeDataGet<sSetNatData>(p_GetNativeData(newSet));
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	dsClassObject * clsObject = ( dsClassObject* )rt->GetEngine()->GetClassObject();
	sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(object));
	dsClassSet * const clsSet = ( dsClassSet* )GetOwnerClass();
	int i;
	
	dsValue * const newSet = clsSet->CreateSet( rt );
	
	sSetNatData &ndNew = dsNativeDataGet<sSetNatData>(p_GetNativeData(newSet));
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	dsClassObject * clsObject = ( dsClassObject* )rt->GetEngine()->GetClassObject();
	const sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(object));
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
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself));
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object ){
		DSTHROW_INFO( dueNullPointer, "set" );
	}
	
	const sSetNatData &nd2 = dsNativeDataGet<sSetNatData>(p_GetNativeData(object));
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
	p_SetNativeDataSize(dsNativeDataSize<sSetNatData>());
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
	AddFunction( new nfNewWith6( init ) );
	AddFunction( new nfNewWith7( init ) );
	AddFunction( new nfNewWith8( init ) );
	AddFunction( new nfNewWith9( init ) );
	AddFunction( new nfNewWith10( init ) );
	AddFunction( new nfNewFromArray( init ) );
	
	AddFunction( new nfGetCount( init ) );
	AddFunction( new nfAdd( init ) );
	AddFunction( new nfAddAll( init ) );
	AddFunction( new nfHas( init ) );
	AddFunction( new nfRemove( init ) );
	AddFunction( new nfRemoveAll( init ) );
	AddFunction( new nfRemoveAll2( init ) );
	AddFunction( new nfForEach( init ) );
	AddFunction( new nfForEachCastable( init ) );
	AddFunction( new nfForEachWhile( init ) );
	AddFunction( new nfForEachWhileCastable( init ) );
	AddFunction( new nfMap( init ) );
	AddFunction( new nfCollect( init ) );
	AddFunction( new nfCollectCastable( init ) );
	AddFunction( new nfFold( init ) );
	AddFunction( new nfInject( init ) );
	AddFunction( new nfFind( init ) );
	AddFunction( new nfFindCastable( init ) );
	AddFunction( new nfRemoveIf( init ) );
	AddFunction( new nfRemoveIfCastable( init ) );
	AddFunction( new nfCount( init ) );
	AddFunction( new nfCountCastable( init ) );
	
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
	
	try{
		newSet = rt->CreateValue( this );
		rt->CreateObjectNaked( newSet, this );
		dsNativeDataNew<sSetNatData>(p_GetNativeData(newSet->GetRealObject()->GetBuffer()));
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
	
	return newSet;
}

dsValue *dsClassSet::CreateSet( dsRunTime *rt, int argumentCount ){
	if( argumentCount < 0 ){
		DSTHROW_INFO( dueInvalidParam, "argumentCount < 0" );
	}
	
	dsValue *newSet = NULL;
	int i;
	
	try{
		newSet = CreateSet( rt );
		sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(newSet->GetRealObject()->GetBuffer()));
		
		nd.SetSize( argumentCount, rt, p_ClsObj );
		for( i=0; i<argumentCount; i++ ){
			if( ! nd.Has( *rt, *rt->GetValue( i ) ) ){
				rt->CopyValue( rt->GetValue( i ), nd.values[ nd.count ] );
				nd.count++;
			}
		}
		
	}catch( ... ){
		if( newSet ){
			rt->FreeValue( newSet );
		}
		throw;
	}
	
	return newSet;
}

void dsClassSet::AddObject( dsRunTime *rt, dsRealObject *myself, dsValue *value ) const{
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself->GetBuffer()));
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

void dsClassSet::RemoveObject( dsRunTime* rt, dsRealObject* myself, int index ) const{
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself->GetBuffer()));
	
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

int dsClassSet::GetObjectCount( dsRunTime *rt, dsRealObject *myself ) const{
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	const sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself->GetBuffer()));
	return nd.count;
}

dsValue *dsClassSet::GetObjectAt( dsRunTime *rt, dsRealObject *myself, int index ) const{
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	const sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself->GetBuffer()));
	if( index < 0 || index >= nd.count ){
		DSTHROW( dueInvalidParam );
	}
	
	return nd.values[ index ];
}

bool dsClassSet::HasObject( dsRunTime *rt, dsRealObject *myself, dsValue *value ) const{
	if( ! rt || ! myself || ! value ){
		DSTHROW( dueNullPointer );
	}
	
	sSetNatData &nd = dsNativeDataGet<sSetNatData>(p_GetNativeData(myself->GetBuffer()));
	return nd.Has( *rt, *value );
}
