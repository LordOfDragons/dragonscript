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
#include <string.h>
#include "../dragonscript_config.h"
#include "dsClassParserInfo.h"
#include "../exceptions.h"


// class dsClassParserInfo
////////////////////////////
// constructor, destructor
dsClassParserInfo::dsClassParserInfo(){
	p_parent = NULL;
	p_base = NULL;
	p_ifaces = NULL;
	p_ifaceCount = 0;
	p_reqs = NULL;
	p_reqCount = 0;
	try{
		p_parent = new char[1];
		if(!p_parent) DSTHROW(dueOutOfMemory);
		p_parent[0] = '\0';
		p_base = new char[1];
		if(!p_base) DSTHROW(dueOutOfMemory);
		p_base[0] = '\0';
	}catch( ... ){
		if(p_parent) delete [] p_parent;
		if(p_base) delete [] p_base;
		throw;
	}
}
dsClassParserInfo::~dsClassParserInfo(){
	if(p_reqs){
		for(int i=0; i<p_reqCount; i++){
			if(p_reqs[i]) delete [] p_reqs[i];
		}
		delete [] p_reqs;
	}
	if(p_ifaces){
		for(int i=0; i<p_ifaceCount; i++){
			if(p_ifaces[i]) delete [] p_ifaces[i];
		}
		delete [] p_ifaces;
	}
	if(p_parent) delete [] p_parent;
	if(p_base) delete [] p_base;
}
// management

const char *dsClassParserInfo::GetInterface(int index) const{
	if(index<0 || index>=p_ifaceCount) DSTHROW(dueOutOfBoundary);
	return (const char *)p_ifaces[index];
}

const char *dsClassParserInfo::GetRequires(int index) const{
	if(index<0 || index>=p_reqCount) DSTHROW(dueOutOfBoundary);
	return (const char *)p_reqs[index];
}

/* set the full classname of the parent class. "" means top-level.
 */
void dsClassParserInfo::SetParent(const char *className){
	if(!className) DSTHROW(dueInvalidParam);
	if(!p_IsValid(className)) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( className );
	char *newStr = new char[size+1];
	if(!newStr) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( newStr, size + 1, className, size );
	#else
		strncpy(newStr, className, size );
	#endif
	newStr[ size ] = 0;
	if(p_parent) delete [] p_parent;
	p_parent = newStr;
}

/* set the full classname of the base class. "" means no base class.
 */
void dsClassParserInfo::SetBase(const char *className){
	if(!className) DSTHROW(dueInvalidParam);
	if(!p_IsValid(className)) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( className );
	char *newStr = new char[size+1];
	if(!newStr) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( newStr, size + 1, className, size );
	#else
		strncpy(newStr, className, size);
	#endif
	newStr[ size ] = 0;
	if(p_base) delete [] p_base;
	p_base = newStr;
}

/* add interface by full classname.
 */
void dsClassParserInfo::AddInterface(const char *className){
	if(!className) DSTHROW(dueInvalidParam);
	if(!p_IsValid(className)) DSTHROW(dueInvalidParam);
	if(className[0] == '\0') DSTHROW(dueInvalidParam);
	// enlarge array
	char **newArray = new char*[p_ifaceCount+1];
	if(!newArray) DSTHROW(dueOutOfMemory);
	if(p_ifaces){
		for(int i=0; i<p_ifaceCount; i++) newArray[i] = p_ifaces[i];
		delete [] p_ifaces;
		p_ifaces = newArray;
	}
	// add entry
	const int size = ( int )strlen( className );
	p_ifaces[p_ifaceCount] = new char[size+1];
	if(!p_ifaces[p_ifaceCount]) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_ifaces[p_ifaceCount], size + 1, className, size );
	#else
		strncpy(p_ifaces[p_ifaceCount], className, size);
	#endif
	p_ifaces[ p_ifaceCount ][ size ] = 0;
	p_ifaceCount++;
}

/* add required package.
 */
void dsClassParserInfo::AddRequiredPackage(const char *packageName){
	if(!packageName) DSTHROW(dueInvalidParam);
	if(packageName[0] == '\0') DSTHROW(dueInvalidParam);
	// enlarge array
	char **newArray = new char*[p_reqCount+1];
	if(!newArray) DSTHROW(dueOutOfMemory);
	if(p_reqs){
		for(int i=0; i<p_reqCount; i++) newArray[i] = p_reqs[i];
		delete [] p_reqs;
		p_reqs = newArray;
	}
	// add entry
	const int size = ( int )strlen( packageName );
	p_reqs[p_reqCount] = new char[size+1];
	if(!p_reqs[p_reqCount]) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_reqs[p_reqCount], size + 1, packageName, size );
	#else
		strncpy(p_reqs[p_reqCount], packageName, size);
	#endif
	p_reqs[ p_reqCount ][ size ] = 0;
	p_reqCount++;
}

/* check if the given name is a valid full classname.
 * a valid full classname is of the form
 * fullname = [ <name> [ '.' <name> ] ... ]
 */
bool dsClassParserInfo::p_IsValid(const char *name) const{
	if(!name) return false;
	char *curPos=(char*)name, *nextPos, *endPos=(char*)(name+strlen(name));
	while(true){
		// find next part
		nextPos = strchr(curPos, '.');
		if(!nextPos) nextPos = endPos;
		// if part is of 0 length there's something wrong
		if(nextPos == curPos) return false;
		// advance pointer or break
		if(nextPos == endPos) break;
		curPos = nextPos + 1;
	}
	// all ok
	return true;
}
