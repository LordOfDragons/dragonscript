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
#include "../../../config.h"
#include "dsClassWeakRef.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// native data structure
struct sWeakNatData{
	dsRealObject *obj;
};

// native functions
/////////////////////
// func new()
dsClassWeakRef::nfNew::nfNew(const sInitData &init) : dsFunction(init.clsWeak,
DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassWeakRef::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sWeakNatData *nd = (sWeakNatData*)p_GetNativeData(myself);
	nd->obj = NULL;
}

// func new(object obj)
dsClassWeakRef::nfNew2::nfNew2(const sInitData &init) : dsFunction(init.clsWeak,
DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObj); // obj
}
void dsClassWeakRef::nfNew2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sWeakNatData *nd = (sWeakNatData*)p_GetNativeData(myself);
	nd->obj = NULL;
	dsValue *vObj = rt->GetValue(0);
	// should we bail out if this is a primitive object?
	if(vObj->GetType()->GetPrimitiveType()==DSPT_OBJECT && vObj->GetRealObject()){
		nd->obj = vObj->GetRealObject();
		rt->AddWeakRef(nd->obj);
	}
}

// func destructor
dsClassWeakRef::nfDestructor::nfDestructor(const sInitData &init) : dsFunction(init.clsWeak,
DSFUNC_DESTRUCTOR, DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassWeakRef::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sWeakNatData *nd = (sWeakNatData*)p_GetNativeData(myself);
	if(nd->obj){
		rt->RemoveWeakRef(nd->obj);
		nd->obj = NULL;
	}
}

// func object get()
dsClassWeakRef::nfGet::nfGet(const sInitData &init) : dsFunction(init.clsWeak,
"get", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsObj){
}
void dsClassWeakRef::nfGet::RunFunction( dsRunTime *rt, dsValue *myself ){
	sWeakNatData *nd = (sWeakNatData*)p_GetNativeData(myself);
	if(nd->obj && (nd->obj->GetRefCount() > 0)){
		rt->PushObject(nd->obj);
	}else{
		rt->PushObject(NULL);
	}
}

// func void set(object obj)
dsClassWeakRef::nfSet::nfSet(const sInitData &init) : dsFunction(init.clsWeak,
"set", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsObj); // obj
}
void dsClassWeakRef::nfSet::RunFunction( dsRunTime *rt, dsValue *myself ){
	sWeakNatData *nd = (sWeakNatData*)p_GetNativeData(myself);
	dsValue *vObj = rt->GetValue(0);
	if(nd->obj){
		rt->RemoveWeakRef(nd->obj);
		nd->obj = NULL;
	}
	if(vObj->GetType()->GetPrimitiveType()==DSPT_OBJECT && vObj->GetRealObject()){
		nd->obj = vObj->GetRealObject();
		rt->AddWeakRef(nd->obj);
	}
}

// public func int hashCode()
dsClassWeakRef::nfHashCode::nfHashCode(const sInitData &init) : dsFunction(init.clsWeak,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassWeakRef::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	sWeakNatData *nd = (sWeakNatData*)p_GetNativeData(myself);
	rt->PushInt( ( intptr_t )nd->obj );
}

// public func bool equals(Object obj)
dsClassWeakRef::nfEquals::nfEquals(const sInitData &init) : dsFunction(init.clsWeak,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsObj); // obj
}
void dsClassWeakRef::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsRealObject *thisObj = ((sWeakNatData*)p_GetNativeData(myself))->obj;
	dsClass *clsWeakRef = (dsClassWeakRef*)GetOwnerClass();
	dsValue *obj = rt->GetValue(0);
	// check if the object is a weak reference and not null
	if( p_IsObjOfType(obj, clsWeakRef) ){
		dsRealObject *otherObj = ((sWeakNatData*)p_GetNativeData(obj))->obj;
		rt->PushBool( thisObj == otherObj );
	}else{
		rt->PushBool(false);
	}
}



// class dsClassWeakRef
/////////////////////////
// constructor, destructor
dsClassWeakRef::dsClassWeakRef() : dsClass("WeakReference", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE) {
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(sizeof(sWeakNatData));
}
dsClassWeakRef::~dsClassWeakRef(){ }

// management
void dsClassWeakRef::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsWeak = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsBool = engine->GetClassBool();
	init.clsObj = engine->GetClassObject();
	// functions
	AddFunction(new nfNew(init));
	AddFunction(new nfNew2(init));
	AddFunction(new nfDestructor(init));
	AddFunction(new nfGet(init));
	AddFunction(new nfSet(init));
	AddFunction(new nfHashCode(init));
	AddFunction(new nfEquals(init));
}
