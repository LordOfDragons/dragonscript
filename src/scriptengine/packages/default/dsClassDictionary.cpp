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
#include "dsClassArray.h"
#include "dsClassBlock.h"
#include "dsClassDictionary.h"
#include "dsClassObject.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

static const char * const errorModifyWhileLocked = "modify while locked";

// native data structure
struct sDictEntry{
	unsigned int hash;
	dsValue *key;
	dsValue *value;
	sDictEntry *next;
};

struct sDictNatData{
	sDictEntry **buckets;
	int bucketCount;
	int entryCount;
	int lockModify;
};

class cDictNatDatLockModifyGuard{
	int &pLockModify;
	
public:
	cDictNatDatLockModifyGuard( int &lockModify ) : pLockModify( lockModify ){
		pLockModify++;
	}
	
	~cDictNatDatLockModifyGuard(){
		pLockModify--;
	}
};

// Native Functions
/////////////////////

// public func new()
dsClassDictionary::nfCreate::nfCreate( const sInitData &init ) : dsFunction( init.clsDict,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassDictionary::nfCreate::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	int i;
	
	nd.buckets = NULL;
	nd.bucketCount = 8;
	nd.entryCount = 0;
	nd.lockModify = 0;
	
	nd.buckets = new sDictEntry*[ nd.bucketCount ];
	for( i=0; i<nd.bucketCount; i++ ){
		nd.buckets[ i ] = NULL;
	}
}

// public func new( Dictionary dict )
dsClassDictionary::nfCopy::nfCopy( const sInitData &init ) : dsFunction( init.clsDict,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsDict ); // dict
}
void dsClassDictionary::nfCopy::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	
	nd.buckets = NULL;
	nd.bucketCount = 0;
	nd.entryCount = 0;
	nd.lockModify = 0;
	
	dsValue * const object = rt->GetValue( 0 );
	if( ! object->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "dict" );
	}
	
	const sDictNatData &nd2 = *( ( sDictNatData* )p_GetNativeData( object ) );
	sDictEntry *iterEntry;
	sDictEntry *lastEntry;
	sDictEntry *newEntry;
	int i;
	
	nd.bucketCount = nd2.bucketCount;
	nd.entryCount = nd2.entryCount;
	
	nd.buckets = new sDictEntry*[ nd2.bucketCount ];
	for( i=0; i<nd2.bucketCount; i++ ){
		nd.buckets[ i ] = NULL;
	}
	
	try{
		for( i=0; i<nd2.bucketCount; i++ ){
			iterEntry = nd2.buckets[ i ];
			lastEntry = NULL;
			newEntry = NULL;
			
			while( iterEntry ){
				newEntry = new sDictEntry;
				
				newEntry->hash = iterEntry->hash;
				newEntry->key = NULL;
				newEntry->value = NULL;
				newEntry->next = NULL;
				
				newEntry->key = rt->CreateValue();
				rt->CopyValue( iterEntry->key, newEntry->key );
				
				newEntry->value = rt->CreateValue();
				rt->CopyValue( iterEntry->value, newEntry->value );
				
				if( lastEntry ){
					lastEntry->next = newEntry;
					
				}else{
					nd.buckets[ i ] = newEntry;
				}
				lastEntry = newEntry;
				
				iterEntry = iterEntry->next;
			}
		}
		
	}catch( ... ){
		if( newEntry ){
			if( newEntry->key ){
				rt->FreeValue( newEntry->key );
				newEntry->key = NULL;
			}
			if( newEntry->value ){
				rt->FreeValue( newEntry->value );
				newEntry->value = NULL;
			}
			delete newEntry;
		}
		throw;
	}
}

// public func destructor()
dsClassDictionary::nfDestructor::nfDestructor( const sInitData &init ) : dsFunction( init.clsDict,
"destructor", DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassDictionary::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	
	if( nd.buckets ){
		sDictEntry *iterEntry;
		int i;
		
		for( i=0; i<nd.bucketCount; i++ ){
			if( nd.buckets[ i ] ){
				iterEntry = nd.buckets[ i ];
				nd.buckets[ i ] = NULL;
				
				while( iterEntry ){
					if( iterEntry->key ){
						rt->FreeValue( iterEntry->key );
						iterEntry->key = NULL;
					}
					if( iterEntry->value ){
						rt->FreeValue( iterEntry->value );
						iterEntry->value = NULL;
					}
					
					sDictEntry * const delbucket = iterEntry;
					iterEntry = iterEntry->next;
					delete delbucket;
				}
			}
		}
		
		delete [] nd.buckets;
	}
	
	nd.buckets = NULL;
	nd.bucketCount = 0;
	nd.entryCount = 0;
	nd.lockModify = 0;
}



// public func int getCount()
dsClassDictionary::nfGetCount::nfGetCount( const sInitData &init ) : dsFunction( init.clsDict,
"getCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassDictionary::nfGetCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	rt->PushInt( nd.entryCount );
}

// public func bool has( Object key )
dsClassDictionary::nfHas::nfHas( const sInitData &init ) : dsFunction( init.clsDict,
"has", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // key
}
void dsClassDictionary::nfHas::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	
	rt->PushBool( clsDict.GetValue( rt, myself->GetRealObject(), rt->GetValue( 0 ), NULL ) );
}

// public func Object getAt( Object key )
dsClassDictionary::nfGetAt::nfGetAt( const sInitData &init ) : dsFunction( init.clsDict,
"getAt", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // key
}
void dsClassDictionary::nfGetAt::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	dsValue * value = NULL;
	
	if( ! clsDict.GetValue( rt, myself->GetRealObject(), rt->GetValue( 0 ), &value ) ){
		DSTHROW_INFO_FMT( dueInvalidParam, "key absent: %s", ErrorValueInfo( *rt, rt->GetValue( 0 ) ).Pointer() );
	}
	rt->PushValue( value );
}

// public func Object getAt( Object key, Object valueIfMissing )
dsClassDictionary::nfGetAt2::nfGetAt2( const sInitData &init ) : dsFunction( init.clsDict,
"getAt", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // key
	p_AddParameter( init.clsObj ); // valueIfMissing
}
void dsClassDictionary::nfGetAt2::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	dsValue * value = NULL;
	
	if( clsDict.GetValue( rt, myself->GetRealObject(), rt->GetValue( 0 ), &value ) ){
		rt->PushValue( value );
		
	}else{
		rt->PushValue( rt->GetValue( 1 ) );
	}
}

// public func void setAt( Object key, Object value )
dsClassDictionary::nfSetAt::nfSetAt( const sInitData &init ) : dsFunction( init.clsDict,
"setAt", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsObj ); // key
	p_AddParameter( init.clsObj ); // value
}
void dsClassDictionary::nfSetAt::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	
	clsDict.SetValue( rt, myself->GetRealObject(), rt->GetValue( 0 ), rt->GetValue( 1 ) );
}

// public func void setAll( Dictionary dictionary )
dsClassDictionary::nfSetAll::nfSetAll( const sInitData &init ) :
dsFunction( init.clsDict, "setAll", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsDict ); // dictionary
}
void dsClassDictionary::nfSetAll::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	
	dsValue * const dict = rt->GetValue( 0 );
	if( ! dict->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "dictionary" );
	}
	if( dict == myself ){
		return;
	}
	
	const sDictNatData &nd2 = *( ( sDictNatData* )p_GetNativeData( dict ) );
	sDictEntry *entry;
	int i;
	for( i=0; i<nd2.bucketCount; i++ ){
		entry = nd2.buckets[ i ];
		while( entry ){
			clsDict.SetValue( rt, myself->GetRealObject(), entry->key, entry->value );
			entry = entry->next;
		}
	}
}

// public func void remove( Object key )
dsClassDictionary::nfRemove::nfRemove( const sInitData &init ) : dsFunction( init.clsDict,
"remove", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsObj ); // key
}
void dsClassDictionary::nfRemove::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	
	if( ! clsDict.RemoveKey( rt, myself->GetRealObject(), rt->GetValue( 0 ) ) ){
		DSTHROW_INFO_FMT( dueInvalidParam, "key absent: %s", ErrorValueInfo( *rt, rt->GetValue( 0 ) ).Pointer() );
	}
}

// public func void removeIfExisting( Object key )
dsClassDictionary::nfRemoveIfExisting::nfRemoveIfExisting( const sInitData &init ) : dsFunction( init.clsDict,
"removeIfExisting", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsObj ); // key
}
void dsClassDictionary::nfRemoveIfExisting::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	
	clsDict.RemoveKey( rt, myself->GetRealObject(), rt->GetValue( 0 ) );
}

// public func void removeAll()
dsClassDictionary::nfRemoveAll::nfRemoveAll( const sInitData &init ) : dsFunction( init.clsDict,
"removeAll", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassDictionary::nfRemoveAll::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	
	clsDict.RemoveAllKeys( rt, myself->GetRealObject() );
}



// public func Array getKeys()
dsClassDictionary::nfGetKeys::nfGetKeys( const sInitData &init ) : dsFunction( init.clsDict,
"getKeys", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
}
void dsClassDictionary::nfGetKeys::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsClassArray &clsArray = *( ( dsClassArray* )rt->GetEngine()->GetClassArray() );
	dsValue *array = NULL;
	sDictEntry *iterEntry;
	int i;
	
	try{
		array = clsArray.CreateArray( rt );
		
		for( i=0; i<nd.bucketCount; i++ ){
			iterEntry = nd.buckets[ i ];
			while( iterEntry ){
				clsArray.AddObject( rt, array->GetRealObject(), iterEntry->key );
				iterEntry = iterEntry->next;
			}
		}
		
		rt->PushValue( array );
		rt->FreeValue( array );
		
	}catch( ... ){
		if( array ){
			rt->FreeValue( array );
		}
		throw;
	}
}

// public func Array getValues()
dsClassDictionary::nfGetValues::nfGetValues( const sInitData &init ) : dsFunction( init.clsDict,
"getValues", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
}
void dsClassDictionary::nfGetValues::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsClassArray &clsArray = *( ( dsClassArray* )rt->GetEngine()->GetClassArray() );
	dsValue *array = NULL;
	sDictEntry *iterEntry;
	int i;
	
	try{
		array = clsArray.CreateArray( rt );
		
		for( i=0; i<nd.bucketCount; i++ ){
			iterEntry = nd.buckets[ i ];
			while( iterEntry ){
				clsArray.AddObject( rt, array->GetRealObject(), iterEntry->value );
				iterEntry = iterEntry->next;
			}
		}
		
		rt->PushValue( array );
		rt->FreeValue( array );
		
	}catch( ... ){
		if( array ){
			rt->FreeValue( array );
		}
		throw;
	}
}



// public func void forEach( Block ablock )
dsClassDictionary::nfForEach::nfForEach( const sInitData &init ) : dsFunction( init.clsDict,
"forEach", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfForEach::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	if( nd.entryCount == 0 ){
		return;
	}
	
	sDictEntry *iterEntry;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			rt->PushValue( iterEntry->key );
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( key, value )
			iterEntry = iterEntry->next;
		}
	}
}

// public func void forEachKey( Block ablock )
dsClassDictionary::nfForEachKey::nfForEachKey( const sInitData &init ) : dsFunction( init.clsDict,
"forEachKey", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfForEachKey::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	if( nd.entryCount == 0 ){
		return;
	}
	
	sDictEntry *iterEntry;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		while( iterEntry ){
			rt->PushValue( iterEntry->key );
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( key )
			iterEntry = iterEntry->next;
		}
	}
}

// public func void forEachValue( Block ablock )
dsClassDictionary::nfForEachValue::nfForEachValue( const sInitData &init ) : dsFunction( init.clsDict,
"forEachValue", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfForEachValue::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	if( nd.entryCount == 0 ){
		return;
	}
	
	sDictEntry *iterEntry;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( value )
			iterEntry = iterEntry->next;
		}
	}
}

// public func Object find( Block ablock )
dsClassDictionary::nfFind::nfFind( const sInitData &init ) : dsFunction( init.clsDict,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfFind::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	if( nd.entryCount == 0 ){
		rt->PushObject( NULL );
		return;
	}
	
	sDictEntry *iterEntry;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			rt->PushValue( iterEntry->key );
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( key, value )
			
			if( rt->GetReturnBool() ){
				rt->PushValue( iterEntry->value );
				return;
			}
			iterEntry = iterEntry->next;
		}
	}
	
	rt->PushObject( NULL );
}

// public func Object findKey( Block ablock )
dsClassDictionary::nfFindKey::nfFindKey( const sInitData &init ) : dsFunction( init.clsDict,
"findKey", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfFindKey::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	if( nd.entryCount == 0 ){
		rt->PushObject( NULL );
		return;
	}
	
	sDictEntry *iterEntry;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			rt->PushValue( iterEntry->key );
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( key, value )
			
			if( rt->GetReturnBool() ){
				rt->PushValue( iterEntry->key );
				return;
			}
			iterEntry = iterEntry->next;
		}
	}
	
	rt->PushObject( NULL );
}

// public func Dictionary map( Block ablock )
dsClassDictionary::nfMap::nfMap( const sInitData &init ) : dsFunction( init.clsDict,
"map", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsDict ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfMap::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsClassDictionary * const clsDict = ( dsClassDictionary* )GetOwnerClass();
	sDictEntry *newEntry = NULL;
	sDictEntry *iterEntry;
	sDictEntry *lastEntry;
	dsValue *dict = NULL;
	sDictNatData *ndnew;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	try{
		dict = rt->CreateValue( clsDict );
		rt->CreateObjectNaked( dict, clsDict );
		ndnew = ( sDictNatData* )p_GetNativeData( dict );
		ndnew->buckets = NULL;
		ndnew->bucketCount = nd.bucketCount;
		ndnew->entryCount = nd.entryCount;
		ndnew->lockModify = 0;
		
		ndnew->buckets = new sDictEntry*[ nd.bucketCount ];
		for( i=0; i<nd.bucketCount; i++ ){
			ndnew->buckets[ i ] = NULL;
		}
		
		for( i=0; i<nd.bucketCount; i++ ){
			iterEntry = nd.buckets[ i ];
			lastEntry = NULL;
			newEntry = NULL;
			
			while( iterEntry ){
				rt->PushValue( iterEntry->value );
				rt->PushValue( iterEntry->key );
				rt->RunFunctionFast( block, funcIndexRun ); // Object run( key, value )
				
				newEntry = new sDictEntry;
				
				newEntry->hash = iterEntry->hash;
				newEntry->key = NULL;
				newEntry->value = NULL;
				newEntry->next = NULL;
				
				newEntry->key = rt->CreateValue();
				rt->CopyValue( iterEntry->key, newEntry->key );
				
				newEntry->value = rt->CreateValue();
				rt->CopyValue( rt->GetReturnValue(), newEntry->value );
				
				if( lastEntry ){
					lastEntry->next = newEntry;
					
				}else{
					ndnew->buckets[ i ] = newEntry;
				}
				lastEntry = newEntry;
				newEntry = NULL;
				
				iterEntry = iterEntry->next;
			}
		}
		
		rt->PushValue( dict );
		rt->FreeValue( dict );
		
	}catch( ... ){
		if( newEntry ){
			if( newEntry->key ){
				rt->FreeValue( newEntry->key );
				newEntry->key = NULL;
			}
			if( newEntry->value ){
				rt->FreeValue( newEntry->value );
				newEntry->value = NULL;
			}
			delete newEntry;
		}
		if( dict ){
			rt->FreeValue( dict );
		}
		throw;
	}
}

// public func Dictionary collect( Block ablock )
dsClassDictionary::nfCollect::nfCollect( const sInitData &init ) : dsFunction( init.clsDict,
"collect", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsDict ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfCollect::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsClassDictionary * const clsDict = ( dsClassDictionary* )GetOwnerClass();
	sDictEntry *newEntry = NULL;
	sDictEntry *iterEntry;
	sDictEntry *lastEntry;
	dsValue *dict = NULL;
	sDictNatData *ndnew;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	try{
		dict = rt->CreateValue( clsDict );
		rt->CreateObjectNaked( dict, clsDict );
		ndnew = ( sDictNatData* )p_GetNativeData( dict );
		ndnew->buckets = NULL;
		ndnew->bucketCount = nd.bucketCount;
		ndnew->entryCount = 0;
		ndnew->lockModify = 0;
		
		ndnew->buckets = new sDictEntry*[ nd.bucketCount ];
		for( i=0; i<nd.bucketCount; i++ ){
			ndnew->buckets[ i ] = NULL;
		}
		
		for( i=0; i<nd.bucketCount; i++ ){
			iterEntry = nd.buckets[ i ];
			lastEntry = NULL;
			newEntry = NULL;
			
			while( iterEntry ){
				rt->PushValue( iterEntry->value );
				rt->PushValue( iterEntry->key );
				rt->RunFunctionFast( block, funcIndexRun ); // Object run( key, value )
				
				if( rt->GetReturnBool() ){
					newEntry = new sDictEntry;
					
					newEntry->hash = iterEntry->hash;
					newEntry->key = NULL;
					newEntry->value = NULL;
					newEntry->next = NULL;
					
					newEntry->key = rt->CreateValue();
					rt->CopyValue( iterEntry->key, newEntry->key );
					
					newEntry->value = rt->CreateValue();
					rt->CopyValue( iterEntry->value, newEntry->value );
					
					if( lastEntry ){
						lastEntry->next = newEntry;
						
					}else{
						ndnew->buckets[ i ] = newEntry;
					}
					lastEntry = newEntry;
					newEntry = NULL;
					
					ndnew->entryCount++;
				}
				
				iterEntry = iterEntry->next;
			}
		}
		
		rt->PushValue( dict );
		rt->FreeValue( dict );
		
	}catch( ... ){
		if( newEntry ){
			if( newEntry->key ){
				rt->FreeValue( newEntry->key );
				newEntry->key = NULL;
			}
			if( newEntry->value ){
				rt->FreeValue( newEntry->value );
				newEntry->value = NULL;
			}
			delete newEntry;
		}
		if( dict ){
			rt->FreeValue( dict );
		}
		throw;
	}
}

// public func int count( Block ablock )
dsClassDictionary::nfCount::nfCount( const sInitData &init ) : dsFunction( init.clsDict,
"count", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	if( nd.entryCount == 0 ){
		rt->PushInt( 0 );
		return;
	}
	
	sDictEntry *iterEntry;
	int count = 0;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			rt->PushValue( iterEntry->key );
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( key, value )
			
			if( rt->GetReturnBool() ){
				count++;
			}
			
			iterEntry = iterEntry->next;
		}
	}
	
	rt->PushInt( count );
}

// public func void removeIf( Block ablock )
dsClassDictionary::nfRemoveIf::nfRemoveIf( const sInitData &init ) : dsFunction( init.clsDict,
"removeIf", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfRemoveIf::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	if( nd.entryCount == 0 ){
		return;
	}
	
	sDictEntry *iterEntry;
	sDictEntry *lastEntry;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		lastEntry = NULL;
		
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			rt->PushValue( iterEntry->key );
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( key, value )
			
			if( rt->GetReturnBool() ){
				if( lastEntry ){
					lastEntry->next = iterEntry->next;
					
				}else{
					nd.buckets[ i ] = iterEntry->next;
				}
				nd.entryCount--;
				
				sDictEntry * const delentry = iterEntry;
				iterEntry = iterEntry->next;
				
				if( delentry->key ){
					rt->FreeValue( delentry->key );
					delentry->key = NULL;
				}
				if( delentry->value ){
					rt->FreeValue( delentry->value );
					delentry->value = NULL;
				}
				delete delentry;
				
			}else{
				lastEntry = iterEntry;
				iterEntry = iterEntry->next;
			}
		}
	}
}



// public func Object inject( Object injectValue, Block ablock )
dsClassDictionary::nfInject::nfInject( const sInitData &init ) : dsFunction( init.clsDict,
"inject", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfInject::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsValue * const injectValue = rt->GetValue( 0 );
	
	if( nd.entryCount == 0 ){
		rt->PushValue( injectValue );
		return;
	}
	
	dsValue * const block = rt->GetValue( 1 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun3();
	bool firstEntry = true;
	sDictEntry *iterEntry;
	int i;
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			rt->PushValue( iterEntry->key );
			
			if( firstEntry ){
				rt->PushValue( injectValue );
				firstEntry = false;
				
			}else{
				rt->PushReturnValue();
			}
			
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( injectValue, key, value )
			iterEntry = iterEntry->next;
		}
	}
	
	rt->PushReturnValue();
}

// public func Object injectKey( Object injectValue, Block ablock )
dsClassDictionary::nfInjectKey::nfInjectKey( const sInitData &init ) : dsFunction( init.clsDict,
"injectKey", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfInjectKey::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsValue * const injectValue = rt->GetValue( 0 );
	
	if( nd.entryCount == 0 ){
		rt->PushValue( injectValue );
		return;
	}
	
	dsValue * const block = rt->GetValue( 1 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	bool firstEntry = true;
	sDictEntry *iterEntry;
	int i;
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		while( iterEntry ){
			rt->PushValue( iterEntry->key );
			
			if( firstEntry ){
				rt->PushValue( injectValue );
				firstEntry = false;
				
			}else{
				rt->PushReturnValue();
			}
			
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( injectValue, key )
			iterEntry = iterEntry->next;
		}
	}
	
	rt->PushReturnValue();
}

// public func Object injectValue( Object injectValue, Block ablock )
dsClassDictionary::nfInjectValue::nfInjectValue( const sInitData &init ) : dsFunction( init.clsDict,
"injectValue", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassDictionary::nfInjectValue::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsValue * const injectValue = rt->GetValue( 0 );
	
	if( nd.entryCount == 0 ){
		rt->PushValue( injectValue );
		return;
	}
	
	dsValue * const block = rt->GetValue( 1 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	bool firstEntry = true;
	sDictEntry *iterEntry;
	int i;
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		iterEntry = nd.buckets[ i ];
		while( iterEntry ){
			rt->PushValue( iterEntry->value );
			
			if( firstEntry ){
				rt->PushValue( injectValue );
				firstEntry = false;
				
			}else{
				rt->PushReturnValue();
			}
			
			rt->RunFunctionFast( block, funcIndexRun ); // Object run( injectValue, value )
			iterEntry = iterEntry->next;
		}
	}
	
	rt->PushReturnValue();
}



// public func Dictionary +( Dictionary array )
dsClassDictionary::nfOpAdd::nfOpAdd( const sInitData &init ) :
dsFunction( init.clsDict, "+", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsDict ){
	p_AddParameter( init.clsDict ); // array
}
void dsClassDictionary::nfOpAdd::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsClassDictionary &clsDict = *( ( dsClassDictionary* )GetOwnerClass() );
	
	dsValue * const dict = rt->GetValue( 0 );
	if( ! dict->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "dictionary" );
	}
	
	const sDictNatData &nd2 = *( ( sDictNatData* )p_GetNativeData( dict ) );
	
	dsValue * const newDictionary = clsDict.CreateDictionary( rt );
	sDictEntry *entry;
	int i;
	try{
		for( i=0; i<nd.bucketCount; i++ ){
			entry = nd.buckets[ i ];
			while( entry ){
				clsDict.SetValue( rt, newDictionary->GetRealObject(), entry->key, entry->value );
				entry = entry->next;
			}
		}
		for( i=0; i<nd2.bucketCount; i++ ){
			entry = nd2.buckets[ i ];
			while( entry ){
				clsDict.SetValue( rt, newDictionary->GetRealObject(), entry->key, entry->value );
				entry = entry->next;
			}
		}
		
		rt->PushValue( newDictionary );
		rt->FreeValue( newDictionary );
		
	}catch( ... ){
		if( newDictionary ){
			rt->FreeValue( newDictionary );
		}
		throw;
	}
}



// public func bool equals( Object object )
dsClassDictionary::nfEquals::nfEquals( const sInitData &init ) :
dsFunction(init.clsDict, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // object
}
void dsClassDictionary::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	dsClassDictionary * const clsDict = ( dsClassDictionary* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( object->GetType()->GetPrimitiveType() != DSPT_OBJECT || ! object->GetRealObject()
	|| object->GetRealObject()->GetType() != clsDict ){
		rt->PushBool( false );
		return;
	}
	
	const sDictNatData &nd2 = *( ( sDictNatData* )p_GetNativeData( object ) );
	if( nd2.entryCount != nd.entryCount ){
		rt->PushBool( false );
		return;
	}
	
	dsClass * const clsBool = rt->GetEngine()->GetClassBool();
	int b;
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( b=0; b<nd2.bucketCount; b++ ){
		sDictEntry *entry = nd2.buckets[ b ];
		while( entry ){
			dsValue *value = NULL;
			if( ! clsDict->GetValue( rt, myself->GetRealObject(), entry->key, &value ) ){
				rt->PushBool( false );
				return;
			}
			
			rt->PushValue( value );
			rt->RunFunction( entry->value, "equals", 1 );
			
			if( rt->GetReturnValue()->GetType() != clsBool ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
			}
			if( ! rt->GetReturnBool() ){
				rt->PushBool( false );
				return;
			}
			
			entry = entry->next;
		}
	}
	
	rt->PushBool( true );
}

// public func String toString()
dsClassDictionary::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsDict,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassDictionary::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself ) );
	sDictEntry *iterEntry;
	char *buffer = NULL;
	bool first = true;
	int maxLen = 5000;
	int curLen = 0;
	int i;
	
	const int funcIndexToString = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexToString();
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	try{
		buffer = new char[ maxLen + 7 ]; // } + ... + '->' just in case
		buffer[ 0 ] = '{';
		buffer[ 1 ] = '\0';
		curLen = 1;
		
		for( i=0; i<nd.bucketCount; i++ ){
			iterEntry = nd.buckets[ i ];
			while( iterEntry ){
				// key
				rt->RunFunctionFast( iterEntry->key, funcIndexToString ); // String toString()
				const char * const keystr = rt->GetReturnValue()->GetString();
				const int kslen = strlen( keystr );
				
				if( first ){
					first = false;
					
				}else{
					buffer[ curLen++ ] = ',';
					buffer[ curLen ] = '\0';
				}
				
				if( curLen + kslen > maxLen ){
					strcat( buffer, "..." );
					curLen += 3;
					break;
					
				}else{
					strcat( buffer, keystr );
					curLen += kslen;
				}
				
				// ->
				strcat( buffer, "->" );
				curLen += 2;
				
				// value
				const char *valuestr;
				
				if( iterEntry->value->GetRealType()->GetPrimitiveType() != DSPT_OBJECT
				|| iterEntry->value->GetRealObject() ){
					rt->RunFunctionFast( iterEntry->value, funcIndexToString ); // String toString()
					valuestr = rt->GetReturnValue()->GetString();
					
				}else{
					valuestr = "null";
				}
				
				const int vslen = strlen( valuestr );
				
				if( curLen + vslen > maxLen ){
					strcat( buffer, "..." );
					curLen += 3;
					break;
					
				}else{
					strcat( buffer, valuestr );
					curLen += vslen;
				}
				
				// next
				iterEntry = iterEntry->next;
			}
			
			if( iterEntry ){
				break;
			}
		}
		
		buffer[ curLen ] = '}';
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



// Class dsClassDictionary
////////////////////////////

// Constructor, destructor
////////////////////////////

dsClassDictionary::dsClassDictionary() : dsClass( "Dictionary", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sDictNatData ) );
}

dsClassDictionary::~dsClassDictionary(){
}


// Management
///////////////

void dsClassDictionary::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	// store classes
	init.clsDict = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsObj = engine->GetClassObject();
	init.clsBool = engine->GetClassBool();
	init.clsBlock = engine->GetClassBlock();
	init.clsStr = engine->GetClassString();
	init.clsArr = engine->GetClassArray();
	
	// functions
	AddFunction( new nfCreate( init ) );
	AddFunction( new nfCopy( init ) );
	AddFunction( new nfDestructor( init ) );
	
	AddFunction( new nfGetCount( init ) );
	AddFunction( new nfHas( init ) );
	AddFunction( new nfGetAt( init ) );
	AddFunction( new nfGetAt2( init ) );
	AddFunction( new nfSetAt( init ) );
	AddFunction( new nfSetAll( init ) );
	AddFunction( new nfRemove( init ) );
	AddFunction( new nfRemoveIfExisting( init ) );
	AddFunction( new nfRemoveAll( init ) );
	
	AddFunction( new nfGetKeys( init ) );
	AddFunction( new nfGetValues( init ) );
	
	AddFunction( new nfForEach( init ) );
	AddFunction( new nfForEachKey( init ) );
	AddFunction( new nfForEachValue( init ) );
	AddFunction( new nfFind( init ) );
	AddFunction( new nfFindKey( init ) );
	AddFunction( new nfMap( init ) );
	AddFunction( new nfCollect( init ) );
	AddFunction( new nfCount( init ) );
	AddFunction( new nfRemoveIf( init ) );
	AddFunction( new nfInject( init ) );
	AddFunction( new nfInjectKey( init ) );
	AddFunction( new nfInjectValue( init ) );
	
	AddFunction( new nfOpAdd( init ) );
	
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfToString( init ) );
}



dsValue *dsClassDictionary::CreateDictionary( dsRunTime *rt ){
	dsValue *newDict = NULL;
	sDictNatData *nd;
	int i;
	
	try{
		newDict = rt->CreateValue( this );
		rt->CreateObjectNaked( newDict, this );
		nd = ( sDictNatData* )p_GetNativeData( newDict->GetRealObject()->GetBuffer() );
		nd->buckets = NULL;
		nd->bucketCount = 8;
		nd->entryCount = 0;
		nd->lockModify = 0;
		
		nd->buckets = new sDictEntry*[ nd->bucketCount ];
		for( i=0; i<nd->bucketCount; i++ ){
			nd->buckets[ i ] = NULL;
		}
		
	}catch( ... ){
		if( newDict ){
			rt->FreeValue( newDict );
		}
		throw;
	}
	
	return newDict;
}

bool dsClassDictionary::GetValue( dsRunTime *rt, dsRealObject *myself, dsValue *key, dsValue **value ){
	if( ! rt || ! myself || ! key ){
		DSTHROW( dueNullPointer );
	}
	if( key->GetRealType()->GetPrimitiveType() == DSPT_OBJECT && ! key->GetRealObject() ){
		DSTHROW( dueNullPointer );
	}
	
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself->GetBuffer() ) );
	
	const int funcIndexHashCode = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexHashCode();
	rt->RunFunctionFast( key, funcIndexHashCode ); // int hashCode()
	const unsigned int hash = ( unsigned int )rt->GetReturnInt();
	
	const int funcIndexEquals = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexEquals();
	sDictEntry *iterEntry = nd.buckets[ hash % nd.bucketCount ];
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	while( iterEntry ){
		if( iterEntry->hash == hash ){
			rt->PushValue( key );
			rt->RunFunctionFast( iterEntry->key, funcIndexEquals ); // bool equals( key )
			if( rt->GetReturnBool() ){
				if( value ){
					*value = iterEntry->value;
				}
				return true;
			}
		}
		iterEntry = iterEntry->next;
	}
	
	return false;
}

void dsClassDictionary::SetValue( dsRunTime *rt, dsRealObject *myself, dsValue *key, dsValue *value ){
	if( ! rt || ! myself || ! key || ! value ){
		DSTHROW( dueNullPointer );
	}
	if( key->GetRealType()->GetPrimitiveType() == DSPT_OBJECT && ! key->GetRealObject() ){
		DSTHROW( dueNullPointer );
	}
	
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself->GetBuffer() ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	const int funcIndexHashCode = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexHashCode();
	rt->RunFunctionFast( key, funcIndexHashCode ); // int hashCode()
	const unsigned int hash = ( unsigned int )rt->GetReturnInt();
	const int bucketIndex = hash % nd.bucketCount;
	
	const int funcIndexEquals = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexEquals();
	sDictEntry *iterEntry = nd.buckets[ bucketIndex ];
	sDictEntry *lastEntry = NULL;
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	while( iterEntry ){
		if( iterEntry->hash == hash ){
			rt->PushValue( key );
			rt->RunFunctionFast( iterEntry->key, funcIndexEquals ); // bool equals( key )
			if( rt->GetReturnBool() ){
				rt->CopyValue( value, iterEntry->value );
				return;
			}
		}
		lastEntry = iterEntry;
		iterEntry = iterEntry->next;
	}
	
	sDictEntry *newEntry = NULL;
	try{
		newEntry = new sDictEntry;
		
		newEntry->hash = hash;
		newEntry->key = NULL;
		newEntry->value = NULL;
		newEntry->next = NULL;
		
		newEntry->key = rt->CreateValue();
		rt->CopyValue( key, newEntry->key );
		
		newEntry->value = rt->CreateValue();
		rt->CopyValue( value, newEntry->value );
		
		if( lastEntry ){
			lastEntry->next = newEntry;
			
		}else{
			nd.buckets[ bucketIndex ] = newEntry;
		}
		
		newEntry = NULL;
		nd.entryCount++;
		CheckLoad( rt, myself );
		
	}catch( ... ){
		if( newEntry ){
			if( newEntry->key ){
				rt->FreeValue( newEntry->key );
				newEntry->key = NULL;
			}
			if( newEntry->value ){
				rt->FreeValue( newEntry->value );
				newEntry->value = NULL;
			}
			delete newEntry;
		}
		throw;
	}
}

bool dsClassDictionary::RemoveKey( dsRunTime *rt, dsRealObject *myself, dsValue *key ){
	if( ! rt || ! myself || ! key ){
		DSTHROW( dueNullPointer );
	}
	if( key->GetRealType()->GetPrimitiveType() == DSPT_OBJECT && ! key->GetRealObject() ){
		DSTHROW( dueNullPointer );
	}
	
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself->GetBuffer() ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	const int funcIndexHashCode = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexHashCode();
	rt->RunFunctionFast( key, funcIndexHashCode ); // int hashCode()
	const unsigned int hash = ( unsigned int )rt->GetReturnInt();
	const int bucketIndex = hash % nd.bucketCount;
	
	const int funcIndexEquals = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexEquals();
	sDictEntry *iterEntry = nd.buckets[ bucketIndex ];
	sDictEntry *lastEntry = NULL;
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	while( iterEntry ){
		if( iterEntry->hash == hash ){
			rt->PushValue( key );
			rt->RunFunctionFast( iterEntry->key, funcIndexEquals ); // bool equals( key )
			if( rt->GetReturnBool() ){
				if( lastEntry ){
					lastEntry->next = iterEntry->next;
					
				}else{
					nd.buckets[ bucketIndex ] = iterEntry->next;
				}
				
				nd.entryCount--;
				
				if( iterEntry->key ){
					rt->FreeValue( iterEntry->key );
					iterEntry->key = NULL;
				}
				if( iterEntry->value ){
					rt->FreeValue( iterEntry->value );
					iterEntry->value = NULL;
				}
				delete iterEntry;
				
				return true;
			}
		}
		lastEntry = iterEntry;
		iterEntry = iterEntry->next;
	}
	
	return false;
}

void dsClassDictionary::RemoveAllKeys( dsRunTime *rt, dsRealObject *myself ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself->GetBuffer() ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	if( ! nd.buckets ){
		return;
	}
	
	sDictEntry *iterEntry;
	int i;
	
	cDictNatDatLockModifyGuard lock( nd.lockModify );
	for( i=0; i<nd.bucketCount; i++ ){
		if( nd.buckets[ i ] ){
			iterEntry = nd.buckets[ i ];
			nd.buckets[ i ] = NULL;
			
			while( iterEntry ){
				if( iterEntry->key ){
					rt->FreeValue( iterEntry->key );
					iterEntry->key = NULL;
				}
				if( iterEntry->value ){
					rt->FreeValue( iterEntry->value );
					iterEntry->value = NULL;
				}
				
				sDictEntry * const delbucket = iterEntry;
				iterEntry = iterEntry->next;
				delete delbucket;
			}
		}
	}
	nd.entryCount = 0;
}

void dsClassDictionary::CheckLoad( dsRunTime *rt, dsRealObject *myself ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sDictNatData &nd = *( ( sDictNatData* )p_GetNativeData( myself->GetBuffer() ) );
	
	if( ( float )nd.entryCount / ( float )nd.bucketCount > 0.7 ){
		const int newBucketCount = nd.bucketCount + ( nd.bucketCount >> 1 ); // +50%
		sDictEntry ** const newBuckets = new sDictEntry*[ newBucketCount ];
		int i;
		
		if( ! newBuckets ){
			DSTHROW( dueInvalidParam );
		}
		for( i=0; i<newBucketCount; i++ ){
			newBuckets[ i ] = NULL;
		}
		
		for( i=0; i<nd.bucketCount; i++ ){
			sDictEntry *iterEntry = nd.buckets[ i ];
			
			while( iterEntry ){
				sDictEntry * const moveEntry = iterEntry;
				iterEntry = iterEntry->next;
				
				const int bucketIndex = ( moveEntry->hash % newBucketCount );
				sDictEntry *iterEntry2 = newBuckets[ bucketIndex ];
				
				if( iterEntry2 ){
					while( iterEntry2->next ){
						iterEntry2 = iterEntry2->next;
					}
					iterEntry2->next = moveEntry;
					
				}else{
					newBuckets[ bucketIndex ] = moveEntry;
				}
				
				moveEntry->next = NULL;
			}
		}
		
		//printf( "Dictionary grows from %i buckets to %i buckets\n", nd.bucketCount, newBucketCount );
		delete [] nd.buckets;
		nd.buckets = newBuckets;
		nd.bucketCount = newBucketCount;
	}
}
