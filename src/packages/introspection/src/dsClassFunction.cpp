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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../../../config.h"
#include "introspection.h"
#include "dsClassFunction.h"
#include "dsClassClass.h"

// native data structure
struct sFuncNatData{
	dsFunction *function;
};



// native functions
/////////////////////

// construction



// Informations
// public func String getName()
dsClassFunction::nfGetName::nfGetName(const sInitData &init) : dsFunction(init.clsFunc,
"getName", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassFunction::nfGetName::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	if(!nd->function) DSTHROW(dueNullPointer);
	rt->PushString( nd->function->GetName() );
}
// public func String getFullName()
dsClassFunction::nfGetFullName::nfGetFullName(const sInitData &init) : dsFunction(init.clsFunc,
"getFullName", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassFunction::nfGetFullName::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	dsClassFunction *clsFunc = (dsClassFunction*)GetOwnerClass();
	if(!nd->function) DSTHROW(dueNullPointer);
	clsFunc->PushFullName(rt, nd->function);
}
// public func Class getType()
dsClassFunction::nfGetType::nfGetType(const sInitData &init) : dsFunction(init.clsFunc,
"getType", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
}
void dsClassFunction::nfGetType::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = ((dsClassFunction*)GetOwnerClass())->GetClassClass();
	if(!nd->function) DSTHROW(dueNullPointer);
	clsClass->PushClass( rt, nd->function->GetType() );
}
// public func int getTypeModifiers()
dsClassFunction::nfGetTypeModifiers::nfGetTypeModifiers(const sInitData &init) : dsFunction(init.clsFunc,
"getTypeModifiers", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassFunction::nfGetTypeModifiers::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	if(!nd->function) DSTHROW(dueNullPointer);
	rt->PushInt( nd->function->GetTypeModifiers() );
}
// public func Class getParent()
dsClassFunction::nfGetParent::nfGetParent(const sInitData &init) : dsFunction(init.clsFunc,
"getParent", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
}
void dsClassFunction::nfGetParent::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = ((dsClassFunction*)GetOwnerClass())->GetClassClass();
	if( !nd->function ) DSTHROW(dueNullPointer);
	clsClass->PushClass( rt, nd->function->GetOwnerClass() );
}
// public func Array getSignature()
dsClassFunction::nfGetSignature::nfGetSignature(const sInitData &init) : dsFunction(init.clsFunc,
"getSignature", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr){
}
void dsClassFunction::nfGetSignature::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	if( !nd->function ) DSTHROW(dueNullPointer);
	((dsClassFunction*)GetOwnerClass())->PushSignature( rt, nd->function->GetSignature() );
}
// public func Object callDirect(Object obj, Array args)
dsClassFunction::nfCallDirect::nfCallDirect(const sInitData &init) : dsFunction(init.clsFunc,
"callDirect", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj){
	p_AddParameter(init.clsObj); // obj
	p_AddParameter(init.clsArr); // args
}
void dsClassFunction::nfCallDirect::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	try{
		((dsClassFunction*)GetOwnerClass())->CallFunction(rt, nd->function, true );
	}catch( ... ){
		rt->AddNativeTrace(this, __LINE__);
		throw;
	}
}
// public func Object call(Object obj, Array args)
dsClassFunction::nfCall::nfCall(const sInitData &init) : dsFunction(init.clsFunc,
"call", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj){
	p_AddParameter(init.clsObj); // obj
	p_AddParameter(init.clsArr); // args
}
void dsClassFunction::nfCall::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	try{
		((dsClassFunction*)GetOwnerClass())->CallFunction(rt, nd->function, false );
	}catch( ... ){
		rt->AddNativeTrace(this, __LINE__);
		throw;
	}
}


// convenience functions
// public func bool isSet(int flags)
dsClassFunction::nfIsSet::nfIsSet(const sInitData &init) :
dsFunction(init.clsClass, "isSet", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsInt); // flags
}

void dsClassFunction::nfIsSet::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	if(!nd->function) DSTHROW(dueNullPointer);
	int flags = rt->GetValue(0)->GetInt();
	rt->PushBool( (nd->function->GetTypeModifiers() & flags) != 0);
}


// other functions
// public func int hashCode()
dsClassFunction::nfHashCode::nfHashCode(const sInitData &init) :
dsFunction(init.clsFunc, "hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}

void dsClassFunction::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	if(!nd->function) DSTHROW(dueNullPointer);
	rt->PushInt( ( intptr_t )nd->function );
}

// public func bool equals(Object obj)
dsClassFunction::nfEquals::nfEquals(const sInitData &init) :
dsFunction(init.clsFunc, "equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsObj); // obj
}
void dsClassFunction::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	dsClassFunction *clsFunc = (dsClassFunction*)GetOwnerClass();
	if(!nd->function) DSTHROW(dueNullPointer);
	dsValue *obj = rt->GetValue(0);
	// compare objects
	if(p_IsObjOfType(obj, clsFunc)){
		sFuncNatData *nd2 = (sFuncNatData*)p_GetNativeData(obj);
		if(nd->function){
			rt->PushBool( nd->function->IsEqualTo(nd2->function) );
		}else{
			rt->PushBool(false);
		}
	}else{
		rt->PushBool(false);
	}
}

// public func String toString()
dsClassFunction::nfToString::nfToString(const sInitData &init) :
dsFunction(init.clsFunc, "toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassFunction::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sFuncNatData *nd = (sFuncNatData*)p_GetNativeData(myself);
	dsClassFunction *clsFunc = (dsClassFunction*)GetOwnerClass();
	if(!nd->function) DSTHROW(dueNullPointer);
	clsFunc->PushFullName(rt, nd->function);
}



// class dsClassFunction
//////////////////////////
// constructor, destructor
dsClassFunction::dsClassFunction() : dsClass("Function", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE) {
	pClsClass = NULL;
	pClsObj = NULL;
	pClsArr = NULL;
	GetParserInfo()->SetParent("Introspection");
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(sizeof(sFuncNatData));
}
dsClassFunction::~dsClassFunction(){ }

// management
void dsClassFunction::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsFunc = this;
	init.clsClass = engine->GetClass(ISPECT_CLASS);
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsBool = engine->GetClassBool();
	init.clsStr = engine->GetClassString();
	init.clsObj = engine->GetClassObject();
	init.clsArr = engine->GetClassArray();
	pClsClass = (dsClassClass*)init.clsClass;
	pClsObj = init.clsObj;
	pClsArr = init.clsArr;
	// functions
	AddFunction(new nfGetName(init));
	AddFunction(new nfGetFullName(init));
	AddFunction(new nfGetType(init));
	AddFunction(new nfGetTypeModifiers(init));
	AddFunction(new nfGetParent(init));
	AddFunction(new nfGetSignature(init));
	AddFunction(new nfCallDirect(init));
	AddFunction(new nfCall(init));
	AddFunction(new nfIsSet(init));
	AddFunction(new nfHashCode(init));
	AddFunction(new nfEquals(init));
	AddFunction(new nfToString(init));
}

// internal functions
void dsClassFunction::PushFunction(dsRunTime *rt, dsFunction *function){
	if(!rt || !function) DSTHROW(dueInvalidParam);
	// create new introspection variable object
	dsValue *value = rt->CreateValue(this);
	sFuncNatData *nd;
	try{
		rt->CreateObjectNaked(value, this);
		nd = (sFuncNatData*)p_GetNativeData(value->GetRealObject()->GetBuffer());
		nd->function = function;
		rt->PushValue(value);
		rt->FreeValue(value);
	}catch( ... ){
		if(value) rt->FreeValue(value);
		throw;
	}
}
void dsClassFunction::PushSignature(dsRunTime *rt, dsSignature *sig ){
	if(!rt || !sig) DSTHROW(dueInvalidParam);
	dsValue *array = NULL;
	try{
		// create array
		array = rt->CreateValue(pClsArr);
		if(!array) DSTHROW(dueOutOfMemory);
		rt->CreateObject(array, pClsArr, 0);
		// add signature types to array
		for(int i=0; i<sig->GetCount(); i++){
			pClsClass->PushClass(rt, sig->GetParameter(i));
			rt->RunFunction(array, "add", 1 );
		}
		// push array
		rt->PushValue(array);
		rt->FreeValue(array);
	}catch( ... ){
		if(array) rt->FreeValue(array);
		throw;
	}
}
void dsClassFunction::PushFullName(dsRunTime *rt, dsFunction *function){
	if(!rt || !function) DSTHROW(dueInvalidParam);
	const char *name = function->GetName();
	// create and push full name string
	int nameLen = strlen(name);
	int fullNameLen = function->GetOwnerClass()->GetFullName(NULL, 0);
	char *fullName = new char[fullNameLen+nameLen+2];
	try{
		function->GetOwnerClass()->GetFullName(fullName, fullNameLen);
		sprintf(fullName+fullNameLen, ".%s", name);
		rt->PushString(fullName);
		delete [] fullName;
	}catch( ... ){
		if(fullName) delete [] fullName;
		throw;
	}
}

void dsClassFunction::CallFunction(dsRunTime *rt, dsFunction *func, bool direct){
	if( !func ) DSTHROW(dueNullPointer);
	dsValue *obj = rt->GetValue(0);
	dsValue *args = rt->GetValue(1);
	dsValue *castArg = NULL;
	dsSignature *funcSig = func->GetSignature();
	dsClass *argType, *funcArgType;
	int i, count=0, argsPushed=0, funcIndex;
	// do it
	try{
		castArg = rt->CreateValue();
		// push arguments on stack and check signature the same time
		if(args->GetRealObject()){
			rt->RunFunction(args, "getCount", 0 );
			count = rt->GetReturnInt();
			if(count != funcSig->GetCount()) DSTHROW(dseInvalidSignature);
			for(i=count-1; i>=0; i--){
				// get argument from array
				rt->PushInt(i);
				rt->RunFunction(args, "getAt", 1 );
				// cast to appropriate type
				argType = rt->GetReturnValue()->GetType();
				funcArgType = funcSig->GetParameter(i);
				if( argType->IsEqualTo(funcArgType) ){
					rt->PushReturnValue();
				}else{
					rt->CastValueTo(rt->GetReturnValue(), castArg, funcArgType);
					rt->PushValue(castArg);
				}
				argsPushed++;
			}
		}
		rt->ClearValue(castArg);
		// call the function
		funcIndex = func->GetOwnerClass()->GetFuncList()->GetIndexOf( func );
		argsPushed = 0;
		if( func->GetFuncType() == DSFT_CONSTRUCTOR ){
			rt->CreateAndPushObject(func->GetOwnerClass(), count);
		}else{
			if(func->GetTypeModifiers() & DSTM_STATIC){
				rt->RunStaticFunction(func->GetOwnerClass(), funcIndex, count);
			}else{
				if(direct){
					rt->CastValueTo(obj, castArg, func->GetOwnerClass());
					rt->RunFunction(castArg, funcIndex, count );
				}else{
					rt->RunFunction(obj, func->GetName(), count );
				}
			}
			if(func->GetType()->GetPrimitiveType() == DSPT_VOID){
				rt->PushObject(NULL, pClsObj);
			}else{
				rt->PushReturnValue();
			}
		}
		// cleanup
		rt->FreeValue(castArg); castArg = NULL;
	}catch( ... ){
		if(castArg) rt->FreeValue(castArg);
		if(argsPushed>0) rt->RemoveValues(argsPushed);
		throw;
	}
}
