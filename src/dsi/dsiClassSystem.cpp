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
// File: script class system

// includes
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "dsiClassSystem.h"

// native functions
/////////////////////
// public static func void Print(string text)
dsiClassSystem::nfPrint::nfPrint(const sInitData &init) : dsFunction(init.clsSys, "print",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsVoid){
	p_AddParameter(init.clsStr); // text
}
void dsiClassSystem::nfPrint::RunFunction( dsRunTime *rt, dsValue *myself ){
	printf("%s",rt->GetValue(0)->GetString());
}

// public static func string GetString()
dsiClassSystem::nfGetString::nfGetString(const sInitData &init) : dsFunction(init.clsSys,
"getString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsStr){
}
void dsiClassSystem::nfGetString::RunFunction( dsRunTime *rt, dsValue *myself ){
	char * const buffer = new char[ 81 ];
	if( ! buffer ){
		DSTHROW( dueOutOfMemory );
	}
	
	try{
		if( ! fgets( buffer, 80, stdin ) ){
			DSTHROW( dueInvalidAction );
		}
		
		const int last = ( int )strlen( buffer );
		if( last > 0 && buffer[ last - 1 ] == '\n' ){
			buffer[ last - 1 ] = '\0';
		}
		
		rt->PushString( buffer );
		
	}catch( ... ){
		delete [] buffer;
		throw;
	}
	
	delete [] buffer;
}

// public static float GetTickTime()
dsiClassSystem::nfGetTickTime::nfGetTickTime(const sInitData &init) : dsFunction(init.clsSys,
"getTickTime", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
}
void dsiClassSystem::nfGetTickTime::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat((float)clock() / CLOCKS_PER_SEC);
}




// dsiClassSystem
///////////////////
// constructors, destructor
dsiClassSystem::dsiClassSystem() : dsClass("System", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE){
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(0);
	CalcMemberOffsets();
}
dsiClassSystem::~dsiClassSystem(){
}
// management
void dsiClassSystem::CreateClassMembers(dsEngine *engine){
	sInitData init;
	// store classes
	init.clsSys = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsFlt = engine->GetClassFloat();
	init.clsStr = engine->GetClassString();
	// functions
	AddFunction(new nfPrint(init));
	AddFunction(new nfGetString(init));
	AddFunction(new nfGetTickTime(init));
}
