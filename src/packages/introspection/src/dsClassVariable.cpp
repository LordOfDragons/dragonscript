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
#include "dsClassVariable.h"
#include "dsClassClass.h"

// native data structure
struct sVarNatData{
	dsClass *parent;
	dsConstant *constant;
	dsVariable *variable;
};



// native functions
/////////////////////

// construction



// Informations
// public func String getName()
dsClassVariable::nfGetName::nfGetName(const sInitData &init) : dsFunction(init.clsVar,
"getName", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassVariable::nfGetName::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	if(nd->constant){
		rt->PushString( nd->constant->GetName() );
	}else if(nd->variable){
		rt->PushString( nd->variable->GetName() );
	}else DSTHROW(dueNullPointer);
}
// public func String getFullName()
dsClassVariable::nfGetFullName::nfGetFullName(const sInitData &init) : dsFunction(init.clsVar,
"getFullName", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassVariable::nfGetFullName::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	dsClassVariable *clsVar = (dsClassVariable*)GetOwnerClass();
	clsVar->PushFullName(rt, nd->parent, nd->constant, nd->variable);
}
// public func Class getType()
dsClassVariable::nfGetType::nfGetType(const sInitData &init) : dsFunction(init.clsVar,
"getType", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
}
void dsClassVariable::nfGetType::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = ((dsClassVariable*)GetOwnerClass())->GetClassClass();
	if(nd->constant){
		clsClass->PushClass( rt, nd->constant->GetType() );
	}else if(nd->variable){
		clsClass->PushClass( rt, nd->variable->GetType() );
	}else DSTHROW(dueNullPointer);
}
// public func int getTypeModifiers()
dsClassVariable::nfGetTypeModifiers::nfGetTypeModifiers(const sInitData &init) : dsFunction(init.clsVar,
"getTypeModifiers", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassVariable::nfGetTypeModifiers::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	if(nd->constant){
		rt->PushInt( DSTM_PUBLIC | DSTM_STATIC | DSTM_FIXED );
	}else if(nd->variable){
		rt->PushInt( nd->variable->GetTypeModifiers() );
	}else DSTHROW(dueNullPointer);
}
// public func Class getParent()
dsClassVariable::nfGetParent::nfGetParent(const sInitData &init) : dsFunction(init.clsVar,
"getParent", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
}
void dsClassVariable::nfGetParent::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = ((dsClassVariable*)GetOwnerClass())->GetClassClass();
	if( !nd->parent ) DSTHROW(dueNullPointer);
	clsClass->PushClass( rt, nd->parent );
}
// public func Object getValue(Object obj)
dsClassVariable::nfGetValue::nfGetValue(const sInitData &init) : dsFunction(init.clsVar,
"getValue", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj){
	p_AddParameter(init.clsObj); // obj
}
void dsClassVariable::nfGetValue::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	dsValue *obj = rt->GetValue(0);
	if(nd->constant){
		switch(nd->constant->GetType()->GetPrimitiveType()){
		case DSPT_BYTE:
			rt->PushByte( nd->constant->GetByte() ); break;
		case DSPT_BOOL:
			rt->PushBool( nd->constant->GetBool() ); break;
		case DSPT_INT:
			rt->PushInt( nd->constant->GetInt() ); break;
		case DSPT_FLOAT:
			rt->PushFloat( nd->constant->GetFloat() ); break;
		default:
			DSTHROW(dueInvalidParam);
		}
	}else if(nd->variable){
		if(nd->variable->GetTypeModifiers() & DSTM_FIXED){
			rt->PushValue( nd->variable->GetStaticValue() );
		}else{
			dsRealObject *robj = obj->GetRealObject();
			if(obj->GetType()->GetPrimitiveType() != DSPT_OBJECT){
				DSTHROW(dueInvalidParam);
			}else if(!robj){
				DSTHROW(dueNullPointer);
			}else if(!robj->GetType()->CastableTo(nd->parent)){
				DSTHROW(dseInvalidCast);
			}else{
				rt->PushValue( nd->variable->GetValue(robj->GetBuffer()) );
			}
		}
	}else DSTHROW(dueNullPointer);
}


// convenience functions
// public func bool isSet(int flags)
dsClassVariable::nfIsSet::nfIsSet(const sInitData &init) :
dsFunction(init.clsClass, "isSet", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsInt); // flags
}

void dsClassVariable::nfIsSet::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	int flags = rt->GetValue(0)->GetInt();
	if(nd->constant){
		rt->PushBool( ((DSTM_PUBLIC | DSTM_STATIC | DSTM_FIXED) & flags) != 0);
	}else if(nd->variable){
		rt->PushBool( (nd->variable->GetTypeModifiers() & flags) != 0);
	}else DSTHROW(dueNullPointer);
}


// other functions
// public func int hashCode()
dsClassVariable::nfHashCode::nfHashCode(const sInitData &init) :
dsFunction(init.clsVar, "hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}

void dsClassVariable::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	if(nd->constant){
		rt->PushInt( ( intptr_t )nd->constant );
	}else if(nd->variable){
		rt->PushInt( ( intptr_t )nd->variable );
	}else DSTHROW(dueNullPointer);
}

// public func bool equals(Object obj)
dsClassVariable::nfEquals::nfEquals(const sInitData &init) :
dsFunction(init.clsVar, "equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsObj); // obj
}
void dsClassVariable::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	dsClassVariable *clsVar = (dsClassVariable*)GetOwnerClass();
	if(!nd->constant && !nd->variable) DSTHROW(dueNullPointer);
	dsValue *obj = rt->GetValue(0);
	// compare objects
	if(p_IsObjOfType(obj, clsVar)){
		sVarNatData *nd2 = (sVarNatData*)p_GetNativeData(obj);
		if(nd->constant){
			rt->PushBool( nd->constant == nd2->constant );
		}else{
			rt->PushBool( nd->variable == nd2->variable );
		}
	}else{
		rt->PushBool(false);
	}
}

// public func String toString()
dsClassVariable::nfToString::nfToString(const sInitData &init) :
dsFunction(init.clsVar, "toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassVariable::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sVarNatData *nd = (sVarNatData*)p_GetNativeData(myself);
	dsClassVariable *clsVar = (dsClassVariable*)GetOwnerClass();
	clsVar->PushFullName(rt, nd->parent, nd->constant, nd->variable);
}




// class dsClassVariable
//////////////////////////
// constructor, destructor
dsClassVariable::dsClassVariable() : dsClass("Variable", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE) {
	pClsClass = NULL;
	GetParserInfo()->SetParent("Introspection");
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(sizeof(sVarNatData));
}
dsClassVariable::~dsClassVariable(){ }

// management
void dsClassVariable::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsVar = this;
	init.clsClass = engine->GetClass(ISPECT_CLASS);
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsBool = engine->GetClassBool();
	init.clsStr = engine->GetClassString();
	init.clsObj = engine->GetClassObject();
	pClsClass = (dsClassClass*)init.clsClass;
	// functions
	AddFunction(new nfGetName(init));
	AddFunction(new nfGetFullName(init));
	AddFunction(new nfGetType(init));
	AddFunction(new nfGetTypeModifiers(init));
	AddFunction(new nfGetParent(init));
	AddFunction(new nfGetValue(init));
	AddFunction(new nfIsSet(init));
	AddFunction(new nfHashCode(init));
	AddFunction(new nfEquals(init));
	AddFunction(new nfToString(init));
}

// internal functions
void dsClassVariable::PushConstant(dsRunTime *rt, dsClass *parent, dsConstant *constant){
	if(!parent || !constant) DSTHROW(dueInvalidParam);
	// create new introspection variable object
	dsValue *value = rt->CreateValue(this);
	sVarNatData *nd;
	try{
		rt->CreateObjectNaked(value, this);
		nd = (sVarNatData*)p_GetNativeData(value->GetRealObject()->GetBuffer());
		nd->parent = parent;
		nd->constant = constant;
		nd->variable = NULL;
		rt->PushValue(value);
		rt->FreeValue(value);
	}catch( ... ){
		if(value) rt->FreeValue(value);
		throw;
	}
}
void dsClassVariable::PushVariable(dsRunTime *rt, dsVariable *variable){
	if(!variable) DSTHROW(dueInvalidParam);
	// create new introspection variable object
	dsValue *value = rt->CreateValue(this);
	sVarNatData *nd;
	try{
		rt->CreateObjectNaked(value, this);
		nd = (sVarNatData*)p_GetNativeData(value->GetRealObject()->GetBuffer());
		nd->parent = variable->GetOwnerClass();
		nd->constant = NULL;
		nd->variable = variable;
		rt->PushValue(value);
		rt->FreeValue(value);
	}catch( ... ){
		if(value) rt->FreeValue(value);
		throw;
	}
}
void dsClassVariable::PushFullName(dsRunTime *rt, dsClass *parent, dsConstant *constant, dsVariable *variable){
	if(!rt || !parent) DSTHROW(dueInvalidParam);
	// determine the name of the constant/variable
	const char *name;
	if(constant){
		name = constant->GetName();
	}else if(variable){
		name = variable->GetName();
	}else DSTHROW(dueNullPointer);
	// create and push full name string
	int nameLen = strlen(name);
	int fullNameLen = parent->GetFullName(NULL, 0);
	char *fullName = new char[fullNameLen+nameLen+2];
	try{
		parent->GetFullName(fullName, fullNameLen);
		sprintf(fullName+fullNameLen, ".%s", name);
		rt->PushString(fullName);
		delete [] fullName;
	}catch( ... ){
		if(fullName) delete [] fullName;
		throw;
	}
}
