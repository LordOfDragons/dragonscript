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
// File: class engine manager

// includes
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include "../scriptengine/libdscript.h"
#include "dsiEngineManager.h"

// dsiEngineManager
/////////////////////
// constructor, destructor
dsiEngineManager::dsiEngineManager(){ }
dsiEngineManager::~dsiEngineManager(){ }

// message output
void dsiEngineManager::OutputMessage(const char *Message){
	printf("%s\n", Message);
}
void dsiEngineManager::OutputWarning(const char *Message, int WarnID, dsScriptSource *Script, int Line, int Position){
	printf("[W#%i] %s(%i,%i): %s\n", WarnID, Script->GetName(), Line, Position, Message);
}
void dsiEngineManager::OutputWarningMore(const char *Message){
	printf("   %s\n", Message);
}
void dsiEngineManager::OutputError(const char *Message, int ErrorID, dsScriptSource *Script, int Line, int Position){
	printf("[E#%i] %s(%i,%i): %s\n", ErrorID, Script->GetName(), Line, Position, Message);
}
void dsiEngineManager::OutputErrorMore(const char *Message){
	printf("   %s\n", Message);
}
// parser management
bool dsiEngineManager::ContinueParsing(){
	return true;
}
