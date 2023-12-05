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
#include <stdio.h>
#include <string.h>
#include "introspection.h"
#include "dsClassDS.h"
#include "dsClassClass.h"
#include "dsClassPackage.h"


// native functions
/////////////////////

// Classes
// public static func int getClassCount()
dsClassDS::nfGetClassCount::nfGetClassCount(const sInitData &init) : dsFunction(init.clsDS,
"getClassCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsInt){
}
void dsClassDS::nfGetClassCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( rt->GetEngine()->GetClassCount() );
}
// public static func Class getClass(int index)
dsClassDS::nfGetClass::nfGetClass(const sInitData &init) : dsFunction(init.clsDS,
"getClass", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsClass){
	p_AddParameter(init.clsInt); // index
}
void dsClassDS::nfGetClass::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassClass *clsClass = ((dsClassDS*)GetOwnerClass())->GetClassClass();
	int index = rt->GetValue(0)->GetInt();
	if(index<0 || index>=rt->GetEngine()->GetClassCount()) DSTHROW(dueOutOfBoundary);
	clsClass->PushClass( rt, rt->GetEngine()->GetClass(index) );
}



// Packages
// public static func int getPackageCount()
dsClassDS::nfGetPackageCount::nfGetPackageCount(const sInitData &init) : dsFunction(init.clsDS,
"getPackageCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsInt){
}
void dsClassDS::nfGetPackageCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( rt->GetEngine()->GetPackageCount() );
}
// public static func Class getPackage(int index)
dsClassDS::nfGetPackage::nfGetPackage(const sInitData &init) : dsFunction(init.clsDS,
"getPackage", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_STATIC | DSTM_NATIVE, init.clsPak){
	p_AddParameter(init.clsInt); // index
}
void dsClassDS::nfGetPackage::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassPackage *clsPak = ((dsClassDS*)GetOwnerClass())->GetClassPackage();
	int index = rt->GetValue(0)->GetInt();
	if(index<0 || index>=rt->GetEngine()->GetPackageCount()) DSTHROW(dueOutOfBoundary);
	clsPak->PushPackage( rt, rt->GetEngine()->GetPackage(index) );
}




// class dsClassDS
///////////////////////
// constructor, destructor
dsClassDS::dsClassDS() : dsClass("DS", DSCT_CLASS, DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE) {
	pClsClass = NULL;
	pClsPak = NULL;
	GetParserInfo()->SetParent("Introspection");
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(0);
}
dsClassDS::~dsClassDS(){ }

// management
void dsClassDS::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsDS = this;
	init.clsClass = engine->GetClass(ISPECT_CLASS);
	init.clsPak = engine->GetClass(ISPECT_PACKAGE);
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsStr = engine->GetClassString();
	pClsClass = (dsClassClass*)init.clsClass;
	pClsPak = (dsClassPackage*)init.clsPak;
	// functions
	AddFunction(new nfGetClassCount(init));
	AddFunction(new nfGetClass(init));
	AddFunction(new nfGetPackageCount(init));
	AddFunction(new nfGetPackage(init));
}

// internal functions
