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
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "../../../config.h"
#include "dsClassException.h"
#include "../../dsEngine.h"
#include "../../dsBaseEngineManager.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../dsExceptionTrace.h"
#include "../../dsExceptionTracePoint.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"
#include "../../objects/dsByteCode.h"

// native data structure
struct sExcepNatData{
	dsExceptionTrace *trace;
};



// native functions
/////////////////////
// public func new()
dsClassException::nfNew::nfNew(const sInitData &init) : dsFunction(init.clsExcep, "new",
DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassException::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	// clear
	nd->trace = NULL;
	// create
	nd->trace = new dsExceptionTrace;
	if( ! nd->trace ) DSTHROW( dueOutOfMemory );
}

// public func new(String reason)
dsClassException::nfNew2::nfNew2(const sInitData &init) : dsFunction(init.clsExcep,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
	p_AddParameter(init.clsStr); // reason
}
void dsClassException::nfNew2::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	// clear
	nd->trace = NULL;
	// create
	const char *reason = rt->GetValue(0)->GetString();
	nd->trace = new dsExceptionTrace( reason );
	if( ! nd->trace ) DSTHROW( dueOutOfMemory );
}

// func destructor()
dsClassException::nfDestructor::nfDestructor(const sInitData &init) : dsFunction(init.clsExcep,
"destructor", DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassException::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	if( nd->trace ){
		delete nd->trace;
		nd->trace = NULL;
	}
}



// public func String getReason()
dsClassException::nfGetReason::nfGetReason(const sInitData &init) : dsFunction(init.clsExcep,
"getReason", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassException::nfGetReason::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	if( ! nd->trace ) DSTHROW( dueNullPointer );
	rt->PushString( nd->trace->GetReason() );
}

// public func void printTrace()
dsClassException::nfPrintTrace::nfPrintTrace(const sInitData &init) : dsFunction(init.clsExcep,
"printTrace", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid){
}
void dsClassException::nfPrintTrace::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	dsClassException *clsExcep = (dsClassException*)GetOwnerClass();
	if( ! nd->trace ) DSTHROW( dueNullPointer );
	clsExcep->PrintTrace( rt, myself );
}



// func int getTraceCount()
dsClassException::nfGetTraceCount::nfGetTraceCount(const sInitData &init) : dsFunction(init.clsExcep,
"getTraceCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassException::nfGetTraceCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	if( ! nd->trace ) DSTHROW( dueNullPointer );
	rt->PushInt( nd->trace->GetPointCount() );
}

// func String getTraceClass(int index)
dsClassException::nfGetTraceClass::nfGetTraceClass(const sInitData &init) : dsFunction(init.clsExcep,
"getTraceClass", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
	p_AddParameter(init.clsInt); // index
}
void dsClassException::nfGetTraceClass::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	if( ! nd->trace ) DSTHROW( dueNullPointer );
	int index = rt->GetValue( 0 )->GetInt();
	dsClassException *clsExcep = (dsClassException*)GetOwnerClass();
	dsExceptionTracePoint *point = nd->trace->GetPointAt( index );
	char *fullName = clsExcep->BuildFullName( point->GetClass() );
	try{
		rt->PushString( fullName );
		delete [] fullName;
	}catch( ... ){
		delete [] fullName;
		throw;
	}
}

// func String getTraceFunction(int index)
dsClassException::nfGetTraceFunction::nfGetTraceFunction(const sInitData &init) : dsFunction(init.clsExcep,
"getTraceFunction", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
	p_AddParameter(init.clsInt); // index
}
void dsClassException::nfGetTraceFunction::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	if( ! nd->trace ) DSTHROW( dueNullPointer );
	int index = rt->GetValue(0)->GetInt();
	dsExceptionTracePoint *point = nd->trace->GetPointAt( index );
	rt->PushString( point->GetFunction()->GetName() );
}

// func int getTraceLine(int index)
dsClassException::nfGetTraceLine::nfGetTraceLine(const sInitData &init) : dsFunction(init.clsExcep,
"getTraceLine", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
	p_AddParameter(init.clsInt); // index
}
void dsClassException::nfGetTraceLine::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	if( ! nd->trace ) DSTHROW( dueNullPointer );
	int index = rt->GetValue(0)->GetInt();
	dsExceptionTracePoint *point = nd->trace->GetPointAt( index );
	rt->PushInt( point->GetLine() );
}



// public func int hashCode()
dsClassException::nfHashCode::nfHashCode(const sInitData &init) : dsFunction(init.clsExcep,
"hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsClassException::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( ( intptr_t )myself->GetRealType() );
}

// public func bool equals(Object obj)
dsClassException::nfEquals::nfEquals(const sInitData &init) : dsFunction(init.clsExcep,
"equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool){
	p_AddParameter(init.clsObj); // obj
}
void dsClassException::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassException *clsExcep = (dsClassException*)GetOwnerClass();
	dsValue *obj = rt->GetValue(0);
	if(!p_IsObjOfType(obj, clsExcep)){
		rt->PushBool( false );
	}else{
		rt->PushBool( myself->GetRealType()->IsEqualTo( obj->GetRealType() ) );
	}
}

// public func String toString()
dsClassException::nfToString::nfToString(const sInitData &init) : dsFunction(init.clsExcep,
"toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
}
void dsClassException::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(myself);
	if( ! nd->trace ) DSTHROW( dueNullPointer );
	rt->PushString( nd->trace->GetReason() );
}



// class dsClassException
///////////////////////////
// constructor, destructor
dsClassException::dsClassException() : dsClass("Exception", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE) {
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(sizeof(sExcepNatData));
}
dsClassException::~dsClassException(){ }

// management
void dsClassException::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsExcep = this;
	init.clsObj = engine->GetClassObject();
	init.clsVoid = engine->GetClassVoid();
	init.clsByte = engine->GetClassByte();
	init.clsBool = engine->GetClassBool();
	init.clsInt = engine->GetClassInt();
	init.clsStr = engine->GetClassString();
	// functions
	AddFunction(new nfNew(init));
	AddFunction(new nfNew2(init));
	AddFunction(new nfDestructor(init));
	AddFunction(new nfGetReason(init));
	AddFunction(new nfPrintTrace(init));
	AddFunction(new nfGetTraceCount(init));
	AddFunction(new nfGetTraceClass(init));
	AddFunction(new nfGetTraceFunction(init));
	AddFunction(new nfGetTraceLine(init));
	AddFunction(new nfHashCode(init));
	AddFunction(new nfEquals(init));
	AddFunction(new nfToString(init));
}

// internal functions
dsExceptionTrace *dsClassException::GetTrace( dsRealObject *myself ){
	if( ! myself ) DSTHROW( dueInvalidParam );
	return ( ( sExcepNatData* )p_GetNativeData( myself->GetBuffer() ) )->trace;
}
void dsClassException::PushException(dsRunTime *rt, const char *reason){
	if(!rt || !reason) DSTHROW(dueInvalidParam);
	// create new exception with the given reason
	dsValue *value = rt->CreateValue(this);
	sExcepNatData *nd;
	try{
		rt->CreateObjectNaked(value, this);
		nd = (sExcepNatData*)p_GetNativeData(value->GetRealObject()->GetBuffer());
		// clear
		nd->trace = NULL;
		// create
		nd->trace = new dsExceptionTrace( reason );
		if( ! nd->trace ) DSTHROW( dueOutOfMemory );
		// push 'n go home
		rt->PushValue(value);
		rt->FreeValue(value);
	}catch( ... ){
		rt->FreeValue(value);
		throw;
	}
}
void dsClassException::AddTrace(dsValue *obj, dsClass *callClass, dsFunction *function, int codeOffset){
	if(!obj || !callClass || !function || codeOffset<0) DSTHROW(dueInvalidParam);
	dsRealObject *realObj = obj->GetRealObject();
	dsByteCode *byteCode;
	int line;
	if(!realObj) DSTHROW(dueNullPointer);
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(realObj->GetBuffer());
	// determine line number
	byteCode = function->GetByteCode();
	line = byteCode ? byteCode->GetDebugLine(codeOffset) : codeOffset;
	// add entry
	nd->trace->AddPoint( callClass, function, line );
}
void dsClassException::PrintTrace(dsRunTime *rt, dsValue *obj){
	if(!rt || !obj) DSTHROW(dueInvalidParam);
	dsRealObject *realObj = obj->GetRealObject();
	sExcepNatData *nd = (sExcepNatData*)p_GetNativeData(realObj->GetBuffer());
	dsBaseEngineManager *engMgr = rt->GetEngine()->GetEngineManager();
	dsExceptionTracePoint *point;
	int i, strLen, line;
	const char *reason, *funcName;
	char *buffer=NULL, *fullName=NULL;
	// print exception trace only if there is at last one element in the list
	// and the object is not null
	if( ! realObj || nd->trace->GetPointCount() == 0 ) return;
	try{
		// print out the reason for the exception
		fullName = BuildFullName( realObj->GetType() );
		reason = nd->trace->GetReason();
		strLen = 30 + strlen(fullName) + strlen(reason);
		buffer = new char[strLen+1];
		if(!buffer) DSTHROW(dueOutOfMemory);
		sprintf(buffer, "%s: %s", fullName, reason);
		engMgr->OutputMessage(buffer);
		delete [] buffer; buffer = NULL;
		delete [] fullName; fullName = NULL;
		// print out all trace elements
		for(i=0; i<nd->trace->GetPointCount(); i++){
			point = nd->trace->GetPointAt( i );
			fullName = BuildFullName( point->GetClass() );
			funcName = point->GetFunction()->GetName();
			line = point->GetLine();
			strLen = 40 + strlen(fullName) + strlen(funcName);
			buffer = new char[strLen+1];
			if(!buffer) DSTHROW(dueOutOfMemory);
			if( point->GetFunction()->GetTypeModifiers() & DSTM_NATIVE ){
				sprintf(buffer, "%i) Native %s.%s: line %i", i+1, fullName, funcName, line);
			}else{
				sprintf(buffer, "%i) Script %s.%s: line %i", i+1, fullName, funcName, line);
			}
			engMgr->OutputMessage(buffer);
			delete [] buffer; buffer = NULL;
			delete [] fullName; fullName = NULL;
		}
		
	}catch( const duException &e ){
		if(buffer) delete [] buffer;
		if(fullName) delete [] fullName;
		e.PrintError();
		//throw;
		
	}catch( ... ){
		if(buffer) delete [] buffer;
		if(fullName) delete [] fullName;
		//throw;
	}
}

char *dsClassException::BuildFullName(dsClass *theClass){
	if(!theClass) DSTHROW(dueInvalidParam);
	// determine the length of the needed string first
	int fullNameLen=0, nameLen;
	dsClass *curClass = theClass;
	while(curClass){
		fullNameLen += strlen(curClass->GetName()) + 1;
		curClass = curClass->GetParent();
	}
	fullNameLen--;
	// create string containing full name
	char *fullName = new char[fullNameLen+1];
	if(!fullName) DSTHROW(dueOutOfMemory);
	fullName[fullNameLen] = '\0';
	curClass = theClass;
	while(curClass){
		nameLen = strlen(curClass->GetName());
		fullNameLen -= nameLen;
		strncpy(fullName+fullNameLen, curClass->GetName(), nameLen);
		curClass = curClass->GetParent();
		if(curClass){
			fullNameLen--;
			fullName[fullNameLen] = '.';
		}
	}
	return fullName;
}
