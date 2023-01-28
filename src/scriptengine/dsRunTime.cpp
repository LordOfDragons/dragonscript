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
#include <stdlib.h>
#include "dragonscript_config.h"
#include "dsEngine.h"
#include "dsRunTime.h"
#include "dsMemoryManager.h"
#include "dsGarbageCollector.h"
#include "dsBaseEngineManager.h"
#include "utils/dsuArray.h"
#include "utils/dsuStack.h"
#include "utils/dsuString.h"
#include "objects/dsClass.h"
#include "objects/dsValue.h"
#include "objects/dsByteCode.h"
#include "objects/dsFunction.h"
#include "objects/dsFuncList.h"
#include "objects/dsVariable.h"
#include "objects/dsSignature.h"
#include "objects/dsRealObject.h"
#include "packages/default/dsClassString.h"
#include "packages/default/dsClassBlock.h"
#include "packages/default/dsClassException.h"

// #define __DEBUG__ 1

// definitions

// local classes
//////////////////
class dslCatchHandler : public dsuArrayable {
private:
	dsClass *p_Type;
	byte *p_Offset;
public:
	dslCatchHandler(dsClass *Type, byte *Offset){ p_Type = Type; p_Offset = Offset; }
	~dslCatchHandler(){ }
	inline dsClass *GetType() const{ return p_Type; }
	inline byte *GetOffset() const{ return p_Offset; }
};

class dslTryBlock : public dsuArrayable {
private:
	dsuArray p_Catches;
	int p_LocVarCount;
public:
	dslTryBlock(int LocVarCount){
		if(LocVarCount < 0) DSTHROW(dueInvalidParam);
		p_LocVarCount = LocVarCount;
	}
	~dslTryBlock(){ }
	inline int GetCount() const{ return p_Catches.Length(); }
	inline int GetLocVarCount() const{ return p_LocVarCount; }
	void AddCatch(dsClass *Type, byte *Offset){
		if((Type->GetPrimitiveType() < DSPT_OBJECT) || !Offset) DSTHROW(dueInvalidParam);
		dslCatchHandler *vNewCatch = new dslCatchHandler(Type, Offset);
		if(!vNewCatch) DSTHROW(dueOutOfMemory);
		p_Catches.Add(vNewCatch);
	}
	inline dslCatchHandler *GetCatch(int Index) const{ return (dslCatchHandler*)p_Catches[Index]; }
	dslCatchHandler *FindCatch(dsClass *Type) const{
		if(Type->GetPrimitiveType() < DSPT_OBJECT) return NULL;
		dslCatchHandler *vCatch;
		for(int i=0; i<p_Catches.Length(); i++){
			vCatch = (dslCatchHandler*)p_Catches[i];
			if(Type->CastableTo(vCatch->GetType())) return vCatch;
		}
		return NULL;
	}
};


// class dsRunTime
/////////////////////////////

// constructor, destructor
dsRunTime::dsRunTime(dsEngine *Engine){
	if(!Engine) DSTHROW(dueInvalidParam);
	// engine informations
	p_Engine = Engine;
	p_ClsVoid = Engine->GetClassVoid();
	p_ClsByte = Engine->GetClassByte();
	p_ClsBool = Engine->GetClassBool();
	p_ClsInt = Engine->GetClassInt();
	p_ClsFloat = Engine->GetClassFloat();
	p_ClsObj = Engine->GetClassObject();
	p_ClsNull = Engine->GetClassNull();
	p_ClsStr = (dsClassString*)Engine->GetClassString();
	p_ClsBlock = (dsClassBlock*)Engine->GetClassBlock();
	p_ClsExcep = (dsClassException*)Engine->GetClassException();
	// exception tracing
	p_RaisedException = NULL;
	p_Locked = false;
	// stack
	p_Stack = NULL;
	p_StackSize = 0;
	p_StackTop = 0;
	p_retVal = NULL;
	// create all (save)
	try{
		p_Stack = Engine->GetMemoryManager()->AllocateValueStrip(1024);
		for(p_StackSize=0; p_StackSize<1024; p_StackSize++){
			p_Stack[p_StackSize].SetEmpty(p_ClsObj);
		}
		p_retVal = CreateValue(p_ClsObj);
		p_RaisedException = CreateValue(p_ClsExcep);
	}catch( ... ){
		if(p_RaisedException) FreeValue(p_RaisedException);
		if(p_retVal) FreeValue(p_retVal);
		if(p_Stack){
			for(int i=0; i<p_StackSize; i++) ClearValue(p_Stack + i);
			Engine->GetMemoryManager()->FreeValueStrip(p_Stack);
		}
		throw;
	}
}
dsRunTime::~dsRunTime(){
	if( p_Locked ){
		return; /*DSTHROW( dseEngineLocked );*/ // otherwise terminates
	}
	
	// no clearing of values is allowed here anymore. this is done by CleanUp() and has to be
	// called during an appropriate time. the destructor is only allowed to free memory since
	// classes very well do not exist anymore by now
	p_Engine->GetMemoryManager()->FreeValueStrip(p_Stack);
	p_Engine->GetMemoryManager()->FreeValue( p_retVal );
	p_Engine->GetMemoryManager()->FreeValue( p_RaisedException );
}



// Management
///////////////

void dsRunTime::CleanUp(){
	ClearStack();
	ClearValue( p_retVal );
	ClearValue( p_RaisedException );
}


// stack management
void dsRunTime::PushValue(dsValue *Value){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	CopyValue(Value, p_Stack + p_StackTop);
	p_StackTop++;
}
void dsRunTime::PushObject(dsRealObject *Object, dsClass *Type){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	p_Stack[p_StackTop].SetRealObject(Type, Object);
	if(Object){
		p_Stack[p_StackTop].SetFuncBase(Object->GetType()->GetImplFuncBase(Type));
		Object->IncRefCount();
	}
	p_StackTop++;
}
void dsRunTime::PushObject(dsRealObject *Object){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	p_Stack[p_StackTop].SetRealObject(p_ClsObj, Object);
	if(Object){
		p_Stack[p_StackTop].SetFuncBase(Object->GetType()->GetImplFuncBase(p_ClsObj));
		Object->IncRefCount();
	}
	p_StackTop++;
}
void dsRunTime::PushByte(byte Value){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	p_Stack[p_StackTop].SetByte(p_ClsByte, Value);
	p_StackTop++;
}
void dsRunTime::PushBool(bool Value){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	p_Stack[p_StackTop].SetBool(p_ClsBool, Value);
	p_StackTop++;
}
void dsRunTime::PushInt(int Value){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	p_Stack[p_StackTop].SetInt(p_ClsInt, Value);
	p_StackTop++;
}
void dsRunTime::PushFloat(float Value){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	p_Stack[p_StackTop].SetFloat(p_ClsFloat, Value);
	p_StackTop++;
}
void dsRunTime::PushString(const char *Value){
	if(!Value) DSTHROW(dueInvalidParam);
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	p_Stack[p_StackTop].SetEmpty(p_ClsStr);
	CreateObjectNaked(p_Stack + p_StackTop, p_ClsStr);
	try{
		p_ClsStr->SetRealObjectString(p_Stack[p_StackTop].GetRealObject(), Value);
	}catch( ... ){
		ClearValue(p_Stack + p_StackTop); throw;
	}
	p_StackTop++;
}
void dsRunTime::PushReturnValue(){
	if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
	CopyValue(p_retVal, p_Stack + p_StackTop);
	p_StackTop++;
}
void dsRunTime::PopValue(dsValue *Value){
	if(!CanPopValue()) DSTHROW(dueStackEmpty);
	MoveValue(p_Stack + p_StackTop-1, Value);
	p_StackTop--;
}
dsValue *dsRunTime::GetValue(int Index) const{
	if(Index<0 || Index>=p_StackTop) DSTHROW(dueOutOfBoundary);
	return p_Stack + p_StackTop-1-Index;
}
void dsRunTime::RemoveValues(int Count){
	if(Count<0 || Count>p_StackTop) DSTHROW(dueOutOfBoundary);
	for(int i=0; i<Count; i++){
		ClearValue(p_Stack + p_StackTop-1);
		p_StackTop--;
	}
}
void dsRunTime::RemoveValuesLeaveTop(int Count){
	if(Count<0 || Count>p_StackTop-1) DSTHROW(dueOutOfBoundary);
	for(int i=0; i<Count; i++){
		MoveValue(p_Stack + p_StackTop-1, p_Stack + p_StackTop-2);
		p_StackTop--;
	}
}
void dsRunTime::ClearStack(){
	// exception tracing
	ClearExceptionTrace();
	// clear stack
	ClearValue(p_retVal);
	for(int i=0; i<p_StackSize; i++) ClearValue(p_Stack + i);
	p_StackTop = 0;
}
int dsRunTime::SaveStackPointer(){
	return p_StackTop;
}
void dsRunTime::RestoreStackPointer(int savePointer){
	RemoveValues(p_StackTop - savePointer);
}
// weak references
void dsRunTime::AddWeakRef(dsRealObject *object){
	if(!object) DSTHROW(dueInvalidParam);
	object->IncWeakRefCount();
}
void dsRunTime::RemoveWeakRef(dsRealObject *object){
	if(!object) DSTHROW(dueInvalidParam);
	object->DecWeakRefCount();
	// free object only if both ref count and weak ref count are 0
	if(object->GetWeakRefCount()==0 && object->GetRefCount()==0){
		p_Engine->GetGarbageCollector()->FreeObject(object);
	}
}
// value management
dsValue *dsRunTime::CreateValue(){
	dsValue *value = p_Engine->GetMemoryManager()->AllocateValue();
	value->SetEmpty(p_ClsObj);
	return value;
}
dsValue *dsRunTime::CreateValue(dsClass *Type){
	if(!Type) DSTHROW(dueInvalidParam);
	dsValue *value = p_Engine->GetMemoryManager()->AllocateValue();
	value->SetEmpty(Type);
	return value;
}
void dsRunTime::FreeValue(dsValue *Value){
	ClearValue(Value);
	p_Engine->GetMemoryManager()->FreeValue(Value);
}
void dsRunTime::CopyValue(dsValue *From, dsValue *To){
	if(!From || !To) DSTHROW(dueInvalidParam);
	dsRealObject *vObj;
	// free old value
	ClearValue(To);
	// set new value
	To->SetValue(From);
	if(From->GetType()->GetPrimitiveType() == DSPT_OBJECT){
		vObj = From->GetRealObject();
		if(vObj) vObj->IncRefCount();
	}
}
void dsRunTime::MoveValue(dsValue *From, dsValue *To){
	if(!From || !To) DSTHROW(dueInvalidParam);
	// free old value
	ClearValue(To);
	// set new value
	To->SetValue(From);
	if(From->GetType()->GetPrimitiveType() == DSPT_OBJECT){
		From->SetNull(p_ClsObj);
	}
}

//static int dummy = 0;
void dsRunTime::ClearValue(dsValue *Value){
	if(!Value) DSTHROW(dueInvalidParam);
	dsClass *vType;
	dsRealObject *obj;
	// only operate on real objects
	if(Value->GetType()->GetPrimitiveType() != DSPT_OBJECT) return;
	obj = Value->GetRealObject();
	// only operate on a real object that is not null
	if(!obj) return;
	//for(int I=0; I<dummy; I++) printf( "  " ); printf( "ClearValue %p(%i) IN\n", obj, obj->GetRefCount() ); dummy++;
	// check for reference count errors
	obj->DecRefCount();
	if(obj->GetRefCount() < 0){
//		if(!p_GCCleanUpMode){
//			p_Engine->PrintMessageFormat("[WARNING] Real Object of type %s reached negative Reference Count!\n", obj->GetType()->GetName());
//		}
// 		DSTHROW( dueInvalidParam );
		obj->IncRefCount();
		return;
	}
	// check if the reference count reached 0
	if( obj->GetRefCount() == 0 ){
		// free object content. it is possible the destructor sets the variable we
		// are working on to NULL. to prevent problems in this case the object is
		// first held in a temporary value. we still hold the reference without
		// using CopyValue to properly handle the destruction
		vType = obj->GetType();
		obj->IncRefCount();
		
		dsValue safeguard( vType, obj );
		
		//for(int I=0; I<dummy; I++) printf( "  " ); printf( "destructor object %p refcount %i\n", obj, obj->GetRefCount() ); dummy++;
		try{
			vType->Destructor( this, &safeguard );
			
		}catch( const duException &e ){
			p_PrepareException( e );
			PrintExceptionTrace();
			ClearExceptionTrace();
			
		}catch( ... ){
			PrintExceptionTrace();
			ClearExceptionTrace();
		}
		
		//dummy--; for(int I=0; I<dummy; I++) printf( "  " ); printf( "free variables object %p refcount %i\n", obj, obj->GetRefCount() ); dummy++;
		try{
			vType->FreeVariables( this, &safeguard );
			
		}catch( const duException &e ){
			p_PrepareException( e );
			PrintExceptionTrace();
			ClearExceptionTrace();
		}catch( ... ){
			PrintExceptionTrace();
			ClearExceptionTrace();
		}
		
		obj->DecRefCount();
		//dummy--; for(int I=0; I<dummy; I++) printf( "  " ); printf( "free object object %p refcount %i\n", obj, obj->GetRefCount() );
		
		// free object only if there is not still a weak reference around
		if( obj->GetWeakRefCount() == 0 ){
			p_Engine->GetGarbageCollector()->FreeObject( obj );
			
		}else{
			obj->Clear();
		}
	}
	// set to null so nothing happens later
	Value->SetNull(p_ClsObj);
	//dummy--; for(int I=0; I<dummy; I++) printf( "  " ); printf( "ClearValue %p OUT\n", obj );
}

void dsRunTime::SetByte(dsValue *Value, byte Data){
	ClearValue(Value);
	Value->SetByte(p_ClsByte, Data);
}
void dsRunTime::SetBool(dsValue *Value, bool Data){
	ClearValue(Value);
	Value->SetBool(p_ClsBool, Data);
}
void dsRunTime::SetInt(dsValue *Value, int Data){
	ClearValue(Value);
	Value->SetInt(p_ClsInt, Data);
}
void dsRunTime::SetFloat(dsValue *Value, float Data){
	ClearValue(Value);
	Value->SetFloat(p_ClsFloat, Data);
}
void dsRunTime::SetString(dsValue *Value, const char *Data){
	ClearValue(Value);
	CreateObjectNaked(Value, p_ClsStr);
	try{
		p_ClsStr->SetRealObjectString(Value->GetRealObject(), Data);
	}catch( ... ){
		ClearValue(Value); throw;
	}
}
void dsRunTime::SetObject(dsValue *Value, dsRealObject *RealObject){
	ClearValue(Value);
	if(!RealObject) DSTHROW(dueInvalidParam);
	Value->SetRealObject(RealObject->GetType(), RealObject);
	RealObject->IncRefCount();
}
void dsRunTime::SetNull(dsValue *Value, dsClass *Type){
	if(!Type) DSTHROW(dueInvalidParam);
	if(Type->GetPrimitiveType() < DSPT_NULL) DSTHROW(dseInvalidType);
	ClearValue(Value);
	Value->SetNull(Type);
}
bool dsRunTime::EqualValue(dsValue *Val1, dsValue *Val2){
	if(!Val1 || !Val2) DSTHROW(dueInvalidParam);
	// type has to match
	dsClass *type = Val1->GetType();
	if(type != Val2->GetType()) return false;
	// value has to match
	switch(type->GetPrimitiveType()){
	case DSPT_BYTE:
		return Val1->GetByte() == Val2->GetByte();
	case DSPT_BOOL:
		return Val1->GetBool() == Val2->GetBool();
	case DSPT_INT:
		return Val1->GetInt() == Val2->GetInt();
	case DSPT_FLOAT:
		return Val1->GetFloat() == Val2->GetFloat();
	case DSPT_NULL:
		return true;
	case DSPT_OBJECT:
		return Val1->GetRealObject() == Val2->GetRealObject();
	}
	return false;
}
void dsRunTime::CreateObject(dsValue *Value, dsClass *Type, int ArgCount){
	if(!Value || !Type || ArgCount<0) DSTHROW(dueInvalidParam);
	if(Type->GetPrimitiveType() != DSPT_OBJECT) DSTHROW(dseInvalidType);
	if(Type->GetTypeModifiers() & DSTM_ABSTRACT) DSTHROW(dueInvalidParam);
	if(p_Locked) DSTHROW(dseEngineLocked);
	dsFunction *vConstrFunc;
	dsSignature vCallSig;
	dsRealObject *obj;
	try{
		// clear old value
		ClearValue(Value);
		// construct signature to look for from arguments on the stack
		for(int i=0; i<ArgCount; i++) vCallSig.AddParameter(p_Stack[p_StackTop-1-i].GetType());
		// find constructor function
		vConstrFunc = Type->GetFuncList()->FindBestFunction(Type, DSFUNC_CONSTRUCTOR, &vCallSig);
		if(!vConstrFunc){
			p_Engine->PrintMessageFormat("[wtf?] %s (%i)", Type->GetName(), ArgCount);
			for(int d=0; d<ArgCount; d++)
				p_Engine->PrintMessageFormat("  param %s", vCallSig.GetParameter(d)->GetName());
			DSTHROW(dueInvalidParam);
		}
		if(vConstrFunc->GetFuncType() != DSFT_CONSTRUCTOR) DSTHROW(dueInvalidParam);
		// create real object
		obj = p_Engine->GetGarbageCollector()->CreateObject(Type);
		Type->InitVariables(obj);
		// call the constructor
		Value->SetRealObject(Type, obj);
		obj->IncRefCount();
		vConstrFunc->RunFunction( this, Value );
		if(ArgCount > 0) RemoveValues(ArgCount);
	}catch( ... ){
		ClearValue(Value);
		if(ArgCount > 0) RemoveValues(ArgCount);
		throw;
	}
}

void dsRunTime::CreateObjectNakedOnStack( dsClass *type ){
	if( ! type ){
		DSTHROW( dueInvalidParam );
	}
	if( type->GetPrimitiveType() != DSPT_OBJECT ){
		DSTHROW( dseInvalidType );
	}
	if( type->GetTypeModifiers() & DSTM_ABSTRACT ){
		DSTHROW( dueInvalidParam );
	}
	if( p_Locked ){
		DSTHROW( dseEngineLocked );
	}
	
	dsRealObject * const object = p_Engine->GetGarbageCollector()->CreateObject( type );
	try{
		type->InitVariables( object );
		PushObject( object, type );
		
	}catch( ... ){
		p_Engine->GetGarbageCollector()->FreeObject( object );
		throw;
	}
}

void dsRunTime::CreateObjectNaked(dsValue *Value, dsClass *Type){
	if(!Value || !Type) DSTHROW(dueInvalidParam);
	if(Type->GetPrimitiveType() != DSPT_OBJECT) DSTHROW(dseInvalidType);
	if(Type->GetTypeModifiers() & DSTM_ABSTRACT) DSTHROW(dueInvalidParam);
	if(p_Locked) DSTHROW(dseEngineLocked);
	dsRealObject *obj;
	// free old value
	ClearValue(Value);
	// create a new real object with initialized variables but no constructor is
	// invoked. used for native classes with special constructors.
	obj = p_Engine->GetGarbageCollector()->CreateObject(Type);
	Type->InitVariables(obj);
	// place object in the value
	Value->SetRealObject(Type, obj);
	obj->IncRefCount();
}
void dsRunTime::CreateAndPushObject(dsClass *Type, int ArgCount){
	dsValue vdata(Type);
	CreateObject(&vdata, Type, ArgCount);
	try{
		MoveValue(&vdata, p_Stack + p_StackTop);
		p_StackTop++;
	}catch( ... ){
		ClearValue(&vdata); throw;
	}
}

void dsRunTime::CastValueTo( dsValue *from, dsValue *to, dsClass *toType ){
	if( ! from || ! to || ! toType ) DSTHROW( dueInvalidParam );
	
	dsClass *tmpType;
	dsRealObject *tmpObj;
	
	switch( from->GetType()->GetPrimitiveType() ){
	// cast from byte
	case DSPT_BYTE:
		switch( toType->GetPrimitiveType() ){
		case DSPT_BOOL:
			SetBool( to, from->GetByte() != 0 );
			break;
			
		case DSPT_INT:
			SetInt( to, ( int )from->GetByte() );
			break;
			
		case DSPT_FLOAT:
			SetFloat( to, ( float )from->GetByte() );
			break;
			
		case DSPT_BYTE:
			SetByte( to, from->GetByte() );
			break;
			
		case DSPT_OBJECT:
			if( ! from->GetType()->CastableTo( toType ) ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, toType ) );
			}
			SetByte( to, from->GetByte() );
			break;
			
		default:
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, p_ClsNull ) );
		}
		break;
		
	// cast from bool
	case DSPT_BOOL:
		switch( toType->GetPrimitiveType() ){
		case DSPT_BYTE:
			SetByte( to, from->GetBool() ? 1 : 0 );
			break;
			
		case DSPT_INT:
			SetInt( to, from->GetBool() ? 1 : 0 );
			break;
			
		case DSPT_FLOAT:
			SetFloat( to, from->GetBool() ? 1.0f : 0.0f );
			break;
			
		case DSPT_BOOL:
			SetBool( to, from->GetBool() );
			break;
			
		case DSPT_OBJECT:
			if( ! from->GetType()->CastableTo( toType ) ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, toType ) );
			}
			SetBool( to, from->GetBool() );
			break;
			
		default:
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, p_ClsNull ) );
		}
		break;
		
	// cast from int
	case DSPT_INT:
		switch( toType->GetPrimitiveType() ){
		case DSPT_BYTE:
			SetByte( to, ( byte )from->GetInt() );
			break;
			
		case DSPT_BOOL:
			SetBool( to, from->GetInt() != 0 );
			break;
			
		case DSPT_FLOAT:
			SetFloat( to, ( float )from->GetInt() );
			break;
			
		case DSPT_INT:
			SetInt( to, from->GetInt() );
			break;
			
		case DSPT_OBJECT:
			if( ! from->GetType()->CastableTo( toType ) ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, toType ) );
			}
			SetInt( to, from->GetInt() );
			break;
			
		default:
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, p_ClsNull ) );
		}
		break;
		
	// cast from float
	case DSPT_FLOAT:
		switch( toType->GetPrimitiveType() ){
		case DSPT_BYTE:
			SetByte( to, ( byte )from->GetFloat() );
			break;
			
		case DSPT_BOOL:
			SetBool( to, from->GetFloat() != 0.0f );
			break;
			
		case DSPT_INT:
			SetInt( to, ( int )from->GetFloat() );
			break;
			
		case DSPT_FLOAT:
			SetFloat( to, from->GetFloat() );
			break;
			
		case DSPT_OBJECT:
			if( ! from->GetType()->CastableTo( toType ) ){
				DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, toType ) );
			}
			SetFloat( to, from->GetFloat() );
			break;
			
		default:
			DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, p_ClsNull ) );
		}
		break;
		
	// cast from null
	case DSPT_NULL:
		p_Engine->PrintMessage( "[VEEERRY BAD! NULL]" );
		DSTHROW( dueInvalidAction );
		
	// cast from object
	case DSPT_OBJECT:
		if( toType->GetPrimitiveType() == DSPT_BOOL ){
			SetBool( to, from->GetRealObject() != NULL );
			
		}else{
			tmpType = from->GetType();
			tmpObj = from->GetRealObject();
			
			if( tmpObj ){
				tmpType = tmpObj->GetType();
				if( ! tmpType->CastableTo( toType ) ){
					DSTHROW_INFO( dseInvalidCast, ErrorCastInfo( from, toType ) );
				}
			}
			
			if( from != to ){
				CopyValue( from, to );
			}
			to->SetType( toType );
			to->SetFuncBase( tmpType->GetImplFuncBase( toType ) );
		}
		break;
	}
}



// Find Functions
///////////////////

dsFunction *dsRunTime::FindRunFunction( const dsClass *objectClass, const char *funcName, int argCount ){
	if( ! objectClass || ! funcName || argCount < 0 ){
		DSTHROW( dueInvalidParam );
	}
	
	// construct signature to look for from arguments on the stack
	dsSignature callSig;
	int i;
	
	for( i=0; i<argCount; i++ ){
		callSig.AddParameter( p_Stack[ p_StackTop - 1 - i ].GetType() );
	}
	
	// find function
	return objectClass->GetFuncList()->FindBestFunction( NULL, funcName, &callSig );
}

int dsRunTime::FindRunFunctionIndex( const dsClass *objectClass, const char *funcName, int argCount ){
	dsFunction * const function = FindRunFunction( objectClass, funcName, argCount );
	if( function ){
		return objectClass->GetFuncList()->GetIndexOf( function );
		
	}else{
		return -1;
	}
}

dsFunction *dsRunTime::FindRunFunction( dsValue *value, const char *funcName, int argCount ){
	if( ! value ){
		DSTHROW( dueInvalidParam );
	}
	
	const dsClass *callType = value->GetType();
	
	if( callType->GetPrimitiveType() == DSPT_OBJECT ){
		const dsRealObject * const obj = value->GetRealObject();
		if( ! obj ){
			DSTHROW( dueNullPointer );
		}
		return FindRunFunction( obj->GetType(), funcName, argCount );
		
	}else{
		return FindRunFunction( callType, funcName, argCount );
	}
}

int dsRunTime::FindRunFunctionIndex( dsValue *value, const char *funcName, int argCount ){
	if( ! value ){
		DSTHROW( dueInvalidParam );
	}
	
	const dsClass *callType = value->GetType();
	
	if( callType->GetPrimitiveType() == DSPT_OBJECT ){
		const dsRealObject * const obj = value->GetRealObject();
		if( ! obj ){
			DSTHROW( dueNullPointer );
		}
		return FindRunFunctionIndex( obj->GetType(), funcName, argCount ) - value->GetFuncBase();
		
	}else{
		return FindRunFunctionIndex( callType, funcName, argCount ) - value->GetFuncBase();
	}
}

// function execution
// IMPORTANT NOTE:
// Before calling the functions listed here you have to push the
// arguments on the stack using the Push* functions. The order
// is inverse as you defined in the Function, thus push first
// parameter N then parameter N-1 until parameter 0.
// The return value can then be retrieved using one of the
// GetReturn* functions provided.
void dsRunTime::RunFunction(dsValue *Value, const char *FuncName, int ArgCount ){
	if(!Value || !FuncName || ArgCount<0) DSTHROW(dueInvalidParam);
	dsClass *vCallType;
	dsFunction *vFunc;
	dsFuncList *vFuncList;
	dsSignature vCallSig;
	dsRealObject *obj=NULL;
	try{
		// construct signature to look for from arguments on the stack
		for(int i=0; i<ArgCount; i++) vCallSig.AddParameter(p_Stack[p_StackTop-1-i].GetType());
		// find function
		vCallType = Value->GetType();
		if(vCallType->GetPrimitiveType() == DSPT_OBJECT){
			obj = Value->GetRealObject();
			if(!obj) DSTHROW(dueNullPointer);
			vFuncList = obj->GetType()->GetFuncList();
		}else{
			vFuncList = vCallType->GetFuncList();
		}
		vFunc = vFuncList->FindBestFunction(NULL, FuncName, &vCallSig);
		if(!vFunc) DSTHROW_INFO(dseInvalidSignature, FuncName);
		// run function
		dsValue vdata(vCallType, obj);
		if(obj) obj->IncRefCount();
		try{
			vFunc->RunFunction( this, Value );
		}catch( ... ){
			ClearValue(&vdata); throw;
		}
		ClearValue(&vdata);
		if(vFunc->HasRetVal()) PopValue(p_retVal);
		// remove arguments from stack
		if(ArgCount > 0) RemoveValues(ArgCount);
	}catch( ... ){
//		PrintExceptionTrace();
		if(ArgCount > 0) RemoveValues(ArgCount);
		throw;
	}
	
}

void dsRunTime::RunFunction(dsValue *Value, int FuncIndex, int ArgCount ){
	if(!Value || FuncIndex<0 || ArgCount<0) DSTHROW(dueInvalidParam);
	dsClass *vCallType;
	dsFunction *vFunc;
	dsFuncList *vFuncList;
	dsRealObject *obj=NULL;
	try{
		// find function
		vCallType = Value->GetType();
		if(vCallType->GetPrimitiveType() == DSPT_OBJECT){
			obj = Value->GetRealObject();
			if(!obj) DSTHROW(dueNullPointer);
			vFuncList = obj->GetType()->GetFuncList();
		}else{
			vFuncList = vCallType->GetFuncList();
		}
		vFunc = vFuncList->GetFunction(FuncIndex);
		if(!IsStackMatchingSig(vFunc->GetSignature(), ArgCount)) DSTHROW(dseInvalidSignature);
		// run function
		dsValue vdata(vCallType, obj);
		if(obj) obj->IncRefCount();
		try{
			vFunc->RunFunction( this, Value );
		}catch( ... ){
			ClearValue(&vdata); throw;
		}
		ClearValue(&vdata);
		if(vFunc->HasRetVal()) PopValue(p_retVal);
		// remove arguments from stack
		if(ArgCount > 0) RemoveValues(ArgCount);
	}catch( ... ){
//		PrintExceptionTrace();
		if(ArgCount > 0) RemoveValues(ArgCount);
		throw;
	}
}

void dsRunTime::RunFunctionFast( dsValue *value, dsFunction *function ){
	if( ! value || ! function ){
		DSTHROW( dueInvalidParam );
	}
	
	// find object to call the function on
	dsClass * const callType = value->GetType();
	dsRealObject *object = NULL;
	
	if( callType->GetPrimitiveType() == DSPT_OBJECT ){
		object = value->GetRealObject();
		if( ! object ){
			DSTHROW( dueNullPointer );
		}
	}
	
	try{
		// run function
		dsValue vdata( callType, object );
		if( object ){
			object->IncRefCount();
		}
		
		try{
			function->RunFunction( this, value );
			
		}catch( ... ){
			ClearValue( &vdata );
			throw;
		}
		ClearValue( &vdata );
		
		if( function->HasRetVal() ){
			PopValue( p_retVal );
		}
		
		// remove arguments from stack
		RemoveValues( function->GetSignature()->GetCount() );
		
	}catch( ... ){
		RemoveValues( function->GetSignature()->GetCount() );
		throw;
	}
}

void dsRunTime::RunFunctionFast( dsValue *value, int funcIndex ){
	if( ! value || funcIndex < 0 ){
		DSTHROW( dueInvalidParam );
	}
	
	// find function
	dsClass * const callType = value->GetType();
	const dsFuncList *funcList;
	dsRealObject *object = NULL;
	
	if( callType->GetPrimitiveType() == DSPT_OBJECT ){
		object = value->GetRealObject();
		if( ! object ){
			DSTHROW( dueNullPointer );
		}
		
		funcList = object->GetType()->GetFuncList();
		
	}else{
		funcList = callType->GetFuncList();
	}
	
	dsFunction * const function = funcList->GetFunction( value->GetFuncBase() + funcIndex );
	
	try{
		// run function
		dsValue vdata( callType, object );
		if( object ){
			object->IncRefCount();
		}
		
		try{
			function->RunFunction( this, value );
			
		}catch( ... ){
			ClearValue( &vdata );
			throw;
		}
		ClearValue( &vdata );
		
		if( function->HasRetVal() ){
			PopValue( p_retVal );
		}
		
		// remove arguments from stack
		RemoveValues( function->GetSignature()->GetCount() );
		
	}catch( ... ){
		RemoveValues( function->GetSignature()->GetCount() );
		throw;
	}
}

void dsRunTime::RunStaticFunction(const char *Type, const char *Function, int ArgCount){
	if(!Type || !Function || ArgCount<0) DSTHROW(dueInvalidParam);
	dsClass *vClass;
	dsFunction *vFunc;
	dsSignature vCallSig;
	try{
		if(!(vClass = p_Engine->GetClass(Type))) DSTHROW(dseInvalidType);
		// construct signature to look for from arguments on the stack
		for(int i=0; i<ArgCount; i++) vCallSig.AddParameter(p_Stack[p_StackTop-1-i].GetType());
		// find function
		if(!(vFunc = vClass->FindFunction(Function, &vCallSig, true))) DSTHROW(dseInvalidSignature);
		if(!(vFunc->GetTypeModifiers() & DSTM_STATIC)) DSTHROW(dseInvalidSignature);
		// run function
		dsValue vdata(vClass);
		vFunc->RunFunction( this, &vdata );
		if(vFunc->HasRetVal()) PopValue(p_retVal);
		// remove arguments from stack
		if(ArgCount > 0) RemoveValues(ArgCount);
	}catch( ... ){
//		PrintExceptionTrace();
		if(ArgCount > 0) RemoveValues(ArgCount);
		throw;
	}
}
void dsRunTime::RunStaticFunction(dsClass *type, int FuncIndex, int ArgCount){
	if(!type || FuncIndex<0 || ArgCount<0) DSTHROW(dueInvalidParam);
	dsFunction *vFunc;
	try{
		// find function
		vFunc = type->GetFuncList()->GetFunction(FuncIndex);
		if(!IsStackMatchingSig(vFunc->GetSignature(), ArgCount)) DSTHROW(dseInvalidSignature);
		// run function
		dsValue vdata(type);
		vFunc->RunFunction( this, &vdata );
		if(vFunc->HasRetVal()) PopValue(p_retVal);
		// remove arguments from stack
		if(ArgCount > 0) RemoveValues(ArgCount);
	}catch( ... ){
//		PrintExceptionTrace();
		if(ArgCount > 0) RemoveValues(ArgCount);
		throw;
	}
}
bool dsRunTime::IsStackMatchingSig(dsSignature *sig, int argCount){
	if(!sig || argCount<0) DSTHROW(dueInvalidParam);
	int i;
	dsSignature callSig;
	dsClass *type1, *type2;
	// determine call sig
	for(i=0; i<argCount; i++) callSig.AddParameter(p_Stack[p_StackTop-1-i].GetType());
	// determine if the stack matches the signature. this means all types must match
	// with one exception. if the type in the signature is 'Object' also primitive
	// types are considered equal as they can not take on the 'Object' type
	if(argCount != sig->GetCount()) return false;
	for(i=0; i<argCount; i++){
		type1 = callSig.GetParameter(i);
		type2 = sig->GetParameter(i);
		if(!type1->IsEqualTo(type2)){
			if(!type2->IsEqualTo(p_ClsObj) || type1->GetPrimitiveType()==DSPT_OBJECT) return false;
		}
	}
	return true;
}
byte dsRunTime::GetReturnByte() const{
	return p_retVal->GetByte();
}
bool dsRunTime::GetReturnBool() const{
	return p_retVal->GetBool();
}
int dsRunTime::GetReturnInt() const{
	return p_retVal->GetInt();
}
float dsRunTime::GetReturnFloat() const{
	return p_retVal->GetFloat();
}
const char *dsRunTime::GetReturnString() const{
	return p_retVal->GetString();
}
dsValue *dsRunTime::GetReturnValue() const{
	return p_retVal;
}
dsRealObject *dsRunTime::GetReturnRealObject() const{
	return p_retVal->GetRealObject();
}
// exception handling
void dsRunTime::ThrowException(dsValue *Exception){
	if(!Exception) DSTHROW(dueInvalidParam);
	// check type of exception object
	dsClass *vType = Exception->GetType();
	dsRealObject *obj = Exception->GetRealObject();
	if(vType->GetPrimitiveType() == DSPT_OBJECT){
		if(obj) vType = obj->GetType();
	}
	if(!vType->CastableTo(p_ClsExcep)) DSTHROW(dseInvalidType);
	// store exception object and throw it
	CopyValue(Exception, p_RaisedException);
	DSTHROW(dseScriptedException);
}
void dsRunTime::AddNativeTrace(dsFunction *function, int line){
	if(!function || line<0) DSTHROW(dueInvalidParam);
	if(!p_RaisedException->GetRealObject()) DSTHROW(dueInvalidParam);
	p_ClsExcep->AddTrace( p_RaisedException, function->GetOwnerClass(), function, line );
}
void dsRunTime::PrintExceptionTrace(){
	p_ClsExcep->PrintTrace(this, p_RaisedException);
}
void dsRunTime::ClearExceptionTrace(){
	ClearValue(p_RaisedException);
}

// private functions
void dsRunTime::p_PrepareException(const duException &ex){
	// if the exception is a scripted exception all is ok
	if(ex.IsNamed("ScriptedException")) return;
	// if there is already an exception object skip too
	if(p_RaisedException->GetRealObject()) return;
	// otherwise create an object for this exception
	dsClass *vType = NULL;
	if(ex.IsNamed("OutOfMemory")){
		vType = p_Engine->GetClass("EOutOfMemory");
	}else if(ex.IsNamed("OutOfBounds")){
		vType = p_Engine->GetClass("EOutOfBoundary");
	}else if(ex.IsNamed("NullPointer")){
		vType = p_Engine->GetClass("ENullPointer");
	}else if(ex.IsNamed("DivisionByZero")){
		vType = p_Engine->GetClass("EDivisionByZero");
	}else if(ex.IsNamed("InvalidCast")){
		vType = p_Engine->GetClass("EInvalidCast");
	}else if(ex.IsNamed("InvalidParam")){
		vType = p_Engine->GetClass("EInvalidParam");
	}else if(ex.IsNamed("InvalidAction")){
		vType = p_Engine->GetClass("EInvalidAction");
	}else{ // this should never happen and we use the common exception object for this
		dsValue vdata(p_ClsExcep);
		try{
			PushString("Invalid/Unknown exception occured!");
			CreateObject(&vdata, p_ClsExcep, 1);
			MoveValue(&vdata, p_RaisedException);
		}catch( ... ){
			ClearValue(&vdata); throw;
		}
		return;
	}
	// create the exception object if not already existing
	dsValue vdata(vType);
	try{
		if(ex.GetInfo().IsEmpty()){
			CreateObject(&vdata, vType, 0);
		}else{
			PushString( ex.GetInfo() );
			CreateObject(&vdata, vType, 1);
		}
		MoveValue(&vdata, p_RaisedException);
	}catch( ... ){
		ClearValue(&vdata); throw;
	}
}

void dsRunTime::pPrepareUnknownException(){
	// if there is already an exception object skip too
	if( p_RaisedException->GetRealObject() ){
		return;
	}
	
	// otherwise create an object for this exception
	dsValue vdata( p_ClsExcep );
	try{
		PushString( "Invalid/Unknown exception occured!" );
		CreateObject( &vdata, p_ClsExcep, 1 );
		MoveValue( &vdata, p_RaisedException );
		
	}catch( ... ){
		ClearValue( &vdata );
		throw;
	}
}

void dsRunTime::p_PrintValue(dsValue *Value){
	if(Value->GetType() == p_ClsByte){
		printf("byte(%i)", Value->GetByte());
	}else if(Value->GetType() == p_ClsBool){
		printf("bool(%i)", Value->GetBool());
	}else if(Value->GetType() == p_ClsInt){
		printf("int(%i)", Value->GetInt());
	}else if(Value->GetType() == p_ClsFloat){
		printf("float(%f)", Value->GetFloat());
	}else if(Value->GetType() == p_ClsStr){
		printf("string(%s)", Value->GetString());
	}else{
		printf("%s", Value->GetType()->GetName());
	}
}
void dsRunTime::p_ExecFunc(dsRealObject *myself, dsClass *Class, dsFunction *Function, dsRealObject *block ){
	// arguments:
	//    myself: pointer to object executing the function upon it. in the case of a
	//          conventional function call this has to be a valid object. in the
	//          case of a static function this has to be NULL.
	//    Class: current object type used for the function call.
	//    Function: function to call.
	// return value:
	//    returns true if a return argument has been placed on the stack
	//
	// stack layout:
	//    [ function-call-arg-1 ] < top of stack >
	//    [ ... ]
	//    [ function-call-arg-N ]
	//    [ local-variable-N ]
	//    [ ... ]
	//    [ local-variable-1 ] < vFirstLocVar >
	//    [ argument-1 ] < vFirstArg >
	//    [ ... ]
	//    [ argument-N ]
	//    < previous function states >
	// 
	// stack handling:
	//    - function-call-arguments are ALL removed by function after termination
	//    - local-variables are ALL removed by function after termination
	//    - arguments are NOT removed by function after termination
	//    - return value if any will be pushed onto the stack after termination
	//      and becomes element <local-variable-1>
	
	if(!Class || !Function || !Function->GetByteCode()) DSTHROW(dueInvalidParam);
	// general variables
	int i, vCount;
	// working objects (not secured)
	dsValue tmpVal(p_ClsObj);
	// arguments
	int vFirstArg = p_StackTop - 1;
	// local variables
	int vFirstLocVar = p_StackTop;
	int vLocVarSize = Function->GetLocVarSize();
	int vLocVarTop = 0;
	// secured objects
	dsuStack vTryStack; // dslTryBlock*
	// temp objects (secured)
	dsValue callValue(p_ClsObj);
	dsValue *vTmpValPtr, *execpLocVar=NULL;
	dsRealObject *tmpObj;
	dsFunction *tmpFunc;
	dsFuncList *vTmpFuncList;
	dsClass *tmpType;
	dslTryBlock *vTry;
	int primType;
	bool vExcHandled;
//	const char *funcName;
	// execution object
	dsValue *execObj=NULL;
	// allocate space for local variables on stack
	if(p_StackTop + vLocVarSize >= p_StackSize) DSTHROW(dueStackOverflow);
	p_StackTop += vLocVarSize;
	// code execution
	dsByteCode *vFuncBC = Function->GetByteCode();
	byte * const topCP = (byte*)vFuncBC->GetPointer();
	byte *cp = topCP;
	
	dsSignature *funcSig = Function->GetSignature();
	int contextVarCount;
	int blockCVCount;
	int ci, ccount, citer;
	
	// execution loop
#ifdef __DEBUG__
	const byte *callPointTrace[20];
	for( ci=0; ci<20; ci++ ) callPointTrace[ci] = NULL;
	const byte *vFirstInvCP = cp + vFuncBC->GetLength();
	int vPrevTop;
#endif
//	p_Engine->PrintMessage( "[CheckPoint %s(%i)] %s.%s [%p]", __FILE__, __LINE__, Function->GetOwnerClass()->GetName(), Function->GetName(), block );
	while(true){
#ifdef __DEBUG__
		if(cp >= vFirstInvCP){
			printf( "CodePointer Out Of Boundary! %s.%s [%p] cp=%i max=%i\n",
				Function->GetOwnerClass()->GetName(), Function->GetName(), block,
				(int)(cp - topCP), (int)(vFirstInvCP - topCP) );
			printf( "CallPointTrace:" );
			for( ci=0; ci<20; ci++ ){
				if( ! callPointTrace[ci] ){
					break;
				}
				printf( " %i", (int)(callPointTrace[ci] - topCP) );
			}
			printf( "\n" );
			vFuncBC->DebugPrint();
			DSTHROW(dueInvalidParam);
			
		}else{
			for( ci=19; ci>0; ci-- ){
				callPointTrace[ci] = callPointTrace[ci-1];
			}
			callPointTrace[0] = cp;
		}
#endif
		/*{
		const int co = (int)(cp-topCP);
		int I;
		p_Engine->PrintMessageFormat( "Stack(%s.%s:%i cp=%i) execobj(type=%s val=%p)", Function->GetOwnerClass()->GetName(),
			Function->GetName(), vFuncBC->GetDebugLine(co), co, execObj ? execObj->GetType()->GetName() : "-",
			execObj ? execObj->GetRealObject() : NULL );
		for( I=0; I<5; I++ ){
			try{
				dsValue * const val = GetValue( I );
				p_Engine->PrintMessageFormat( "Stack %i type=%s val=%p", I, val->GetType()->GetName(), val->GetRealObject() );
			}catch(...){}
		}
		}*/
		
		try{
//			p_Engine->PrintMessage( "[CheckPoint %s(%i)] bytecode:%i", __FILE__, __LINE__, *cp );
			const dsByteCode::sCode &code = *( ( const dsByteCode::sCode* )cp );
			
			switch( code.code ){
			case dsByteCode::ebcNop:
				cp += DS_BCSTRIDE_CODE;
				break;
				
			case dsByteCode::ebcCByte:{ // const byte value
				const dsByteCode::sCodeCByte &codeCByte = ( const dsByteCode::sCodeCByte & )code;
				cp += DS_BCSTRIDE_CBYTE;
				SetByte( &tmpVal, codeCByte.value );
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcCInt:{ // const int value
				const dsByteCode::sCodeCInt &codeCInt = ( const dsByteCode::sCodeCInt & )code;
				cp += DS_BCSTRIDE_CINT;
				SetInt( &tmpVal, codeCInt.value );
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcCFlt:{ // const float value
				const dsByteCode::sCodeCFloat &codeCFlt = ( const dsByteCode::sCodeCFloat & )code;
				cp += DS_BCSTRIDE_CFLOAT;
				SetFloat( &tmpVal, codeCFlt.value );
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcCStr:{ // const string value
				const dsByteCode::sCodeCString &codeCStr = ( const dsByteCode::sCodeCString & )code;
				cp += DS_BCSTRIDE_CSTRING;
				SetString( &tmpVal, p_Engine->GetConstString( codeCStr.index ));
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcTrue: // const true value
				cp += DS_BCSTRIDE_CODE;
				SetBool(&tmpVal, true);
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcFalse: // const false value
				cp += DS_BCSTRIDE_CODE;
				SetBool(&tmpVal, false);
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcNull:{ // null object
				const dsByteCode::sCodeNull &codeNull = ( const dsByteCode::sCodeNull & )code;
				cp += DS_BCSTRIDE_NULL;
				SetNull( &tmpVal, codeNull.type );
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcThis: // put this into execObj
				cp += DS_BCSTRIDE_CODE;
				SetObject( &tmpVal, myself );
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcSuper: // put super into execObj
				cp += DS_BCSTRIDE_CODE;
				SetObject(&tmpVal, myself);
				tmpVal.SetType(Class->GetBaseClass());
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcClsVar:{ // put class variable into execObj
				const dsByteCode::sCodeClassVar &codeClsVar = ( const dsByteCode::sCodeClassVar & )code;
				cp += DS_BCSTRIDE_CLASSVAR;
				if( codeClsVar.variable->GetTypeModifiers() & DSTM_STATIC){
					execObj = codeClsVar.variable->GetStaticValue();
				}else{
					tmpObj = execObj->GetRealObject();
					if(!tmpObj) DSTHROW(dueNullPointer);
					execObj = codeClsVar.variable->GetValue(tmpObj->GetBuffer());
				}
				}break;
				
			case dsByteCode::ebcParam:{ // put parameter into execObj
				const dsByteCode::sCodeParam &codeParam = ( const dsByteCode::sCodeParam & )code;
				cp += DS_BCSTRIDE_PARAM;
				execObj = p_Stack + vFirstArg - codeParam.index;
				}break;
				
			case dsByteCode::ebcLocVar:{ // put locale variable into execObj
				const dsByteCode::sCodeLocalVar &codeLocVar = ( const dsByteCode::sCodeLocalVar & )code;
				cp += DS_BCSTRIDE_LOCVAR;
				execObj = p_Stack + vFirstLocVar + codeLocVar.index;
				}break;
				
			case dsByteCode::ebcPush: // push execObj onto stack
				cp += DS_BCSTRIDE_CODE;
				if(p_StackTop == p_StackSize) DSTHROW(dueStackOverflow);
				CopyValue(execObj, p_Stack + p_StackTop);
				p_StackTop++;
				break;
				
			case dsByteCode::ebcPop: // pop value from stack into execObj
				cp += DS_BCSTRIDE_CODE;
				if(p_StackTop == 0) DSTHROW(dueStackEmpty);
				MoveValue(p_Stack + p_StackTop-1, &tmpVal);
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpInc: // operator increment prim-datatype
				cp += DS_BCSTRIDE_CODE;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE) execObj->GetValueData()->IncByte();
				else if(primType == DSPT_INT) execObj->GetValueData()->IncInt();
				else if(primType == DSPT_FLOAT) execObj->GetValueData()->IncFloat();
				break;
				
			case dsByteCode::ebcOpDec: // operator decrement prim-datatype
				cp += DS_BCSTRIDE_CODE;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE) execObj->GetValueData()->DecByte();
				else if(primType == DSPT_INT) execObj->GetValueData()->DecInt();
				else if(primType == DSPT_FLOAT) execObj->GetValueData()->DecFloat();
				break;
				
			case dsByteCode::ebcOpMin: // operator minus prim-datatype
				cp += DS_BCSTRIDE_CODE;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE) SetInt(&tmpVal, -execObj->GetByte());
				else if(primType == DSPT_INT) SetInt(&tmpVal, -execObj->GetInt());
				else if(primType == DSPT_FLOAT) SetFloat(&tmpVal, -execObj->GetFloat());
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpNot: // operator not prim-datatype
				cp += DS_BCSTRIDE_CODE;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE) SetBool(&tmpVal, !execObj->GetByte());
				else if(primType == DSPT_BOOL) SetBool(&tmpVal, !execObj->GetBool());
				else if(primType == DSPT_INT) SetBool(&tmpVal, !execObj->GetInt());
				else if(primType == DSPT_FLOAT) SetBool(&tmpVal, !execObj->GetFloat());
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpInv: // operator inverse prim-datatype
				cp += DS_BCSTRIDE_CODE;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					SetInt( &tmpVal, ~execObj->GetByte() );
				}else if( primType == DSPT_INT ){
					SetInt( &tmpVal, ~execObj->GetInt() );
				}
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpMul: // operator multiply prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetInt(&tmpVal, execObj->GetByte() * vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetInt(&tmpVal, execObj->GetInt() * vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					SetFloat(&tmpVal, execObj->GetFloat() * vTmpValPtr->GetFloat());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpDiv: // operator divide prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					if( vTmpValPtr->GetByte() == 0 ){
						DSTHROW( dueDivisionByZero );
					}
					SetByte( &tmpVal, execObj->GetByte() / vTmpValPtr->GetByte() );
					
				}else if( primType == DSPT_INT ){
					if( vTmpValPtr->GetInt() == 0 ){
						DSTHROW( dueDivisionByZero );
					}
					SetInt( &tmpVal, execObj->GetInt() / vTmpValPtr->GetInt() );
					
				}else if( primType == DSPT_FLOAT ){
					if( vTmpValPtr->GetFloat() == 0.0f ){
						DSTHROW( dueDivisionByZero );
					}
					SetFloat( &tmpVal, execObj->GetFloat() / vTmpValPtr->GetFloat() );
				}
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpMod: // operator modulus prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					if( vTmpValPtr->GetByte() == 0 ){
						DSTHROW( dueDivisionByZero );
					}
					SetByte( &tmpVal, execObj->GetByte() % vTmpValPtr->GetByte() );
					
				}else if( primType == DSPT_INT ){
					if( vTmpValPtr->GetInt() == 0 ){
						DSTHROW( dueDivisionByZero );
					}
					SetInt( &tmpVal, execObj->GetInt() % vTmpValPtr->GetInt() );
				}
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpAdd: // operator add prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetInt(&tmpVal, execObj->GetByte() + vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetInt(&tmpVal, execObj->GetInt() + vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					SetFloat(&tmpVal, execObj->GetFloat() + vTmpValPtr->GetFloat());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpSub: // operator subtract prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetInt(&tmpVal, execObj->GetByte() - vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetInt(&tmpVal, execObj->GetInt() - vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					SetFloat(&tmpVal, execObj->GetFloat() - vTmpValPtr->GetFloat());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpLS: // operator left shift prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetInt(&tmpVal, execObj->GetByte() << vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetInt(&tmpVal, execObj->GetInt() << vTmpValPtr->GetInt());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpRS: // operator right shift prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					SetByte( &tmpVal, execObj->GetByte() >> vTmpValPtr->GetByte() );
				}else if( primType == DSPT_INT ){
					SetInt( &tmpVal, execObj->GetInt() >> vTmpValPtr->GetInt() );
				}
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpLe: // operator less prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetBool(&tmpVal, execObj->GetByte() < vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetBool(&tmpVal, execObj->GetInt() < vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					SetBool(&tmpVal, execObj->GetFloat() < vTmpValPtr->GetFloat());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpGr: // operator greater prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetBool(&tmpVal, execObj->GetByte() > vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetBool(&tmpVal, execObj->GetInt() > vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					SetBool(&tmpVal, execObj->GetFloat() > vTmpValPtr->GetFloat());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpLEq: // operator less or equal prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetBool(&tmpVal, execObj->GetByte() <= vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetBool(&tmpVal, execObj->GetInt() <= vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					SetBool(&tmpVal, execObj->GetFloat() <= vTmpValPtr->GetFloat());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpGEq: // operator greater or equal prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					SetBool(&tmpVal, execObj->GetByte() >= vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					SetBool(&tmpVal, execObj->GetInt() >= vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					SetBool(&tmpVal, execObj->GetFloat() >= vTmpValPtr->GetFloat());
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpEq: // operator equal prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == vTmpValPtr->GetType()->GetPrimitiveType()){
					if(primType == DSPT_BYTE)
						SetBool(&tmpVal, execObj->GetByte() == vTmpValPtr->GetByte());
					else if(primType == DSPT_BOOL)
						SetBool(&tmpVal, execObj->GetBool() == vTmpValPtr->GetBool());
					else if(primType == DSPT_INT)
						SetBool(&tmpVal, execObj->GetInt() == vTmpValPtr->GetInt());
					else if(primType == DSPT_FLOAT)
						SetBool(&tmpVal, execObj->GetFloat() == vTmpValPtr->GetFloat());
					else if(primType == DSPT_OBJECT){
						SetBool(&tmpVal, execObj->GetRealObject() == vTmpValPtr->GetRealObject());
						ClearValue(vTmpValPtr);
					}
				}else SetBool(&tmpVal, false);
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpNEq: // operator not equal prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == vTmpValPtr->GetType()->GetPrimitiveType()){
					if(primType == DSPT_BYTE)
						SetBool(&tmpVal, execObj->GetByte() != vTmpValPtr->GetByte());
					else if(primType == DSPT_BOOL)
						SetBool(&tmpVal, execObj->GetBool() != vTmpValPtr->GetBool());
					else if(primType == DSPT_INT)
						SetBool(&tmpVal, execObj->GetInt() != vTmpValPtr->GetInt());
					else if(primType == DSPT_FLOAT)
						SetBool(&tmpVal, execObj->GetFloat() != vTmpValPtr->GetFloat());
					else if(primType == DSPT_OBJECT){
						SetBool(&tmpVal, execObj->GetRealObject() != vTmpValPtr->GetRealObject());
						ClearValue(vTmpValPtr);
					}
				}else SetBool(&tmpVal, true);
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpAnd: // operator and prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					SetByte( &tmpVal, execObj->GetByte() & vTmpValPtr->GetByte() );
				}else if( primType == DSPT_BOOL ){
					SetBool( &tmpVal, execObj->GetBool() & vTmpValPtr->GetBool() );
				}else if( primType == DSPT_INT ){
					SetInt( &tmpVal, execObj->GetInt() & vTmpValPtr->GetInt() );
				}
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpOr: // operator or prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					SetByte( &tmpVal, execObj->GetByte() | vTmpValPtr->GetByte() );
				}else if( primType == DSPT_BOOL ){
					SetBool( &tmpVal, execObj->GetBool() | vTmpValPtr->GetBool() );
				}else if( primType == DSPT_INT ){
					SetInt( &tmpVal, execObj->GetInt() | vTmpValPtr->GetInt() );
				}
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpXor: // operator xor prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					SetByte( &tmpVal, execObj->GetByte() ^ vTmpValPtr->GetByte() );
				}else if( primType == DSPT_BOOL ){
					SetBool( &tmpVal, execObj->GetBool() ^ vTmpValPtr->GetBool() );
				}else if( primType == DSPT_INT ){
					SetInt( &tmpVal, execObj->GetInt() ^ vTmpValPtr->GetInt() );
				}
				p_StackTop--;
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcOpAss: // operator assign prim_datatype
				cp += DS_BCSTRIDE_CODE;
				MoveValue(p_Stack + p_StackTop-1, execObj);
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpMulA: // operator multiply-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->MulAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->MulAInt(vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					execObj->GetValueData()->MulAFloat(vTmpValPtr->GetFloat());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpDivA: // operator divide-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE){
					if(vTmpValPtr->GetByte() == 0) DSTHROW(dueDivisionByZero);
					execObj->GetValueData()->DivAByte(vTmpValPtr->GetByte());
				}else if(primType == DSPT_INT){
					if(vTmpValPtr->GetInt() == 0) DSTHROW(dueDivisionByZero);
					execObj->GetValueData()->DivAInt(vTmpValPtr->GetInt());
				}else if(primType == DSPT_FLOAT){
					if(vTmpValPtr->GetFloat() == 0) DSTHROW(dueDivisionByZero);
					execObj->GetValueData()->DivAFloat(vTmpValPtr->GetFloat());
				}
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpModA: // operator modulate-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if( primType == DSPT_BYTE ){
					if( vTmpValPtr->GetByte() == 0 ){
						DSTHROW( dueDivisionByZero );
					}
					execObj->GetValueData()->ModAByte( vTmpValPtr->GetByte() );
					
				}else if( primType == DSPT_INT ){
					if( vTmpValPtr->GetInt() == 0 ){
						DSTHROW( dueDivisionByZero );
					}
					execObj->GetValueData()->ModAInt( vTmpValPtr->GetInt() );
				}
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpAddA: // operator add-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->AddAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->AddAInt(vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					execObj->GetValueData()->AddAFloat(vTmpValPtr->GetFloat());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpSubA: // operator subtract-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->SubAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->SubAInt(vTmpValPtr->GetInt());
				else if(primType == DSPT_FLOAT)
					execObj->GetValueData()->SubAFloat(vTmpValPtr->GetFloat());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpLSA: // operator left-shift-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->LSAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->LSAInt(vTmpValPtr->GetInt());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpRSA: // operator right-shift-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->RSAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->RSAInt(vTmpValPtr->GetInt());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpAndA: // operator and-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->AndAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->AndAInt(vTmpValPtr->GetInt());
				else if(primType == DSPT_BOOL)
					execObj->GetValueData()->AndABool(vTmpValPtr->GetBool());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpOrA: // operator or-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->OrAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->OrAInt(vTmpValPtr->GetInt());
				else if(primType == DSPT_BOOL)
					execObj->GetValueData()->OrABool(vTmpValPtr->GetBool());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcOpXorA: // operator xor-assign prim-datatype
				cp += DS_BCSTRIDE_CODE;
				vTmpValPtr = p_Stack + p_StackTop-1;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE)
					execObj->GetValueData()->XorAByte(vTmpValPtr->GetByte());
				else if(primType == DSPT_INT)
					execObj->GetValueData()->XorAInt(vTmpValPtr->GetInt());
				else if(primType == DSPT_BOOL)
					execObj->GetValueData()->XorABool(vTmpValPtr->GetBool());
				p_StackTop--;
				break;
				
			case dsByteCode::ebcPInc: // post increment prim-datatype
				cp += DS_BCSTRIDE_CODE;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE){
					SetByte(&tmpVal, execObj->GetByte());
					execObj->GetValueData()->IncByte();
				}else if(primType == DSPT_INT){
					SetInt(&tmpVal, execObj->GetInt());
					execObj->GetValueData()->IncInt();
				}else if(primType == DSPT_FLOAT){
					SetFloat(&tmpVal, execObj->GetFloat());
					execObj->GetValueData()->IncFloat();
				}
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcPDec: // post decrement prim-datatype
				cp += DS_BCSTRIDE_CODE;
				primType = execObj->GetType()->GetPrimitiveType();
				if(primType == DSPT_BYTE){
					SetByte(&tmpVal, execObj->GetByte());
					execObj->GetValueData()->DecByte();
				}else if(primType == DSPT_INT){
					SetInt(&tmpVal, execObj->GetInt());
					execObj->GetValueData()->DecInt();
				}else if(primType == DSPT_FLOAT){
					SetFloat(&tmpVal, execObj->GetFloat());
					execObj->GetValueData()->DecFloat();
				}
				execObj = &tmpVal;
				break;
				
			case dsByteCode::ebcALV:{ // add local variable
				const dsByteCode::sCodeAllocLocalVar &codeALV = ( const dsByteCode::sCodeAllocLocalVar & )code;
				cp += DS_BCSTRIDE_ALV;
#ifdef __DEBUG__
				if( vLocVarTop == vLocVarSize ){
					p_Engine->PrintMessageFormat( "[OOPS] ALV (cp=%d) executed on invalid stack"
						" (locVarTop=%d locVarSize=%d firstLocVar=%d stackTop=%d)",
						(int)(cp - topCP), vLocVarTop, vLocVarSize, vFirstLocVar, p_StackTop );
					DSTHROW( dueStackOverflow );
				}
#endif
				vTmpValPtr = p_Stack + vFirstLocVar+vLocVarTop;
				vTmpValPtr->SetEmpty( codeALV.type );
				vLocVarTop++;
				execObj = vTmpValPtr;
				}break;
				
			case dsByteCode::ebcFLV:{ // free local variables
				const dsByteCode::sCodeFreeLocalVars &codeFLV = ( const dsByteCode::sCodeFreeLocalVars & )code;
				cp += DS_BCSTRIDE_FLV;
#ifdef __DEBUG__
				if(vLocVarTop == 0){
					p_Engine->PrintMessageFormat( "[OOPS] ALV (cp=%d) executed on invalid stack"
						" (locVarTop=%d locVarSize=%d firstLocVar=%d stackTop=%d)",
						(int)(cp - topCP), vLocVarTop, vLocVarSize, vFirstLocVar, p_StackTop );
					DSTHROW(dueStackEmpty);
				}
#endif
				for(i=0; i<codeFLV.count; i++){
					ClearValue(p_Stack + vFirstLocVar+vLocVarTop-1-i);
				}
				vLocVarTop -= codeFLV.count;
				}break;
				
			case dsByteCode::ebcRet: // return
				cp += DS_BCSTRIDE_CODE;
				goto gotoReturn;
				
			case dsByteCode::ebcJMPB:{ // unconditional jump byte
				const dsByteCode::sCodeJumpByte &codeJump = ( const dsByteCode::sCodeJumpByte & )code;
				cp += DS_BCSTRIDE_JBYTE + codeJump.offset;
				}break;
				
			case dsByteCode::ebcJMPS:{ // unconditional jump short
				const dsByteCode::sCodeJumpShort &codeJump = ( const dsByteCode::sCodeJumpShort & )code;
				cp += DS_BCSTRIDE_JSHORT + codeJump.offset;
				}break;
				
			case dsByteCode::ebcJMPL:{ // unconditional jump long
				const dsByteCode::sCodeJumpShort &codeJump = ( const dsByteCode::sCodeJumpShort & )code;
				cp += DS_BCSTRIDE_JLONG + codeJump.offset;
				}break;
				
			case dsByteCode::ebcJEQB:{ // jump if equal byte
				const dsByteCode::sCodeJumpByte &codeJump = ( const dsByteCode::sCodeJumpByte & )code;
				cp += DS_BCSTRIDE_JBYTE;
				if(execObj->GetBool()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJEQS:{ // jump if equal short
				const dsByteCode::sCodeJumpShort &codeJump = ( const dsByteCode::sCodeJumpShort & )code;
				cp += DS_BCSTRIDE_JSHORT;
				if(execObj->GetBool()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJEQL:{ // jump if equal long
				const dsByteCode::sCodeJumpByte &codeJump = ( const dsByteCode::sCodeJumpByte & )code;
				cp += DS_BCSTRIDE_JBYTE;
				if(execObj->GetBool()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJNEB:{ // jump if not equal byte
				const dsByteCode::sCodeJumpByte &codeJump = ( const dsByteCode::sCodeJumpByte & )code;
				cp += DS_BCSTRIDE_JBYTE;
				if(!execObj->GetBool()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJNES:{ // jump if not equal short
				const dsByteCode::sCodeJumpShort &codeJump = ( const dsByteCode::sCodeJumpShort & )code;
				cp += DS_BCSTRIDE_JSHORT;
				if(!execObj->GetBool()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJNEL:{ // jump if not equal long
				const dsByteCode::sCodeJumpShort &codeJump = ( const dsByteCode::sCodeJumpShort & )code;
				cp += DS_BCSTRIDE_JLONG;
				if(!execObj->GetBool()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJCEB:{ // jump if case equal byte
				const dsByteCode::sCodeJumpCaseByte &codeJump = ( const dsByteCode::sCodeJumpCaseByte & )code;
				cp += DS_BCSTRIDE_JCBYTE;
				if(execObj->GetInt() == codeJump.caseValue){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJCES:{ // jump if case equal short
				const dsByteCode::sCodeJumpCaseShort &codeJump = ( const dsByteCode::sCodeJumpCaseShort & )code;
				cp += DS_BCSTRIDE_JCSHORT;
				if(execObj->GetInt() == codeJump.caseValue){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJCEL:{ // jump if case equal long
				const dsByteCode::sCodeJumpCaseLong &codeJump = ( const dsByteCode::sCodeJumpCaseLong & )code;
				cp += DS_BCSTRIDE_JCLONG;
				if(execObj->GetInt() == codeJump.caseValue){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJCESB:{ // jump if case static equal byte
				const dsByteCode::sCodeJumpCaseStaticByte &codeJump = ( const dsByteCode::sCodeJumpCaseStaticByte & )code;
				cp += DS_BCSTRIDE_JCSBYTE;
				if(execObj->GetRealObject() == codeJump.caseValue->GetStaticValue()->GetRealObject()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJCESS:{ // jump if case static equal short
				const dsByteCode::sCodeJumpCaseStaticShort &codeJump = ( const dsByteCode::sCodeJumpCaseStaticShort & )code;
				cp += DS_BCSTRIDE_JCSSHORT;
				if(execObj->GetRealObject() == codeJump.caseValue->GetStaticValue()->GetRealObject()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJCESL:{ // jump if case static equal long
				const dsByteCode::sCodeJumpCaseStaticLong &codeJump = ( const dsByteCode::sCodeJumpCaseStaticLong & )code;
				cp += DS_BCSTRIDE_JCSLONG;
				if(execObj->GetRealObject() == codeJump.caseValue->GetStaticValue()->GetRealObject()){
					cp += codeJump.offset;
				}
				}break;
				
			case dsByteCode::ebcJOEB:{ // jump on exception byte
				const dsByteCode::sCodeJumpExcepByte &codeJump = ( const dsByteCode::sCodeJumpExcepByte & )code;
				cp += DS_BCSTRIDE_JEBYTE;
				( (dslTryBlock*)vTryStack.Top() )->AddCatch( codeJump.type, cp + codeJump.offset );
				}break;
				
			case dsByteCode::ebcJOES:{ // jump on exception short
				const dsByteCode::sCodeJumpExcepShort &codeJump = ( const dsByteCode::sCodeJumpExcepShort & )code;
				cp += DS_BCSTRIDE_JESHORT;
				( (dslTryBlock*)vTryStack.Top() )->AddCatch( codeJump.type, cp + codeJump.offset );
				}break;
				
			case dsByteCode::ebcJOEL:{ // jump on exception long
				const dsByteCode::sCodeJumpExcepLong &codeJump = ( const dsByteCode::sCodeJumpExcepLong & )code;
				cp += DS_BCSTRIDE_JELONG;
				( (dslTryBlock*)vTryStack.Top() )->AddCatch( codeJump.type, cp + codeJump.offset );
				}break;
				
			case dsByteCode::ebcCast:{ // castto operator
				const dsByteCode::sCodeCast &codeCast = ( const dsByteCode::sCodeCast & )code;
				cp += DS_BCSTRIDE_CAST;
				CastValueTo(execObj, &tmpVal, codeCast.type);
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcCaTo:{ // castableto operator
				const dsByteCode::sCodeCast &codeCast = ( const dsByteCode::sCodeCast & )code;
				cp += DS_BCSTRIDE_CAST;
				tmpType = execObj->GetType();
				if(tmpType->GetPrimitiveType() == DSPT_OBJECT){
					tmpObj = execObj->GetRealObject();
					if(tmpObj) tmpType = tmpObj->GetType();
				}
				SetBool(&tmpVal, tmpType->CastableTo( codeCast.type ));
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcTypO:{ // istypeof operator
				const dsByteCode::sCodeCast &codeCast = ( const dsByteCode::sCodeCast & )code;
				cp += DS_BCSTRIDE_CAST;
				tmpType = execObj->GetType();
				if(tmpType->GetPrimitiveType() == DSPT_OBJECT){
					tmpObj = execObj->GetRealObject();
					if(tmpObj) tmpType = tmpObj->GetType();
				}
				SetBool(&tmpVal, tmpType->IsEqualTo( codeCast.type ));
				execObj = &tmpVal;
				}break;
				
			case dsByteCode::ebcCall:{ // call function (late binding)
				const dsByteCode::sCodeCall &codeCall = ( const dsByteCode::sCodeCall & )code;
				cp += DS_BCSTRIDE_CALL;
				
				// execute function (late binding)
				tmpType = execObj->GetType();
				if( tmpType->GetPrimitiveType() == DSPT_OBJECT ){
					tmpObj = execObj->GetRealObject();
					if( ! tmpObj ){
						DSTHROW( dueNullPointer );
					}
					
				}else{
					tmpObj = NULL;
				}
				vTmpFuncList = execObj->GetRealType()->GetFuncList();
				tmpFunc = vTmpFuncList->GetFunction(execObj->GetFuncBase() + codeCall.index);
				#ifdef __DEBUG__
				if(!tmpFunc){
					DSTHROW(dueOutOfMemory);
				}
				#endif
				vCount = tmpFunc->GetSignature()->GetCount();
				
				callValue.SetRealObject( tmpType, tmpObj );
				if( tmpObj ){
					tmpObj->IncRefCount();
				}
				
				#ifdef __DEBUG__
				vPrevTop = p_StackTop;
				#endif
				
				try{
					tmpFunc->RunFunction( this, execObj );
					
				}catch( ... ){
					ClearValue( &callValue );
					throw;
				}
				
				ClearValue( &callValue );
				#ifdef __DEBUG__
				if( p_StackTop != ( tmpFunc->HasRetVal() ? vPrevTop + 1 : vPrevTop ) ){
					p_Engine->PrintMessageFormat( "[OOPS] Function call %s.%s(%i args, %d ret) left behind invalid stack (top is %i should be %i)",
						tmpFunc->GetOwnerClass()->GetName(), tmpFunc->GetName(), vCount, tmpFunc->HasRetVal(), p_StackTop, vPrevTop );
					DSTHROW(dueInvalidAction);
				}
				#endif
				
				// pop return value from stack if present
				if( tmpFunc->HasRetVal() ){
					MoveValue( p_Stack + p_StackTop-1, &tmpVal );
					p_StackTop--;
					execObj = &tmpVal;
				}
				
				// remove arguments from stack
				if( vCount > 0 ){
					RemoveValues( vCount );
				}
				}break;
				
			case dsByteCode::ebcDCall:{ // direct call function
				const dsByteCode::sCodeDirectCall &codeDCall = ( const dsByteCode::sCodeDirectCall & )code;
				cp += DS_BCSTRIDE_DCALL;
				vCount = codeDCall.function->GetSignature()->GetCount();
				
				#ifdef __DEBUG__
				vPrevTop = p_StackTop;
				#endif
				
				// execute function
				if( codeDCall.function->GetTypeModifiers() & DSTM_STATIC ){
					callValue.SetRealObject( codeDCall.function->GetOwnerClass(), NULL );
					codeDCall.function->RunFunction( this, &callValue );
					
				}else{
					tmpType = execObj->GetType();
					if( tmpType->GetPrimitiveType() == DSPT_OBJECT ){
						tmpObj = execObj->GetRealObject();
						if( ! tmpObj ){
							DSTHROW( dueNullPointer );
						}
						
					}else{
						tmpObj = NULL;
					}
					
					callValue.SetRealObject( tmpType, tmpObj );
					if( tmpObj ){
						tmpObj->IncRefCount();
					}
					
					try{
						codeDCall.function->RunFunction( this, execObj );
						
					}catch( ... ){
						ClearValue( &callValue );
						throw;
					}
					
					ClearValue( &callValue );
				}
				// error check
				#ifdef __DEBUG__
				if( p_StackTop != ( codeDCall.function->HasRetVal() ? vPrevTop + 1 : vPrevTop ) ){
					p_Engine->PrintMessageFormat( "[OOPS] Function call %s.%s(%i args, %d ret) left behind invalid stack (top is %i should be %i)",
						codeDCall.function->GetOwnerClass()->GetName(), codeDCall.function->GetName(),
						vCount, codeDCall.function->HasRetVal(), p_StackTop, vPrevTop );
					DSTHROW( dueInvalidAction );
				}
				#endif
				
				// pop return value from stack if present
				if( codeDCall.function->HasRetVal() ){
					MoveValue( p_Stack + p_StackTop-1, &tmpVal );
					p_StackTop--;
					execObj = &tmpVal;
				}
				
				// remove arguments from stack
				if( vCount > 0 ){
					RemoveValues( vCount );
				}
				}break;
				
			case dsByteCode::ebcCCall:{ // constructor function call
				const dsByteCode::sCodeDirectCall &codeDCall = ( const dsByteCode::sCodeDirectCall & )code;
				cp += DS_BCSTRIDE_DCALL;
				vCount = codeDCall.function->GetSignature()->GetCount();
				tmpType = codeDCall.function->GetOwnerClass();
				
				#ifdef __DEBUG__
				vPrevTop = p_StackTop;
				#endif
				
				// create object
				CreateObjectNaked(&tmpVal, tmpType);
				
				try{
					codeDCall.function->RunFunction( this, &tmpVal );
					
				}catch( ... ){
					ClearValue( &tmpVal );
					throw;
				}
				
				execObj = &tmpVal;
				
				// error check
				#ifdef __DEBUG__
				if( p_StackTop != vPrevTop ){
					p_Engine->PrintMessageFormat( "[OOPS] Constructor call %s.%s(%i args) left behind invalid stack (top is %i should be %i)",
						codeDCall.function->GetOwnerClass()->GetName(), codeDCall.function->GetName(), vCount, p_StackTop, vPrevTop );
					DSTHROW( dueInvalidAction );
				}
				#endif
				
				// remove arguments from stack
				if( vCount > 0 ){
					RemoveValues( vCount );
				}
				}break;
				
			case dsByteCode::ebcBTB: // begin try block
				cp += DS_BCSTRIDE_CODE;
				if(!(vTry = new dslTryBlock(vLocVarTop))) DSTHROW(dueOutOfMemory);
				vTryStack.Push(vTry);
				vTry = NULL;
				break;
				
			case dsByteCode::ebcETB: // end try block
				cp += DS_BCSTRIDE_CODE;
				vTryStack.Remove(1);
				break;
				
			case dsByteCode::ebcThrow: // throws an exception
				cp += DS_BCSTRIDE_CODE;
				tmpType = execObj->GetType();
				if(tmpType->GetPrimitiveType() == DSPT_OBJECT){
					tmpObj = execObj->GetRealObject();
					if(tmpObj) tmpType = tmpObj->GetType();
				}
				if(tmpType->GetPrimitiveType() != DSPT_OBJECT) DSTHROW(dseInvalidType);
				if(!tmpType->CastableTo(p_ClsExcep)) DSTHROW(dseInvalidType);
				ThrowException(execObj);
				
			case dsByteCode::ebcReThrow: // rethrows an exception
				cp += DS_BCSTRIDE_CODE;
				CopyValue(execpLocVar, p_RaisedException);
				DSTHROW(dseScriptedException);
				
			case dsByteCode::ebcBlock:{ // block object
				const dsByteCode::sCodeDirectCall &codeDCall = ( const dsByteCode::sCodeDirectCall & )code;
				cp += DS_BCSTRIDE_DCALL;
				
				blockCVCount = 0;
				
				if( block ){
					blockCVCount = p_ClsBlock->GetContextVariableCount( block );
				}
				
				contextVarCount = blockCVCount + vLocVarTop + funcSig->GetCount();
				
				p_ClsBlock->CreateBlock( this, myself, codeDCall.function, &tmpVal, contextVarCount );
				execObj = &tmpVal;
				tmpObj = tmpVal.GetRealObject();
				citer = 0;
				
				for( ci=0; ci<blockCVCount; ci++, citer++ ){
					CopyValue( p_ClsBlock->GetContextVariableAt( block, ci ), p_ClsBlock->GetContextVariableAt( tmpObj, citer ) );
				}
				
				ccount = funcSig->GetCount();
				for( ci=0; ci<ccount; ci++, citer++ ){
					CopyValue( p_Stack + vFirstArg - ci, p_ClsBlock->GetContextVariableAt( tmpObj, citer ) );
				}
				
				for( ci=0; ci<vLocVarTop; ci++, citer++ ){
					CopyValue( p_Stack + vFirstLocVar + ci, p_ClsBlock->GetContextVariableAt( tmpObj, citer ) );
				}
				
				/*
				p_Engine->PrintMessageFormat( "create-block: %i context variables (%i,%i,%i)[%p] => %p",
					contextVarCount, blockCVCount, vLocVarSize, funcSig->GetCount(), block, execObj->GetRealObject() );
				*/
				}break;
				
			case dsByteCode::ebcCVar:{ // retrieves a context variable
				const dsByteCode::sCodeLocalVar &codeLocVar = ( const dsByteCode::sCodeLocalVar & )code;
				cp += DS_BCSTRIDE_LOCVAR;
				
				if( ! block ){
					DSTHROW( dueInvalidParam );
				}
				
				execObj = p_ClsBlock->GetContextVariableAt( block, codeLocVar.index );
				//SetInt( &tmpVal, codeLocVar.index );
				//execObj = &tmpVal;
				
				}break;
				
			default:
				p_Engine->PrintMessageFormat( "Internal Error: Invalid Code %i at %i", code.code, (int)(cp - topCP) );
				vFuncBC->DebugPrint();
				DSTHROW( dueInvalidParam );
			}
			
		}catch( const duException &e ){
			// clean up function arguments if pushed on the stack
			ClearValue(&tmpVal);
			vCount = p_StackTop - vFirstLocVar - vLocVarSize;
			for(i=0; i<vCount; i++) ClearValue(p_Stack + p_StackTop-1-i);
			p_StackTop -= vCount;
			// prepare exception object
			p_PrepareException(e);
			// add trace
			p_ClsExcep->AddTrace( p_RaisedException, Function->GetOwnerClass(), Function, (int)(cp - topCP) );
			// check all catch handlers to see if one matches
			tmpType = p_RaisedException->GetType();
			dslTryBlock *vTry;
			dslCatchHandler *vCatch;
			vExcHandled = false;
			while(vTryStack.CanPop()){
				vTry = (dslTryBlock*)vTryStack.Top();
				// adjust local variables
				for(i=vTry->GetLocVarCount(); i<vLocVarTop; i++){
					ClearValue(p_Stack + vFirstLocVar+i);
				}
				vLocVarTop = vTry->GetLocVarCount();
				// search for a matching catch handler
				if( ( vCatch = vTry->FindCatch(tmpType) ) ){
					execpLocVar = p_Stack + vFirstLocVar+vLocVarTop;
					MoveValue(p_RaisedException, execpLocVar);
					vLocVarTop++;
					cp = vCatch->GetOffset();
//					ClearExceptionTrace();
					vTryStack.Remove(1);
					vExcHandled = true;
					break;
				}
				vTryStack.Remove(1);
			}
			// check if exception has been handled
			if(!vExcHandled){
				// exception not handled so clean up
				for(i=0; i<vLocVarTop; i++) ClearValue(p_Stack + vFirstLocVar+i);
				p_StackTop -= vLocVarSize;
				// and rethrow it
//				p_ClsExcep->AddTrace( p_RaisedException, Function->GetOwnerClass(), Function,
//					(int)(cp - topCP) );
				throw;
			}
			
		}catch( ... ){ // an unknown exception has been thrown.
			// clean up function arguments if pushed on the stack
			ClearValue(&tmpVal);
			vCount = p_StackTop - vFirstLocVar - vLocVarSize;
			for(i=0; i<vCount; i++) ClearValue(p_Stack + p_StackTop-1-i);
			p_StackTop -= vCount;
			// prepare exception object
			pPrepareUnknownException();
			// add trace
			p_ClsExcep->AddTrace( p_RaisedException, Function->GetOwnerClass(), Function, (int)(cp - topCP) );
			// check all catch handlers to see if one matches
			tmpType = p_RaisedException->GetType();
			dslTryBlock *vTry;
			dslCatchHandler *vCatch;
			vExcHandled = false;
			while(vTryStack.CanPop()){
				vTry = (dslTryBlock*)vTryStack.Top();
				// adjust local variables
				for(i=vTry->GetLocVarCount(); i<vLocVarTop; i++){
					ClearValue(p_Stack + vFirstLocVar+i);
				}
				vLocVarTop = vTry->GetLocVarCount();
				// search for a matching catch handler
				if( ( vCatch = vTry->FindCatch(tmpType) ) ){
					execpLocVar = p_Stack + vFirstLocVar+vLocVarTop;
					MoveValue(p_RaisedException, execpLocVar);
					vLocVarTop++;
					cp = vCatch->GetOffset();
//					ClearExceptionTrace();
					vTryStack.Remove(1);
					vExcHandled = true;
					break;
				}
				vTryStack.Remove(1);
			}
			// check if exception has been handled
			if(!vExcHandled){
				// exception not handled so clean up
				for(i=0; i<vLocVarTop; i++) ClearValue(p_Stack + vFirstLocVar+i);
				p_StackTop -= vLocVarSize;
				// and rethrow it
//				p_ClsExcep->AddTrace( p_RaisedException, Function->GetOwnerClass(), Function,
//					(int)(cp - topCP) );
				throw;
			}
		}
	}
	// we should never end up here!
	ClearValue(&tmpVal);
	DSTHROW(dueInvalidParam);

	// return from function
gotoReturn:
	// store away return value if present
	if(Function->HasRetVal()){
		if(p_StackTop == 0) DSTHROW(dueStackEmpty);
		MoveValue(p_Stack + p_StackTop-1, &tmpVal);
		p_StackTop--;
	}
	// check for stack problems
#ifdef __DEBUG__
//p_Engine->PrintMessageFormat( "[CheckPoint %s(%i)] %i,%i,%i", __FILE__, __LINE__, p_StackTop, vFirstLocVar, vLocVarSize );
	vCount = p_StackTop - vFirstLocVar - vLocVarSize;
	if(vCount > 0){
		for(i=0; i<vCount; i++){
			printf("[EE-STACK] %i: ", i);
			p_PrintValue(p_Stack + p_StackTop-1-i);
			printf("\n");
			ClearValue(p_Stack + p_StackTop-1-i);
		}
		p_StackTop -= vCount;
		p_Engine->PrintMessageFormat( "[OOPS] %i Function Call Arguments left on stack at end of function %s.%s!",
			vCount, Function->GetOwnerClass()->GetName(), Function->GetName() );
	}else if(vCount < 0){
		p_Engine->PrintMessageFormat( "[OOPS] Stack corruption at end of function %s.%s! Less values on stack than local variables",
			Function->GetOwnerClass()->GetName(), Function->GetName() );
		vLocVarSize += vCount;
	}
	if(vLocVarTop > 0){
		for(i=0; i<vLocVarTop; i++) ClearValue(p_Stack + vFirstLocVar+i);
		p_Engine->PrintMessageFormat( "[OOPS] %i Local Variables left on stack at end of function %s.%s!",
			vLocVarTop, Function->GetOwnerClass()->GetName(), Function->GetName() );
	}
	if(vCount>0 || vLocVarTop>0){
		ClearValue(&tmpVal);
		DSTHROW(dueInvalidAction);
	}
#endif
	// clear space for local variables (no more present so just adjust the pointer)
//	p_Engine->PrintMessageFormat( "[hm...] %i, %i, %i", p_StackTop, vLocVarTop, vLocVarSize );
	p_StackTop -= vLocVarSize;
	// push return value on stack
	if(Function->HasRetVal()){
		MoveValue(&tmpVal, p_Stack + p_StackTop);
		p_StackTop++;
	}else{
		ClearValue(&tmpVal);
	}
}
