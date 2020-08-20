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



// includes
#include <stdio.h>
#include <string.h>
#include "../../../config.h"
#include "dsClassArray.h"
#include "dsClassObject.h"
#include "dsClassBlock.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

static const char * const errorModifyWhileLocked = "modify while locked";

// native data structure
class cArrNatDatLockModifyGuard{
	int &pLockModify;
	
public:
	cArrNatDatLockModifyGuard( int &lockModify ) : pLockModify( lockModify ){
		pLockModify++;
	}
	
	~cArrNatDatLockModifyGuard(){
		pLockModify--;
	}
};

struct sArrNatData{
	dsValue **values;
	int count;
	int size;
	int lockModify;
	
	// helper functions
	void SetSize(int newSize, dsRunTime *rt, dsClass *clsObj){
		int i, oldSize=size;
		dsValue **vNewArray;
		// only growing allowed
		if(newSize > oldSize){
			if(!(vNewArray = new dsValue*[newSize])) DSTHROW(dueOutOfMemory);
			for(i=0; i<oldSize; i++) vNewArray[i] = values[i];
			if(values) delete [] values;
			values = vNewArray;
			for(i=oldSize; i<newSize; i++){
				values[i] = rt->CreateValue(clsObj);
				if(!values[i]) DSTHROW(dueOutOfMemory);
				size++;
			}
		}
	}
	
	// sort using quick-sort
	void Sort( dsRunTime *rt ){
		if( count == 0 ){
			return;
		}
		
		const int funcIndexCompare = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexCompare();
		pSortAscending( rt, funcIndexCompare, 0, count - 1 );
	}
	
	void pSortAscending( dsRunTime *rt, int funcIndexCompare, int left, int right ){
		const int r_hold = right;
		const int l_hold = left;
		
		rt->PushValue( values[ left ] ); // put pivot on stack
		dsValue * const pivot = rt->GetValue( 0 );
		
		try{
			while( left < right ){
				// while ( values[right] >= pivot ) && ( left < right )
				while( left < right ){
					// if values[right] < pivot then break
					rt->PushValue( pivot );
					rt->RunFunctionFast( values[ right ], funcIndexCompare ); // int objPrev.compare( objNext )
					if( rt->GetReturnInt() < 0 ){
						break;
					}
					
					right--;
				}
				if( left != right ){
					rt->CopyValue( values[ right ], values[ left ] );
					left++;
				}
				
				// while( ( *pStrings[ left ] <= *pivot ) && ( left < right ) )
				while( left < right ){
					// if values[left] > pivot then break
					rt->PushValue( pivot );
					rt->RunFunctionFast( values[ left ], funcIndexCompare ); // int objPrev.compare( objNext )
					if( rt->GetReturnInt() > 0 ){
						break;
					}
					
					left++;
				}
				if( left != right ){
					rt->CopyValue( values[ left ], values[ right ] );
					right--;
				}
			}
			
			rt->CopyValue( pivot, values[ left ] );
			rt->RemoveValues( 1 ); // remove pivot from stack
			
		}catch( ... ){
			rt->RemoveValues( 1 ); // remove pivot from stack
			throw;
		}
		
		if( l_hold < left ){
			pSortAscending( rt, funcIndexCompare, l_hold, left - 1 );
		}
		if( r_hold > left ){
			pSortAscending( rt, funcIndexCompare, left + 1, r_hold );
		}
	}
	
	void Sort( dsRunTime *rt, dsValue *block ){
		if( count == 0 ){
			return;
		}
		
		const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
		pSortAscending( rt, block, funcIndexRun, 0, count - 1 );
	}
	
	void pSortAscending( dsRunTime *rt, dsValue *block, int funcIndexRun, int left, int right ){
		dsClass * const clsInt = rt->GetEngine()->GetClassInt();
		const int r_hold = right;
		const int l_hold = left;
		
		rt->PushValue( values[ left ] ); // put pivot on stack
		dsValue * const pivot = rt->GetValue( 0 );
		
		try{
			while( left < right ){
				// while ( values[right] >= pivot ) && ( left < right )
				while( left < right ){
					// if values[right] < pivot then break
					rt->PushValue( pivot );
					rt->PushValue( values[ right ] );
					rt->RunFunctionFast( block, funcIndexRun ); // Object run( objPrev, objNext )
					if( rt->GetReturnValue()->GetType() != clsInt ){
						DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsInt ) );
					}
					if( rt->GetReturnInt() < 0 ){
						break;
					}
					
					right--;
				}
				if( left != right ){
					rt->CopyValue( values[ right ], values[ left ] );
					left++;
				}
				
				// while( ( *pStrings[ left ] <= *pivot ) && ( left < right ) )
				while( left < right ){
					// if values[left] > pivot then break
					rt->PushValue( pivot );
					rt->PushValue( values[ left ] );
					rt->RunFunctionFast( block, funcIndexRun ); // Object run( objPrev, objNext )
					if( rt->GetReturnValue()->GetType() != clsInt ){
						DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsInt ) );
					}
					if( rt->GetReturnInt() > 0 ){
						break;
					}
					
					left++;
				}
				if( left != right ){
					rt->CopyValue( values[ left ], values[ right ] );
					right--;
				}
			}
			
			rt->CopyValue( pivot, values[ left ] );
			rt->RemoveValues( 1 ); // remove pivot from stack
			
		}catch( ... ){
			rt->RemoveValues( 1 ); // remove pivot from stack
			throw;
		}
		
		if( l_hold < left ){
			pSortAscending( rt, block, funcIndexRun, l_hold, left - 1 );
		}
		if( r_hold > left ){
			pSortAscending( rt, block, funcIndexRun, left + 1, r_hold );
		}
	}
};

// native functions
/////////////////////
// constructor Create()
dsClassArray::nfCreate::nfCreate(const sInitData &init) : dsFunction(init.clsArr,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassArray::nfCreate::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	nd->values = NULL;
	nd->count = 0;
	nd->size = 0;
	nd->lockModify = 0;
}

// constructor CreateSize(int Size)
dsClassArray::nfCreateSize::nfCreateSize(const sInitData &init) : dsFunction(init.clsArr,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // Size
}
void dsClassArray::nfCreateSize::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	dsClass *clsObj = ((dsClassArray*)GetOwnerClass())->GetClassObject();
	nd->values = NULL;
	nd->count = 0;
	nd->size = 0;
	nd->lockModify = 0;
	int i, vSize = rt->GetValue(0)->GetInt();
	if(vSize > 0){
		nd->values = new dsValue*[vSize];
		if(!nd->values) DSTHROW(dueOutOfMemory);
		for(i=0; i<vSize; i++){
			nd->values[i] = rt->CreateValue(clsObj);
			if(!nd->values[i]) DSTHROW(dueOutOfMemory);
			nd->size++;
		}
	}else if(vSize < 0){
		DSTHROW_INFO_FMT(dueInvalidParam, "size(%d) < 0", vSize);
	}
}

// constructor CreateCount(int count, Object element)
dsClassArray::nfCreateCount::nfCreateCount(const sInitData &init) : dsFunction(init.clsArr,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // count
	p_AddParameter(init.clsObj); // element
}
void dsClassArray::nfCreateCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	dsClass *clsObj = ((dsClassArray*)GetOwnerClass())->GetClassObject();
	nd->values = NULL;
	nd->count = 0;
	nd->size = 0;
	nd->lockModify = 0;
	int i, count = rt->GetValue(0)->GetInt();
	dsValue *vObj = rt->GetValue(1);
	if(count > 0){
		nd->values = new dsValue*[count];
		if(!nd->values) DSTHROW(dueOutOfMemory);
		for(i=0; i<count; i++){
			nd->values[i] = rt->CreateValue(clsObj);
			if(!nd->values[i]) DSTHROW(dueOutOfMemory);
			nd->size++;
			rt->CopyValue(vObj,nd->values[i]);
		}
		nd->count = nd->size;
	}else if(count < 0){
		DSTHROW_INFO_FMT(dueInvalidParam, "count(%d) < 0", count);
	}
}

// constructor Copy(array arr)
dsClassArray::nfCopy::nfCopy(const sInitData &init) : dsFunction(init.clsArr,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsArr); // arr
}
void dsClassArray::nfCopy::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	dsClass *clsObj = ((dsClassArray*)GetOwnerClass())->GetClassObject();
	nd->values = NULL;
	nd->count = 0;
	nd->size = 0;
	nd->lockModify = 0;
	dsValue *vObj = rt->GetValue(0);
	sArrNatData *nd2 = (sArrNatData*)p_GetNativeData(vObj);
	int i, vSize = nd2->count;
	if(vSize > 0){
		nd->values = new dsValue*[vSize];
		if(!nd->values) DSTHROW(dueOutOfMemory);
		for(i=0; i<vSize; i++){
			nd->values[i] = rt->CreateValue(clsObj);
			if(!nd->values[i]) DSTHROW(dueOutOfMemory);
			nd->size++;
			rt->CopyValue(nd2->values[i], nd->values[i]);
		}
		nd->count = nd->size;
	}
}

// destructor Destructor
dsClassArray::nfDestructor::nfDestructor(const sInitData &init) : dsFunction(init.clsArr,
"destructor", DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassArray::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
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



// public static func Array newWith( Object obj )
dsClassArray::nfNewWith1::nfNewWith1(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassArray::nfNewWith1::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 1 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2 )
dsClassArray::nfNewWith2::nfNewWith2(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
}
void dsClassArray::nfNewWith2::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 2 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3 )
dsClassArray::nfNewWith3::nfNewWith3(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
}
void dsClassArray::nfNewWith3::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 3 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3, Object obj4 )
dsClassArray::nfNewWith4::nfNewWith4(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
	p_AddParameter( init.clsObj ); // obj4
}
void dsClassArray::nfNewWith4::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 4 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3, Object obj4, Object obj5 )
dsClassArray::nfNewWith5::nfNewWith5(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
	p_AddParameter( init.clsObj ); // obj4
	p_AddParameter( init.clsObj ); // obj5
}
void dsClassArray::nfNewWith5::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 5 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6 )
dsClassArray::nfNewWith6::nfNewWith6(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
	p_AddParameter( init.clsObj ); // obj4
	p_AddParameter( init.clsObj ); // obj5
	p_AddParameter( init.clsObj ); // obj6
}
void dsClassArray::nfNewWith6::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 6 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7 )
dsClassArray::nfNewWith7::nfNewWith7(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
	p_AddParameter( init.clsObj ); // obj4
	p_AddParameter( init.clsObj ); // obj5
	p_AddParameter( init.clsObj ); // obj6
	p_AddParameter( init.clsObj ); // obj7
}
void dsClassArray::nfNewWith7::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 7 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7, Object obj8 )
dsClassArray::nfNewWith8::nfNewWith8(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
	p_AddParameter( init.clsObj ); // obj4
	p_AddParameter( init.clsObj ); // obj5
	p_AddParameter( init.clsObj ); // obj6
	p_AddParameter( init.clsObj ); // obj7
	p_AddParameter( init.clsObj ); // obj8
}
void dsClassArray::nfNewWith8::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 8 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7, Object obj8, Object obj9 )
dsClassArray::nfNewWith9::nfNewWith9(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
	p_AddParameter( init.clsObj ); // obj4
	p_AddParameter( init.clsObj ); // obj5
	p_AddParameter( init.clsObj ); // obj6
	p_AddParameter( init.clsObj ); // obj7
	p_AddParameter( init.clsObj ); // obj8
	p_AddParameter( init.clsObj ); // obj9
}
void dsClassArray::nfNewWith9::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 9 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public static func Array newWith( Object obj1, Object obj2, Object obj3, Object obj4,
// Object obj5, Object obj6, Object obj7, Object obj8, Object obj9, Object obj10 )
dsClassArray::nfNewWith10::nfNewWith10(const sInitData &init) : dsFunction( init.clsArr,
"newWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsObj ); // obj1
	p_AddParameter( init.clsObj ); // obj2
	p_AddParameter( init.clsObj ); // obj3
	p_AddParameter( init.clsObj ); // obj4
	p_AddParameter( init.clsObj ); // obj5
	p_AddParameter( init.clsObj ); // obj6
	p_AddParameter( init.clsObj ); // obj7
	p_AddParameter( init.clsObj ); // obj8
	p_AddParameter( init.clsObj ); // obj9
	p_AddParameter( init.clsObj ); // obj10
}
void dsClassArray::nfNewWith10::RunFunction( dsRunTime *rt, dsValue* ){
	dsValue * const newArray = ( ( dsClassArray* )GetOwnerClass() )->CreateArray( rt, 10 );
	try{
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}



// function const int Length()
dsClassArray::nfLength::nfLength(const sInitData &init) : dsFunction(init.clsArr, "getCount",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassArray::nfLength::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( ((sArrNatData*)p_GetNativeData(myself))->count );
}

// function const int size()
dsClassArray::nfSize::nfSize(const sInitData &init) : dsFunction(init.clsArr,
"getSize", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassArray::nfSize::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( ((sArrNatData*)p_GetNativeData(myself))->size );
}

// function void setSize(int Size)
dsClassArray::nfSetSize::nfSetSize(const sInitData &init) : dsFunction(init.clsArr,
"setSize", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // Size
}
void dsClassArray::nfSetSize::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	dsClass *clsObj = ((dsClassArray*)GetOwnerClass())->GetClassObject();
	int vSize = rt->GetValue(0)->GetInt();
	if(vSize < nd->count){
		DSTHROW_INFO_FMT(dueInvalidParam, "size(%d) < count(%d)", vSize, nd->count);
	}
	nd->SetSize(vSize, rt, clsObj);
}

// function void Resize(int Size)
dsClassArray::nfResize::nfResize(const sInitData &init) : dsFunction(init.clsArr, "resize",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // Size
}
void dsClassArray::nfResize::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	dsClass *clsObj = ((dsClassArray*)GetOwnerClass())->GetClassObject();
	int i, vSize = rt->GetValue(0)->GetInt();
	if(vSize < 0){
		DSTHROW_INFO_FMT(dueInvalidParam, "size(%d) < 0", vSize);
	}
	if(vSize > nd->count){
		nd->SetSize(vSize, rt, clsObj);
	}else if(vSize < nd->count){
		for(i=vSize; i<nd->count; i++){
			rt->ClearValue(nd->values[i]);
		}
	}
	nd->count = vSize;
}

// function void Add(Object Obj)
dsClassArray::nfAdd::nfAdd(const sInitData &init) : dsFunction(init.clsArr, "add",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObj); // Obj
}
void dsClassArray::nfAdd::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	dsClass *clsObj = ((dsClassArray*)GetOwnerClass())->GetClassObject();
	if(nd->count == nd->size){
		nd->SetSize(nd->size * 3 / 2 + 1, rt, clsObj);
	}
	rt->CopyValue(rt->GetValue(0), nd->values[nd->count]);
	nd->count++;
}

// function void AddAll( Array array )
dsClassArray::nfAddAll::nfAddAll( const sInitData &init ) :
dsFunction( init.clsArr, "addAll", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsArr ); // array
}
void dsClassArray::nfAddAll::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	dsClassArray &clsArray = *( ( dsClassArray* )GetOwnerClass() );
	dsValue * const array = rt->GetValue( 0 );
	if( ! array->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "array" );
	}
	
	const sArrNatData &nd2 = *( ( sArrNatData* )p_GetNativeData( array ) );
	if( nd2.count == 0 ){
		return;
	}
	
	const int requiredCount = nd.count + nd2.count;
	int newSize = nd.size;
	while( requiredCount > newSize ){
		newSize = newSize * 3 / 2 + 1;
	}
	if( newSize > nd.size ){
		nd.SetSize( newSize, rt, clsArray.GetClassObject() );
	}
	
	int i;
	for( i=0; i<nd2.count; i++ ){
		rt->CopyValue( nd2.values[ i ], nd.values[ nd.count + i ] );
	}
	nd.count += nd2.count;
}

// function void Insert(int Position, Object Obj)
dsClassArray::nfInsert::nfInsert(const sInitData &init) : dsFunction(init.clsArr, "insert",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // Position
	p_AddParameter(init.clsObj); // Obj
}
void dsClassArray::nfInsert::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	dsClass *clsObj = ((dsClassArray*)GetOwnerClass())->GetClassObject();
	int i, vPos = rt->GetValue(0)->GetInt();
	if( vPos < 0){
		DSTHROW_INFO_FMT(dueInvalidParam, "position(%d) < 0", vPos);
	}
	if(vPos > nd->count){
		DSTHROW_INFO_FMT(dueInvalidParam, "position(%d) > count(%d)", vPos, nd->count);
	}
	if(nd->count == nd->size){
		nd->SetSize(nd->size * 3 / 2 + 1, rt, clsObj);
	}
	for(i=nd->count; i>vPos; i--){
		rt->MoveValue(nd->values[i-1], nd->values[i]);
	}
	rt->CopyValue(rt->GetValue(1), nd->values[vPos]);
	nd->count++;
}

// function void InsertAll( int position, Array array )
dsClassArray::nfInsertAll::nfInsertAll( const sInitData &init ) :
dsFunction( init.clsArr, "insertAll", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInt ); // position
	p_AddParameter( init.clsArr ); // array
}
void dsClassArray::nfInsertAll::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	const int position = rt->GetValue( 0 )->GetInt();
	if( position < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) < 0", position );
	}
	if( position > nd.count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) > count(%d)", position, nd.count );
	}
	
	dsClassArray &clsArray = *( ( dsClassArray* )GetOwnerClass() );
	dsValue * const array = rt->GetValue( 1 );
	if( ! array->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "array" );
	}
	
	const sArrNatData &nd2 = *( ( sArrNatData* )p_GetNativeData( array ) );
	if( nd2.count == 0 ){
		return;
	}
	
	const int requiredCount = nd.count + nd2.count;
	int newSize = nd.size;
	while( requiredCount > newSize ){
		newSize = newSize * 3 / 2 + 1;
	}
	if( newSize > nd.size ){
		nd.SetSize( newSize, rt, clsArray.GetClassObject() );
	}
	
	const int last = position + nd2.count - 1;
	int i;
	for( i=nd.count + nd2.count - 1; i>last; i-- ){
		rt->MoveValue( nd.values[ i - nd2.count ], nd.values[ i ] );
	}
	for( i=0; i<nd2.count; i++ ){
		rt->CopyValue( nd2.values[ i ], nd.values[ position + i ] );
	}
	nd.count += nd2.count;
}

// const function int indexOf(Object obj)
dsClassArray::nfIndexOf::nfIndexOf(const sInitData &init) : dsFunction(init.clsArr, "indexOf",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
	p_AddParameter(init.clsObj); // obj
}
void dsClassArray::nfIndexOf::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsValue * const testValue = rt->GetValue( 0 );
	int i;
	
	if( testValue->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! testValue->GetRealObject() ){
		// test value is null, special compare to avoid null pointer exception
		for( i=0; i<nd.count; i++ ){
			if( nd.values[ i ]->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! nd.values[ i ]->GetRealObject() ){
				rt->PushInt( i );
				return;
			}
		}
		
	}else{
		const int funcIndexEquals = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexEquals();
		
		for( i=0; i<nd.count; i++ ){
			rt->PushValue( nd.values[ i ] );
			rt->RunFunctionFast( testValue, funcIndexEquals ); // bool equals( key )
			if( rt->GetReturnBool() ){
				rt->PushInt( i );
				return;
			}
		}
	}
	
	rt->PushInt( -1 );
}

// const function bool has( Object obj )
dsClassArray::nfHas::nfHas( const sInitData &init ) : dsFunction( init.clsArr, "has",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassArray::nfHas::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsValue * const testValue = rt->GetValue( 0 );
	int i;
	
	if( testValue->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! testValue->GetRealObject() ){
		// test value is null, special compare to avoid null pointer exception
		for( i=0; i<nd.count; i++ ){
			if( nd.values[ i ]->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! nd.values[ i ]->GetRealObject() ){
				rt->PushBool( true );
				return;
			}
		}
		
	}else{
		const int funcIndexEquals = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexEquals();
		
		for( i=0; i<nd.count; i++ ){
			rt->PushValue( nd.values[ i ] );
			rt->RunFunctionFast( testValue, funcIndexEquals ); // bool equals( key )
			if( rt->GetReturnBool() ){
				rt->PushBool( true );
				return;
			}
		}
	}
	
	rt->PushBool( false );
}

// function void Remove(int Position)
dsClassArray::nfRemove::nfRemove(const sInitData &init) : dsFunction(init.clsArr, "remove",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // Position
}
void dsClassArray::nfRemove::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	int i, vPos = rt->GetValue(0)->GetInt();
	if( vPos < 0 ){
		DSTHROW_INFO_FMT(dueInvalidParam, "position(%d) < 0", vPos);
	}
	if(vPos >= nd->count){
		DSTHROW_INFO_FMT(dueInvalidParam, "position(%d) >= count(%d)", vPos, nd->count);
	}
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for(i=vPos; i<nd->count-1; i++){
		rt->MoveValue( nd->values[i+1], nd->values[i] );
	}
	rt->ClearValue( nd->values[ nd->count - 1 ] );
	nd->count--;
}

// function void Remove(Object obj)
dsClassArray::nfRemove2::nfRemove2(const sInitData &init) : dsFunction(init.clsArr, "remove",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObj); // obj
}
void dsClassArray::nfRemove2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const testValue = rt->GetValue( 0 );
	
	// find object in list
	int i, p;
	
	if( testValue->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! testValue->GetRealObject() ){
		// test value is null, special compare to avoid null pointer exception
		for( p=0; p<nd.count; p++ ){
			if( nd.values[ p ]->GetType()->GetPrimitiveType() == DSPT_OBJECT && ! nd.values[ p ]->GetRealObject() ){
				break;
			}
		}
		
	}else{
		const int funcIndexEquals = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexEquals();
		
		for( p=0; p<nd.count; p++ ){
			rt->PushValue( nd.values[ p ] );
			rt->RunFunctionFast( testValue, funcIndexEquals ); // bool equals( key )
			if( rt->GetReturnBool() ){
				break;
			}
		}
	}
	
	if( p == nd.count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "object absent: %s", ErrorValueInfo( *rt, testValue ).Pointer() );
	}
	
	// remove object from list
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	for( i=p; i<nd.count-1; i++ ){
		rt->MoveValue( nd.values[ i + 1 ], nd.values[ i ] );
	}
	rt->ClearValue( nd.values[ nd.count - 1 ] );
	nd.count--;
}

// public func removeFrom( int position )
dsClassArray::nfRemoveFrom::nfRemoveFrom( const sInitData &init ) : dsFunction( init.clsArr, "removeFrom",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInt ); // position
}
void dsClassArray::nfRemoveFrom::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	int i, position = rt->GetValue( 0 )->GetInt();
	
	if( position < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) < 0", position );
	}
	if( position >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) >= count(%d)", position, nd->count );
	}
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=position; i<nd->count-1; i++ ){
		rt->MoveValue( nd->values[ i + 1 ], nd->values[ i ] );
	}
	rt->ClearValue( nd->values[ nd->count - 1 ] );
	nd->count--;
}

// function void RemoveAll()
dsClassArray::nfRemoveAll::nfRemoveAll(const sInitData &init) : dsFunction(init.clsArr,
"removeAll", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassArray::nfRemoveAll::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for(int i=0; i<nd->count; i++){
		rt->ClearValue(nd->values[i]);
	}
	nd->count = 0;
}

// function const object getAt(int Position)
dsClassArray::nfGetObject::nfGetObject(const sInitData &init) : dsFunction(init.clsArr,
"getAt", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj){
	p_AddParameter(init.clsInt); // Position
}
void dsClassArray::nfGetObject::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	const int position = rt->GetValue( 0 )->GetInt();
	
	if( position < 0 ){
		if( position < -nd.count ){
			DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) < -count(%d)", position, -nd.count );
		}
		rt->PushValue( nd.values[ nd.count + position ] );
		
	}else{
		if( position >= nd.count ){
			DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) >= count(%d)", position, nd.count );
		}
		rt->PushValue( nd.values[ position ] );
	}
}

// function void SetObject(int Position, object Obj)
dsClassArray::nfSetObject::nfSetObject(const sInitData &init) : dsFunction(init.clsArr,
"setAt", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // Position
	p_AddParameter(init.clsObj); // Obj
}
void dsClassArray::nfSetObject::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	const int position = rt->GetValue( 0 )->GetInt();
	
	if( position < 0 ){
		if( position < -nd.count ){
			DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) < -count(%d)", position, -nd.count );
		}
		rt->CopyValue( rt->GetValue( 1 ), nd.values[ nd.count + position ] );
		
	}else{
		if( position >= nd.count ){
			DSTHROW_INFO_FMT( dueInvalidParam, "position(%d) >= count(%d)", position, nd.count );
		}
		rt->CopyValue( rt->GetValue( 1 ), nd.values[ position ] );
	}
}

// public func void move( int from, int to )
dsClassArray::nfMove::nfMove(const sInitData &init) : dsFunction(init.clsArr,
"move", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsInt); // from
	p_AddParameter(init.clsInt); // to
}
void dsClassArray::nfMove::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = (sArrNatData*)p_GetNativeData(myself);
	if( nd->lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	int from = rt->GetValue( 0 )->GetInt();
	int to = rt->GetValue( 1 )->GetInt();
	int i;
	if( from < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "from(%d) < 0", from );
	}
	if( from >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "from(%d) >= count(%d)", from, nd->count );
	}
	if( to < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "to(%d) < 0", to );
	}
	if( to >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "to(%d) >= count(%d)", to, nd->count );
	}
	
	if( from == to ) return;
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	rt->PushValue( nd->values[ from ] );
	if( from < to ){
		for( i=from; i<to; i++ ){
			rt->MoveValue( nd->values[ i + 1 ], nd->values[ i ] );
		}
	}else{
		for( i=from; i>to; i-- ){
			rt->MoveValue( nd->values[ i - 1 ], nd->values[ i ] );
		}
	}
	rt->PopValue( nd->values[ to ] );
}



// Enumeration
////////////////

// public func void forEach( Block ablock )
dsClassArray::nfForEach::nfForEach( const sInitData &init ) : dsFunction( init.clsArr,
"forEach", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEach::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=0; i<nd->count; i++ ){
		rt->PushValue( nd->values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
	}
}

// public func void forEach( int fromIndex, int toIndex, Block ablock )
dsClassArray::nfForEach2::nfForEach2( const sInitData &init ) : dsFunction( init.clsArr,
"forEach", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEach2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue *block = rt->GetValue( 2 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "block" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=fromIndex; i<toIndex; i++ ){
		if( i >= nd->count ) break;
		rt->PushValue( nd->values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
	}
}

// public func void forEach( int fromIndex, int toIndex, int step, Block ablock )
dsClassArray::nfForEach3::nfForEach3( const sInitData &init ) : dsFunction( init.clsArr,
"forEach", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEach3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	dsValue *block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	if( step > 0 ){
		for( i=fromIndex; i<toIndex; i+=step ){
			if( i >= nd->count ){
				break;
			}
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
		}
		
	}else{
		for( i=fromIndex; i>toIndex; i+=step ){
			if( i >= nd->count ){
				continue;
			}
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
		}
	}
}

// public func void forEachReverse( Block ablock )
dsClassArray::nfForEachReverse::nfForEachReverse( const sInitData &init ) : dsFunction( init.clsArr,
"forEachReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEachReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=nd->count-1; i>=0; i-- ){
		if( i >= nd->count ){
			continue;
		}
		rt->PushValue( nd->values[ i ] );
		rt->RunFunctionFast( block, funcIndexRun );
	}
}

// public func void forEachWhile( Block ablock )
dsClassArray::nfForEachWhile::nfForEachWhile( const sInitData &init ) : dsFunction( init.clsArr,
"forEachWhile", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEachWhile::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
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

// public func void forEachWhile( int fromIndex, int toIndex, Block ablock )
dsClassArray::nfForEachWhile2::nfForEachWhile2( const sInitData &init ) : dsFunction( init.clsArr,
"forEachWhile", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEachWhile2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue *block = rt->GetValue( 2 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=fromIndex; i<toIndex; i++ ){
		if( i >= nd->count ){
			break;
		}
		
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

// public func void forEachWhile( int fromIndex, int toIndex, int step, Block ablock )
dsClassArray::nfForEachWhile3::nfForEachWhile3( const sInitData &init ) : dsFunction( init.clsArr,
"forEachWhile", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEachWhile3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	dsValue *block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	if( step > 0 ){
		for( i=fromIndex; i<toIndex; i+=step ){
			if( i >= nd->count ){
				break;
			}
			
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			
			if( rt->GetReturnValue()->GetType() != clsBool ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
			}
			if( ! rt->GetReturnBool() ){
				break;
			}
		}
		
	}else{
		for( i=fromIndex; i>toIndex; i+=step ){
			if( i >= nd->count ){
				continue;
			}
			
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
}

// public func void forEachWhileReverse( Block ablock )
dsClassArray::nfForEachWhileReverse::nfForEachWhileReverse( const sInitData &init ) : dsFunction( init.clsArr,
"forEachWhileReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfForEachWhileReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=nd->count-1; i>=0; i-- ){
		if( i >= nd->count ){
			continue;
		}
		
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

// public func Array map( Block ablock )
dsClassArray::nfMap::nfMap( const sInitData &init ) : dsFunction( init.clsArr,
"map", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfMap::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	try{
		// create the new array of the same capacity as this array
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		ndnew->SetSize( nd->count, rt, clsObj );
		
		// do the mapping
		for( i=0; i<nd->count; i++ ){
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			rt->CopyValue( rt->GetReturnValue(), ndnew->values[ i ] );
			ndnew->count++;
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}

// public func Array map( int fromIndex, int toIndex, Block ablock )
dsClassArray::nfMap2::nfMap2( const sInitData &init ) : dsFunction( init.clsArr,
"map", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfMap2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i, ncount;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue *block = rt->GetValue( 2 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	ncount = toIndex - fromIndex;
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	try{
		// create the new array with the size of ncount
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		ndnew->SetSize( ncount, rt, clsObj );
		
		// do the mapping
		for( i=0; i<ncount; i++ ){
			rt->PushValue( nd->values[ fromIndex + i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			rt->CopyValue( rt->GetReturnValue(), ndnew->values[ i ] );
			ndnew->count++;
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}

// public func Array map( int fromIndex, int toIndex, int step, Block ablock )
dsClassArray::nfMap3::nfMap3( const sInitData &init ) : dsFunction( init.clsArr,
"map", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfMap3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i, ncount;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	dsValue *block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	ncount = ( toIndex - fromIndex ) / step + 1;
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	try{
		// create the new array with the capacity of ncount
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		ndnew->SetSize( ncount, rt, clsObj );
		
		// do the mapping
		if( step > 0 ){
			for( i=fromIndex; i<toIndex; i+=step ){
				if( i >= nd->count ) break;
				rt->PushValue( nd->values[ i ] );
				rt->RunFunctionFast( block, funcIndexRun );
				rt->CopyValue( rt->GetReturnValue(), ndnew->values[ ndnew->count ] );
				ndnew->count++;
			}
			
		}else{
			for( i=fromIndex; i>toIndex; i+=step ){
				if( i < nd->count ){
					rt->PushValue( nd->values[ i ] );
					rt->RunFunctionFast( block, funcIndexRun );
					rt->CopyValue( rt->GetReturnValue(), ndnew->values[ ndnew->count ] );
					ndnew->count++;
				}
			}
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}

// public func Array mapReverse( Block ablock )
dsClassArray::nfMapReverse::nfMapReverse( const sInitData &init ) : dsFunction( init.clsArr,
"mapReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfMapReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	
	try{
		// create the new array with the capacity of ncount
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		ndnew->SetSize( nd->count, rt, clsObj );
		
		// do the mapping
		for( i=nd->count-1; i>=0; i-- ){
			if( i < nd->count ){
				rt->PushValue( nd->values[ i ] );
				rt->RunFunctionFast( block, funcIndexRun );
				rt->CopyValue( rt->GetReturnValue(), ndnew->values[ ndnew->count ] );
				ndnew->count++;
			}
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}

// public func Array collect( Block ablock )
dsClassArray::nfCollect::nfCollect( const sInitData &init ) : dsFunction( init.clsArr,
"collect", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCollect::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	
	try{
		// create the new array
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
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
				ndnew->SetSize( ndnew->size * 3 / 2 + 1, rt, clsObj );
			}
			rt->CopyValue( nd->values[ i ], ndnew->values[ ndnew->count ] );
			ndnew->count++;
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}

// public func Array collect( int fromIndex, int toIndex, Block ablock )
dsClassArray::nfCollect2::nfCollect2( const sInitData &init ) : dsFunction( init.clsArr,
"collect", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCollect2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue *block = rt->GetValue( 2 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	
	try{
		// create the new array
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		// do the mapping
		for( i=fromIndex; i<toIndex; i++ ){
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			
			if( rt->GetReturnValue()->GetType() != clsBool ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
			}
			if( ! rt->GetReturnBool() ){
				continue;
			}
			
			if( ndnew->count == ndnew->size ){
				ndnew->SetSize( ndnew->size * 3 / 2 + 1, rt, clsObj );
			}
			rt->CopyValue( nd->values[ i ], ndnew->values[ ndnew->count ] );
			ndnew->count++;
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}

// public func Array collect( int fromIndex, int toIndex, int step, Block ablock )
dsClassArray::nfCollect3::nfCollect3( const sInitData &init ) : dsFunction( init.clsArr,
"collect", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCollect3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	dsValue *block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	
	try{
		// create the new array
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		// do the mapping
		if( step > 0 ){
			for( i=fromIndex; i<toIndex; i+=step ){
				if( i >= nd->count ) break;
				rt->PushValue( nd->values[ i ] );
				rt->RunFunctionFast( block, funcIndexRun );
				
				if( rt->GetReturnValue()->GetType() != clsBool ){
					DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
				}
				if( ! rt->GetReturnBool() ){
					continue;
				}
				
				if( ndnew->count == ndnew->size ){
					ndnew->SetSize( ndnew->size * 3 / 2 + 1, rt, clsObj );
				}
				rt->CopyValue( nd->values[ i ], ndnew->values[ ndnew->count ] );
				ndnew->count++;
			}
			
		}else{
			for( i=fromIndex; i>toIndex; i+=step ){
				if( i < nd->count ){
					rt->PushValue( nd->values[ i ] );
					rt->RunFunctionFast( block, funcIndexRun );
					
					if( rt->GetReturnValue()->GetType() != clsBool ){
						DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
					}
					if( ! rt->GetReturnBool() ){
						continue;
					}
					
					if( ndnew->count == ndnew->size ){
						ndnew->SetSize( ndnew->size * 3 / 2 + 1, rt, clsObj );
					}
					rt->CopyValue( nd->values[ i ], ndnew->values[ ndnew->count ] );
					ndnew->count++;
				}
			}
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}

// public func Array collectReverse( Block ablock )
dsClassArray::nfCollectReverse::nfCollectReverse( const sInitData &init ) : dsFunction( init.clsArr,
"collectReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCollectReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClassArray *clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	
	try{
		// create the new array
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		// do the mapping
		for( i=nd->count-1; i>=0; i-- ){
			if( i < nd->count ){
				rt->PushValue( nd->values[ i ] );
				rt->RunFunctionFast( block, funcIndexRun );
				
				if( rt->GetReturnValue()->GetType() != clsBool ){
					DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
				}
				if( ! rt->GetReturnBool() ){
					continue;
				}
				
				if( ndnew->count == ndnew->size ){
					ndnew->SetSize( ndnew->size * 3 / 2 + 1, rt, clsObj );
				}
				rt->CopyValue( nd->values[ i ], ndnew->values[ ndnew->count ] );
				ndnew->count++;
			}
		}
		
		// push the result and clean up
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ) rt->FreeValue( newArray );
		throw;
	}
}



// public func Object fold( Block ablock )
dsClassArray::nfFold::nfFold( const sInitData &init ) : dsFunction( init.clsArr,
"fold", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFold::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	if( nd->count == 0 ){
		rt->PushObject( NULL, clsObj );
		
	}else if( nd->count == 1 ){
		rt->PushValue( nd->values[ 0 ] );
		
	}else{
		const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
		cArrNatDatLockModifyGuard lock( nd->lockModify );
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

// public func Object fold( int fromIndex, int toIndex, Block ablock )
dsClassArray::nfFold2::nfFold2( const sInitData &init ) : dsFunction( init.clsArr,
"fold", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFold2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	int i, ncount;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue *block = rt->GetValue( 2 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	ncount = toIndex - fromIndex;
	
	if( ncount == 0 ){
		rt->PushObject( NULL, clsObj );
		
	}else if( ncount == 1 ){
		rt->PushValue( nd->values[ fromIndex ] );
		
	}else{
		const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
		cArrNatDatLockModifyGuard lock( nd->lockModify );
		
		fromIndex++;
		
		for( i=fromIndex; i<toIndex; i++ ){
			rt->PushValue( nd->values[ i ] );
			
			if( i == fromIndex ){
				rt->PushValue( nd->values[ fromIndex - 1 ] );
				
			}else{
				rt->PushReturnValue();
			}
			
			rt->RunFunctionFast( block, funcIndexRun );
		}
		
		rt->PushReturnValue();
	}
}

// public func Object fold( int fromIndex, int toIndex, int step, Block ablock )
dsClassArray::nfFold3::nfFold3( const sInitData &init ) : dsFunction( init.clsArr,
"fold", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFold3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	int i, visited = 0;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	dsValue *block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	
	if( step > 0 ){
		for( i=fromIndex; i<toIndex; i+=step ){
			if( i >= nd->count ) break;
			
			visited++;
			
			if( visited > 1 ){
				rt->PushValue( nd->values[ i ] );
				
				if( visited == 2 ){
					rt->PushValue( nd->values[ fromIndex ] );
					
				}else{
					rt->PushReturnValue();
				}
				
				rt->RunFunctionFast( block, funcIndexRun );
			}
		}
		
	}else{
		for( i=fromIndex; i>toIndex; i+=step ){
			if( i < nd->count ){
				visited++;
				
				if( visited > 1 ){
					rt->PushValue( nd->values[ i ] );
					
					if( visited == 2 ){
						rt->PushValue( nd->values[ fromIndex ] );
						
					}else{
						rt->PushReturnValue();
					}
					
					rt->RunFunctionFast( block, funcIndexRun );
				}
			}
		}
	}
	
	if( visited == 0 ){
		rt->PushObject( NULL, clsObj );
		
	}else if( visited == 1 ){
		rt->PushValue( nd->values[ fromIndex ] );
		
	}else{
		rt->PushReturnValue();
	}
}

// public func Object foldReverse( Block ablock )
dsClassArray::nfFoldReverse::nfFoldReverse( const sInitData &init ) : dsFunction( init.clsArr,
"foldReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFoldReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	int i, visited = 0;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	
	for( i=nd->count-1; i>=0; i-- ){
		if( i < nd->count ){
			visited++;
			
			if( visited > 1 ){
				rt->PushValue( nd->values[ i ] );
				
				if( visited == 2 ){
					rt->PushValue( nd->values[ nd->count - 1 ] );
					
				}else{
					rt->PushReturnValue();
				}
				
				rt->RunFunctionFast( block, funcIndexRun );
			}
		}
	}
	
	if( visited == 0 ){
		rt->PushObject( NULL, clsObj );
		
	}else if( visited == 1 ){
		rt->PushValue( nd->values[ nd->count - 1 ] );
		
	}else{
		rt->PushReturnValue();
	}
}



// public func Object inject( Object injectValue, Block ablock )
dsClassArray::nfInject::nfInject( const sInitData &init ) : dsFunction( init.clsArr,
"inject", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfInject::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	
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
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
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

// public func Object inject( int fromIndex, int toIndex, Object injectValue, Block ablock )
dsClassArray::nfInject2::nfInject2( const sInitData &init ) : dsFunction( init.clsArr,
"inject", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsObj ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfInject2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	
	const int fromIndex = rt->GetValue( 0 )->GetInt();
	const int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue * const injectValue = rt->GetValue( 2 );
	dsValue * const block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd.count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd.count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd.count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd.count );
	}
	const int ncount = toIndex - fromIndex;
	
	if( ncount == 0 ){
		rt->PushValue( injectValue );
		return;
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	int i;
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	for( i=fromIndex; i<toIndex; i++ ){
		rt->PushValue( nd.values[ i ] );
		
		if( i == fromIndex ){
			rt->PushValue( injectValue );
			
		}else{
			rt->PushReturnValue();
		}
		
		rt->RunFunctionFast( block, funcIndexRun );
	}
	
	rt->PushReturnValue();
}

// public func Object inject( int fromIndex, int toIndex, int step, Object injectValue, Block ablock )
dsClassArray::nfInject3::nfInject3( const sInitData &init ) : dsFunction( init.clsArr,
"inject", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsObj ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfInject3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	
	const int fromIndex = rt->GetValue( 0 )->GetInt();
	const int toIndex = rt->GetValue( 1 )->GetInt();
	const int step = rt->GetValue( 2 )->GetInt();
	dsValue * const injectValue = rt->GetValue( 3 );
	dsValue * const block = rt->GetValue( 4 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd.count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd.count );
	}
	if( toIndex < ( step > 0 ? 0 : -1 ) ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < %d", step > 0 ? 0 : -1 );
	}
	if( toIndex > nd.count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd.count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	if( fromIndex == toIndex ){
		rt->PushValue( injectValue );
		return;
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	int i;
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	if( step > 0 ){
		for( i=fromIndex; i<toIndex; i+=step ){
			rt->PushValue( nd.values[ i ] );
			
			if( i == fromIndex ){
				rt->PushValue( injectValue );
				
			}else{
				rt->PushReturnValue();
			}
			
			rt->RunFunctionFast( block, funcIndexRun );
		}
		
	}else{
		for( i=fromIndex; i>toIndex; i+=step ){
			rt->PushValue( nd.values[ i ] );
			
			if( i == fromIndex ){
				rt->PushValue( injectValue );
				
			}else{
				rt->PushReturnValue();
			}
			
			rt->RunFunctionFast( block, funcIndexRun );
		}
	}
	
	rt->PushReturnValue();
}

// public func Object injectReverse( Object injectValue, Block ablock )
dsClassArray::nfInjectReverse::nfInjectReverse( const sInitData &init ) : dsFunction( init.clsArr,
"injectReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // injectValue
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfInjectReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	
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
	const int first = nd.count - 1;
	int i;
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	for( i=first; i>=0; i-- ){
		rt->PushValue( nd.values[ i ] );
		
		if( i == first ){
			rt->PushValue( injectValue );
			
		}else{
			rt->PushReturnValue();
		}
		
		rt->RunFunctionFast( block, funcIndexRun );
	}
	
	rt->PushReturnValue();
}



// public func int count( Block ablock )
dsClassArray::nfCount::nfCount( const sInitData &init ) : dsFunction( init.clsArr,
"count", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i, count = 0;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=0; i<nd->count; i++ ){
		rt->PushValue( nd->values[ i ] );
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

// public func int count( int fromIndex, int toIndex, Block ablock )
dsClassArray::nfCount2::nfCount2( const sInitData &init ) : dsFunction( init.clsArr,
"count", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCount2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i, count = 0;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue *block = rt->GetValue( 2 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=fromIndex; i<toIndex; i++ ){
		rt->PushValue( nd->values[ i ] );
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

// public func int count( int fromIndex, int toIndex, int step, Block ablock )
dsClassArray::nfCount3::nfCount3( const sInitData &init ) : dsFunction( init.clsArr,
"count", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCount3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i, count = 0;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	dsValue *block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	if( step > 0 ){
		for( i=fromIndex; i<toIndex; i+=step ){
			if( i >= nd->count ){
				break;
			}
			
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			
			if( rt->GetReturnValue()->GetType() != clsBool ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
			}
			if( rt->GetReturnBool() ){
				count++;
			}
		}
		
	}else{
		for( i=fromIndex; i>toIndex; i+=step ){
			if( i >= nd->count ){
				continue;
			}
			
			rt->PushValue( nd->values[ i ] );
			rt->RunFunctionFast( block, funcIndexRun );
			
			if( rt->GetReturnValue()->GetType() != clsBool ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
			}
			if( rt->GetReturnBool() ){
				count++;
			}
		}
	}
	
	rt->PushInt( count );
}

// public func int countReverse( Block ablock )
dsClassArray::nfCountReverse::nfCountReverse( const sInitData &init ) : dsFunction( init.clsArr,
"countReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfCountReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i, count = 0;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=nd->count-1; i>=0; i-- ){
		if( i >= nd->count ){
			continue;
		}
		
		rt->PushValue( nd->values[ i ] );
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

// public func Object find( Block ablock )
dsClassArray::nfFind::nfFind( const sInitData &init ) : dsFunction( init.clsArr,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFind::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
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
	
	rt->PushObject( NULL, clsObj );
}

// public func Object find( int fromIndex, int toIndex, Block ablock )
dsClassArray::nfFind2::nfFind2( const sInitData &init ) : dsFunction( init.clsArr,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFind2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	dsValue *block = rt->GetValue( 2 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=fromIndex; i<toIndex; i++ ){
		if( i >= nd->count ){
			break;
		}
		
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
	
	rt->PushObject( NULL, clsObj );
}

// public func Object find( int fromIndex, int toIndex, int step, Block ablock )
dsClassArray::nfFind3::nfFind3( const sInitData &init ) : dsFunction( init.clsArr,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFind3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	int fromIndex = rt->GetValue( 0 )->GetInt();
	int toIndex = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	dsValue *block = rt->GetValue( 3 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	if( fromIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) < 0", fromIndex );
	}
	if( fromIndex >= nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "fromIndex(%d) >= count(%d)", fromIndex, nd->count );
	}
	if( toIndex < 0 ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) < 0", toIndex );
	}
	if( toIndex > nd->count ){
		DSTHROW_INFO_FMT( dueInvalidParam, "toIndex(%d) > count(%d)", toIndex, nd->count );
	}
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	if( step > 0 ){
		for( i=fromIndex; i<toIndex; i+=step ){
			if( i >= nd->count ){
				break;
			}
			
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
		
	}else{
		for( i=fromIndex; i>toIndex; i+=step ){
			if( i >= nd->count ){
				continue;
			}
			
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
	}
	
	rt->PushObject( NULL, clsObj );
}

// public func Object findReverse( Block ablock )
dsClassArray::nfFindReverse::nfFindReverse( const sInitData &init ) : dsFunction( init.clsArr,
"findReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfFindReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData *nd = ( sArrNatData* )p_GetNativeData( myself );
	dsClass *clsObj = rt->GetEngine()->GetClassObject();
	dsClass *clsBool = rt->GetEngine()->GetClassBool();
	int i;
	
	dsValue *block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun1();
	
	cArrNatDatLockModifyGuard lock( nd->lockModify );
	for( i=nd->count-1; i>=0; i-- ){
		if( i >= nd->count ){
			continue;
		}
		
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
	
	rt->PushObject( NULL, clsObj );
}

// public func void removeIf( Block ablock )
dsClassArray::nfRemoveIf::nfRemoveIf( const sInitData &init ) : dsFunction( init.clsArr,
"removeIf", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfRemoveIf::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
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
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
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

// public func Array slice( int indexFrom )
dsClassArray::nfSlice::nfSlice( const sInitData &init ) : dsFunction( init.clsArr,
"slice", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsInt ); // indexFrom
}
void dsClassArray::nfSlice::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsClassArray * const clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass * const clsObj = rt->GetEngine()->GetClassObject();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	int indexFrom = rt->GetValue( 0 )->GetInt();
	if( indexFrom < 0 ){
		indexFrom += nd.count;
	}
	
	if( indexFrom < 0 ){
		indexFrom = 0;
	}
	if( indexFrom > nd.count ){
		indexFrom = nd.count;
	}
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	try{
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		if( indexFrom < nd.count ){
			ndnew->SetSize( nd.count - indexFrom, rt, clsObj );
			
			for( i=indexFrom; i<nd.count; i++ ){
				rt->CopyValue( nd.values[ i ], ndnew->values[ ndnew->count ] );
				ndnew->count++;
			}
		}
		
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public func Array slice( int indexFrom, int indexTo )
dsClassArray::nfSlice2::nfSlice2( const sInitData &init ) : dsFunction( init.clsArr,
"slice", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsInt ); // indexFrom
	p_AddParameter( init.clsInt ); // indexTo
}
void dsClassArray::nfSlice2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsClassArray * const clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass * const clsObj = rt->GetEngine()->GetClassObject();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	int indexFrom = rt->GetValue( 0 )->GetInt();
	int indexTo = rt->GetValue( 1 )->GetInt();
	if( indexFrom < 0 ){
		indexFrom += nd.count;
	}
	if( indexTo < 0 ){
		indexTo += nd.count;
	}
	
	if( indexFrom < 0 ){
		indexFrom = 0;
	}
	if( indexFrom > nd.count ){
		indexFrom = nd.count;
	}
	
	if( indexTo < 0 ){
		indexTo = 0;
	}
	if( indexTo > nd.count ){
		indexTo = nd.count;
	}
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	try{
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		if( indexTo > indexFrom ){
			ndnew->SetSize( indexTo - indexFrom, rt, clsObj );
			
			for( i=indexFrom; i<indexTo; i++ ){
				rt->CopyValue( nd.values[ i ], ndnew->values[ ndnew->count ] );
				ndnew->count++;
			}
		}
		
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public func Array slice( int indexFrom, int indexTo, int step )
dsClassArray::nfSlice3::nfSlice3( const sInitData &init ) : dsFunction( init.clsArr,
"slice", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsInt ); // fromIndex
	p_AddParameter( init.clsInt ); // toIndex
	p_AddParameter( init.clsInt ); // step
}
void dsClassArray::nfSlice3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsClassArray * const clsArr = ( dsClassArray* )GetOwnerClass();
	dsClass * const clsObj = rt->GetEngine()->GetClassObject();
	dsValue *newArray = NULL;
	sArrNatData *ndnew;
	int i;
	
	int indexFrom = rt->GetValue( 0 )->GetInt();
	int indexTo = rt->GetValue( 1 )->GetInt();
	int step = rt->GetValue( 2 )->GetInt();
	if( indexFrom < 0 ){
		indexFrom += nd.count;
	}
	if( indexTo < 0 ){
		indexTo += nd.count;
	}
	
	if( indexFrom < 0 ){
		indexFrom = 0;
	}
	if( indexTo < 0 ){
		indexTo = 0;
	}
	if( indexTo > nd.count ){
		indexTo = nd.count;
	}
	
	if( step > 0 ){
		if( indexFrom > nd.count ){
			indexFrom = nd.count;
		}
		
	}else{
		if( indexFrom >= nd.count ){
			indexFrom = nd.count - 1;
		}
	}
	
	if( step == 0 ){
		DSTHROW_INFO( dueInvalidParam, "step == 0" );
	}
	
	int newCount = 0;
	if( step > 0 ){
		for( i=indexFrom; i<indexTo; i+=step ){
			newCount++;
		}
		
	}else{
		for( i=indexFrom; i>indexTo; i+=step ){
			newCount++;
		}
	}
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	try{
		newArray = rt->CreateValue( clsArr );
		rt->CreateObjectNaked( newArray, clsArr );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		if( newCount > 0 ){
			ndnew->SetSize( newCount, rt, clsObj );
			
			if( step > 0 ){
				for( i=indexFrom; i<indexTo; i+=step ){
					rt->CopyValue( nd.values[ i ], ndnew->values[ ndnew->count ] );
					ndnew->count++;
				}
				
			}else{
				for( i=indexFrom; i>indexTo; i+=step ){
					rt->CopyValue( nd.values[ i ], ndnew->values[ ndnew->count ] );
					ndnew->count++;
				}
			}
		}
		
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}



// public func void sort()
dsClassArray::nfSort::nfSort( const sInitData &init ) : dsFunction( init.clsArr,
"sort", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassArray::nfSort::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	// quick-sort
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	nd.Sort( rt );
	
	/*
	// ground truth: bubble sort
	dsClass * const clsInt = rt->GetEngine()->GetClassInt();
	int i;
	
	const int funcIndexCompare = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexCompare();
	
	for( i=1; i<nd.count; i++ ){
		rt->PushValue( nd.values[ i ] );
		rt->RunFunctionFast( nd.values[ i - 1 ], funcIndexCompare ); // int objPrev.compare( objNext )
		
		if( rt->GetReturnInt() > 0 ){ // i > i+1
			dsValue * const exchange = nd.values[ i - 1 ];
			nd.values[ i - 1 ] = nd.values[ i ];
			nd.values[ i ] = exchange;
			
			if( i > 1 ){
				i -= 2;
			}
		}
	}
	*/
}

// public func void sort( Block ablock )
dsClassArray::nfSort2::nfSort2( const sInitData &init ) : dsFunction( init.clsArr,
"sort", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfSort2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	if( nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	// quick-sort
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	nd.Sort( rt, block );
	
	/*
	// ground truth: bubble sort
	dsClass * const clsInt = rt->GetEngine()->GetClassInt();
	int i;
	
	const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
	
	for( i=1; i<nd.count; i++ ){
		rt->PushValue( nd.values[ i ] );
		rt->PushValue( nd.values[ i - 1 ] );
		rt->RunFunctionFast( block, funcIndexRun ); // Object run( objPrev, objNext )
		if( rt->GetReturnValue()->GetType() != clsInt ){
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsInt ) );
		}
		
		if( rt->GetReturnInt() > 0 ){ // i > i+1
			dsValue * const exchange = nd.values[ i - 1 ];
			nd.values[ i - 1 ] = nd.values[ i ];
			nd.values[ i ] = exchange;
			
			if( i > 1 ){
				i -= 2;
			}
		}
	}
	*/
}

// public func Array sorted()
dsClassArray::nfSorted::nfSorted( const sInitData &init ) : dsFunction( init.clsArr,
"sorted", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
}
void dsClassArray::nfSorted::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsClass * const clsArray = ( dsClassArray* )GetOwnerClass();
// 	dsClass * const clsInt = rt->GetEngine()->GetClassInt();
	dsClass * const clsObj = rt->GetEngine()->GetClassObject();
	sArrNatData *ndnew = NULL;
	dsValue *newArray = NULL;
	int i;
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	try{
		newArray = rt->CreateValue( clsArray );
		rt->CreateObjectNaked( newArray, clsArray );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		ndnew->SetSize( nd.count, rt, clsObj );
		ndnew->count = nd.count;
		
		for( i=0; i<nd.count; i++ ){
			rt->CopyValue( nd.values[ i ], ndnew->values[ i ] );
		}
		
		// quick-sort
		ndnew->Sort( rt );
		
		/*
		// ground truth: bubble sort
		const int funcIndexCompare = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexCompare();
		
		for( i=1; i<ndnew->count; i++ ){
			rt->PushValue( ndnew->values[ i ] );
			rt->RunFunctionFast( ndnew->values[ i - 1 ], funcIndexCompare ); // int objPrev.compare( objNext )
			
			if( rt->GetReturnInt() > 0 ){ // i > i+1
				dsValue * const exchange = ndnew->values[ i - 1 ];
				ndnew->values[ i - 1 ] = ndnew->values[ i ];
				ndnew->values[ i ] = exchange;
				
				if( i > 1 ){
					i -= 2;
				}
			}
		}
		*/
		
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}

// public func Array sorted( Block ablock )
dsClassArray::nfSorted2::nfSorted2( const sInitData &init ) : dsFunction( init.clsArr,
"sorted", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsBlock ); // ablock
}
void dsClassArray::nfSorted2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsClass * const clsArray = ( dsClassArray* )GetOwnerClass();
// 	dsClass * const clsInt = rt->GetEngine()->GetClassInt();
	dsClass * const clsObj = rt->GetEngine()->GetClassObject();
	sArrNatData *ndnew = NULL;
	dsValue *newArray = NULL;
	int i;
	
	dsValue * const block = rt->GetValue( 0 );
	if( ! block->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "ablock" );
	}
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	try{
		newArray = rt->CreateValue( clsArray );
		rt->CreateObjectNaked( newArray, clsArray );
		ndnew = ( sArrNatData* )p_GetNativeData( newArray );
		ndnew->values = NULL;
		ndnew->count = 0;
		ndnew->size = 0;
		ndnew->lockModify = 0;
		
		ndnew->SetSize( nd.count, rt, clsObj );
		ndnew->count = nd.count;
		
		for( i=0; i<nd.count; i++ ){
			rt->CopyValue( nd.values[ i ], ndnew->values[ i ] );
		}
		
		// quick-sort
		ndnew->Sort( rt, block );
		
		/*
		// ground truth: bubble sort
		const int funcIndexRun = ( ( dsClassBlock* )rt->GetEngine()->GetClassBlock() )->GetFuncIndexRun2();
		
		for( i=1; i<ndnew->count; i++ ){
			rt->PushValue( ndnew->values[ i ] );
			rt->PushValue( ndnew->values[ i - 1 ] );
			rt->RunFunctionFast( block, funcIndexCompare ); // Object run( objPrev, objNext )
			if( rt->GetReturnValue()->GetType() != clsInt ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsInt ) );
			}
			
			if( rt->GetReturnInt() > 0 ){ // i > i+1
				dsValue * const exchange = ndnew->values[ i - 1 ];
				ndnew->values[ i - 1 ] = ndnew->values[ i ];
				ndnew->values[ i ] = exchange;
				
				if( i > 1 ){
					i -= 2;
				}
			}
		}
		*/
		
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}



// public func bool equals( Object object )
dsClassArray::nfEquals::nfEquals( const sInitData &init ) :
dsFunction(init.clsArr, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // object
}
void dsClassArray::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsClass * const clsArr = ( dsClassArray* )GetOwnerClass();
	dsValue * const object = rt->GetValue( 0 );
	
	if( object->GetType()->GetPrimitiveType() != DSPT_OBJECT || ! object->GetRealObject()
	|| object->GetRealObject()->GetType() != clsArr ){
		rt->PushBool( false );
		return;
	}
	
	const sArrNatData &nd2 = *( ( sArrNatData* )p_GetNativeData( object ) );
	if( nd2.count != nd.count ){
		rt->PushBool( false );
		return;
	}
	
	dsClass * const clsBool = rt->GetEngine()->GetClassBool();
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	int i;
	for( i=0; i<nd.count; i++ ){
		rt->PushValue( nd2.values[ i ] );
		rt->RunFunction( nd.values[ i ], "equals", 1 );
		
		if( rt->GetReturnValue()->GetType() != clsBool ){
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( rt->GetReturnValue(), clsBool ) );
		}
		if( ! rt->GetReturnBool() ){
			rt->PushBool( false );
			return;
		}
	}
	
	rt->PushBool( true );
}

// public func Object random()
dsClassArray::nfRandom::nfRandom( const sInitData &init ) : dsFunction( init.clsArr,
"random", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
}
void dsClassArray::nfRandom::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	
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



// public func Array +( Array array )
dsClassArray::nfOpAdd::nfOpAdd( const sInitData &init ) :
dsFunction( init.clsArr, "+", DSFT_OPERATOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsArr ); // array
}
void dsClassArray::nfOpAdd::RunFunction( dsRunTime *rt, dsValue *myself ){
	const sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	dsClassArray &clsArray = *( ( dsClassArray* )GetOwnerClass() );
	
	dsValue * const array = rt->GetValue( 0 );
	if( ! array->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "array" );
	}
	
	const sArrNatData &nd2 = *( ( sArrNatData* )p_GetNativeData( array ) );
	
	dsValue * const newArray = clsArray.CreateArray( rt );
	int i;
	try{
		sArrNatData &nd3 = *( ( sArrNatData* )p_GetNativeData( newArray ) );
		nd3.SetSize( nd.count + nd2.count, rt, clsArray.GetClassObject() );
		for( i=0; i<nd.count; i++ ){
			rt->CopyValue( nd.values[ i ], nd3.values[ i ] );
		}
		for( i=0; i<nd2.count; i++ ){
			rt->CopyValue( nd2.values[ i ], nd3.values[ nd.count + i ] );
		}
		nd3.count = nd.count + nd2.count;
		
		rt->PushValue( newArray );
		rt->FreeValue( newArray );
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
}



// public func String toString()
dsClassArray::nfToString::nfToString( const sInitData &init ) : dsFunction( init.clsArr,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassArray::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself ) );
	char *buffer = NULL;
	int maxLen = 5000;
	int curLen = 0;
	int i;
	
	const int funcIndexToString = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexToString();
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
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



// Class dsClassArray
///////////////////////

// Constructor, destructor
////////////////////////////

dsClassArray::dsClassArray() : dsClass( "Array", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sArrNatData ) );
}

dsClassArray::~dsClassArray(){
}


// Management
///////////////

void dsClassArray::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	// store classes
	init.clsArr = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsObj = engine->GetClassObject();
	init.clsBool = engine->GetClassBool();
	init.clsBlock = engine->GetClassBlock();
	init.clsStr = engine->GetClassString();
	
	// get object class from engine
	p_ClsObj = init.clsObj;
	
	// functions
	AddFunction( new nfCreate( init ) );
	AddFunction( new nfCreateSize( init ) );
	AddFunction( new nfCreateCount( init ) );
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
	
	AddFunction( new nfLength( init ) );
	AddFunction( new nfSize( init ) );
	AddFunction( new nfSetSize( init ) );
	AddFunction( new nfResize( init ) );
	AddFunction( new nfAdd( init ) );
	AddFunction( new nfAddAll( init ) );
	AddFunction( new nfInsert( init ) );
	AddFunction( new nfInsertAll( init ) );
	AddFunction( new nfIndexOf( init ) );
	AddFunction( new nfHas( init ) );
	AddFunction( new nfRemove( init ) );
	AddFunction( new nfRemove2( init ) );
	AddFunction( new nfRemoveFrom( init ) );
	AddFunction( new nfRemoveAll( init ) );
	AddFunction( new nfGetObject( init ) );
	AddFunction( new nfSetObject( init ) );
	AddFunction( new nfMove( init ) );
	
	AddFunction( new nfForEach( init ) );
	AddFunction( new nfForEach2( init ) );
	AddFunction( new nfForEach3( init ) );
	AddFunction( new nfForEachReverse( init ) );
	AddFunction( new nfForEachWhile( init ) );
	AddFunction( new nfForEachWhile2( init ) );
	AddFunction( new nfForEachWhile3( init ) );
	AddFunction( new nfForEachWhileReverse( init ) );
	AddFunction( new nfMap( init ) );
	AddFunction( new nfMap2( init ) );
	AddFunction( new nfMap3( init ) );
	AddFunction( new nfMapReverse( init ) );
	AddFunction( new nfCollect( init ) );
	AddFunction( new nfCollect2( init ) );
	AddFunction( new nfCollect3( init ) );
	AddFunction( new nfCollectReverse( init ) );
	AddFunction( new nfFold( init ) );
	AddFunction( new nfFold2( init ) );
	AddFunction( new nfFold3( init ) );
	AddFunction( new nfFoldReverse( init ) );
	AddFunction( new nfInject( init ) );
	AddFunction( new nfInject2( init ) );
	AddFunction( new nfInject3( init ) );
	AddFunction( new nfInjectReverse( init ) );
	AddFunction( new nfCount( init ) );
	AddFunction( new nfCount2( init ) );
	AddFunction( new nfCount3( init ) );
	AddFunction( new nfCountReverse( init ) );
	AddFunction( new nfFind( init ) );
	AddFunction( new nfFind2( init ) );
	AddFunction( new nfFind3( init ) );
	AddFunction( new nfFindReverse( init ) );
	AddFunction( new nfRemoveIf( init ) );
	AddFunction( new nfSlice( init ) );
	AddFunction( new nfSlice2( init ) );
	AddFunction( new nfSlice3( init ) );
	
	AddFunction( new nfSort( init ) );
	AddFunction( new nfSort2( init ) );
	AddFunction( new nfSorted( init ) );
	AddFunction( new nfSorted2( init ) );
	
	AddFunction( new nfRandom( init ) );
	
	AddFunction( new nfOpAdd( init ) );
	
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfToString( init ) );
}



dsValue *dsClassArray::CreateArray( dsRunTime *rt ){
	dsValue *newArray = NULL;
	sArrNatData *nd;
	
	try{
		newArray = rt->CreateValue( this );
		rt->CreateObjectNaked( newArray, this );
		nd = ( sArrNatData* )p_GetNativeData( newArray->GetRealObject()->GetBuffer() );
		nd->values = NULL;
		nd->count = 0;
		nd->size = 0;
		nd->lockModify = 0;
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
	
	return newArray;
}

dsValue *dsClassArray::CreateArray( dsRunTime *rt, int argumentCount ){
	if( argumentCount < 0 ){
		DSTHROW_INFO( dueInvalidParam, "argumentCount < 0" );
	}
	
	dsValue *newArray = NULL;
	int i;
	
	try{
		newArray = CreateArray( rt );
		sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( newArray->GetRealObject()->GetBuffer() ) );
		
		nd.SetSize( argumentCount, rt, p_ClsObj );
		for( i=0; i<argumentCount; i++ ){
			rt->CopyValue( rt->GetValue( i ), nd.values[ i ] );
		}
		nd.count = argumentCount;
		
	}catch( ... ){
		if( newArray ){
			rt->FreeValue( newArray );
		}
		throw;
	}
	
	return newArray;
}

void dsClassArray::AddObject( dsRunTime *rt, dsRealObject *myself, dsValue *value ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself->GetBuffer() ) );
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

void dsClassArray::RemoveObject( dsRunTime* rt, dsRealObject* myself, int index ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself->GetBuffer() ) );
	
	if( index < 0 || index >= nd.count || nd.lockModify != 0 ){
		DSTHROW_INFO( dueInvalidAction, errorModifyWhileLocked );
	}
	
	cArrNatDatLockModifyGuard lock( nd.lockModify );
	int i;
	for( i=index; i<nd.count-1; i++ ){
		rt->MoveValue( nd.values[ i + 1 ], nd.values[ i ] );
	}
	rt->ClearValue( nd.values[ nd.count - 1 ] );
	nd.count--;
}

int dsClassArray::GetObjectCount( dsRunTime *rt, dsRealObject *myself ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	const sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself->GetBuffer() ) );
	return nd.count;
}

dsValue *dsClassArray::GetObjectAt( dsRunTime *rt, dsRealObject *myself, int index ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	const sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself->GetBuffer() ) );
	if( index < 0 || index >= nd.count ){
		DSTHROW( dueInvalidParam );
	}
	
	return nd.values[ index ];
}

void dsClassArray::SetObjectAt( dsRunTime *rt, dsRealObject *myself, int index, dsRealObject *object ){
	if( ! rt || ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	sArrNatData &nd = *( ( sArrNatData* )p_GetNativeData( myself->GetBuffer() ) );
	
	if( index < 0 || index >= nd.count ){
		DSTHROW( dueInvalidParam );
	}
	if( nd.lockModify != 0 ){
		DSTHROW( dueInvalidAction );
	}
	
	rt->SetObject( nd.values[ index ], object );
}
