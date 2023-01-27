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
#include "dsClassPackage.h"
#include "dsClassClass.h"

// native data structure
struct sPakNatData{
	dsPackage *package;
};



// native functions
/////////////////////

// construction
// public func new(String name)
dsClassPackage::nfNew::nfNew(const sInitData &init) : dsFunction(init.clsPak, "new",
DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsStr); // name
}
void dsClassPackage::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	nd->package = NULL;
	// determine package matching the string
	const char *name = rt->GetValue(0)->GetString();
	nd->package = rt->GetEngine()->GetPackage(name);
	if(!nd->package) DSTHROW(dueInvalidParam);
}



// Informations
// public func String getName()
dsClassPackage::nfGetName::nfGetName(const sInitData &init) : dsFunction(init.clsPak,
"getName", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassPackage::nfGetName::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	if(!nd->package) DSTHROW(dueNullPointer);
	rt->PushString( nd->package->GetName() );
}



// Classes
// public func int getClassCount()
dsClassPackage::nfGetClassCount::nfGetClassCount(const sInitData &init) : dsFunction(init.clsPak,
"getClassCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassPackage::nfGetClassCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	if(!nd->package) DSTHROW(dueNullPointer);
	rt->PushInt( nd->package->GetClassCount() );
}
// public func Class getClass(int index)
dsClassPackage::nfGetClass::nfGetClass(const sInitData &init) : dsFunction(init.clsPak,
"getClass", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsClass){
	p_AddParameter(init.clsInt); // index
}
void dsClassPackage::nfGetClass::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	int index = rt->GetValue(0)->GetInt();
	dsClassClass *clsClass = ((dsClassPackage*)GetOwnerClass())->GetClassClass();
	if(!nd->package) DSTHROW(dueNullPointer);
	if(index<0 || index>=nd->package->GetClassCount()) DSTHROW(dueOutOfBoundary);
	clsClass->PushClass( rt, nd->package->GetClass(index) );
}
// public func bool containsClass(Class aClass)
dsClassPackage::nfContainsClass::nfContainsClass(const sInitData &init) : dsFunction(init.clsPak,
"containsClass", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsClass); // name
}
void dsClassPackage::nfContainsClass::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	dsClassClass *clsClass = ((dsClassPackage*)GetOwnerClass())->GetClassClass();
	dsRealObject *obj = rt->GetValue(0)->GetRealObject();
	if(!nd->package) DSTHROW(dueNullPointer);
	rt->PushBool( nd->package->ExistsClass( clsClass->GetClassFromObj(obj) ) );
}


// other functions
// public func int hashCode()
dsClassPackage::nfHashCode::nfHashCode(const sInitData &init) :
dsFunction(init.clsPak, "hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}

void dsClassPackage::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	if(!nd->package) DSTHROW(dueNullPointer);
	rt->PushInt( ( intptr_t )nd->package );
}

// public func bool equals(Object obj)
dsClassPackage::nfEquals::nfEquals(const sInitData &init) :
dsFunction(init.clsPak, "equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsObj); // obj
}
void dsClassPackage::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	dsClassPackage *clsPak = (dsClassPackage*)GetOwnerClass();
	if(!nd->package) DSTHROW(dueNullPointer);
	dsValue *obj = rt->GetValue(0);
	// compare objects
	if(p_IsObjOfType(obj, clsPak)){
		sPakNatData *nd2 = (sPakNatData*)p_GetNativeData(obj);
		if(nd2->package){
			rt->PushBool( nd->package == nd2->package );
		}else{
			rt->PushBool(false);
		}
	}else{
		rt->PushBool(false);
	}
}

// public func String toString()
dsClassPackage::nfToString::nfToString(const sInitData &init) :
dsFunction(init.clsPak, "toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassPackage::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sPakNatData *nd = (sPakNatData*)p_GetNativeData(myself);
	if(!nd->package) DSTHROW(dueNullPointer);
	rt->PushString( nd->package->GetName() );
}




// class dsClassPackage
///////////////////////
// constructor, destructor
dsClassPackage::dsClassPackage() : dsClass("Package", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE) {
	pClsClass = NULL;
	GetParserInfo()->SetParent("Introspection");
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(sizeof(sPakNatData));
}
dsClassPackage::~dsClassPackage(){ }

// management
void dsClassPackage::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsPak = this;
	init.clsClass = engine->GetClass(ISPECT_CLASS);
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsBool = engine->GetClassBool();
	init.clsStr = engine->GetClassString();
	init.clsObj = engine->GetClassObject();
	pClsClass = (dsClassClass*)init.clsClass;
	// functions
	AddFunction(new nfNew(init));
	AddFunction(new nfGetName(init));
	AddFunction(new nfGetClassCount(init));
	AddFunction(new nfGetClass(init));
	AddFunction(new nfContainsClass(init));
	AddFunction(new nfHashCode(init));
	AddFunction(new nfEquals(init));
	AddFunction(new nfToString(init));
}

// internal functions
void dsClassPackage::PushPackage(dsRunTime *rt, dsPackage *pak){
	if(!rt || !pak) DSTHROW(dueInvalidParam);
	// create new introspection object
	dsValue *value = rt->CreateValue(this);
	sPakNatData *nd;
	try{
		rt->CreateObjectNaked(value, this);
		nd = (sPakNatData*)p_GetNativeData(value->GetRealObject()->GetBuffer());
		nd->package = pak;
		rt->PushValue(value);
		rt->FreeValue(value);
	}catch( ... ){
		if(value) rt->FreeValue(value);
		throw;
	}
}
dsPackage *dsClassPackage::GetPackageFromObj(dsRealObject *obj){
	if(!obj) DSTHROW(dueInvalidParam);
	return ((sPakNatData*)p_GetNativeData(obj->GetBuffer()))->package;
}
