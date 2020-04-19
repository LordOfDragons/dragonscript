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
#include "../../config.h"
#include "dspNodes.h"
#include "dspParserCheckType.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"

// dspParserCheckType
///////////////////////
// constructor, destructor
dspParserCheckType::dspParserCheckType(const char *Name, dspParserCheckType *Type){
	p_Names = NULL;
	p_NameCount = 0;
	if(Type){
		for(int i=0; i<Type->p_NameCount; i++) AddName(Type->p_Names[i]);
	}
	AddName(Name);
}
dspParserCheckType::~dspParserCheckType(){
	if(p_Names){
		for(int i=0; i<p_NameCount; i++) delete [] p_Names[i];
		delete [] p_Names;
	}
}
// management
const char *dspParserCheckType::GetName(int Index) const{
	if(Index<0 || Index>=p_NameCount) DSTHROW(dueOutOfBoundary);
	return (const char *)p_Names[Index];
}
const char *dspParserCheckType::GetLastName() const{
	if(p_NameCount == 0) DSTHROW(dueOutOfBoundary);
	return (const char *)p_Names[p_NameCount-1];
}
void dspParserCheckType::AddName(const char *Name){
	if(!Name || Name[0]=='\0') DSTHROW(dueInvalidParam);
	char **vNewNames = new char*[p_NameCount+1];
	if(!vNewNames) DSTHROW(dueOutOfMemory);
	if(p_Names){
		for(int i=0; i<p_NameCount; i++) vNewNames[i] = p_Names[i];
		delete [] p_Names;
	}
	p_Names = vNewNames;
	if(!(p_Names[p_NameCount] = new char[strlen(Name)+1])) DSTHROW(dueOutOfMemory);
	strcpy(p_Names[p_NameCount], Name);
	p_NameCount++;
}
void dspParserCheckType::AddNameFront(const char *Name){
	if(!Name || Name[0]=='\0') DSTHROW(dueInvalidParam);
	char **vNewNames = new char*[p_NameCount+1];
	if(!vNewNames) DSTHROW(dueOutOfMemory);
	char *vNewName = new char[strlen(Name)+1];
	if(!vNewName){ delete [] vNewNames; DSTHROW(dueOutOfMemory); }
	strcpy(vNewName, Name);
	if(p_Names){
		for(int i=0; i<p_NameCount; i++) vNewNames[i+1] = p_Names[i];
		delete [] p_Names;
	}
	p_Names = vNewNames;
	p_Names[0] = vNewName;
	p_NameCount++;
}

dsClass *dspParserCheckType::GetClass( dsEngine *engine, dsClass *baseClass ) const{
	if( ! engine ){
		DSTHROW( dueInvalidParam );
	}
	
	dsClass *curClass = NULL;
	dsClass *checkClass = NULL;
	int i;
	
	// check base class path
	checkClass = baseClass;
	
	while( true ){
		if( checkClass ){
			curClass = checkClass->GetInnerClass( p_Names[ 0 ] );
			
		}else{
			curClass = engine->GetClass( p_Names[ 0 ] );
		}
		
		if( curClass ){
			for( i=1; i<p_NameCount; i++ ){
				curClass = curClass->GetInnerClass( p_Names[ i ] );
				if( ! curClass ){
					break;
				}
			}
			
			if( curClass ){
				return curClass;
			}
		}
		
		if( ! checkClass ){
			break;
		}
		
		checkClass = checkClass->GetBaseClass();
	}
	
	// check parent class path
	checkClass = baseClass;
	
	while( true ){
		if( checkClass ){
			curClass = checkClass->GetInnerClass( p_Names[ 0 ] );
			
		}else{
			curClass = engine->GetClass( p_Names[ 0 ] );
		}
		
		if( curClass ){
			for( i=1; i<p_NameCount; i++ ){
				curClass = curClass->GetInnerClass( p_Names[ i ] );
				if( ! curClass ){
					break;
				}
			}
			
			if( curClass ){
				return curClass;
			}
		}
		
		if( ! checkClass ){
			break;
		}
		
		checkClass = checkClass->GetParent();
	}
	
	// nothing found
	return NULL;
}

bool dspParserCheckType::Exists(dsEngine *Engine) const{
	if(!Engine) DSTHROW(dueInvalidParam);
	return Engine->GetClass(p_Names[0]) != NULL;
}
dspParserCheckType *dspParserCheckType::Copy(){
	dspParserCheckType *vNewType = NULL;
	try{
		if(!(vNewType = new dspParserCheckType(p_Names[0], NULL))) DSTHROW(dueOutOfBoundary);
		for(int i=1; i<p_NameCount; i++){
			vNewType->AddName(p_Names[i]);
		}
	}catch( ... ){
		if(vNewType) delete vNewType;
		throw;
	}
	return vNewType;
}
// information
bool dspParserCheckType::MatchesType(dspParserCheckType *Type) const{
	if(!Type) DSTHROW(dueInvalidParam);
	if(p_NameCount != Type->p_NameCount) return false;
	for(int i=0; i<p_NameCount; i++){
		if(strcmp(p_Names[i], Type->p_Names[i]) != 0) return false;
	}
	return true;
}

// helper functions
dspParserCheckType *dspParserCheckType::GetTypeFromNode(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dspParserCheckType *vNewType=NULL;
	dspBaseNode *vCurNode=Node;
	const char *vName;
	try{
		while(vCurNode){
			if(vCurNode->GetNodeType() == ntIdent){
				vName = ((dspNodeIdent*)vCurNode)->GetName();
				vCurNode = NULL;
			}else if(vCurNode->GetNodeType() == ntMembVar){
				vName = ((dspNodeMembVar*)vCurNode)->GetName();
				vCurNode = ((dspNodeMembVar*)vCurNode)->GetRealObject();
			}else{
				DSTHROW(dueInvalidParam);
			}
			if(vNewType){
				vNewType->AddNameFront(vName);
			}else{
				if(!(vNewType = new dspParserCheckType(vName))) DSTHROW(dueOutOfMemory);
			}
		}
		
	}catch( ... ){
		if(vNewType) delete vNewType;
		throw;
	}
	return vNewType;
}
dspParserCheckType *dspParserCheckType::GetTypeFromFullName(const char *name){
	if(!name) DSTHROW(dueInvalidParam);
	dspParserCheckType *newType=NULL;
	int orgLen=strlen(name);
	char *curName=NULL, *curPos, *nextPos, *endPos=(char*)(name+orgLen);
	if(orgLen == 0) return NULL;
	try{
		curName = new char[orgLen+1]; // that should be enough for all parts
		if(!curName) DSTHROW(dueOutOfMemory);
		curPos = (char*)name;
		while(true){
			// find next part
			nextPos = strchr(curPos, '.');
			if(!nextPos) nextPos = endPos;
			// if part is of 0 length there's something wrong
			if(nextPos == curPos) DSTHROW(dueInvalidParam);
			// copy name part to name buffer
			strncpy(curName, curPos, nextPos - curPos);
			curName[nextPos-curPos] = '\0';
			// add to type
			if(newType){
				newType->AddName(curName);
			}else{
				newType = new dspParserCheckType(curName);
				if(!newType) DSTHROW(dueOutOfMemory);
			}
			// advance pointer or break
			if(nextPos == endPos) break;
			curPos = nextPos + 1;
		}
		delete [] curName;
	}catch( ... ){
		if(newType) delete newType;
		if(curName) delete curName;
		throw;
	}
	return newType;
}

// debugging
#ifndef __NODBGPRINTF__
void dspParserCheckType::DebugPrint() const{
	printf("[TYPE] %s", p_Names[0]);
	for(int i=1; i<p_NameCount; i++) printf(".%s", p_Names[i]);
	printf("\n");
}
#endif

