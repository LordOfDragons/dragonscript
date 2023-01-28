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
#include <stdlib.h>
#include "dragonscript_config.h"
#include "dsScriptSource.h"
#include "dsDefaultEngineManager.h"

// class dsDefaultEngineManager
/////////////////////////////////
// message output
void dsDefaultEngineManager::OutputMessage(const char *Message){
	printf("%s\n", Message);
}
void dsDefaultEngineManager::OutputWarning(const char *Message, int WarnID, dsScriptSource *Script, int Line, int Position){
	printf("[WARN#%i] %s(%i/%i): %s\n", WarnID, Script->GetName(), Line, Position, Message);
}
void dsDefaultEngineManager::OutputWarningMore(const char *Message){
	printf("   %s\n", Message);
}
void dsDefaultEngineManager::OutputError(const char *Message, int ErrorID, dsScriptSource *Script, int Line, int Position){
	printf("[ERR#%i] %s(%i/%i): %s\n", ErrorID, Script->GetName(), Line, Position, Message);
}
void dsDefaultEngineManager::OutputErrorMore(const char *Message){
	printf("   %s\n", Message);
}
// parser management
bool dsDefaultEngineManager::ContinueParsing(){
	return true;
}
