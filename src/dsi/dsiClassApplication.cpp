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



// Project: DragonScript - Interpreter
// Version: 1.0
// File: script class application

// includes
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../config.h"
#include "dsiClassApplication.h"


// native functions
/////////////////////
// protected new()
dsiClassApplication::nfCreate::nfCreate(const sInitData &init) : dsFunction(init.clsApp,
DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR, DSTM_PROTECTED | DSTM_NATIVE, init.clsVoid){
}
void dsiClassApplication::nfCreate::RunFunction( dsRunTime *rt, dsValue *myself ){
}

// public func int getArgumentCount()
dsiClassApplication::nfGetArgumentCount::nfGetArgumentCount(const sInitData &init) :
dsFunction(init.clsApp, "getArgumentCount", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE,
init.clsInt){
	argc = init.argc;
}
void dsiClassApplication::nfGetArgumentCount::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt(argc);
}

// public func string getArgument(int index)
dsiClassApplication::nfGetArgument::nfGetArgument(const sInitData &init) : dsFunction(init.clsApp,
"getArgument", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr){
	p_AddParameter(init.clsInt); // index
	argc = init.argc;
	args = init.args;
}
void dsiClassApplication::nfGetArgument::RunFunction( dsRunTime *rt, dsValue *myself ){
	int index = rt->GetValue(0)->GetInt();
	if((index < 0) || (index >= argc)) DSTHROW(dueOutOfBoundary);
	rt->PushString(args[index]);
}

// public func int run()
dsiClassApplication::nfRun::nfRun(const sInitData &init) : dsFunction(init.clsApp, "run",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt){
}
void dsiClassApplication::nfRun::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt(0);
}



// dsiClassApplication
////////////////////////
// constructors, destructor
dsiClassApplication::dsiClassApplication(int argc, char **args) : dsClass("Application",
DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE){
	p_argc = argc;
	p_args = args;
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(0);
	CalcMemberOffsets();
}
dsiClassApplication::~dsiClassApplication(){
}
// management
void dsiClassApplication::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsApp = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInt = engine->GetClassInt();
	init.clsStr = engine->GetClassString();
	init.argc = p_argc;
	init.args = p_args;
	// functions
	AddFunction(new nfCreate(init));
	AddFunction(new nfGetArgumentCount(init));
	AddFunction(new nfGetArgument(init));
	AddFunction(new nfRun(init));
}
