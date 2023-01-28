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
#include "dspNodes.h"
#include "dspParserCheck.h"
#include "dspParserCheckType.h"
#include "dspParserCheckFunction.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsFunction.h"
#include "../objects/dsSignature.h"

// dspParserCheckFunction
///////////////////////////
// constructor, destructor
dspParserCheckFunction::dspParserCheckFunction(dspNodeScript *Script, dspNodeClassFunction *Node){
	if(!Script || !Node) DSTHROW(dueInvalidParam);
	p_Script = Script;
	p_FuncNode = Node;
	p_Function = NULL;
	p_Signature = NULL;
	p_ArgNames = NULL;
	p_ArgNameCount = 0;
}
dspParserCheckFunction::dspParserCheckFunction(dspNodeScript *Script, dsFunction *Function){
	if(!Script || !Function) DSTHROW(dueInvalidParam);
	p_Script = Script;
	p_FuncNode = NULL;
	p_Function = Function;
	p_Signature = Function->GetSignature();
	p_ArgNames = NULL;
	p_ArgNameCount = 0;
}
dspParserCheckFunction::~dspParserCheckFunction(){
	if(p_ArgNames){
		for(int i=0; i<p_ArgNameCount; i++) delete [] p_ArgNames[i];
		delete [] p_ArgNames;
	}
}
// management
void dspParserCheckFunction::CreateFunction(dsClass *OwnerClass, dspParserCheck *ParserCheck){
	if(!OwnerClass || !ParserCheck || p_Function) DSTHROW(dueInvalidParam);
	dspNodeFuncArg *vArgNode;
	dsClass *vTypeClass;
	
	// check for invalid type modifiers
	if( ! dsFunction::AreTypeModsValid( p_FuncNode->GetFuncType(), p_FuncNode->GetTypeMods() ) ){
		ParserCheck->ErrorFuncInvTypeMods( p_FuncNode, OwnerClass->GetName(), p_FuncNode->GetName() );
		DSTHROW( dseInvalidSyntax );
	}
	
	// create function
	vTypeClass = ParserCheck->GetClassFromNode(p_FuncNode->GetTypeNode(), OwnerClass);
	p_FuncNode->SetDSType(vTypeClass);
	if(!(p_Function = new dsFunction(OwnerClass, p_FuncNode->GetName(), p_FuncNode->GetFuncType(), p_FuncNode->GetTypeMods(), vTypeClass))) DSTHROW(dueOutOfMemory);
	p_Signature = p_Function->GetSignature();
	// add arguments
	dspListNodeIterator vArgItr(p_FuncNode->GetArgList());
	while(vArgItr.HasNext()){
		vArgNode = (dspNodeFuncArg*)*vArgItr.GetNext();
		vTypeClass = ParserCheck->GetClassFromNode(vArgNode->GetTypeNode(), OwnerClass);
		AddArgName(vArgNode->GetName());
		p_Signature->AddParameter(vTypeClass);
//		printf("[FUNCARG] Argument '%s' added (type '%s')\n", vArgNode->GetName(), vTypeClass->GetName());
	}
}
void dspParserCheckFunction::SetFunction(dsFunction *func){
	if(!func) DSTHROW(dueInvalidParam);
	p_Function = func;
	p_Signature = p_Function->GetSignature();
}
// arguments
const char *dspParserCheckFunction::GetArgumentAt( int index ) const{
	if( index < 0 || index >= p_ArgNameCount ) DSTHROW( dueInvalidParam );
	
	return ( const char * )p_ArgNames[ index ];
}
int dspParserCheckFunction::FindArgument(const char *Name) const{
	for(int i=0; i<p_ArgNameCount; i++){
		if(strcmp(p_ArgNames[i], Name) == 0) return i;
	}
	return -1;
}
// private functions
void dspParserCheckFunction::AddArgName(const char *Name){
	if(!Name) DSTHROW(dueInvalidParam);
	char **vNewArray = new char*[p_ArgNameCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_ArgNames){
		for(int i=0; i<p_ArgNameCount; i++) vNewArray[i] = p_ArgNames[i];
		delete [] p_ArgNames;
	}
	p_ArgNames = vNewArray;
	const int size = ( int )strlen( Name );
	if(!(p_ArgNames[p_ArgNameCount] = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_ArgNames[p_ArgNameCount], size + 1, Name, size );
	#else
		strncpy(p_ArgNames[p_ArgNameCount], Name, size + 1);
	#endif
	p_ArgNames[p_ArgNameCount][ size ] = 0;
	p_ArgNameCount++;
}

