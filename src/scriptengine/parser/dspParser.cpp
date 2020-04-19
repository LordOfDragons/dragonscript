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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../config.h"
#include "dspNodes.h"
#include "dspParser.h"
#include "dspScanner.h"
#include "dspParserCheck.h"
#include "dspTypeModifiers.h"
#include "../dsEngine.h"
#include "../dsPackage.h"
#include "../exceptions.h"
#include "../dsBaseEngineManager.h"
#include "../objects/dsClass.h"

// definitions
#define DSP_QUEUESIZE		10
#define DSP_MSGBUFSIZE		250

// dspParser
//////////////
// constructor, destructor
dspParser::dspParser(dsEngine *Engine){
	if(!Engine) DSTHROW(dueInvalidParam);
	p_Engine = Engine;
	p_Package = NULL;
	if(!(p_Scanner = new dspScanner(Engine))) DSTHROW(dueOutOfMemory);
	p_NodeQueue = NULL;
	p_NodeQueueSize = 0;
	p_NodeQueueEffSize = 0;
	p_ScriptNodes = NULL;
	p_ScriptNodeCount = 0;
	p_ErrCount = p_WarnCount = 0;
	p_LastErrLine = 0;
	if(!(p_MsgBuf = new char[DSP_MSGBUFSIZE+1])) DSTHROW(dueOutOfMemory);
}
dspParser::~dspParser(){
	Clear();
	if(p_NodeQueue) delete [] p_NodeQueue;
	delete p_Scanner;
	delete [] p_MsgBuf;
}
// parser management
void dspParser::Clear(){
	if(p_ScriptNodes){
		for(int i=0; i<p_ScriptNodeCount; i++) delete p_ScriptNodes[i];
		delete [] p_ScriptNodes;
		p_ScriptNodes = NULL;
		p_ScriptNodeCount = 0;
	}
	p_ClearQueue();
	p_Scanner->ClearScript();
	p_Package = NULL;
}

bool dspParser::ParsePackage( dsPackage *Package ){
	if( ! Package){
		DSTHROW( dueInvalidParam );
	}
	int i;
	dsBaseEngineManager * const vEngMgr = p_Engine->GetEngineManager();
	
	// prepare for parsing package
	Clear();
	p_Package = Package;
	p_ErrCount = 0;
	p_WarnCount = 0;
	p_LastErrLine = 0;
	
	p_ScriptNodes = new dspNodeScript*[ Package->GetScriptCount() ];
	if( ! p_ScriptNodes ){
		DSTHROW( dueOutOfMemory );
	}
	
	// parse scripts
	for( i=0; i<Package->GetScriptCount(); i++ ){
		if( ! vEngMgr->ContinueParsing()){
			p_Scanner->ClearScript();
			return false;
		}
		
		p_Scanner->SetScript( Package->GetScript( i ) );
		
		try{
			p_ScriptNodes[ p_ScriptNodeCount ] = p_ParseScript( p_Scanner->GetScript() );
			p_ScriptNodeCount++;
			
		}catch( ... ){
			Clear();
			throw;
		}
		p_ErrCount += p_Scanner->GetErrorCount();
		p_WarnCount += p_Scanner->GetWarningCount();
		p_Scanner->ClearScript();
		p_ClearQueue();
	}
	if( p_ErrCount > 0 ){
		return false;
	}
	
	// check and compile scripts
	#ifndef __NODBGPRINTF__
	//for( i=0; i<Package->GetScriptCount(); i++ ) p_ScriptNodes[ i ]->DebugPrint( 0, "" );
	#endif
	
	dspParserCheck vParserCheck( this );
	for( i=0; i<p_ScriptNodeCount; i++ ){
		vParserCheck.ProcessScript( p_ScriptNodes[ i ] );
	}
	if( p_ErrCount > 0 ){
		return false;
	}

	vParserCheck.CheckScripts();
	#ifndef __NODBGPRINTF__
	//for( i=0; i<Package->GetScriptCount(); i++ ) p_ScriptNodes[ i ]->DebugPrint( 0, "" );
	#endif
	if( p_ErrCount > 0 ){
		return false;
	}
	
	vParserCheck.CompileScripts();
	if( p_ErrCount > 0 ){
		return false;
	}
	
	p_Package = NULL;
	return true;
}

// script node informations
dspNodeScript *dspParser::GetScriptNode(int Index) const{
	if(Index<0 || Index>=p_ScriptNodeCount) DSTHROW(dueOutOfBoundary);
	return p_ScriptNodes[Index];
}
// private functions
// node queue management
bool dspParser::p_QueueNextNode(){
	dspBaseNode *vNode = p_Scanner->GetNextNode();
	if(!vNode) return false;
	if(p_NodeQueueSize == p_NodeQueueEffSize){
		dspBaseNode **vNewArr = new dspBaseNode*[p_NodeQueueEffSize+1];
		if(!vNewArr) DSTHROW(dueOutOfMemory);
		if(p_NodeQueue){
			for(int i=0; i<p_NodeQueueSize; i++) vNewArr[i] = p_NodeQueue[i];
			delete [] p_NodeQueue;
		}
		p_NodeQueue = vNewArr;
		p_NodeQueueEffSize++;
	}
	p_NodeQueue[p_NodeQueueSize] = vNode;
	p_NodeQueueSize++;
	return true;
}
void dspParser::p_DropNode(){
	if(p_NodeQueueSize==0 && !p_QueueNextNode()) return;
	delete p_NodeQueue[0];
	for(int i=1; i<p_NodeQueueSize; i++) p_NodeQueue[i-1] = p_NodeQueue[i];
	p_NodeQueueSize--;
}
dspBaseNode *dspParser::p_PopNode(){
	if(p_NodeQueueSize==0 && !p_QueueNextNode()) return NULL;
	dspBaseNode *vNode = p_NodeQueue[0];
	for(int i=1; i<p_NodeQueueSize; i++) p_NodeQueue[i-1] = p_NodeQueue[i];
	p_NodeQueueSize--;
	return vNode;
}
dspBaseNode *dspParser::p_PeekNode(int Index){
	if(Index < 0) DSTHROW(dueInvalidParam);
	while(Index >= p_NodeQueueSize){
		if(!p_QueueNextNode()) return NULL;
	}
	return p_NodeQueue[Index];
}
void dspParser::p_ClearQueue(){
	if(p_NodeQueue){
		for(int i=0; i<p_NodeQueueSize; i++) delete p_NodeQueue[i];
		p_NodeQueueSize = 0;
	}
}
// matching functions
bool dspParser::p_MatchesConst(dspBaseNode *Node){
	if(!Node) return false;
	if(Node->GetNodeType() == ntByte) return true;
	if(Node->GetNodeType() == ntBool) return true;
	if(Node->GetNodeType() == ntInt) return true;
	if(Node->GetNodeType() == ntFloat) return true;
	if(Node->GetNodeType() == ntString) return true;
	return false;
}
bool dspParser::p_MatchesType(dspBaseNode *Node, int Type){
	return Node && Node->GetNodeType()==Type;
}
bool dspParser::p_MatchesIdent(dspBaseNode *Node, const char *Ident){
	if(!Node || Node->GetNodeType()!=ntIdent) return false;
	dspNodeIdent *vIdentNode = (dspNodeIdent*)Node;
	return strcmp(vIdentNode->GetName(), Ident) == 0;
}
bool dspParser::p_MatchesKeyword(dspBaseNode *Node, const char *Keyword){
	if(!Node || Node->GetNodeType()!=ntKeyword) return false;
	dspNodeKeyword *vKeywordNode = (dspNodeKeyword*)Node;
	return strcmp(vKeywordNode->GetKeyword(), Keyword) == 0;
}
bool dspParser::p_MatchesOperator(dspBaseNode *Node, const char *Operator){
	if(!Node || Node->GetNodeType()!=ntOperator) return false;
	dspNodeOperator *vOpNode = (dspNodeOperator*)Node;
	return strcmp(vOpNode->GetOperator(), Operator) == 0;
}
bool dspParser::p_MatchesOperatorList(dspBaseNode *Node, const char **Ops, int Count){
	if(!Node || Node->GetNodeType()!=ntOperator) return false;
	dspNodeOperator *vOpNode = (dspNodeOperator*)Node;
	for(int i=0; i<Count; i++){
		if(strcmp(vOpNode->GetOperator(), Ops[i]) == 0) return true;
	}
	return false;
}
void dspParser::p_HasToBeOperator(const char *Operator){
	dspBaseNode *vCheckNode = p_PeekNode(0);
	if(vCheckNode){
		if(vCheckNode->GetNodeType()!=ntOperator || strcmp(Operator, ((dspNodeOperator*)vCheckNode)->GetOperator())!=0){
			p_ErrorUnexpNode(Operator, vCheckNode); DSTHROW(dseInvalidSyntax);
		}else{
			p_DropNode();
		}
	}else{
		p_ErrorUnexpEOF(); DSTHROW(dseInvalidSyntax);
	}
}
void dspParser::p_HasToBeKeyword(const char *Keyword){
	dspBaseNode *vCheckNode = p_PeekNode(0);
	if(vCheckNode){
		if(vCheckNode->GetNodeType()!=ntKeyword || strcmp(Keyword, ((dspNodeKeyword*)vCheckNode)->GetKeyword())!=0){
			p_ErrorUnexpNode(Keyword, vCheckNode); DSTHROW(dseInvalidSyntax);
		}else{
			p_DropNode();
		}
	}else{
		p_ErrorUnexpEOF(); DSTHROW(dseInvalidSyntax);
	}
}
void dspParser::p_HasToBeBlockEnd(const char *Keyword){
	dspBaseNode *vCheckNode = p_PeekNode(0);
	if(vCheckNode){
		if(vCheckNode->GetNodeType()!=ntKeyword || strcmp(Keyword, ((dspNodeKeyword*)vCheckNode)->GetKeyword())!=0){
			p_ErrorUnexpNode(Keyword, vCheckNode); DSTHROW(dseInvalidSyntax);
		}else{
			p_DropNode();
			p_HasToBeOperator("NL");
		}
	}else{
		p_ErrorUnexpEOF(); DSTHROW(dseInvalidSyntax);
	}
}
void dspParser::p_DropEmptyLines(){
	dspBaseNode *vPeekNode;
	
	while( ( vPeekNode = p_PeekNode(0) ) ){
		if( ! p_MatchesOperator( vPeekNode, "NL" ) ){
			break;
		}
		p_DropNode();
	}
}
// error functions
void dspParser::p_ErrorUnexpEOF(){
	p_Engine->GetEngineManager()->OutputError("Unexpected end of file reached",
		dsEngine::peUnexpectedEOF, p_Scanner->GetScript(), p_Scanner->GetLineNum(), p_Scanner->GetCharNum());
	p_ErrCount++;
}
void dspParser::p_ErrorUnexpNode(const char *Expected, dspBaseNode *Found){
//	if(Found->GetLineNum() == p_LastErrLine) return;
	sprintf(p_MsgBuf, "Unexpected token '%s' found (Expected '%s')", Found->GetTerminalString(), Expected);
	p_Engine->GetEngineManager()->OutputError(p_MsgBuf, dsEngine::peUnexpectedToken, Found->GetSource(), Found->GetLineNum(), Found->GetCharNum());
	p_ErrCount++; //p_LastErrLine = Found->GetLineNum();
}
void dspParser::p_ErrorInvName(const char *ErrName, dspBaseNode *Node, int ErrID){
	sprintf(p_MsgBuf, "Invalid %s name '%s' found", ErrName, Node->GetTerminalString());
	p_Engine->GetEngineManager()->OutputError(p_MsgBuf, ErrID, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ErrCount++;
}
void dspParser::p_ErrorInvOperator(const char *ErrName, dspBaseNode *Node, int ErrID){
	sprintf(p_MsgBuf, "Invalid %s '%s' found", ErrName, Node->GetTerminalString());
	p_Engine->GetEngineManager()->OutputError(p_MsgBuf, ErrID, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ErrCount++;
}
void dspParser::p_ErrorInvTypeMod(dspBaseNode *Node){
	sprintf(p_MsgBuf, "Invalid type modifier '%s' specified", Node->GetTerminalString());
	p_Engine->GetEngineManager()->OutputError(p_MsgBuf, dsEngine::peInvalidTypeModifier, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ErrCount++;
}
void dspParser::p_ErrorIncompTypeMod(dspBaseNode *Node, const char *TypeMod){
	sprintf(p_MsgBuf, "Type modifier '%s' cannot be used together with '%s'", Node->GetTerminalString(), TypeMod);
	p_Engine->GetEngineManager()->OutputError(p_MsgBuf, dsEngine::peIncompatibleTypeModifier, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ErrCount++;
}
void dspParser::p_ErrorNonEmptyIFFunc(dspBaseNode *Node){
	p_Engine->GetEngineManager()->OutputError("Interface functions cannot have a body", dsEngine::peNonEmptyIFFunc, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ErrCount++;
}

void dspParser::p_ErrorUnexpectedToken( dspBaseNode *node ){
	sprintf( p_MsgBuf, "Unexpected token '%s' found", node->GetTerminalString() );
	p_Engine->GetEngineManager()->OutputError( p_MsgBuf, dsEngine::peInvalidToken, node->GetSource(), node->GetLineNum(), node->GetCharNum() );
	p_ErrCount++;
}

// warning functions
void dspParser::p_WarnDupTypeMod(dspBaseNode *Node){
	sprintf(p_MsgBuf, "Duplicate type modifier '%s' specified", Node->GetTerminalString());
	p_Engine->GetEngineManager()->OutputWarning(p_MsgBuf, dsEngine::pwDuplicateTypeModifier, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_WarnCount++;
}
// parsing functions
dspNodeScript *dspParser::p_ParseScript(dsScriptSource *Script){
	dspNodeScript *vNewScript = NULL;
	dspBaseNode *vNewNode = NULL;
	dspTypeModifiers *vTypeMods=NULL;
	try{
		if(!(vNewScript = new dspNodeScript(Script))) DSTHROW(dueOutOfMemory);
		while(true){
			p_DropEmptyLines();
			if( ! p_PeekNode( 0 ) ){
				break;
			}
			try{
				if( (vNewNode = p_ParseRequires()) ||
					(vNewNode = p_ParseNamespace()) ||
					(vNewNode = p_ParsePin()) )
				{
					vNewScript->AddNode(vNewNode); vNewNode=NULL;
				}else{
					vTypeMods = p_ParseTypeModifiers();
					if( ( vNewNode = p_ParseAnyClass(vTypeMods, true) ) ){
						delete vTypeMods; vTypeMods = NULL;
						vNewScript->AddNode(vNewNode); vNewNode=NULL;
					}else{
						p_ErrorUnexpectedToken( p_PeekNode( 0 ) );
						p_DropNode();
						DSTHROW( dseInvalidSyntax );
					}
				}
			}catch( ... ){
				delete vTypeMods; vTypeMods = NULL;
				throw;
			}
		};
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vNewScript) delete vNewScript;
		throw;
	}
	return vNewScript;
}
dspNodeIdent *dspParser::p_ParseName(const char *ErrName, int ErrID){
	dspBaseNode *vIdentNode=NULL;
	try{
		if(!(vIdentNode = p_PopNode())){
			p_ErrorUnexpEOF(); DSTHROW(dseInvalidSyntax);
		}
		if(!p_MatchesType(vIdentNode, ntIdent)){
			p_ErrorInvName(ErrName, vIdentNode, ErrID); DSTHROW(dseInvalidSyntax);
		}
	}catch( ... ){
		if(vIdentNode) delete vIdentNode;
		throw;
	}
	return (dspNodeIdent*)vIdentNode;
}
dspNodeString *dspParser::p_ParseString(const char *ErrName, int ErrID){
	dspBaseNode *vStrNode=NULL;
	try{
		if(!(vStrNode = p_PopNode())){
			p_ErrorUnexpEOF(); DSTHROW(dseInvalidSyntax);
		}
		if(!p_MatchesType(vStrNode, ntString)){
			p_ErrorInvName(ErrName, vStrNode, ErrID); DSTHROW(dseInvalidSyntax);
		}
	}catch( ... ){
		if(vStrNode) delete vStrNode;
		throw;
	}
	return (dspNodeString*)vStrNode;
}
dspBaseNode *dspParser::p_ParseRequires(){
	// 'requires' string
	dspBaseNode *vKeyNode=NULL;
	dspNodeString *vStrNode=NULL;
	dspBaseNode *vNewNode=NULL;
	int type = 0;
	if(p_MatchesKeyword(p_PeekNode(0), "requires")) type = 1;
	if(type == 0) return NULL;
	try{
		vKeyNode = p_PopNode();
		vStrNode = p_ParseString("package", dsEngine::peInvalidPackageName);
		if(type == 1){
			if(!(vNewNode = new dspNodeRequires(vKeyNode, vStrNode->GetString()))) DSTHROW(dueOutOfMemory);
		}else{
			if(!(vNewNode = new dspNodeIncludes(vKeyNode, vStrNode->GetString()))) DSTHROW(dueOutOfMemory);
		}
		delete vKeyNode; delete vStrNode; vKeyNode = vStrNode = NULL;
		p_HasToBeOperator("NL");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vStrNode) delete vStrNode;
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vStrNode) delete vStrNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeNamespace *dspParser::p_ParseNamespace(){
	// 'namespace' [ type ]
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL;
	dspNodeNamespace *vNodeNS=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "namespace")) return NULL;
	try{
		vKeyNode = p_PopNode();
		if(!p_MatchesOperator(p_PeekNode(0), "NL")){
			vTypeNode = p_ParseType();
		}
		if(!(vNodeNS = new dspNodeNamespace(vKeyNode, vTypeNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vTypeNode = NULL;
		p_HasToBeOperator("NL");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vNodeNS) delete vNodeNS;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vNodeNS) delete vNodeNS;
		throw;
	}
	return vNodeNS;
}
dspNodePin *dspParser::p_ParsePin(){
	// 'pin' type
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL;
	dspNodePin *vNodePin=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "pin")) return NULL;
	try{
		vKeyNode = p_PopNode();
		vTypeNode = p_ParseType();
		if(!(vNodePin = new dspNodePin(vKeyNode, vTypeNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vTypeNode = NULL;
		p_HasToBeOperator("NL");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vNodePin) delete vNodePin;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vNodePin) delete vNodePin;
		throw;
	}
	return vNodePin;
}
dspNodeClass *dspParser::p_ParseAnyClass(dspTypeModifiers *TypeMods, bool onlyPub){
	// [type-modifiers] [class | interface]
	dspNodeClass *vNewNode=NULL;
	try{
		if( ( vNewNode = p_ParseClass(TypeMods, onlyPub) )
		||  ( vNewNode = p_ParseInterface(TypeMods, onlyPub) )
		||  ( vNewNode = p_ParseEnumeration(TypeMods, onlyPub) )
		){
			return vNewNode;
		}
		return NULL;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeClass *dspParser::p_ParseClass(dspTypeModifiers *TypeMods, bool onlyPub){
	// [public | protected | private] [native] [abstract | fixed]
	// 'class' name ['extends' name] ['implements' class-list]
	//     class-body
	// 'end'
	dspBaseNode *vKeyNode=NULL, *vBaseNode=NULL, *vNewNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	dspNodeClass *vClsNode=NULL;
	dspTypeModifiers *vMembTypeMods=NULL;
	int vTypeMod;
	int allowed = DSTM_PUBLIC | DSTM_NATIVE | DSTM_ABSTRACT | DSTM_FIXED;
	if(!p_MatchesKeyword(p_PeekNode(0), "class")) return NULL;
	// check type modifiers
	if(TypeMods->IsTypeModSet(DSTM_ABSTRACT)){
		if(TypeMods->IsTypeModSet(DSTM_FIXED)){
			p_ErrorIncompTypeMod(TypeMods->GetTypeNode(DSTM_ABSTRACT), "fixed");
			TypeMods->UnsetTypeMod(DSTM_FIXED);
		}
	}
	if(onlyPub){
		p_CheckTypeMods(TypeMods, allowed);
	}else{
		p_CheckTypeMods(TypeMods, allowed | DSTM_PROTECTED | DSTM_PRIVATE);
	}
	vTypeMod = TypeMods->GetTypeModValue();
	if(!(vTypeMod & (DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE))) vTypeMod |= DSTM_PUBLIC;
	// header
	try{
		// class declaration
		vKeyNode = p_PopNode();
		vIdentNode = p_ParseName("class", dsEngine::peInvalidClassName);
		if(!(vClsNode = new dspNodeClass(vKeyNode, vIdentNode->GetName(), DSCT_CLASS, vTypeMod))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; delete vIdentNode; vKeyNode = vIdentNode = NULL;
		// optional base class (extends)
		if(p_MatchesKeyword(p_PeekNode(0), "extends")){
			p_DropNode(); vBaseNode = p_ParseType();
			vClsNode->SetBaseClass(vBaseNode); vBaseNode = NULL;
		}
		// optional implemented classes (implements)
		if(p_MatchesKeyword(p_PeekNode(0), "implements")){
			p_DropNode();
			while(true){
				vBaseNode = p_ParseType();
				vClsNode->AddImplClass(vBaseNode); vBaseNode = NULL;
				if(!p_MatchesOperator(p_PeekNode(0), ",")) break;
				p_DropNode();
			}
		}
		// end of line has to follow
		p_HasToBeOperator("NL");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vIdentNode) delete vIdentNode;
		if(vClsNode) delete vClsNode;
		if(vBaseNode) delete vBaseNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(")","NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vIdentNode) delete vIdentNode;
		if(vClsNode) delete vClsNode;
		if(vBaseNode) delete vBaseNode;
		throw;
	}
	// body
	try{
		while(true){
			p_DropEmptyLines();
			if(!p_PeekNode(0)){
				p_ErrorUnexpEOF();
				DSTHROW(dseInvalidSyntax);
			}
			if(p_MatchesKeyword(p_PeekNode(0), "end")) break;
			try{
				p_DropEmptyLines();

				vMembTypeMods = p_ParseTypeModifiers();
				if( (vNewNode = p_ParseAnyClass(vMembTypeMods, false) )
				|| (vNewNode = p_ParseClassMemberDef(vMembTypeMods)) )
				{
					delete vMembTypeMods; vMembTypeMods = NULL;
					vClsNode->AddMember(vNewNode); vNewNode = NULL;
				}else{
					p_DropNode(); DSTHROW(dseInvalidSyntax);
				}
				
			}catch( const duException &e ){
				if(vMembTypeMods) delete vMembTypeMods;
				if(e.IsNamed("InvalidSyntax")) p_DropNode(); else throw;
				
			}catch( ... ){
				if(vMembTypeMods) delete vMembTypeMods;
				throw;
			}
		}
		p_HasToBeBlockEnd("end");
	}catch( ... ){
		if(vClsNode) delete vClsNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vClsNode;
}
dspNodeClass *dspParser::p_ParseInterface(dspTypeModifiers *TypeMods, bool onlyPub){
	// [public | protected | private] 'interface' name ['implements' class-list]
	//     interface-body
	// 'end'
	dspBaseNode *vKeyNode=NULL, *vBaseNode=NULL, *vNewNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	dspNodeClass *vClsNode=NULL;
	dspTypeModifiers *vMembTypeMods=NULL;
	int vTypeMod;
	int allowed = DSTM_PUBLIC;
	if(!p_MatchesKeyword(p_PeekNode(0), "interface")) return NULL;
	// check type modifiers
	if(onlyPub){
		p_CheckTypeMods(TypeMods, allowed);
	}else{
		p_CheckTypeMods(TypeMods, allowed | DSTM_PROTECTED | DSTM_PRIVATE);
	}
	vTypeMod = TypeMods->GetTypeModValue();
	if(!(vTypeMod & (DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE))) vTypeMod |= DSTM_PUBLIC;
	// header
	try{
		// interface declaration
		vKeyNode = p_PopNode();
		vIdentNode = p_ParseName("interface", dsEngine::peInvalidInterfaceName);
		if(!(vClsNode = new dspNodeClass(vKeyNode, vIdentNode->GetName(), DSCT_INTERFACE, vTypeMod))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; delete vIdentNode; vKeyNode = vIdentNode = NULL;
		// optional implemented classes (implements)
		if(p_MatchesKeyword(p_PeekNode(0), "implements")){
			p_DropNode();
			while(true){
				vBaseNode = p_ParseType();
				vClsNode->AddImplClass(vBaseNode); vBaseNode = NULL;
				if(!p_MatchesOperator(p_PeekNode(0), ",")) break;
				p_DropNode();
			}
		}
		// end of line has to follow
		p_HasToBeOperator("NL");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vIdentNode) delete vIdentNode;
		if(vClsNode) delete vClsNode;
		if(vBaseNode) delete vBaseNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(")","NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vIdentNode) delete vIdentNode;
		if(vClsNode) delete vClsNode;
		if(vBaseNode) delete vBaseNode;
		throw;
	}
	// body
	try{
		while(!p_MatchesKeyword(p_PeekNode(0), "end")){
			if(!p_PeekNode(0)) DSTHROW(dseInvalidSyntax);
			try{
				p_DropEmptyLines();
				if(!p_PeekNode(0)) DSTHROW(dseInvalidSyntax);
				if(p_MatchesKeyword(p_PeekNode(0), "end")) break;
				
				vMembTypeMods = p_ParseTypeModifiers();
				if( (vNewNode = p_ParseEnumeration(vMembTypeMods, false)) ||
					(vNewNode = p_ParseInterfaceMemberDef(vMembTypeMods)) )
				{
					delete vMembTypeMods; vMembTypeMods = NULL;
					vClsNode->AddMember(vNewNode); vNewNode = NULL;
				}else{
					p_ErrorUnexpNode("member declaration", p_PeekNode(0));
					p_SkipToOperator("NL"); DSTHROW(dseInvalidSyntax);
				}
				
			}catch( const duException &e ){
				if(vMembTypeMods) delete vMembTypeMods;
				if(!e.IsNamed("InvalidSyntax")) throw;
				
			}catch( ... ){
				if(vMembTypeMods) delete vMembTypeMods;
				throw;
			}
		}
		p_HasToBeBlockEnd("end");
	}catch( ... ){
		if(vClsNode) delete vClsNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vClsNode;
}
dspTypeModifiers *dspParser::p_ParseTypeModifiers(){
	dspTypeModifiers *vTypeMods=NULL;
	dspBaseNode *vPeekNode;
	int vNewTypeMod;
	try{
		if(!(vTypeMods = new dspTypeModifiers)) DSTHROW(dueOutOfMemory);
		// gather type modifiers
		while( ( vPeekNode = p_PeekNode(0) ) ){
			// find matching type modifier
			if(p_MatchesKeyword(vPeekNode, "public")) vNewTypeMod = DSTM_PUBLIC;
			else if(p_MatchesKeyword(vPeekNode, "protected")) vNewTypeMod = DSTM_PROTECTED;
			else if(p_MatchesKeyword(vPeekNode, "private")) vNewTypeMod = DSTM_PRIVATE;
			else if(p_MatchesKeyword(vPeekNode, "abstract")) vNewTypeMod = DSTM_ABSTRACT;
			else if(p_MatchesKeyword(vPeekNode, "fixed")) vNewTypeMod = DSTM_FIXED;
			else if(p_MatchesKeyword(vPeekNode, "static")) vNewTypeMod = DSTM_STATIC;
			else break;
			// check for duplicate type modifier
			if(vTypeMods->IsTypeModSet(vNewTypeMod)){
				p_WarnDupTypeMod(vPeekNode);
				p_DropNode();
			}else{
				vTypeMods->SetTypeMod(vNewTypeMod, p_PopNode());
			}
		}
		// check for invalid access level combinations
		if(vTypeMods->IsTypeModSet(DSTM_PRIVATE)){
			if(vTypeMods->IsTypeModSet(DSTM_PROTECTED)){
				p_ErrorIncompTypeMod(vTypeMods->GetTypeNode(DSTM_PROTECTED), "private");
				vTypeMods->UnsetTypeMod(DSTM_PROTECTED);
			}
			if(vTypeMods->IsTypeModSet(DSTM_PUBLIC)){
				p_ErrorIncompTypeMod(vTypeMods->GetTypeNode(DSTM_PUBLIC), "private");
				vTypeMods->UnsetTypeMod(DSTM_PUBLIC);
			}
		}else if(vTypeMods->IsTypeModSet(DSTM_PROTECTED)){
			if(vTypeMods->IsTypeModSet(DSTM_PUBLIC)){
				p_ErrorIncompTypeMod(vTypeMods->GetTypeNode(DSTM_PUBLIC), "protected");
				vTypeMods->UnsetTypeMod(DSTM_PUBLIC);
			}
		}
	}catch( ... ){
		if(vTypeMods) delete vTypeMods;
		throw;
	}
	return vTypeMods;
}
dspBaseNode *dspParser::p_ParseClassMemberDef(dspTypeModifiers *TypeMods){
	dspBaseNode *vNewNode=NULL;
	try{
		if( (vNewNode = p_ParseClassFunction(TypeMods)) ||
			(vNewNode = p_ParseClassVariableList(TypeMods)) )
		{
			return vNewNode;
		}
		p_ErrorUnexpNode("member declaration", p_PeekNode(0));
		p_DropNode(); DSTHROW(dseInvalidSyntax);
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeClassFunction *dspParser::p_ParseClassFunction(dspTypeModifiers *TypeMods){
	// 'func' type ident '(' function-arguments ')'
	//     statement-block
	// 'end'
	// 'func' 'new' '(' function-arguments ')' [{'super'|'this'} '(' func-args ')']
	//     statement-block
	// 'end'
	// 'func' 'destructor' '(' function-arguments ')'
	//     statement-block
	// 'end'
	// 'func' type op '(' function-arguments ')'
	//     statement-block
	// 'end'
	// 'abstract' 'func' type ident '(' function-arguments ')'
	const char *vOps[] = {"++","--","+","-","!","~","*","/","%","<<",">>",
		"&","|","^","<",">","<=",">=","*=","/=","%=","+=","-=","<<=",">>=",
		"&=","|=","^="}; // not allowed are: =, ==, !=
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL, *vStaNode=NULL;
	dspBaseNode *vSuperNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	dspNodeOperator *vOpNode=NULL;
	dspNodeClassFunction *vNewNode=NULL;
	dspNodeFuncArg *vArgNode=NULL;
	dspNodeInitConstrCall *vInitConstr=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	int vTypeMod, vFuncType;
	int allowed = DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE | DSTM_NATIVE;
	const char *vName;
	// determine function type
	if(!p_MatchesKeyword(vPeekNode, "func")) return NULL;
	// parse function
	try{
		vKeyNode = p_PopNode();
		// check for constructor
		if(p_MatchesIdent(p_PeekNode(0), DSFUNC_CONSTRUCTOR)){
			vFuncType = DSFT_CONSTRUCTOR;
			vIdentNode = (dspNodeIdent*)p_PopNode();
			vName = vIdentNode->GetName();
		// check for destructor
		}else if(p_MatchesIdent(p_PeekNode(0), DSFUNC_DESTRUCTOR)){
			vFuncType = DSFT_DESTRUCTOR;
			vIdentNode = (dspNodeIdent*)p_PopNode();
			vName = vIdentNode->GetName();
		// check for operator
		}else{
			vTypeNode = p_ParseType();
			// check if an operator token follows
			if(p_MatchesOperatorList(p_PeekNode(0), &vOps[0], 28)){
				vFuncType = DSFT_OPERATOR;
				vOpNode = (dspNodeOperator*)p_PopNode();
				vName = vOpNode->GetOperator();
			// this has to be a normal function
			}else{
				vFuncType = DSFT_FUNCTION;
				vIdentNode = p_ParseName("function", dsEngine::peInvalidFuncName);
				vName = vIdentNode->GetName();
			}
		}
		// check type modifiers again
		if(vFuncType != DSFT_FUNCTION){
			p_CheckTypeMods(TypeMods, allowed);
		}else{
			p_CheckTypeMods(TypeMods, allowed | DSTM_FIXED | DSTM_ABSTRACT | DSTM_STATIC);
		}
		vTypeMod = TypeMods->GetTypeModValue();
		if(!(vTypeMod & (DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE))) vTypeMod |= DSTM_PUBLIC;
		// init the rest
		if(!(vNewNode = new dspNodeClassFunction(vKeyNode, vName, vFuncType, vTypeMod, vTypeNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vTypeNode = NULL;
		if(vIdentNode){ delete vIdentNode; vIdentNode = NULL; }
		if(vOpNode){ delete vOpNode; vOpNode = NULL; }
		p_HasToBeOperator("(");
		if(!p_MatchesOperator(p_PeekNode(0), ")")){
			while(true){
				vArgNode = p_ParseFuncArg();
				vNewNode->AddArgument(vArgNode); vArgNode = NULL;
				if(p_MatchesOperator(p_PeekNode(0), ",")) p_DropNode(); else break;
			}
		}
		p_HasToBeOperator(")");
		if(vFuncType == DSFT_CONSTRUCTOR){
			if(p_MatchesType(p_PeekNode(0), ntSuper) || p_MatchesType(p_PeekNode(0), ntThis)){
				vSuperNode = p_PopNode();
				if(!(vInitConstr = new dspNodeInitConstrCall(vSuperNode))) DSTHROW(dueOutOfMemory);
				delete vSuperNode; vSuperNode = NULL;
				p_ParseFuncArgs(vInitConstr->GetArgumentList());
				vNewNode->SetInitConstrCall(vInitConstr); vInitConstr=NULL;
			}
		}
		p_HasToBeOperator("NL");
		// add check for 'native' keyword
		// parse statement only if nott abstract
		if(!(vTypeMod & DSTM_ABSTRACT)){
			vStaNode = p_ParseGroupedSta("end");
			vNewNode->SetStatement(vStaNode); vStaNode = NULL;
			p_HasToBeBlockEnd("end");
		}
		
	}catch( const duException &e ){
		if(vSuperNode) delete vSuperNode;
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vIdentNode) delete vIdentNode;
		if(vArgNode) delete vArgNode;
		if(vStaNode) delete vStaNode;
		if(vOpNode) delete vOpNode;
		if(vInitConstr) delete vInitConstr;
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vSuperNode) delete vSuperNode;
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vIdentNode) delete vIdentNode;
		if(vArgNode) delete vArgNode;
		if(vStaNode) delete vStaNode;
		if(vOpNode) delete vOpNode;
		if(vInitConstr) delete vInitConstr;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeClassVariableList *dspParser::p_ParseClassVariableList(dspTypeModifiers *TypeMods){
	// [pbulic | protected | private] [static] [fixed]
	//    'var' type cls-var-def [',' ... ]
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL;
	dspNodeClassVariable *vVarNode=NULL;
	dspNodeClassVariableList *vNewNode=NULL;
	int vTypeMod;
	int allowed = DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE | DSTM_STATIC | DSTM_FIXED;
	if(!p_MatchesKeyword(p_PeekNode(0), "var")) return NULL;
	// check type modifiers
	p_CheckTypeMods(TypeMods, allowed);
	vTypeMod = TypeMods->GetTypeModValue();
	if(!(vTypeMod & (DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE))) vTypeMod |= DSTM_PRIVATE;
	if(vTypeMod & DSTM_FIXED) vTypeMod |= DSTM_STATIC;
	// parse variable
	try{
		vKeyNode = p_PopNode(); vTypeNode = p_ParseType();
		if(!(vNewNode = new dspNodeClassVariableList(vKeyNode, vTypeMod, vTypeNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vTypeNode = NULL;
		while(true){
			try{
				vVarNode = p_ParseClassVariable();
				vNewNode->AddVariable(vVarNode); vVarNode=NULL;
				
			}catch( const duException &e ){
				if(vVarNode) delete vVarNode;
				if(!e.IsNamed("InvalidSyntax")) throw;
				
			}catch( ... ){
				if(vVarNode) delete vVarNode;
				throw;
			}
			if(p_MatchesOperator(p_PeekNode(0), ",")) p_DropNode();
			else break;
		}
		p_HasToBeOperator("NL");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vVarNode) delete vVarNode;
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vVarNode) delete vVarNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeClassVariable *dspParser::p_ParseClassVariable(){
	// ident ['=' expression]
	dspBaseNode *vInitNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	dspNodeClassVariable *vNewNode=NULL;
	try{
		vIdentNode = p_ParseName("variable", dsEngine::peInvalidVarName);
		if(p_MatchesOperator(p_PeekNode(0), "=")){
			p_DropNode(); vInitNode = p_ParseExpr();
		}
		if(!(vNewNode = new dspNodeClassVariable(vIdentNode, vInitNode))) DSTHROW(dueOutOfMemory);
		delete vIdentNode; vIdentNode = NULL; vInitNode = NULL;
		
	}catch( const duException &e ){
		if(vIdentNode) delete vIdentNode;
		if(vInitNode) delete vInitNode;
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vIdentNode) delete vIdentNode;
		if(vInitNode) delete vInitNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseInterfaceMemberDef(dspTypeModifiers *TypeMods){
	dspBaseNode *vNewNode=NULL;
	try{
		if(!(vNewNode = p_ParseInterfaceFuncDef(TypeMods))){
			p_ErrorUnexpNode("member declaration", p_PeekNode(0));
			p_DropNode(); DSTHROW(dseInvalidSyntax);
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeClassFunction *dspParser::p_ParseInterfaceFuncDef(dspTypeModifiers *TypeMods){
	// 'func' type ident '(' function-arguments ')'
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL, *vStaNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	dspNodeClassFunction *vNewNode=NULL;
	dspNodeFuncArg *vArgNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	int vTypeMod;
	const char *vName;
	// determine function type
	if(!p_MatchesKeyword(vPeekNode, "func")) return NULL;
	// check type modifiers
	p_CheckTypeMods(TypeMods, DSTM_PUBLIC | DSTM_ABSTRACT);
	vTypeMod = TypeMods->GetTypeModValue();
	if(!(vTypeMod & DSTM_PUBLIC)) vTypeMod |= DSTM_PUBLIC;
	if(!(vTypeMod & DSTM_ABSTRACT)) vTypeMod |= DSTM_ABSTRACT;
	// parse function declaration
	try{
		vKeyNode = p_PopNode();
		vTypeNode = p_ParseType();
		vIdentNode = p_ParseName("function", dsEngine::peInvalidFuncName);
		vName = vIdentNode->GetName();
		if(!(vNewNode = new dspNodeClassFunction(vKeyNode, vName, DSFT_FUNCTION, vTypeMod, vTypeNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vTypeNode = NULL;
		delete vIdentNode; vIdentNode = NULL;
		p_HasToBeOperator("(");
		if(!p_MatchesOperator(p_PeekNode(0), ")")){
			while(true){
				vArgNode = p_ParseFuncArg();
				vNewNode->AddArgument(vArgNode); vArgNode = NULL;
				if(p_MatchesOperator(p_PeekNode(0), ",")) p_DropNode(); else break;
			}
		}
		p_HasToBeOperator(")");
		if(p_MatchesOperator(p_PeekNode(0), "{")){
			vStaNode = p_ParseSta();
			p_ErrorNonEmptyIFFunc(vStaNode);
			delete vStaNode; vStaNode = NULL; DSTHROW(dseInvalidSyntax);
		}
		p_HasToBeOperator("NL");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vIdentNode) delete vIdentNode;
		if(vArgNode) delete vArgNode;
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(")","NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vIdentNode) delete vIdentNode;
		if(vArgNode) delete vArgNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeFuncArg *dspParser::p_ParseFuncArg(){
	// type ident ['=' expression]
	dspBaseNode *vTypeNode=NULL, *vDefNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	dspNodeFuncArg *vNewNode=NULL;
	try{
		vTypeNode = p_ParseType();
		vIdentNode = p_ParseName("function argument", dsEngine::peInvalidFuncArgName);
//		if(p_MatchesOperator(p_PeekNode(0), "=")){
//			p_DropNode(); vDefNode = p_ParseExpr();
//		}
		if(!(vNewNode = new dspNodeFuncArg(vTypeNode, vIdentNode->GetName(), vTypeNode, vDefNode))) DSTHROW(dueOutOfMemory);
		delete vIdentNode; vIdentNode = NULL; vTypeNode = vDefNode = NULL;
	}catch( ... ){
		if(vTypeNode) delete vTypeNode;
		if(vIdentNode) delete vIdentNode;
		if(vDefNode) delete vDefNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspNodeEnumeration *dspParser::p_ParseEnumeration(dspTypeModifiers *TypeMods, bool onlyPub){
	// 'enum' ident
	//     ident
	//     [ ... ]
	// 'end'
	dspNodeEnumeration *vNodeEnum=NULL;
	dspNodeEnumValue *nodeEntry = NULL;
	dspNodeIdent *vNodeIdent=NULL;
	dspBaseNode *vKeyNode=NULL;
	dspNodeClassVariableList *nodeVarList = NULL;
	dspNodeClassVariable *nodeVar = NULL;
	int nextOrder = 0;
	
	if(!p_MatchesKeyword(p_PeekNode(0), "enum")) return NULL;
	// check typemods
	int vTypeMod;
	if(onlyPub){
		p_CheckTypeMods(TypeMods, DSTM_PUBLIC);
	}else{
		p_CheckTypeMods(TypeMods, DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE);
	}
	vTypeMod = TypeMods->GetTypeModValue();
	if( ! ( vTypeMod & ( DSTM_PUBLIC | DSTM_PROTECTED | DSTM_PRIVATE ) ) ){
		vTypeMod |= DSTM_PUBLIC;
	}
	
	// go on
	try{
		vKeyNode = p_PopNode();
		vNodeIdent = p_ParseName("enumeration", dsEngine::peInvalidEnumName);
		if(!(vNodeEnum = new dspNodeEnumeration(vKeyNode, vNodeIdent->GetName(), vTypeMod))) DSTHROW(dueOutOfMemory);
		vNodeEnum->SetBaseClass( new dspNodeIdent(vKeyNode->GetSource(), vKeyNode->GetLineNum(), vKeyNode->GetCharNum(), "Enumeration" ) );
		if(!(nodeVarList = new dspNodeClassVariableList(vKeyNode, DSTM_STATIC | DSTM_PUBLIC | DSTM_FIXED, vNodeIdent))) DSTHROW(dueOutOfMemory);
		vNodeIdent = NULL;
		delete vKeyNode; vKeyNode = NULL;
		p_HasToBeOperator("NL");
		while(!p_MatchesKeyword(p_PeekNode(0), "end")){
			if(!p_PeekNode(0)) DSTHROW(dseInvalidSyntax);
			try{
				p_DropEmptyLines();
				vNodeIdent = p_ParseName("enumeration item", dsEngine::peInvalidEnumEntryName);
				nodeEntry = new dspNodeEnumValue( vNodeIdent, vNodeIdent->GetName(), nextOrder++ );
				nodeVar = new dspNodeClassVariable( vNodeIdent, nodeEntry );
				nodeVarList->AddVariable( nodeVar );
				nodeVar = NULL;
				nodeEntry = NULL;
				delete vNodeIdent; vNodeIdent = NULL;
				if(p_MatchesOperator(p_PeekNode(0), "NL")) p_DropNode();
			}catch( const duException &e ){
				if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
				else throw;
			}
		}
		p_HasToBeBlockEnd("end");
		vNodeEnum->AddMember( nodeVarList );
	}catch( ... ){
		if(vNodeIdent) delete vNodeIdent;
		if(vNodeEnum) delete vNodeEnum;
		if(nodeVarList) delete nodeVarList;
		if(nodeEntry) delete nodeEntry;
		if(nodeVar) delete nodeVar;
		throw;
	}
	return vNodeEnum;
}
dspBaseNode *dspParser::p_ParseSta(){
	// <empty> | expression | ...
	dspBaseNode *vNewNode=NULL;
	dspBaseNode *vNodeUnOp;
	try{
		if( ( vNewNode = p_ParseIfElse() ) ) return vNewNode;
		if( ( vNewNode = p_ParseSelect() ) ) return vNewNode;
		if( ( vNewNode = p_ParseWhile() ) ) return vNewNode;
		if( ( vNewNode = p_ParseFor() ) ) return vNewNode;
		if( ( vNewNode = p_ParseBreak() ) ) return vNewNode;
		if( ( vNewNode = p_ParseContinue() ) ) return vNewNode;
		if( ( vNewNode = p_ParseThrow() ) ) return vNewNode;
		if( ( vNewNode = p_ParseVarDefList() ) ) return vNewNode;
		if( ( vNewNode = p_ParseRetVal() ) ) return vNewNode;
		if( ( vNewNode = p_ParseTry() ) ) return vNewNode;
		// parse expression
		vNewNode = p_ParseExpr();
		p_HasToBeOperator("NL");
		// simplify post inc/dec if basic expr
		if(vNewNode->GetNodeType() == ntPostInc){
			vNodeUnOp = ((dspNodePostIncrement*)vNewNode)->Reduce();
			delete vNewNode; vNewNode = vNodeUnOp;
		}else if(vNewNode->GetNodeType() == ntPostDec){
			vNodeUnOp = ((dspNodePostDecrement*)vNewNode)->Reduce();
			delete vNewNode; vNewNode = vNodeUnOp;
		}
		
	}catch( const duException &e ){
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseGroupedSta(const char *end, const char *enda1, const char *enda2){
	// [statement [...] ]
	if(!end) DSTHROW(dueInvalidParam);
	dspNodeStaList *vStaList=NULL;
	dspBaseNode *vNewNode=NULL, *vPeekNode;
	try{
		if(!(vStaList = new dspNodeStaList(p_PeekNode(0)))) DSTHROW(dueOutOfMemory);
		while(true){
			if(!(vPeekNode = p_PeekNode(0))){
				p_ErrorUnexpEOF(); return vStaList;
			}
			p_DropEmptyLines();
			vPeekNode = p_PeekNode(0);
			if(p_MatchesKeyword(vPeekNode, end)) break;
			if(enda1 && p_MatchesKeyword(vPeekNode, enda1)) break;
			if(enda2 && p_MatchesKeyword(vPeekNode, enda2)) break;
			try{
				vNewNode = p_ParseSta();
			}catch( const duException &e ){
				if(e.IsNamed("InvalidSyntax")) continue; else throw;
			}
			vStaList->AddNode(vNewNode); vNewNode = NULL;
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vStaList) delete vStaList;
		throw;
	}
	return vStaList;
}
dspBaseNode *dspParser::p_ParseIfElse(bool isElif){
	// 'if' expression
	//     statement-list
	// [elif expression
	//     statement-list ]
	// [else
	//     statement-list ]
	// 'end'
	dspBaseNode *vKeyNode=NULL, *vCondNode=NULL;
	dspBaseNode *vIfNode=NULL, *vElseNode=NULL;
	dspBaseNode *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), isElif ? "elif" : "if")) return NULL;
	try{
		vKeyNode = p_PopNode();
		vCondNode = p_ParseExpr();
		p_HasToBeOperator("NL");
		vIfNode = p_ParseGroupedSta("end", "elif", "else");
		if(p_MatchesKeyword(p_PeekNode(0), "elif")){
			vElseNode = p_ParseIfElse(true);
		}else if(p_MatchesKeyword(p_PeekNode(0), "else")){
			p_DropNode(); 
			vElseNode = p_ParseGroupedSta("end");
		}
		if(!isElif) p_HasToBeBlockEnd("end");
		if(!(vNewNode = new dspNodeIfElse(vKeyNode, vCondNode, vIfNode, vElseNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vCondNode = vIfNode = vElseNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vCondNode) delete vCondNode;
		if(vIfNode) delete vIfNode;
		if(vElseNode) delete vElseNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseSelect(){
	// 'select' expression
	// <case-blocks>
	// ['else'
	//     statement]
	// 'end'
	dspBaseNode *vKeyNode=NULL, *vNodeCond=NULL, *vNodeSta=NULL;
	dspNodeCase *vNodeCase=NULL;
	dspNodeSelect *vNodeSelect=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "select")) return NULL;
	try{
		vKeyNode = p_PopNode();
		vNodeCond = p_ParseExpr();
		if(!(vNodeSelect = new dspNodeSelect(vKeyNode, vNodeCond))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vNodeCond = NULL;
		p_HasToBeOperator("NL");
		while(true){
			if(!p_PeekNode(0)) DSTHROW(dseInvalidSyntax);
			if(p_MatchesKeyword(p_PeekNode(0), "end")) break;
			if(p_MatchesOperator(p_PeekNode(0), "NL")){
				p_DropNode();
				continue;
			}
			try{
				if( ( vNodeCase = p_ParseCase() ) ){
					vNodeSelect->AddCase(vNodeCase); vNodeCase = NULL;
				}else break;
				
			}catch( const duException &e ){
				if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreakKW("NL","end");
				else throw;
			}
		}
		if(p_MatchesKeyword(p_PeekNode(0), "else")){
			p_DropNode();
			vNodeSta = p_ParseGroupedSta("end");
			vNodeSelect->SetCaseElse(vNodeSta); vNodeSta = NULL;
		}
		p_HasToBeBlockEnd("end");
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vNodeSta) delete vNodeSta;
		if(vNodeCond) delete vNodeCond;
		if(vNodeCase) delete vNodeCase;
		if(vNodeSelect) delete vNodeSelect;
		throw;
	}
	return vNodeSelect;
}
dspNodeCase *dspParser::p_ParseCase(){
	// 'case' constant [',' constant ...]
	//     statement-block
	dspBaseNode *vKeyNode=NULL, *vNodeSta=NULL;
	dspNodeCase *vNodeCase=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "case")) return NULL;
	try{
		vKeyNode = p_PopNode();
		if(!(vNodeCase = new dspNodeCase(vKeyNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = NULL;
		while(true){
			if(!p_PeekNode(0)) DSTHROW(dseInvalidSyntax);
			try{
				vNodeSta = p_ParseExpr();
				vNodeCase->AddValue(vNodeSta); vNodeSta = NULL;
				if(p_MatchesOperator(p_PeekNode(0), ",")) p_DropNode(); else break;
				
			}catch( const duException &e ){
				if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(",","NL");
				else throw;
			}
		}
		p_HasToBeOperator("NL");
		vNodeSta = p_ParseGroupedSta("end", "case", "else");
		vNodeCase->SetStatement(vNodeSta); vNodeSta = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vNodeSta) delete vNodeSta;
		if(vNodeCase) delete vNodeCase;
		throw;
	}
	return vNodeCase;
}
dspBaseNode *dspParser::p_ParseWhile(){
	// 'while' expression
	//     statement-block
	// 'end'
	dspBaseNode *vKeyNode=NULL, *vCondNode=NULL, *vStaNode=NULL, *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "while")) return NULL;
	try{
		vKeyNode = p_PopNode();
		vCondNode = p_ParseExpr();
		p_HasToBeOperator("NL");
		vStaNode = p_ParseGroupedSta("end");
		p_HasToBeBlockEnd("end");
		if(!(vNewNode = new dspNodeWhile(vKeyNode, vCondNode, vStaNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vCondNode = vStaNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vCondNode) delete vCondNode;
		if(vStaNode) delete vStaNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseFor(){
	// 'for' object '=' expression 'upto' expression ['step' expression]
	//     statement-block
	// 'end'
	// 'for' object '=' expression 'downto' expression ['step' expression]
	//     statement-block
	// 'end'
	dspBaseNode *vKeyNode=NULL, *vVarNode=NULL, *vFromNode=NULL;
	dspBaseNode *vToNode=NULL, *vStepNode=NULL, *vStaNode=NULL;
	dspBaseNode *vNewNode=NULL, *vPeekNode;
	bool vForType;
	if(!p_MatchesKeyword(p_PeekNode(0), "for")) return NULL;
	try{
		vKeyNode = p_PopNode();
		vVarNode = p_ParseObject();
		p_HasToBeOperator("=");
		vFromNode = p_ParseExpr();
		if(!(vPeekNode = p_PeekNode(0))){
			p_ErrorUnexpEOF(); DSTHROW(dseInvalidSyntax);
		}
		if(p_MatchesKeyword(vPeekNode, "to")){ vForType = true;
		}else if(p_MatchesKeyword(vPeekNode, "downto")){ vForType = false;
		}else{
			p_ErrorUnexpNode("(down)to", vPeekNode); DSTHROW(dseInvalidSyntax);
		}
		p_DropNode();
		vToNode = p_ParseExpr();
		if(p_MatchesKeyword(p_PeekNode(0), "step")){
			p_DropNode(); vStepNode = p_ParseExpr();
		}
		p_HasToBeOperator("NL");
		vStaNode = p_ParseGroupedSta("end");
		p_HasToBeBlockEnd("end");
		if(vForType){
			if(!(vNewNode = new dspNodeForUp(vKeyNode, vVarNode, vFromNode, vToNode, vStepNode, vStaNode))) DSTHROW(dueOutOfMemory);
		}else{
			if(!(vNewNode = new dspNodeForDown(vKeyNode, vVarNode, vFromNode, vToNode, vStepNode, vStaNode))) DSTHROW(dueOutOfMemory);
		}
		delete vKeyNode; vKeyNode = vVarNode = vFromNode = vToNode = vStepNode = vStaNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vVarNode) delete vVarNode;
		if(vFromNode) delete vFromNode;
		if(vToNode) delete vToNode;
		if(vStepNode) delete vStepNode;
		if(vStaNode) delete vStaNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseBreak(){
	// 'break'
	dspBaseNode *vKeyNode=NULL, *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "break")) return NULL;
	try{
		vKeyNode = p_PopNode(); p_HasToBeOperator("NL");
		if(!(vNewNode = new dspNodeBreak(vKeyNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseContinue(){
	// 'continue'
	dspBaseNode *vKeyNode=NULL, *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "continue")) return NULL;
	try{
		vKeyNode = p_PopNode(); p_HasToBeOperator("NL");
		if(!(vNewNode = new dspNodeContinue(vKeyNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseThrow(){
	// 'throw' [ expression ]
	dspBaseNode *vKeyNode=NULL, *vExprNode=NULL, *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "throw")) return NULL;
	try{
		vKeyNode = p_PopNode();
		if(!p_MatchesOperator(p_PeekNode(0), "NL")){
			vExprNode = p_ParseExpr();
		}
		p_HasToBeOperator("NL");
		if(!(vNewNode = new dspNodeThrow(vKeyNode, vExprNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vExprNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vExprNode) delete vExprNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseVarDefList(){
	// 'var' type var-def [',' ... ]
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL, *vVarNode=NULL;
	dspNodeVarDefList *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "var")) return NULL;
	try{
		vKeyNode = p_PopNode();
		vTypeNode = p_ParseType();
		if(!(vNewNode = new dspNodeVarDefList(vKeyNode, vTypeNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vTypeNode = NULL;
		while(true){
			try{
				vVarNode = p_ParseVarDef();
				vNewNode->AddVariable((dspNodeVarDef*)vVarNode); vVarNode=NULL;
				
			}catch( const duException &e ){
				if(vVarNode) delete vVarNode;
				if(!e.IsNamed("InvalidSyntax")) throw;
				
			}catch( ... ){
				if(vVarNode) delete vVarNode;
				throw;
			}
			if(p_MatchesOperator(p_PeekNode(0), ",")) p_DropNode(); else break;
		}
		p_HasToBeOperator("NL");
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vVarNode) delete vVarNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseVarDef(){
	// ident ['=' expression]
	dspBaseNode *vInitNode=NULL, *vNewNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	try{
		vIdentNode = p_ParseName("variable", dsEngine::peInvalidVarName);
		if(p_MatchesOperator(p_PeekNode(0), "=")){
			p_DropNode(); vInitNode = p_ParseExpr();
		}
		if(!(vNewNode = new dspNodeVarDef(vIdentNode, vInitNode))) DSTHROW(dueOutOfMemory);
		delete vIdentNode; vIdentNode = NULL;
		vInitNode = NULL;
		
	}catch( const duException &e ){
		if(vInitNode) delete vInitNode;
		if(vIdentNode) delete vIdentNode;
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(",","NL");
		throw;
		
	}catch( ... ){
		if(vInitNode) delete vInitNode;
		if(vIdentNode) delete vIdentNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseExpr(){
	// object [ operations ]*
	dspBaseNode *vObjNode=NULL, *vNewNode=NULL;
	try{
		vObjNode = p_ParseObject();
		while(true){
			if( !(vNewNode = p_ParseSpecOperation(vObjNode)) &&
				!(vNewNode = p_ParseMulOperation(vObjNode)) &&
				!(vNewNode = p_ParseAddOperation(vObjNode)) &&
				!(vNewNode = p_ParseBitOperation(vObjNode)) &&
				!(vNewNode = p_ParseCompOperation(vObjNode)) &&
				!(vNewNode = p_ParseLogCompOperation(vObjNode)) &&
				!(vNewNode = p_ParseAssignOperation(vObjNode)) &&
				!(vNewNode = p_ParseInlineIfOperation(vObjNode)) ) break;
			vObjNode = vNewNode; vNewNode = NULL;
		}
	}catch( ... ){
		if(vObjNode) delete vObjNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vObjNode;
}
dspBaseNode *dspParser::p_ParseObject(){
	// { unary-operation | base-object [ '.' class-member ]* } ['++' | '--']*
	dspBaseNode *vPeekNode, *vObjNode=NULL, *vNewNode=NULL, *vTempNode;
	try{
		if( ( vNewNode = p_ParseUnaryOperation() ) ){
			vObjNode = vNewNode; vNewNode = NULL;
		}else{
			vObjNode = p_ParseBaseObject();
			while(p_MatchesOperator(p_PeekNode(0), ".")){
				p_DropNode();
				vTempNode = vObjNode; vObjNode = NULL;
				vNewNode = p_ParseClassMember(vTempNode);
				vObjNode = vNewNode; vNewNode = NULL;
			}
		}
		while(true){
			vPeekNode = p_PeekNode(0);
			if(p_MatchesOperator(vPeekNode, "++")){
				if(!(vNewNode = new dspNodePostIncrement(vPeekNode, vObjNode))) DSTHROW(dueOutOfMemory);
				p_DropNode(); vObjNode = vNewNode; vNewNode = NULL;
			}else if(p_MatchesOperator(vPeekNode, "--")){
				if(!(vNewNode = new dspNodePostDecrement(vPeekNode, vObjNode))) DSTHROW(dueOutOfMemory);
				p_DropNode(); vObjNode = vNewNode; vNewNode = NULL;
			}else break;
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vObjNode) delete vObjNode;
		throw;
	}
	return vObjNode;
}
dspBaseNode *dspParser::p_ParseBaseObject(){
	// grouped-expression | constant | class-member | block-expr
	dspBaseNode *vPeekNode, *vNewNode;
	if( ( vNewNode = p_ParseGroupedExpr() ) ) return vNewNode;
	if( ( vNewNode = p_ParseBlockExpr() ) ) return vNewNode;
	vPeekNode = p_PeekNode(0);
	if( p_MatchesConst(vPeekNode) ||
		p_MatchesType(vPeekNode, ntNull) ||
		p_MatchesType(vPeekNode, ntThis) ||
		p_MatchesType(vPeekNode, ntSuper) ) return p_PopNode();
	return p_ParseClassMember(NULL);
}
dspBaseNode *dspParser::p_ParseBlockExpr(){
	// 'block' 'nl' statement 'end'
	dspBaseNode *vKeyNode=NULL, *vStaNode=NULL;
	dspNodeBlock *vNewNode=NULL;
	dspNodeFuncArg *vArgNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "block")) return NULL;
	// parse block
	try{
		// create node
		vKeyNode = p_PopNode();
		if(!(vNewNode = new dspNodeBlock(vKeyNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = NULL;
		// parse parameters
		if(!p_MatchesOperator(p_PeekNode(0), "NL")){
			while(true){
				vArgNode = p_ParseFuncArg();
				vNewNode->AddArgument(vArgNode); vArgNode = NULL;
				if(p_MatchesOperator(p_PeekNode(0), ",")) p_DropNode(); else break;
			}
		}
		p_HasToBeOperator("NL");
		// parse statement
		vStaNode = p_ParseGroupedSta("end");
		vNewNode->SetStatement(vStaNode); vStaNode = NULL;
//		p_HasToBeBlockEnd("end");
		p_HasToBeKeyword("end");
		
	}catch( const duException &e ){
		if(vKeyNode) delete vKeyNode;
		if(vStaNode) delete vStaNode;
		if(vNewNode) delete vNewNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOperator("NL");
		throw;
		
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vStaNode) delete vStaNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseType(){
	// ident ['.' ident]*
	dspBaseNode *vObjNode=NULL, *vNewNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	try{
		while(true){
			vIdentNode = p_ParseName("class member", dsEngine::peInvalidClassMember);
			if(!(vNewNode = new dspNodeMembVar(vIdentNode, vObjNode))) DSTHROW(dueOutOfMemory);
			vObjNode = vNewNode; delete vIdentNode; vNewNode = vIdentNode = NULL;
			if(!p_MatchesOperator(p_PeekNode(0), ".")) break;
			p_DropNode();
		}	
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vIdentNode) delete vIdentNode;
		if(vObjNode) delete vObjNode;
		throw;
	}
	return vObjNode;
}
dspBaseNode *dspParser::p_ParseRetVal(){
	// 'return' [expression]
	dspBaseNode *vKeyNode=NULL, *vValNode=NULL, *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "return")) return NULL;
	try{
		vKeyNode = p_PopNode();
		if(!p_MatchesOperator(p_PeekNode(0), "NL")){
			vValNode = p_ParseExpr();
		}
		p_HasToBeOperator("NL");
		if(!(vNewNode = new dspNodeReturn(vKeyNode, vValNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vValNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vValNode) delete vValNode;
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseTry(){
	// 'try'
	//     statement
	// <catch-blocks>
	// 'end'
	dspBaseNode *vKeyNode=NULL, *vStaNode=NULL;
	dspNodeCatch *vCatchNode=NULL;
	dspNodeTry *vNodeTry=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "try")) return NULL;
	try{
		vKeyNode = p_PopNode();
		p_HasToBeOperator("NL");
		vStaNode = p_ParseGroupedSta("catch");
		if(!(vNodeTry = new dspNodeTry(vKeyNode, vStaNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vStaNode = NULL;
		while(true){
			try{
				vCatchNode = p_ParseCatch();
				if(!vCatchNode) break;
				vNodeTry->AddCatch(vCatchNode); vCatchNode = NULL;
				
			}catch( const duException &e ){
				if(vCatchNode) delete vCatchNode;
				if(!e.IsNamed("InvalidSyntax")) throw;
				
			}catch( ... ){
				if(vCatchNode) delete vCatchNode;
				throw;
			}
		}
		p_HasToBeBlockEnd("end");
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vStaNode) delete vStaNode;
		if(vCatchNode) delete vCatchNode;
		if(vNodeTry) delete vNodeTry;
		throw;
	}
	return vNodeTry;
}
dspNodeCatch *dspParser::p_ParseCatch(){
	// 'catch' type ident
	//     statement-block
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL, *vStaNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	dspNodeCatch *vNodeCatch=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "catch")) return NULL;
	try{
		vKeyNode = p_PopNode();
		vTypeNode = p_ParseType();
//		if(!p_MatchesOperator(p_PeekNode(0), "NL")){
			vIdentNode = p_ParseName("exception-variable", dsEngine::peInvalidExcepVarName);
//		}
		p_HasToBeOperator("NL");
		vStaNode = p_ParseGroupedSta("end", "catch");
		if(!(vNodeCatch = new dspNodeCatch(vKeyNode, vTypeNode,
			vIdentNode ? vIdentNode->GetName() : "#exc", vStaNode))) DSTHROW(dueOutOfMemory);
		delete vKeyNode; vKeyNode = vTypeNode = vStaNode = NULL;
		delete vIdentNode; vIdentNode = NULL;
	}catch( ... ){
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		if(vStaNode) delete vStaNode;
		if(vNodeCatch) delete vNodeCatch;
//		if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(",","NL");
		throw;
	}
	return vNodeCatch;
}
dspBaseNode *dspParser::p_ParseClassMember(dspBaseNode *Object){
	// ident ['(' arguments ')']
	dspBaseNode *vNewNode=NULL;
	dspNodeIdent *vIdentNode=NULL;
	try{
		vIdentNode = p_ParseName("class member", dsEngine::peInvalidClassMember);
		if(p_MatchesOperator(p_PeekNode(0), "(")){
			if(!(vNewNode = new dspNodeMembFunc(vIdentNode, Object))) DSTHROW(dueOutOfMemory);
			delete vIdentNode; vIdentNode = NULL; Object = NULL;
			p_ParseFuncArgs(((dspNodeMembFunc*)vNewNode)->GetArgumentList());
		}else{
			if(!(vNewNode = new dspNodeMembVar(vIdentNode, Object))) DSTHROW(dueOutOfMemory);
			delete vIdentNode;
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vIdentNode) delete vIdentNode;
		if(Object) delete Object;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseGroupedExpr(){
	// '(' expression ')'
	dspBaseNode *vNewNode = NULL;
	if(p_MatchesOperator(p_PeekNode(0), "(")){
		try{
			p_DropNode(); vNewNode = p_ParseExpr();
			p_HasToBeOperator(")");
			
		}catch( const duException &e ){
			if(vNewNode) delete vNewNode;
			if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(")","NL");
			throw;
			
		}catch( ... ){
			if(vNewNode) delete vNewNode;
			throw;
		}
	}
	return vNewNode;
}

dspBaseNode *dspParser::p_ParseUnaryOperation(){
	// unary-operator object
	const char *ops[] = { "++", "--", "+", "-", "~" };
	dspBaseNode *opNode = NULL;
	dspBaseNode *objNode = NULL;
	dspBaseNode *newNode = NULL;
	dspBaseNode * const peekNode = p_PeekNode( 0 );
	
	if( ! p_MatchesOperatorList( peekNode, &ops[ 0 ], 5 )
	&& ! p_MatchesKeyword( peekNode, "not" ) ){
		return NULL;
	}
	
	try{
		opNode = p_PopNode(); 
		objNode = p_ParseObject();
		
		if( p_MatchesKeyword( opNode, "not" ) ){
			// operator precedence
			while( true ){
				newNode = p_ParseSpecOperation( objNode );
				if( ! newNode ){
					break;
				}
				
				objNode = newNode;
				newNode = NULL;
			}
			
			// create node
			newNode = new dspNodeLogicalNot( opNode, objNode );
			if( ! newNode ){
				DSTHROW( dueOutOfMemory );
			}
			
		}else{
			newNode = new dspNodeUnaryOperation( ( dspNodeOperator* )opNode, objNode );
			if( ! newNode ){
				DSTHROW( dueOutOfMemory );
			}
		}
		
		delete opNode; 
		opNode = objNode = NULL;
		
	}catch( ... ){
		if( newNode ){
			delete newNode;
		}
		if( opNode ){
			delete opNode;
		}
		if( objNode ){
			delete objNode;
		}
		throw;
	}
	
	return newNode;
}

dspBaseNode *dspParser::p_ParseSpecOperation(dspBaseNode *Object){
	dspBaseNode *vKeyNode=NULL, *vTypeNode=NULL, *vNewNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	try{
		if(p_MatchesKeyword(vPeekNode, "cast")){
			vKeyNode = p_PopNode(); vTypeNode = p_ParseType();
			if(!(vNewNode = new dspNodeCastTo(vKeyNode, Object, vTypeNode))) DSTHROW(dueOutOfMemory);
			delete vKeyNode;
		}else if(p_MatchesKeyword(vPeekNode, "castable")){
			vKeyNode = p_PopNode(); vTypeNode = p_ParseType();
			if(!(vNewNode = new dspNodeCastableTo(vKeyNode, Object, vTypeNode))) DSTHROW(dueOutOfMemory);
			delete vKeyNode;
		}else if(p_MatchesKeyword(vPeekNode, "typeof")){
			vKeyNode = p_PopNode(); vTypeNode = p_ParseType();
			if(!(vNewNode = new dspNodeIsTypeOf(vKeyNode, Object, vTypeNode))) DSTHROW(dueOutOfMemory);
			delete vKeyNode;
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vKeyNode) delete vKeyNode;
		if(vTypeNode) delete vTypeNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseMulOperation(dspBaseNode *Object){
	const char *vOps[] = {"*","/","%"};
	dspBaseNode *vOpNode=NULL, *vArgNode=NULL, *vNewNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	if(!p_MatchesOperatorList(vPeekNode, &vOps[0], 3)) return NULL;
	try{
		vOpNode = p_PopNode(); vArgNode = p_ParseObject();
		while(true){
			if( !(vNewNode = p_ParseSpecOperation(vArgNode)) ) break;
			vArgNode = vNewNode; vNewNode = NULL;
		}
		if(!(vNewNode = new dspNodeBinaryOperation((dspNodeOperator*)vOpNode, Object, vArgNode))) DSTHROW(dueOutOfMemory);
		delete vOpNode;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vOpNode) delete vOpNode;
		if(vArgNode) delete vArgNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseAddOperation(dspBaseNode *Object){
	const char *vOps[] = {"+","-"};
	dspBaseNode *vOpNode=NULL, *vArgNode=NULL, *vNewNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	if(!p_MatchesOperatorList(vPeekNode, &vOps[0], 2)) return NULL;
	try{
		vOpNode = p_PopNode(); vArgNode = p_ParseObject();
		while(true){
			if( !(vNewNode = p_ParseSpecOperation(vArgNode)) &&
				!(vNewNode = p_ParseMulOperation(vArgNode)) ) break;
			vArgNode = vNewNode; vNewNode = NULL;
		}
		if(!(vNewNode = new dspNodeBinaryOperation((dspNodeOperator*)vOpNode, Object, vArgNode))) DSTHROW(dueOutOfMemory);
		delete vOpNode;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vOpNode) delete vOpNode;
		if(vArgNode) delete vArgNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseBitOperation(dspBaseNode *Object){
	const char *vOps[] = {"<<",">>","&","|","^"};
	dspBaseNode *vOpNode=NULL, *vArgNode=NULL, *vNewNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	if(!p_MatchesOperatorList(vPeekNode, &vOps[0], 5)) return NULL;
	try{
		vOpNode = p_PopNode(); vArgNode = p_ParseObject();
		while(true){
			if( !(vNewNode = p_ParseSpecOperation(vArgNode)) &&
				!(vNewNode = p_ParseMulOperation(vArgNode)) &&
				!(vNewNode = p_ParseAddOperation(vArgNode)) ) break;
			vArgNode = vNewNode; vNewNode = NULL;
		}
		if(!(vNewNode = new dspNodeBinaryOperation((dspNodeOperator*)vOpNode, Object, vArgNode))) DSTHROW(dueOutOfMemory);
		delete vOpNode;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vOpNode) delete vOpNode;
		if(vArgNode) delete vArgNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseCompOperation(dspBaseNode *Object){
	const char *vOps[] = {"<",">","<=",">=","==","!="};
	dspBaseNode *vOpNode=NULL, *vArgNode=NULL, *vNewNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	if(!p_MatchesOperatorList(vPeekNode, &vOps[0], 6)) return NULL;
	try{
		vOpNode = p_PopNode(); vArgNode = p_ParseObject();
		while(true){
			if( !(vNewNode = p_ParseSpecOperation(vArgNode)) &&
				!(vNewNode = p_ParseMulOperation(vArgNode)) &&
				!(vNewNode = p_ParseAddOperation(vArgNode)) &&
				!(vNewNode = p_ParseBitOperation(vArgNode)) ) break;
			vArgNode = vNewNode; vNewNode = NULL;
		}
		if(!(vNewNode = new dspNodeBinaryOperation((dspNodeOperator*)vOpNode, Object, vArgNode))) DSTHROW(dueOutOfMemory);
		delete vOpNode;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vOpNode) delete vOpNode;
		if(vArgNode) delete vArgNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseLogCompOperation(dspBaseNode *Object){
	dspBaseNode *vOpNode=NULL, *vCondNode=NULL, *vNewNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	bool vCompType;
	if(p_MatchesKeyword(vPeekNode, "and")) vCompType = true;
	else if(p_MatchesKeyword(vPeekNode, "or")) vCompType = false;
	else return NULL;
	try{
		vOpNode = p_PopNode(); vCondNode = p_ParseObject();
		while(true){
			if( !(vNewNode = p_ParseSpecOperation(vCondNode)) &&
				!(vNewNode = p_ParseMulOperation(vCondNode)) &&
				!(vNewNode = p_ParseAddOperation(vCondNode)) &&
				!(vNewNode = p_ParseBitOperation(vCondNode)) &&
				!(vNewNode = p_ParseCompOperation(vCondNode)) &&
				!(vNewNode = p_ParseLogCompOperation(vCondNode)) ) break;
			vCondNode = vNewNode; vNewNode = NULL;
		}
		if(vCompType){
			if(!(vNewNode = new dspNodeLogicalAnd(vOpNode, Object, vCondNode))) DSTHROW(dueOutOfMemory);
		}else{
			if(!(vNewNode = new dspNodeLogicalOr(vOpNode, Object, vCondNode))) DSTHROW(dueOutOfMemory);
		}
		delete vOpNode;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vOpNode) delete vOpNode;
		if(vCondNode) delete vCondNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseAssignOperation(dspBaseNode *Object){
	const char *vOps[] = {"=","*=","/=","%=","+=","-=","<<=",">>=","&=","|=","^="};
	dspBaseNode *vOpNode=NULL, *vArgNode=NULL, *vNewNode=NULL;
	dspBaseNode *vPeekNode = p_PeekNode(0);
	if(!p_MatchesOperatorList(vPeekNode, &vOps[0], 11)) return NULL;
	try{
		vOpNode = p_PopNode(); vArgNode = p_ParseObject();
		while(true){
			if( !(vNewNode = p_ParseSpecOperation(vArgNode)) &&
				!(vNewNode = p_ParseMulOperation(vArgNode)) &&
				!(vNewNode = p_ParseAddOperation(vArgNode)) &&
				!(vNewNode = p_ParseBitOperation(vArgNode)) &&
				!(vNewNode = p_ParseCompOperation(vArgNode)) &&
				!(vNewNode = p_ParseLogCompOperation(vArgNode)) &&
				!(vNewNode = p_ParseAssignOperation(vArgNode)) &&
				!(vNewNode = p_ParseInlineIfOperation(vArgNode)) ) break;
			vArgNode = vNewNode; vNewNode = NULL;
		}
		if(!(vNewNode = new dspNodeBinaryOperation((dspNodeOperator*)vOpNode, Object, vArgNode))) DSTHROW(dueOutOfMemory);
		delete vOpNode;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vOpNode) delete vOpNode;
		if(vArgNode) delete vArgNode;
		throw;
	}
	return vNewNode;
}
dspBaseNode *dspParser::p_ParseInlineIfOperation(dspBaseNode *Object){
	// <cond> 'if' <expr> 'else' <expr>
	dspBaseNode *vOpNode=NULL, *vIfNode=NULL, *vElseNode=NULL, *vNewNode=NULL;
	if(!p_MatchesKeyword(p_PeekNode(0), "if")) return NULL;
	try{
		vOpNode = p_PopNode(); vIfNode = p_ParseExpr();
		p_HasToBeKeyword("else"); vElseNode = p_ParseExpr();
		if(!(vNewNode = new dspNodeInlineIf(vOpNode, Object, vIfNode, vElseNode))) DSTHROW(dueOutOfMemory);
		delete vOpNode;
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		if(vOpNode) delete vOpNode;
		if(vIfNode) delete vIfNode;
		if(vElseNode) delete vElseNode;
		throw;
	}
	return vNewNode;
}
void dspParser::p_ParseFuncArgs(dspListNodes *ArgList){
	dspBaseNode *vArgNode=NULL, *vPeekNode;
	try{
		p_HasToBeOperator("(");
		if(!p_MatchesOperator(p_PeekNode(0), ")")){
			while(true){
				vArgNode = p_ParseExpr();
				ArgList->AddNode(vArgNode); vArgNode = NULL;
				vPeekNode = p_PeekNode(0);
				if(p_MatchesOperator(vPeekNode, ",")) p_DropNode(); else break;
			}
		}
		p_HasToBeOperator(")");
		
	}catch( const duException &e ){
		if(vArgNode) delete vArgNode;
		if(e.IsNamed("InvalidSyntax")) p_SkipToOpWithBreak(")","NL");
		throw;
		
	}catch( ... ){
		if(vArgNode) delete vArgNode;
		throw;
	}
}


void dspParser::p_CheckTypeMods(dspTypeModifiers *TypeMods, int allowed){
	int types[] = {DSTM_PUBLIC, DSTM_PROTECTED, DSTM_PRIVATE, DSTM_NATIVE,
		DSTM_ABSTRACT, DSTM_FIXED, DSTM_STATIC};
	
	for(int i=0; i<7; i++){
		if(!(allowed & types[i])){
			if(TypeMods->IsTypeModSet(types[i])){
				p_ErrorInvTypeMod(TypeMods->GetTypeNode(types[i]));
				TypeMods->UnsetTypeMod(types[i]);
			}
		}
	}
}
void dspParser::p_SkipToOperator(const char *Operator){
	dspBaseNode *vNode;
	while( ( vNode = p_PeekNode(0) ) ){
		if(p_MatchesOperator(vNode, Operator)){
			p_DropNode(); break;
		}
		p_DropNode();
	}
}
void dspParser::p_SkipToOperators(const char **OperatorList, int Count){
	dspBaseNode *vNode; int i;
	while( ( vNode = p_PeekNode(0) ) ){
		for(i=0; i<Count; i++){
			if(p_MatchesOperator(vNode, OperatorList[i])){
				p_DropNode(); return;
			}
		}
		p_DropNode();
	}
}
void dspParser::p_SkipToOpWithBreak(const char *SkipOp, const char *BreakOp){
	dspBaseNode *vNode;
	while( ( vNode = p_PeekNode(0) ) ){
		if(p_MatchesOperator(vNode, BreakOp)) break;
		if(p_MatchesOperator(vNode, SkipOp)){ p_DropNode(); break; }
		p_DropNode();
	}
}
void dspParser::p_SkipToOpWithBreakKW(const char *SkipOp, const char *BreakKw){
	dspBaseNode *vNode;
	while( ( vNode = p_PeekNode(0) ) ){
		if(p_MatchesKeyword(vNode, BreakKw)) break;
		if(p_MatchesOperator(vNode, SkipOp)){ p_DropNode(); break; }
		p_DropNode();
	}
}
void dspParser::p_SkipToKeyword(const char *Keyword){
	dspBaseNode *vNode;
	while( ( vNode = p_PeekNode(0) ) ){
		if(p_MatchesKeyword(vNode, Keyword)){
			p_DropNode(); break;
		}
		p_DropNode();
	}
}
// add class for dspParserCheckClass
void dspParser::p_AddParsedClass(dsClass *Class){
	if(!Class || !p_Package) DSTHROW(dueInvalidParam);
//	dsClass *vClass = Class->GetParent();
//	while(vClass){
//		if(p_Package->ExistsClass(vClass)) break;
//		vClass = vClass->GetParent();
//	}
//	if(!vClass) p_Package->p_AddClass(Class);
	if(!p_Package->ExistsClass(Class)){
		p_Package->p_AddClass(Class);
	}
}

