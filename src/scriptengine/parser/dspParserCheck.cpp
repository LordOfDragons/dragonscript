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
#include "dspParser.h"
#include "dspParserCheck.h"
#include "dspParserCheckClass.h"
#include "dspParserCheckType.h"
#include "dspParserCheckScript.h"
#include "../dsEngine.h"
#include "../dsPackage.h"
#include "../exceptions.h"
#include "../dsBaseEngineManager.h"
#include "../dsPackageSource.h"
#include "../utils/dsuString.h"
#include "../objects/dsClass.h"
#include "../objects/dsFunction.h"
#include "../objects/dsFuncList.h"
#include "../objects/dsClassParserInfo.h"

// definitions
#define SE_MSGBUFSIZE			250

// dspParserCheck
///////////////////
// constructor, destructor
dspParserCheck::dspParserCheck(dspParser *Parser){
	if(!Parser) DSTHROW(dueInvalidParam);
	p_Parser = Parser;
	p_Classes = NULL;
	p_ClassCount = 0;
	p_Scripts = NULL;
	p_ScriptCount = 0;
	p_curScript = NULL;
	if(!(p_MsgBuf = new char[SE_MSGBUFSIZE+1])) DSTHROW(dueOutOfMemory);
}
dspParserCheck::~dspParserCheck(){
	Clear();
	delete [] p_MsgBuf;
}
// management
void dspParserCheck::Clear(){
	if(p_Scripts){
		for(int i=0; i<p_ScriptCount; i++) delete p_Scripts[i];
		delete [] p_Scripts;
		p_Scripts = NULL;
		p_ScriptCount = 0;
	}
	if(p_Classes){
		for(int i=0; i<p_ClassCount; i++) delete p_Classes[i];
		delete [] p_Classes;
		p_Classes = NULL;
		p_ClassCount = 0;
	}
}
void dspParserCheck::ProcessScript(dspNodeScript *Script){
	if(!Script) DSTHROW(dueInvalidParam);
	dsEngine *vEngine = GetEngine();
	dsPackageSource *pkgSrc;
	dspParserCheckScript *vNewScript = NULL;
	dspParserCheckType *vCurNamespace = NULL;
	dspBaseNode *vCurNode, *vNSNode;
	dsClass *vPinNS;
	dsPackage *vReqPackage;
	const char *vPackageName;
	int type;
	// create script
	if(!(vNewScript = new dspParserCheckScript(Script))) DSTHROW(dueOutOfMemory);
	p_AddScript(vNewScript);
	// preload classes
	dspListNodeIterator vNodeItr(Script->GetNodeList());
	while(vNodeItr.HasNext()){
		vCurNode = *vNodeItr.GetNext();
		type = vCurNode->GetNodeType();
		switch(type){
		case ntRequires:
//		case ntIncludes:
			vPackageName = ((dspNodeRequires*)vCurNode)->GetPackageName();
			if(!vEngine->ExistsPackage(vPackageName)){
				pkgSrc = vEngine->FindPackageSource(vPackageName);
				if(!pkgSrc){
					ErrorPackageNotFound(vCurNode, vPackageName); DSTHROW(dseInvalidSyntax);
				}
				vReqPackage = pkgSrc->LoadPackage(vPackageName);
				try{
//					if(type == ntRequires){
						// if package happens to be empty just print out a warning
						if(vReqPackage->GetScriptCount()==0 && vReqPackage->GetHostClassCount()==0){
							WarnPackageEmpty(vCurNode, vPackageName);
						}
						// add the package
						if(!vEngine->AddPackage(vReqPackage)) DSTHROW(dseInvalidSyntax);
						vReqPackage = NULL;
//					}else{
//						vScrPak->AppendPackage(vReqPackage);
//						delete vReqPackage; vRepPackage = NULL;
//					}
				}catch( ... ){
					if(vReqPackage) delete vReqPackage;
					throw;
				}
			}
			break;
		case ntOption:
			// ignore. no options available for vm parsing right now
			break;
		case ntPin:
			vNSNode = ((dspNodePin*)vCurNode)->GetNamespace();
			try{
				vCurNamespace = dspParserCheckType::GetTypeFromNode(vNSNode);
				CreateNS(vNSNode, vCurNamespace);
				vPinNS = vCurNamespace->GetClass(p_Parser->GetEngine(), NULL);
				if(!vPinNS){
					ErrorClassNotExist(vNSNode, vCurNamespace); DSTHROW(dseInvalidSyntax);
				}
				vNewScript->AddPin(vPinNS);
				delete vCurNamespace; vCurNamespace = NULL;
				
			}catch( const duException &e ){
				if(vCurNamespace) delete vCurNamespace;
				if(e.IsNamed("InvalidSyntax")) throw;
				return;
				
			}catch( ... ){
				if(vCurNamespace) delete vCurNamespace;
				throw;
			}
			break;
		}
	}
	// process classes and interfaces
	vNodeItr.Rewind();
	while(vNodeItr.HasNext()){
		vCurNode = *vNodeItr.GetNext();
		if(vCurNode->GetNodeType() == ntNamespace){
			vNSNode = ((dspNodeNamespace*)vCurNode)->GetNamespace();
			try{
				if(vCurNamespace){ delete vCurNamespace; vCurNamespace = NULL; }
				if(vNSNode){
					vCurNamespace = dspParserCheckType::GetTypeFromNode(vNSNode);
					CreateNS(vNSNode, vCurNamespace);
					if(ExistsClass(vCurNamespace)){
						ErrorNSMemberNotNS(vNSNode, NULL, vCurNamespace->GetLastName()); DSTHROW(dseInvalidSyntax);
					}
				}
				
			}catch( const duException &e ){
				if(vCurNamespace) delete vCurNamespace;
				if(e.IsNamed("InvalidSyntax")) throw;
				return;
				
			}catch( ... ){
				if(vCurNamespace) delete vCurNamespace;
				throw;
			}
		}else if(vCurNode->GetNodeType() == ntClass){
			p_curScript = vNewScript;
			p_ProcessClass((dspNodeClass*)vCurNode, vCurNamespace);
			p_curScript = NULL;
		}
	}
	if(vCurNamespace){ delete vCurNamespace; vCurNamespace = NULL; }
}
void dspParserCheck::CheckScripts(){
	dsPackage *pkg = p_Parser->GetPackage();
	dspParserCheckClass *newCheckClass = NULL;
	dsClassParserInfo *parserInfo;
	dsEngine *vEngine = GetEngine();
	dsPackageSource *pkgSrc;
	dsPackage *vReqPackage;
	dsClass *vClass;
	dsFuncList *vFuncList;
	dsFunction *vFunc;
	dspNodeNatClassErr natErrNode;
	const char *vPackageName;
	int i, j;
	// debugging
#ifndef __NODBGPRINTF__
//	for(i=0; i<p_ClassCount; i++){
//		p_Classes[i]->GetTypeClass()->DebugPrint();
//	}
#endif
	// load all packages mentioned in native classes if not loaded yet
	for(i=0; i<pkg->GetHostClassCount(); i++){
		parserInfo = pkg->GetHostClass(i)->GetParserInfo();
		for(j=0; j<parserInfo->GetRequiresCount(); j++){
			vPackageName = parserInfo->GetRequires(j);
			if(!vEngine->ExistsPackage(vPackageName)){
				pkgSrc = vEngine->FindPackageSource(vPackageName);
				if(!pkgSrc){
					ErrorPackageNotFound(&natErrNode, vPackageName); DSTHROW(dseInvalidSyntax);
				}
				vReqPackage = pkgSrc->LoadPackage(vPackageName);
				try{
					if(vReqPackage->GetScriptCount()==0 && vReqPackage->GetHostClassCount()==0){
						WarnPackageEmpty(&natErrNode, vPackageName);
						delete vReqPackage;
					}else{
						if(!vEngine->AddPackage(vReqPackage)) DSTHROW(dseInvalidSyntax);
					}
					vReqPackage = NULL;
				}catch( ... ){
					if(vReqPackage) delete vReqPackage;
					throw;
				}
			}
		}
	}
	// add native classes to the list so they get processed correctly
	try{
		while(pkg->GetHostClassCount() > 0){
			newCheckClass = new dspParserCheckClass(pkg->GetHostClass(0));
			if(!newCheckClass) DSTHROW(dueOutOfMemory);
			pkg->p_RemoveHostClass();
			p_AddClass(newCheckClass);
			newCheckClass = NULL;
		}
	}catch( ... ){
		if(newCheckClass) delete newCheckClass;
		throw;
	}
	// check for class inheritance errors and create the classes
	if(p_ClassCount > 0){
		p_CheckInheritance();
		if(p_Parser->GetErrorCount() > 0) return;
	}
	// check classes for incorrect members and create them if all is ok
	for(i=0; i<p_ClassCount; i++){
		p_curScript = p_Classes[i]->GetScript();
		p_Classes[i]->CreateClassMembers(this);
	}
	if(p_Parser->GetErrorCount() > 0) return;
	// check for missing function declarations
	for(i=0; i<p_ClassCount; i++){
		vClass = p_Classes[i]->GetClassNew();
		if(vClass->GetClassType() != DSCT_CLASS) continue;
		if(vClass->GetTypeModifiers() & DSTM_ABSTRACT) continue;
		vFuncList = vClass->GetFuncList();
		for(j=0; j<vFuncList->GetCount(); j++){
			vFunc = vFuncList->GetFunction(j);
			if(vFunc->GetTypeModifiers() & DSTM_ABSTRACT){
				ErrorFuncNotImplemented(p_Classes[i]->GetNode(), vClass, vFunc);
			}
		}
	}
	if(p_Parser->GetErrorCount() > 0) return;
	// resolve all compile time constants
	for(i=0; i<p_ClassCount; i++){
		p_curScript = p_Classes[i]->GetScript();
		p_Classes[i]->CheckConstants(this);
	}
	// check functions for incorrect code
	for(i=0; i<p_ClassCount; i++){
		p_curScript = p_Classes[i]->GetScript();
		p_Classes[i]->CheckFunctionCode(this);
	}
	p_curScript = NULL;
	if(p_Parser->GetErrorCount() > 0) return;
	// calculate all member offsets
	for(i=0; i<p_ClassCount; i++){
		p_Classes[i]->GetClassNew()->CalcMemberOffsets();
	}
	// debug
	/*
	for(i=0; i<p_ClassCount; i++){
		printf("[FUNCLIST for %s]\n", p_Classes[i]->GetClassNew()->GetName());
		p_Classes[i]->GetClassNew()->GetFuncList()->DebugPrint();
	}
	*/
}
void dspParserCheck::CompileScripts(){
	int i;
	// compile runtime constants
	for(i=0; i<p_ClassCount; i++){
		p_Classes[i]->CompileConstants(this);
	}
	// compile function code
	for(i=0; i<p_ClassCount; i++){
		p_Classes[i]->CompileFunctionCode(this);
	}
}

void dspParserCheck::OptimizeScripts(){
#ifdef WITH_OPTIMIZATIONS
	int i;
	for( i=0; i<p_ClassCount; i++ ){
		p_Classes[ i ]->OptimizeFunctionCode( *this );
	}
#endif
}

dsClass *dspParserCheck::GetClassFromNode(dspBaseNode *Node, dsClass *BaseClass){
	if(!Node) return p_Parser->GetEngine()->GetClassVoid();
	dspParserCheckType *vCheckType=NULL;
	dsClass *vTypeClass;
	try{
		vCheckType = dspParserCheckType::GetTypeFromNode(Node);
		vTypeClass = GetClassFromType(vCheckType, BaseClass);
		if(!vTypeClass){
			ErrorClassNotExist(Node, vCheckType); DSTHROW(dseInvalidSyntax);
		}
		delete vCheckType;
	}catch( ... ){
		if(vCheckType) delete vCheckType;
		throw;
	}
	return vTypeClass;
}

dsClass *dspParserCheck::GetClassFromName( const char *name, dsClass *baseClass ){
	if( ! name || ! baseClass ){
		DSTHROW( dueInvalidParam );
	}
	
	dsClass *curClass = baseClass;
	dsClass *typeClass;
	
	// match against inner classes in this class and all base classes. if go up the
	// parent chain repeating the check
	while( curClass ){
		// check if inner class with name exists
		typeClass = curClass->GetInnerClass( name );
		if( typeClass ){
			return typeClass;
		}
		
		// check if a base class or an inner class thereof matches the name
		dsClass *baseClass = curClass->GetBaseClass();
		while( baseClass ){
			typeClass = baseClass->GetInnerClass( name );
			if( typeClass ){
				return typeClass;
			}
			
			if( strcmp( baseClass->GetName(), name ) == 0 ){
				return baseClass;
			}
			
			baseClass = baseClass->GetBaseClass();
		}
		
		// continue checking up the parent chain
		curClass = curClass->GetParent();
	}
	
	// if nothing is found try an engine wide class with this name
	typeClass = p_Parser->GetEngine()->GetClass( name );
	if( typeClass ){
		return typeClass;
	}
	
	// if still not found try searching pinned namespaces for a match
	if( ! p_curScript ){
		return NULL;
	}
	
	int i;
	
	for( i=0; i<p_curScript->GetPinCount(); i++ ){
		curClass = p_curScript->GetPin( i );
		
		while( curClass ){
			// check if inner class with name exists
			typeClass = curClass->GetInnerClass( name );
			if( typeClass ){
				return typeClass;
			}
			
			// check if a base class or an inner class thereof matches the name
			dsClass *baseClass = curClass->GetBaseClass();
			while( baseClass ){
				typeClass = baseClass->GetInnerClass( name );
				if( typeClass ){
					return typeClass;
				}
				
				if( strcmp( baseClass->GetName(), name ) == 0 ){
					return baseClass;
				}
				
				baseClass = baseClass->GetBaseClass();
			}
			
			// continue checking up the parent chain
			curClass = curClass->GetParent();
		}
	}
	
	return NULL;
}

dsClass *dspParserCheck::GetClassFromType( dspParserCheckType *type, dsClass *baseClass ){
	dsClass *findClass = type->GetClass( p_Parser->GetEngine(), baseClass );
	
	if( ! findClass && p_curScript ){
		int i;
		
		for( i=0; i<p_curScript->GetPinCount(); i++ ){
			findClass = type->GetClass( p_Parser->GetEngine(), p_curScript->GetPin( i ) );
			if( findClass ){
				break;
			}
		}
	}
	
	return findClass;
}

void dspParserCheck::CreateNS(dspBaseNode *refNode, dspParserCheckType *type){
	dsEngine *vEngine = p_Parser->GetEngine();
	dsClass *vCurNS=NULL, *vNewClass=NULL;
	const char *vName;
	try{
		for(int i=0; i<type->GetNameCount(); i++){
			vName = type->GetName(i);
			if(vCurNS){
				vNewClass = vCurNS->GetInnerClass(vName);
			}else{
				vNewClass = vEngine->GetClass(vName);
			}
			if(vNewClass){
				if(vNewClass->GetClassType() != DSCT_NAMESPACE){
					ErrorNSMemberNotNS(refNode, vCurNS, vNewClass->GetName()); DSTHROW(dseInvalidSyntax);
				}
				vCurNS = vNewClass; vNewClass = NULL;
			}else{
				if(!(vNewClass = new dsClass(vName, DSCT_NAMESPACE, DSTM_PUBLIC))) DSTHROW(dueOutOfMemory);
				if(vCurNS){
					vCurNS->AddInnerClass(vNewClass);
				}else{
					vEngine->p_AddClass(vNewClass);
				}
				p_Parser->p_AddParsedClass(vNewClass);
				vCurNS = vNewClass; vNewClass = NULL;
			}
		}
	}catch( ... ){
		if(vNewClass) delete vNewClass;
		throw;
	}
}
// information
dsEngine *dspParserCheck::GetEngine() const{
	return p_Parser->GetEngine();
}
bool dspParserCheck::HasErrors() const{
	return p_Parser->GetErrorCount() > 0;
}
dsConstant *dspParserCheck::FindClassConstant(dsClass *OwnerClass, const char *ConstName){
	if(!OwnerClass || !ConstName) DSTHROW(dueInvalidParam);
	dsConstant *vConstant = NULL;
	for(int i=0; i<p_ClassCount; i++){
		vConstant = p_Classes[i]->CheckConstant(this, ConstName);
		if(vConstant) break;
	}
	return vConstant;
}
// error functions
void dspParserCheck::ErrorClassExists(dspNodeClass *NodeClass, dspParserCheckType *TypeParent){
	if(!NodeClass) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	if(TypeParent){
		sprintf(p_MsgBuf, "Inner Class '%s' exists already in '%s'", NodeClass->GetName(), TypeParent->GetName(TypeParent->GetNameCount()-1));
	}else{
		sprintf(p_MsgBuf, "Global Class '%s' exists already", NodeClass->GetName());
	}
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peClassExists, NodeClass->GetSource(), NodeClass->GetLineNum(), NodeClass->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorInheritanceLoop(dspParserCheckClass *Class){
	if(!Class) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	dspNodeClass *vNode = Class->GetNode();
	sprintf(p_MsgBuf, "Class '%s' is involved in a Inheritance Loop", vNode->GetName());
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peInheritanceLoop, Class->GetScript()->GetScript()->GetSource(), vNode->GetLineNum(), vNode->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorClassNotExist(dspBaseNode *Node, dspParserCheckType *Type){
	if(!Node || !Type) DSTHROW(dueInvalidParam);
	dsEngine *vEngine = p_Parser->GetEngine();
	dsClass *vCurClass = NULL;
	for(int i=0; i<Type->GetNameCount(); i++){
		if(vCurClass){
			vCurClass = vCurClass->GetInnerClass(Type->GetName(i));
		}else{
			vCurClass = vEngine->GetClass(Type->GetName(i));
		}
		if(!vCurClass){
			if(i == 0){
				sprintf(p_MsgBuf, "Class '%s' does not exist", Type->GetName(0));
			}else{
				sprintf(p_MsgBuf, "Class '%s' does not exist in '%s'", Type->GetName(i), Type->GetName(i-1));
			}
			break;
		}
	}
	vEngine->GetEngineManager()->OutputError(p_MsgBuf, dsEngine::peClassNotExist, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorClassIsFixed(dspBaseNode *Node, dspParserCheckType *Type){
	if(!Node || !Type) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Class '%s' is fixed and can not be extended", Type->GetName(Type->GetNameCount()-1));
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peClassIsFixed, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorDupDestructor(dspBaseNode *Node, const char *ClassName){
	if(!Node || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "There exists already a destructor in '%s'", ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupClassDestr, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorDupFunction(dspBaseNode *Node, const char *FuncName, const char *ClassName){
	if(!Node || !FuncName || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "There exists already a function '%s' with the same signature in '%s'", FuncName, ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupClassFunc, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorDupOperator(dspBaseNode *Node, const char *Op, const char *ClassName){
	if(!Node || !Op || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "There exists already an operator '%s' in '%s'", Op, ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupClassOp, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorInvDupFuncSig(dspBaseNode *Node, const char *FuncName, const char *ClassName){
	if(!Node || !FuncName || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "There exists already a function '%s' with the same signature but a different return type in '%s'", FuncName, ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupClassFunc, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorInvDestrSig(dspBaseNode *Node, const char *Name){
	if(!Node || !Name) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Destructor '%s' cannot have arguments", Name);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peInvalidDestrSig, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorDupVariable(dspBaseNode *Node, const char *ClassName, const char *VarName){
	if(!Node || !ClassName || !VarName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "There exists already a variable '%s' in '%s'", VarName, ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupClassVar, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorDupConstant(dspBaseNode *Node, const char *ClassName, const char *ConstName){
	if(!Node || !ClassName || !ConstName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "There exists already a constant '%s' in '%s'", ConstName, ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupClassConst, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorInvVarTypeVoid(dspBaseNode *Node, const char *VarName){
	if(!Node || !VarName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Invalid type 'void' for variable '%s'", VarName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peInvalidVarTypeVoid, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorInvLocVarTypeVoid(dspBaseNode *Node, const char *VarName){
	if(!Node || !VarName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Invalid type 'void' for local variable '%s'", VarName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peInvalidLocVarTypeVoid, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorDupLocVar(dspNodeVarDef *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Local variable '%s' has already been defined in this context", Node->GetName());
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupLocVar, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorInvOpArgCount(dspBaseNode *Node, const char *Op, const char *ClassName){
	if(!Node || !Op || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Operator %s in class %s has incorrect number of arguments", Op, ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peInvalidOpArgCount, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorNoStaVarInit(dspNodeClassVariable *Node, const char *ClassName){
	if(!Node || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Missing init value for variable %s in %s", Node->GetName(), ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peNoStaVarInit, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorInvalidVarInit(dspNodeClassVariable *Node, const char *ClassName){
	if(!Node || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Invalid init value for variable %s in %s", Node->GetName(), ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peInvalidVarInit, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorMissingReturn(dspBaseNode *Node, const char *FuncName, const char *ClassName){
	if(!Node || !FuncName || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Not all control path do have a return statement in function %s in %s", FuncName, ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peMissingReturn, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorNotInterface(dspBaseNode *Node, dsClass *Class){
	if(!Node || !Class) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "%s is not an Interface", Class->GetName());
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peNotInterface, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorBaseNotSameType(dspBaseNode *Node, dsClass *Base, dsClass *Class){
	if(!Node || !Class) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Base class %s is not of the same type (class/interface) as %s", Base->GetName(), Class->GetName());
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peBaseNotSameType, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorDupImplement(dspBaseNode *Node, dsClass *Class, dsClass *ImplClass){
	if(!Node || !Class || !ImplClass) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Interface %s is already included in %s", ImplClass->GetName(), Class->GetName());
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peDupImplement, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorFuncNotImplemented(dspBaseNode *Node, dsClass *OwnerClass, dsFunction *Function){
	if(!Node || !OwnerClass || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Abstract Function %s in %s has not been implemented in class %s", Function->GetName(), Function->GetOwnerClass()->GetName(), OwnerClass->GetName());
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peFuncNotImplemented, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorNSMemberNotNS(dspBaseNode *Node, dsClass *OwnerNS, const char *MemberClass){
	if(!Node || !MemberClass) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	if(OwnerNS){
		sprintf(p_MsgBuf, "Namespace member %s in %s has to be a namespace itself", MemberClass, OwnerNS->GetName());
	}else{
		sprintf(p_MsgBuf, "%s has to be a namespace", MemberClass);
	}
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peNSMemberNotNS, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorConstLoop(dspNodeClassVariable *Node, const char *ClassName){
	if(!Node || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Constant %s in %s cannot be resolved due to an init loop", Node->GetName(), ClassName);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::peConstLoop, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}
void dspParserCheck::ErrorPackageNotFound(dspBaseNode *Node, const char *Package){
	if(!Node || !Package) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Package %s can not be found", Package);
	vEngMgr->OutputError(p_MsgBuf, dsEngine::pePackageNotFound, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncErrCount();
}

void dspParserCheck::ErrorFuncInvTypeMods( dspBaseNode *node, const char *className, const char *functionName ){
	if( ! node || ! className ){
		DSTHROW( dueInvalidParam );
	}
	dsBaseEngineManager &engMgr = *GetEngine()->GetEngineManager();
	dsuString msgBuf;
	msgBuf = "Invalid type modifier combination for class function ";
	msgBuf += functionName;
	msgBuf += " in ";
	msgBuf += className;
	msgBuf += ".";
	engMgr.OutputError( msgBuf.Pointer(), dsEngine::peFuncInvTypeMods, node->GetSource(), node->GetLineNum(), node->GetCharNum() );
	p_Parser->IncErrCount();
}


void dspParserCheck::WarnPackageEmpty(dspBaseNode *Node, const char *Package){
	if(!Node || !Package) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = p_Parser->GetEngine()->GetEngineManager();
	sprintf(p_MsgBuf, "Package %s is empty and will not be added", Package);
	vEngMgr->OutputWarning(p_MsgBuf, dsEngine::pwPackageEmpty, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_Parser->IncWarnCount();
}
// private functions
// check functions
void dspParserCheck::p_AddClass(dspParserCheckClass *Class){
	if(!Class) DSTHROW(dueInvalidParam);
	dspParserCheckClass **vNewArray = new dspParserCheckClass*[p_ClassCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_Classes){
		for(int i=0; i<p_ClassCount; i++) vNewArray[i] = p_Classes[i];
		delete [] p_Classes;
	}
	vNewArray[p_ClassCount] = Class;
	p_Classes = vNewArray;
	p_ClassCount++;
}
void dspParserCheck::p_RemoveClass(int Index){
	delete p_Classes[Index];
	for(int i=Index; i<p_ClassCount-1; i++) p_Classes[i] = p_Classes[i+1];
	p_ClassCount--;
}
void dspParserCheck::p_ProcessClass(dspNodeClass *Class, dspParserCheckType *ParentType){
	dspParserCheckClass *vNewCheckClass = NULL;
	dspParserCheckClass *vCurCheckClass = NULL;
	dspParserCheckType *vNewTypeBase = NULL;
	dspBaseNode *vMemberNode;
	dsClass *clsCheck;
	try{
		// check if given class already exists in the scripts
		// we are parsing right now
		if(ExistsClass(Class->GetName(), ParentType)){
			ErrorClassExists(Class, ParentType); return;
		}
		// check if the class already exists in the engine
		if(ParentType){
			clsCheck = GetClassFromType(ParentType, NULL);
			if(clsCheck && clsCheck->GetInnerClass(Class->GetName())){
				ErrorClassExists(Class, ParentType); return;
			}
		}else{
			if(GetEngine()->GetClass(Class->GetName())){
				ErrorClassExists(Class, ParentType); return;
			}
		}
		// build types and create check-class
		if(!vCurCheckClass){
			if(Class->GetBaseClassNode()){
				if(!(vNewTypeBase = dspParserCheckType::GetTypeFromNode(Class->GetBaseClassNode()))) DSTHROW(dueOutOfMemory);
			}
			if(!(vNewCheckClass = new dspParserCheckClass(p_curScript, Class, ParentType, vNewTypeBase))) DSTHROW(dueOutOfMemory);
			vNewTypeBase = NULL;
			p_AddClass(vCurCheckClass = vNewCheckClass);
		}
		// check for inner classes
		dspListNodeIterator vMembItr(Class->GetMemberList());
		while(vMembItr.HasNext()){
			vMemberNode = *vMembItr.GetNext();
			if(vMemberNode->GetNodeType() == ntClass){
				p_ProcessClass((dspNodeClass*)vMemberNode, vCurCheckClass->GetTypeClass());
			}
		}
	}catch( ... ){
		if(vNewTypeBase) delete vNewTypeBase;
		if(vNewCheckClass) delete vNewCheckClass;
		throw;
	}
}
dspParserCheckClass *dspParserCheck::p_FindClass(const char *Name, dspParserCheckType *TypeParent){
	for(int i=0; i<p_ClassCount; i++){
		if(p_Classes[i]->MatchesClass(Name, TypeParent)) return p_Classes[i];
	}
	return NULL;
}
bool dspParserCheck::ExistsClass(const char *Name, dspParserCheckType *TypeParent){
	for(int i=0; i<p_ClassCount; i++){
		if(p_Classes[i]->MatchesClass(Name, TypeParent)) return true;
	}
	return false;
}
bool dspParserCheck::ExistsClass(dspParserCheckType *TypeClass){
	if(TypeClass){
		for(int i=0; i<p_ClassCount; i++){
			if(p_Classes[i]->GetTypeClass()->MatchesType(TypeClass)) return true;
		}
	}
	return false;
}
void dspParserCheck::p_CheckInheritance(){
	dspParserCheckClass *vClass;
	int i, vLastCount, vCurCount;
	dspParserCheckClass **newClassList = NULL;
	int newClassListCount = 0;
	// if there are no classes there is something wrong
	if(p_ClassCount == 0) DSTHROW(dueInvalidParam);
	try{
		// create a new array for classes to add. we need a sorted
		// list for working with those classes later
		newClassList = new dspParserCheckClass*[p_ClassCount];
		if(!newClassList) DSTHROW(dueOutOfMemory);
		// check classes
		vLastCount = vCurCount = p_ClassCount;
		while(vCurCount > 0){
			vLastCount = vCurCount;
			// go through all classes and create all of them which are
			// not marked as ready.
			for(i=0; i<p_ClassCount; i++){
				vClass = p_Classes[i];
				p_curScript = vClass->GetScript();
				if(!vClass->IsReady()){
					vClass->CreateClass(this);
					if(vClass->IsReady()){
						// class is good so add it to the list
						newClassList[newClassListCount++] = vClass;
						// one class less to test
						vCurCount--;
					}
				}
			}
			// if no class has been created this round an error has occured
			if(vCurCount == vLastCount){
				while(vCurCount > 0){
					vLastCount = vCurCount;
					// check for missing base classes
					for(i=0; i<p_ClassCount; i++){
						vClass = p_Classes[i];
						p_curScript = vClass->GetScript();
						// if a class is not ready something is wrong
						if(!vClass->IsReady()){
							// check if the base class does not exist
//							if(vClass->GetTypeBase()){
//								if(!ExistsClass(vClass->GetTypeBase())){
//									ErrorClassNotExist(vClass->GetNode(), vClass->GetTypeBase());
//									// remove class and go on
//									p_RemoveClass(i); i--; vCurCount--;
//								}
//							}
							// check for errors in class construction
							if(vClass->CheckForErrors(this)){
								p_RemoveClass(i); i--; vCurCount--;
							}
						}
					}
					if(vCurCount == vLastCount){
						// if there is no more class existing having a missing
						// base class then we must have an inheritance loop.
						// print out all remaining classes as erronous looping.
						for(i=0; i<p_ClassCount; i++){
							vClass = p_Classes[i];
							p_curScript = vClass->GetScript();
							if(!vClass->IsReady()){
								ErrorInheritanceLoop(vClass);
								// remove class and go on
								p_RemoveClass(i); i--;
							}
						}
						vCurCount = 0; break;
					}
				}
				// check for missing interface classes
				
				break;
			}
		}
		// now replace the list of classes with the new one we build.
		// this list is now in the correct order for future processing.
		// no matter if we had an error or not this list contains valid
		// classes.
		delete [] p_Classes;
		p_Classes = newClassList;
		p_ClassCount = newClassListCount;
	}catch( ... ){
		if(newClassList) delete [] newClassList;
		throw;
	}
}
void dspParserCheck::p_AddScript(dspParserCheckScript *Script){
	if(!Script) DSTHROW(dueInvalidParam);
	dspParserCheckScript **vNewArray = new dspParserCheckScript*[p_ScriptCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_Scripts){
		for(int i=0; i<p_ScriptCount; i++) vNewArray[i] = p_Scripts[i];
		delete [] p_Scripts;
	}
	vNewArray[p_ScriptCount] = Script;
	p_Scripts = vNewArray;
	p_ScriptCount++;
}
