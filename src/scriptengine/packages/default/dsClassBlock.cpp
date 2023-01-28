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
#include <stdint.h>
#include "../../dragonscript_config.h"
#include "dsClassBlock.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsFuncList.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// native data structure
struct sBlockNatData{
	dsFunction *function;
	dsValue *owner;
	dsValue **contextVariables;
	int contextVariableCount;
};

// native functions
/////////////////////

// func destructor()
dsClassBlock::nfDestructor::nfDestructor( const sInitData &init ) : dsFunction( init.clsBlock,
"destructor", DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassBlock::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	
	if( nd->contextVariables ){
		while( nd->contextVariableCount > 0 ){
			nd->contextVariableCount--;
			rt->FreeValue( nd->contextVariables[ nd->contextVariableCount ] );
		}
		delete [] nd->contextVariables;
		nd->contextVariables = NULL;
	}
	
	if( nd->owner ){
		rt->FreeValue( nd->owner );
		nd->owner = NULL;
	}
}

// func Object run()
dsClassBlock::nfRun0::nfRun0( const sInitData &init ) : dsFunction( init.clsBlock,
"run", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
}
void dsClassBlock::nfRun0::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	dsClassBlock *clsBlock = ( dsClassBlock* )GetOwnerClass();
	
	if( ! nd->function ) DSTHROW( dueNullPointer );
	
	clsBlock->CastArguments( rt, nd->function->GetSignature(), 0 );
	clsBlock->RunIt( rt, myself );
}

// func Object run(Object arg1)
dsClassBlock::nfRun1::nfRun1( const sInitData &init ) : dsFunction( init.clsBlock,
"run", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // arg1
}
void dsClassBlock::nfRun1::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	dsClassBlock *clsBlock = ( dsClassBlock* )GetOwnerClass();
	
	if( ! nd->function ) DSTHROW( dueNullPointer );
	
	clsBlock->CastArguments( rt, nd->function->GetSignature(), 1 );
	clsBlock->RunIt( rt, myself );
}

// func Object run(Object arg1, Object arg2)
dsClassBlock::nfRun2::nfRun2( const sInitData &init ) : dsFunction( init.clsBlock,
"run", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // arg1
	p_AddParameter( init.clsObj ); // arg2
}
void dsClassBlock::nfRun2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	dsClassBlock *clsBlock = ( dsClassBlock* )GetOwnerClass();
	
	if( ! nd->function ) DSTHROW( dueNullPointer );
	
	clsBlock->CastArguments( rt, nd->function->GetSignature(), 2 );
	clsBlock->RunIt( rt, myself );
}

// func Object run( Object arg1, Object arg2, Object arg3 )
dsClassBlock::nfRun3::nfRun3( const sInitData &init ) : dsFunction( init.clsBlock,
"run", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // arg1
	p_AddParameter( init.clsObj ); // arg2
	p_AddParameter( init.clsObj ); // arg3
}
void dsClassBlock::nfRun3::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	dsClassBlock *clsBlock = ( dsClassBlock* )GetOwnerClass();
	
	if( ! nd->function ) DSTHROW( dueNullPointer );
	
	clsBlock->CastArguments( rt, nd->function->GetSignature(), 3 );
	clsBlock->RunIt( rt, myself );
}

// func Object run(Object arg1, Object arg2, Object arg3, Object arg4)
dsClassBlock::nfRun4::nfRun4( const sInitData &init ) : dsFunction( init.clsBlock,
"run", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // arg1
	p_AddParameter( init.clsObj ); // arg2
	p_AddParameter( init.clsObj ); // arg3
	p_AddParameter( init.clsObj ); // arg4
}
void dsClassBlock::nfRun4::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	dsClassBlock *clsBlock = ( dsClassBlock* )GetOwnerClass();
	
	if( ! nd->function ) DSTHROW( dueNullPointer );
	
	clsBlock->CastArguments( rt, nd->function->GetSignature(), 4 );
	clsBlock->RunIt( rt, myself );
}

// func Object run(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5)
dsClassBlock::nfRun5::nfRun5( const sInitData &init ) : dsFunction( init.clsBlock,
"run", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsObj ); // arg1
	p_AddParameter( init.clsObj ); // arg2
	p_AddParameter( init.clsObj ); // arg3
	p_AddParameter( init.clsObj ); // arg4
	p_AddParameter( init.clsObj ); // arg5
}
void dsClassBlock::nfRun5::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	dsClassBlock *clsBlock = ( dsClassBlock* )GetOwnerClass();
	
	if( ! nd->function ) DSTHROW( dueNullPointer );
	
	clsBlock->CastArguments( rt, nd->function->GetSignature(), 5 );
	clsBlock->RunIt( rt, myself );
}

// func Object runWith(Array arguments)
dsClassBlock::nfRunWith::nfRunWith( const sInitData &init ) : dsFunction( init.clsBlock,
"runWith", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj ){
	p_AddParameter( init.clsArr ); // arguments
}
void dsClassBlock::nfRunWith::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself );
	dsClassBlock *clsBlock = ( dsClassBlock* )GetOwnerClass();
	
	if( ! nd->function ) DSTHROW( dueNullPointer );
	
	dsValue *args = rt->GetValue( 0 );
	dsValue *curArg;
	dsSignature *funcSig;
	dsClass *argType, *funcArgType;
	int i, count = 0, argsPushed = 0;
	
	if( ! args->GetRealObject() ) DSTHROW( dueNullPointer );
	
	// push arguments on stack and check signature the same time
	funcSig = nd->function->GetSignature();
	
	try{
		rt->RunFunction( args, "getCount", 0 );
		count = rt->GetReturnInt();
		
		if( count != funcSig->GetCount() ) DSTHROW( dseInvalidSignature );
		
		for( i=count-1; i>=0; i-- ){
			// get argument from array
			rt->PushInt( i );
			rt->RunFunction( args, "getAt", 1 );
			
			// cast to appropriate type
			argType = rt->GetReturnValue()->GetType();
			funcArgType = funcSig->GetParameter( i );
			curArg = rt->GetReturnValue();
			
			if( ! argType->IsEqualTo( funcArgType ) ){
				rt->CastValueTo( curArg, curArg, funcArgType );
			}
			
			rt->PushReturnValue();
			argsPushed++;
		}
		
		// run function with arguments
		clsBlock->RunIt( rt, myself );
		
		// remove arguments from stack
		if( argsPushed > 0 ){
			rt->RemoveValuesLeaveTop( argsPushed );
			argsPushed = 0;
		}
		
	}catch( ... ){
		if( argsPushed > 0 ) rt->RemoveValues( argsPushed );
		throw;
	}
}



// public func int hashCode()
dsClassBlock::nfHashCode::nfHashCode(const sInitData &init) : dsFunction(init.clsBlock,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassBlock::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = (sBlockNatData*)p_GetNativeData(myself);
	rt->PushInt( ( int )( intptr_t )nd->function );
}

// public func bool equals(Object obj)
dsClassBlock::nfEquals::nfEquals(const sInitData &init) : dsFunction(init.clsBlock,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsObj); // obj
}
void dsClassBlock::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = (sBlockNatData*)p_GetNativeData(myself);
	dsClassBlock *clsBlock = (dsClassBlock*)GetOwnerClass();
	dsValue *obj = rt->GetValue(0);
	// check if the object is a weak reference and not null
	if( p_IsObjOfType(obj, clsBlock) ){
		sBlockNatData *nd2 = (sBlockNatData*)p_GetNativeData(obj);
		rt->PushBool( nd->function == nd2->function );
	}else{
		rt->PushBool(false);
	}
}



// class dsClassBlock
/////////////////////////
// constructor, destructor
dsClassBlock::dsClassBlock() : dsClass("Block", DSCT_CLASS,
DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE) {
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(sizeof(sBlockNatData));
}
dsClassBlock::~dsClassBlock(){ }

// management
void dsClassBlock::CreateClassMembers(dsEngine *engine){
	sInitData init;
	
	// store classes
	init.clsBlock = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsBool = engine->GetClassBool();
	init.clsObj = engine->GetClassObject();
	init.clsArr = engine->GetClassArray();
	
	// functions
	AddFunction(new nfDestructor(init));  // function 0
	
	AddFunction(new nfRun0(init)); // function 1
	AddFunction(new nfRun1(init)); // function 2
	AddFunction(new nfRun2(init)); // function 3
	AddFunction(new nfRun3(init)); // function 4
	AddFunction(new nfRun4(init)); // function 5
	AddFunction(new nfRun5(init)); // function 6
	AddFunction(new nfRunWith(init)); // function 7
	
	AddFunction(new nfHashCode(init)); // function 8
	AddFunction(new nfEquals(init)); // function 9
	
	// store function indices for fast function calling in native code
	const dsFuncList &funcList = *GetFuncList();
	pFuncIndexRun0 = funcList.GetIndexOf( GetFunction( 1 ) );
	pFuncIndexRun1 = funcList.GetIndexOf( GetFunction( 2 ) );
	pFuncIndexRun2 = funcList.GetIndexOf( GetFunction( 3 ) );
	pFuncIndexRun3 = funcList.GetIndexOf( GetFunction( 4 ) );
	pFuncIndexRun4 = funcList.GetIndexOf( GetFunction( 5 ) );
	pFuncIndexRun5 = funcList.GetIndexOf( GetFunction( 6 ) );
	pFuncIndexRunWith = funcList.GetIndexOf( GetFunction( 7 ) );
}

dsRealObject *dsClassBlock::GetOwner( dsRealObject *myself ) const{
	if( ! myself ){
		DSTHROW( dueNullPointer );
	}
	return ( ( sBlockNatData* )p_GetNativeData( myself->GetBuffer() ) )->owner->GetRealObject();
}

int dsClassBlock::GetContextVariableCount( dsRealObject *myself ) const{
	if( ! myself ){
		DSTHROW( dueNullPointer );
	}
	return ( ( sBlockNatData* )p_GetNativeData( myself->GetBuffer() ) )->contextVariableCount;
}

dsValue *dsClassBlock::GetContextVariableAt( dsRealObject *myself, int index ) const{
	if( ! myself ){
		DSTHROW( dueNullPointer );
	}
	
	const sBlockNatData &nd = *( ( sBlockNatData* )p_GetNativeData( myself->GetBuffer() ) );
	
	if( index < 0 || index >= nd.contextVariableCount ){
		DSTHROW( dueOutOfBoundary );
	}
	
	return nd.contextVariables[ index ];
}

const dsSignature &dsClassBlock::GetSignature( dsRealObject *myself ) const{
	if( ! myself ){
		DSTHROW( dueNullPointer );
	}
	return *( ( sBlockNatData* )p_GetNativeData( myself->GetBuffer() ) )->function->GetSignature();
}

void dsClassBlock::CreateBlock( dsRunTime *rt, dsRealObject *owner, dsFunction *func, dsValue *value, int contextVariableCount ){
	if( ! rt || ! func || contextVariableCount < 0 ){
		DSTHROW( dueInvalidParam );
	}
	
	sBlockNatData *nd;
	dsValue *vowner = NULL;
	dsValue **vcvars = NULL;
	int vcvarCount = 0;
	
	rt->CreateObjectNaked( value, this );
	nd = ( sBlockNatData* )p_GetNativeData( value->GetRealObject()->GetBuffer() );
	nd->function = NULL;
	nd->owner = NULL;
	nd->contextVariables = NULL;
	nd->contextVariableCount = 0;
	
	try{
		vowner = rt->CreateValue();
		
		if( owner ){
			rt->SetObject( vowner, owner );
		}
		
		if( contextVariableCount > 0 ){
			vcvars = new dsValue*[ contextVariableCount ];
			
			while( vcvarCount < contextVariableCount ){
				vcvars[ vcvarCount ] = rt->CreateValue();
				vcvarCount++;
			}
		}
		
	}catch( ... ){
		if( vcvars ){
			while( vcvarCount > 0 ){
				vcvarCount--;
				rt->FreeValue( vcvars[ vcvarCount ] );
			}
			delete [] vcvars;
		}
		if( vowner ) rt->FreeValue( vowner );
		throw;
	}
	
	nd->function = func;
	nd->owner = vowner;
	nd->contextVariables = vcvars;
	nd->contextVariableCount = vcvarCount;
}

void dsClassBlock::CastArguments(dsRunTime *rt, dsSignature *funcSig, int argCount){
	if(!funcSig || argCount<0) DSTHROW(dueNullPointer);
	dsClass *argType, *funcArgType;
	dsValue *curArg;
	// check if the count of arguments is the same
	if(argCount != funcSig->GetCount()){
		DSTHROW(dseInvalidSignature);
	}
	// cast arguments on stack to the correct types
	for(int i=0; i<argCount; i++){
		curArg = rt->GetValue(i);
		argType = curArg->GetType();
		funcArgType = funcSig->GetParameter(i);
		if( !argType->IsEqualTo(funcArgType) ){
			rt->CastValueTo(curArg, curArg, funcArgType);
		}
	}
}

void dsClassBlock::RunIt( dsRunTime *rt, dsValue *myself ){
	sBlockNatData *nd = ( sBlockNatData* )p_GetNativeData( myself->GetRealObject()->GetBuffer() );
	
	#ifdef __DEBUG__
	if( ! nd->function->GetByteCode() ) DSTHROW( dueInvalidAction );
	#endif
	rt->p_ExecFunc( nd->owner->GetRealObject(), nd->function->GetOwnerClass(), nd->function, myself->GetRealObject() );
	
//	nd->function->RunFunction( rt, nd->owner, myself->GetRealObject() );
}
