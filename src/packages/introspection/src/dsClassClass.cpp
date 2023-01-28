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
#include "introspection.h"
#include "dsClassClass.h"
#include "dsClassVariable.h"
#include "dsClassFunction.h"

// native data structure
struct sClassNatData{
	dsClass *theClass;
};



// native functions
/////////////////////

// construction
// public func new(String fullName)
dsClassClass::nfNew::nfNew(const sInitData &init) : dsFunction(init.clsClass, "new",
DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsStr); // fullName
}
void dsClassClass::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	nd->theClass = NULL;
	// determine class matching the string
	const char *fullName = rt->GetValue(0)->GetString();
	dsEngine *engine = rt->GetEngine();
	nd->theClass = engine->GetClass(fullName);
	if(!nd->theClass) DSTHROW(dueInvalidParam);
}

// public func new(Object obj)
dsClassClass::nfNew2::nfNew2(const sInitData &init) : dsFunction(init.clsClass, "new",
DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObj); // obj
}
void dsClassClass::nfNew2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	nd->theClass = NULL;
	// determine class matching the string
	dsValue *obj = rt->GetValue(0);
	nd->theClass = obj->GetRealType();
}



// class informations
// public func String getName()
dsClassClass::nfGetName::nfGetName(const sInitData &init) : dsFunction(init.clsClass,
"getName", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassClass::nfGetName::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	rt->PushString( nd->theClass->GetName() );
}
// public func String getFullName()
dsClassClass::nfGetFullName::nfGetFullName(const sInitData &init) : dsFunction(init.clsClass,
"getFullName", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassClass::nfGetFullName::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	clsClass->PushFullName(rt, nd->theClass);
}
// public func int getClassType()
dsClassClass::nfGetClassType::nfGetClassType(const sInitData &init) : dsFunction(init.clsClass,
"getClassType", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassClass::nfGetClassType::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	rt->PushInt( nd->theClass->GetClassType() );
}
// public func int getTypeModifiers()
dsClassClass::nfGetTypeModifiers::nfGetTypeModifiers(const sInitData &init) : dsFunction(init.clsClass,
"getTypeModifiers", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassClass::nfGetTypeModifiers::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	rt->PushInt( nd->theClass->GetTypeModifiers() );
}
// public func Class getBase()
dsClassClass::nfGetBase::nfGetBase(const sInitData &init) : dsFunction(init.clsClass,
"getBase", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
}
void dsClassClass::nfGetBase::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	if(nd->theClass->GetPrimitiveType() <= DSPT_FLOAT){
		clsClass->PushClass( rt, NULL );
	}else{
		clsClass->PushClass( rt, nd->theClass->GetBaseClass() );
	}
}
// public func Class getParent()
dsClassClass::nfGetParent::nfGetParent(const sInitData &init) : dsFunction(init.clsClass,
"getParent", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
}
void dsClassClass::nfGetParent::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	clsClass->PushClass( rt, nd->theClass->GetParent() );
}
// public func bool isSubClassOf(Class *cls)
dsClassClass::nfIsSubClassOf::nfIsSubClassOf(const sInitData &init) : dsFunction(init.clsClass,
"isSubClassOf", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsClass); // cls
}
void dsClassClass::nfIsSubClassOf::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsValue *objCls = rt->GetValue(0);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	if(!objCls->GetRealObject()) DSTHROW(dueNullPointer);
	sClassNatData *nd2 = (sClassNatData*)p_GetNativeData(objCls);
	rt->PushBool( nd->theClass->IsSubClassOf(nd2->theClass) &&
		!nd->theClass->IsEqualTo(nd2->theClass));
}



// interfaces
// public func int getInterfaceCount()
dsClassClass::nfGetInterfaceCount::nfGetInterfaceCount(const sInitData &init) : dsFunction(init.clsClass,
"getInterfaceCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassClass::nfGetInterfaceCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	rt->PushInt( nd->theClass->GetImplementsCount() );
}
// public func Class getInterface(int index)
dsClassClass::nfGetInterface::nfGetInterface(const sInitData &init) : dsFunction(init.clsClass,
"getInterface", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
	p_AddParameter(init.clsInt); // index
}
void dsClassClass::nfGetInterface::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	int index = rt->GetValue(0)->GetInt();
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	if(index<0 || index>=nd->theClass->GetImplementsCount()) DSTHROW(dueOutOfBoundary);
	clsClass->PushClass( rt, nd->theClass->GetImplement(index) );
}



// inner Classes
// public func int getClassCount()
dsClassClass::nfGetClassCount::nfGetClassCount(const sInitData &init) : dsFunction(init.clsClass,
"getClassCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassClass::nfGetClassCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	rt->PushInt( nd->theClass->GetInnerClassCount() );
}
// public func Class getClass(int index)
dsClassClass::nfGetClass::nfGetClass(const sInitData &init) : dsFunction(init.clsClass,
"getClass", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
	p_AddParameter(init.clsInt); // index
}
void dsClassClass::nfGetClass::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	int index = rt->GetValue(0)->GetInt();
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	if(index<0 || index>=nd->theClass->GetInnerClassCount()) DSTHROW(dueOutOfBoundary);
	clsClass->PushClass( rt, nd->theClass->GetInnerClass(index) );
}
	


// variables
// public func int getVariableCount()
dsClassClass::nfGetVariableCount::nfGetVariableCount(const sInitData &init) : dsFunction(init.clsClass,
"getVariableCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassClass::nfGetVariableCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	rt->PushInt( nd->theClass->GetConstantCount() + nd->theClass->GetVariableCount() );
}
// public func Variable getVariable(int index)
dsClassClass::nfGetVariable::nfGetVariable(const sInitData &init) : dsFunction(init.clsClass,
"getVariable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVar){
	p_AddParameter(init.clsInt); // index
}
void dsClassClass::nfGetVariable::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	int index = rt->GetValue(0)->GetInt();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	int countConst = nd->theClass->GetConstantCount();
	int count = countConst + nd->theClass->GetVariableCount();
	if(index<0 || index>=count) DSTHROW(dueOutOfBoundary);
	if(index < countConst){
		clsClass->GetClassVariable()->PushConstant(rt, nd->theClass,
			nd->theClass->GetConstant(index));
	}else{
		clsClass->GetClassVariable()->PushVariable(rt,
			nd->theClass->GetVariable(index - countConst));
	}
}
// public func Variable getVariable(String name)
dsClassClass::nfGetVariable2::nfGetVariable2(const sInitData &init) : dsFunction(init.clsClass,
"getVariable", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVar){
	p_AddParameter(init.clsStr); // name
}
void dsClassClass::nfGetVariable2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	const char *name = rt->GetValue(0)->GetString();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	dsConstant *constant = nd->theClass->FindConstant( name, true );
	if(constant){
		clsClass->GetClassVariable()->PushConstant(rt, nd->theClass, constant);
	}else{
		dsVariable *var = nd->theClass->FindVariable( name, true );
		if(var){
			clsClass->GetClassVariable()->PushVariable(rt, var);
		}else{
			DSTHROW(dueInvalidParam);
		}
	}
}


		
// functions
// public func int getFunctionCount()
dsClassClass::nfGetFunctionCount::nfGetFunctionCount(const sInitData &init) : dsFunction(init.clsClass,
"getFunctionCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassClass::nfGetFunctionCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	rt->PushInt( nd->theClass->GetFunctionCount() );
}
// public func Function getFunction(int index)
dsClassClass::nfGetFunction::nfGetFunction(const sInitData &init) : dsFunction(init.clsClass,
"getFunction", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsFunc){
	p_AddParameter(init.clsInt); // index
}
void dsClassClass::nfGetFunction::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	int index = rt->GetValue(0)->GetInt();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	if(index<0 || index>=nd->theClass->GetFunctionCount()) DSTHROW(dueOutOfBoundary);
	clsClass->GetClassFunction()->PushFunction(rt, nd->theClass->GetFunction(index));
}
// public func Function getFunction(String name, Array signature)
dsClassClass::nfGetFunction2::nfGetFunction2(const sInitData &init) : dsFunction(init.clsClass,
"getFunction", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsFunc){
	p_AddParameter(init.clsStr); // name
	p_AddParameter(init.clsArr); // signature
}
void dsClassClass::nfGetFunction2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	const char *name = rt->GetValue(0)->GetString();
	dsValue *valSig = rt->GetValue(1);
	dsSignature funcSig;
	int count;
	if(!nd->theClass) DSTHROW(dueNullPointer);
	// build signature if one has been specified.
	if(valSig->GetRealObject()){
		// determine number of arguments
		rt->RunFunction(valSig, "getCount", 0 );
		count = rt->GetReturnInt();
		// get arguments and build signature from them
		for(int i=0; i<count; i++){
			rt->PushInt(i);
			rt->RunFunction(valSig, "getAt", 1 );
			if(!rt->GetReturnValue()->GetRealType()->CastableTo(clsClass)) DSTHROW(dueInvalidParam);
			funcSig.AddParameter( clsClass->GetClassFromObj(rt->GetReturnRealObject()) );
		}
	}
	// find function
	dsFunction *func = nd->theClass->FindFunction(name, &funcSig, false);
	if(func){
		clsClass->GetClassFunction()->PushFunction(rt, func);
	}else{
		DSTHROW(dueInvalidParam);
	}
}
// public func Function findFunction(String name, Array signature)
dsClassClass::nfFindFunction::nfFindFunction(const sInitData &init) : dsFunction(init.clsClass,
"findFunction", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsFunc){
	p_AddParameter(init.clsStr); // name
	p_AddParameter(init.clsArr); // signature
}
void dsClassClass::nfFindFunction::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	const char *name = rt->GetValue(0)->GetString();
	dsValue *valSig = rt->GetValue(1);
	dsSignature funcSig;
	int count;
	if(!nd->theClass) DSTHROW(dueNullPointer);
	// build signature if one has been specified.
	if(valSig->GetRealObject()){
		// determine number of arguments
		rt->RunFunction(valSig, "getCount", 0 );
		count = rt->GetReturnInt();
		// get arguments and build signature from them
		for(int i=0; i<count; i++){
			rt->PushInt(i);
			rt->RunFunction(valSig, "getAt", 1 );
			if(!rt->GetReturnValue()->GetRealType()->CastableTo(clsClass)) DSTHROW(dueInvalidParam);
			funcSig.AddParameter( clsClass->GetClassFromObj(rt->GetReturnRealObject()) );
		}
	}
	// find function
	dsFunction *func = nd->theClass->GetFuncList()->FindBestFunction(NULL, name, &funcSig, false);
	if(func){
		clsClass->GetClassFunction()->PushFunction(rt, func);
	}else{
		DSTHROW(dueInvalidParam);
	}
}


// convenience functions
// public func bool isSet(int flags)
dsClassClass::nfIsSet::nfIsSet(const sInitData &init) :
dsFunction(init.clsClass, "isSet", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsInt); // flags
}

void dsClassClass::nfIsSet::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	int flags = rt->GetValue(0)->GetInt();
	rt->PushBool( (nd->theClass->GetTypeModifiers() & flags) != 0);
}


// other functions
// public func int hashCode()
dsClassClass::nfHashCode::nfHashCode(const sInitData &init) :
dsFunction(init.clsClass, "hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}

void dsClassClass::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	if(!nd->theClass) DSTHROW(dueNullPointer);
	rt->PushInt( ( int )( intptr_t )nd->theClass );
}

// public func bool equals(Object obj)
dsClassClass::nfEquals::nfEquals(const sInitData &init) :
dsFunction(init.clsClass, "equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsObj); // obj
}
void dsClassClass::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	dsValue *obj = rt->GetValue(0);
	// compare objects
	if(p_IsObjOfType(obj, clsClass)){
		sClassNatData *nd2 = (sClassNatData*)p_GetNativeData(obj);
		if(nd2->theClass){
			rt->PushBool( nd->theClass->IsEqualTo(nd2->theClass) );
		}else{
			rt->PushBool(false);
		}
	}else{
		rt->PushBool(false);
	}
}

// public func String toString()
dsClassClass::nfToString::nfToString(const sInitData &init) :
dsFunction(init.clsClass, "toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassClass::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sClassNatData *nd = (sClassNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = (dsClassClass*)GetOwnerClass();
	if(!nd->theClass) DSTHROW(dueNullPointer);
	clsClass->PushFullName(rt, nd->theClass);
}




// class dsClassClass
///////////////////////
// constructor, destructor
dsClassClass::dsClassClass() : dsClass("Class", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE) {
	p_ClsObj = NULL;
	pClsVar = NULL;
	GetParserInfo()->SetParent("Introspection");
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(sizeof(sClassNatData));
}
dsClassClass::~dsClassClass(){ }

// management
void dsClassClass::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsClass = this;
	init.clsVar = engine->GetClass(ISPECT_VARIABLE);
	init.clsFunc = engine->GetClass(ISPECT_FUNCTION);
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsBool = engine->GetClassBool();
	init.clsStr = engine->GetClassString();
	init.clsObj = engine->GetClassObject();
	init.clsArr = engine->GetClassArray();
	p_ClsObj = init.clsObj;
	pClsVar = (dsClassVariable*)init.clsVar;
	pClsFunc = (dsClassFunction*)init.clsFunc;
	// enumeration eWindowStates:
	AddConstant( new dsConstant("CLASS", init.clsInt, 0 ) );
	AddConstant( new dsConstant("INTERFACE", init.clsInt, 1 ) );
	AddConstant( new dsConstant("NAMESPACE", init.clsInt, 2 ) );
	// functions
	AddFunction(new nfNew(init));
	AddFunction(new nfNew2(init));
	AddFunction(new nfGetName(init));
	AddFunction(new nfGetFullName(init));
	AddFunction(new nfGetClassType(init));
	AddFunction(new nfGetTypeModifiers(init));
	AddFunction(new nfGetBase(init));
	AddFunction(new nfGetParent(init));
	AddFunction(new nfIsSubClassOf(init));
	AddFunction(new nfGetInterfaceCount(init));
	AddFunction(new nfGetInterface(init));
	AddFunction(new nfGetClassCount(init));
	AddFunction(new nfGetClass(init));
	AddFunction(new nfGetVariableCount(init));
	AddFunction(new nfGetVariable(init));
	AddFunction(new nfGetVariable2(init));
	AddFunction(new nfGetFunctionCount(init));
	AddFunction(new nfGetFunction(init));
	AddFunction(new nfGetFunction2(init));
	AddFunction(new nfFindFunction(init));
	AddFunction(new nfIsSet(init));
	AddFunction(new nfHashCode(init));
	AddFunction(new nfEquals(init));
	AddFunction(new nfToString(init));
}

// internal functions
void dsClassClass::PushClass(dsRunTime *rt, dsClass *theClass){
	// return null if class has no base class
	if(!theClass){
		rt->PushObject(NULL, this);
	// create new introspection class object from the given class
	}else{
		dsValue *value = rt->CreateValue(this);
		sClassNatData *nd;
		try{
			rt->CreateObjectNaked(value, this);
			nd = (sClassNatData*)p_GetNativeData(value->GetRealObject()->GetBuffer());
			nd->theClass = theClass;
			rt->PushValue(value);
			rt->FreeValue(value);
		}catch( ... ){
			if(value) rt->FreeValue(value);
			throw;
		}
	}
}
void dsClassClass::PushFullName(dsRunTime *rt, dsClass *theClass){
	if(!rt || !theClass) DSTHROW(dueInvalidParam);
	// determine the length of the needed string first
	int fullNameLen=0, nameLen;
	dsClass *curClass = theClass;
	while(curClass){
		fullNameLen += ( int )strlen(curClass->GetName()) + 1;
		curClass = curClass->GetParent();
	}
	fullNameLen--;
	// create and push string containing full name
	char *fullName = new char[fullNameLen+1];
	if(!fullName) DSTHROW(dueOutOfMemory);
	try{
		fullName[fullNameLen] = '\0';
		curClass = theClass;
		while(curClass){
			nameLen = ( int )strlen(curClass->GetName());
			fullNameLen -= nameLen;
			#ifdef OS_W32_VS
				strncpy_s( fullName + fullNameLen, nameLen, curClass->GetName(), nameLen );
			#else
				strncpy(fullName+fullNameLen, curClass->GetName(), nameLen);
			#endif
			curClass = curClass->GetParent();
			if(curClass){
				fullNameLen--;
				fullName[fullNameLen] = '.';
			}
		}
		rt->PushString(fullName);
		delete [] fullName;
	}catch( ... ){
		if(fullName) delete [] fullName;
		throw;
	}
}
dsClass *dsClassClass::GetClassFromObj(dsRealObject *obj){
	if(!obj) DSTHROW(dueInvalidParam);
	return ((sClassNatData*)p_GetNativeData(obj->GetBuffer()))->theClass;
}
