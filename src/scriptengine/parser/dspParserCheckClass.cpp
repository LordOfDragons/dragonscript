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
#include "dspParser.h"
#include "dspParserCheck.h"
#include "dspParserCheckCode.h"
#include "dspParserCheckType.h"
#include "dspParserCheckClass.h"
#include "dspParserCheckFunction.h"
#include "dspParserCheckConstant.h"
#include "dspParserCheckInterface.h"
#include "dspParserCompileCode.h"
#include "dspParserCheckScript.h"
#include "dspOptimizeFunction.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsClassParserInfo.h"
#include "../objects/dsFunction.h"
#include "../objects/dsFuncList.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsVariable.h"
#include "../objects/dsSignature.h"
#include "../packages/default/dsClassEnumeration.h"


// dspParserCheckClass
////////////////////////
// constructor, destructor
dspParserCheckClass::dspParserCheckClass(dspParserCheckScript *Script, dspNodeClass *Node, dspParserCheckType *TypeParent, dspParserCheckType *TypeBase){
	if(!Script || !Node) DSTHROW(dueInvalidParam);
	dspParserCheckType *typeIFace = NULL;
	dspListNodeIterator vImplItr(Node->GetImplList());
	dspBaseNode *vCurNode;
	int i;
	// reset
	p_Script = Script;
	p_natErrNode = NULL;
	p_Node = Node;
	p_TypeClass = NULL;
	p_TypeParent = NULL;
	p_TypeBase = TypeBase;
	p_ClassNew = NULL;
	p_Functions = NULL;
	p_FuncCount = 0;
	p_Constants = NULL;
	p_ConstCount = 0;
	p_Interfaces = NULL;
	p_IFaceCount = p_Node->GetImplList()->GetNodeCount();
	p_ready = false;
	try{
		p_TypeClass = new dspParserCheckType(Node->GetName(), TypeParent);
		if(!p_TypeClass) DSTHROW(dueOutOfMemory);
		if(TypeParent){
			p_TypeParent = TypeParent->Copy();
		}
		if(p_IFaceCount > 0){
			p_Interfaces = new dspParserCheckInterface*[p_IFaceCount];
			if(!p_Interfaces) DSTHROW(dueOutOfMemory);
			for(i=0; i<p_IFaceCount; i++) p_Interfaces[i] = NULL;
			for(i=0; i<p_IFaceCount; i++){
				vCurNode = *vImplItr.GetNext();
				typeIFace = dspParserCheckType::GetTypeFromNode(vCurNode);
				p_Interfaces[i] = new dspParserCheckInterface(vCurNode, typeIFace);
				if(!p_Interfaces[i]) DSTHROW(dueOutOfMemory);
				typeIFace = NULL;
			}
		}
		
	}catch( ... ){
		if(typeIFace) delete typeIFace;
		if(p_TypeClass) delete p_TypeClass;
		if(p_TypeParent) delete p_TypeParent;
		if(p_TypeBase) delete p_TypeBase;
		if(p_Interfaces){
			for(int i=0; i<p_IFaceCount; i++){
				if(p_Interfaces[i]) delete p_Interfaces[i];
			}
			delete [] p_Interfaces;
		}
		throw;
	}	
}
dspParserCheckClass::dspParserCheckClass(dsClass *nativeClass){
	if(!nativeClass) DSTHROW(dueInvalidParam);
	dsClassParserInfo *pinfo = nativeClass->GetParserInfo();
	dspParserCheckType *typeIFace = NULL;
	int i;
	// reset
	p_Script = NULL;
	p_Node = NULL;
	p_natErrNode = NULL;
	p_TypeClass = NULL;
	p_TypeParent = NULL;
	p_TypeBase = NULL;
	p_ClassNew = NULL;
	p_Functions = NULL;
	p_FuncCount = 0;
	p_Constants = NULL;
	p_ConstCount = 0;
	p_Interfaces = NULL;
	p_IFaceCount = pinfo->GetInterfaceCount();
	p_ready = false;
	// build types from parser info of native class
	try{
		// parent
		p_TypeParent = dspParserCheckType::GetTypeFromFullName(pinfo->GetParent());
		// base class
		p_TypeBase = dspParserCheckType::GetTypeFromFullName(pinfo->GetBase());
		// our class
		p_TypeClass = new dspParserCheckType(nativeClass->GetName(), p_TypeParent);
		if(!p_TypeClass) DSTHROW(dueOutOfMemory);
		// native error node
		p_natErrNode = new dspNodeNatClassErr;
		if(!p_natErrNode) DSTHROW(dueOutOfMemory);
		// create list of interfaces
		if(p_IFaceCount > 0){
			p_Interfaces = new dspParserCheckInterface*[p_IFaceCount];
			if(!p_Interfaces) DSTHROW(dueOutOfMemory);
			for(i=0; i<p_IFaceCount; i++) p_Interfaces[i] = NULL;
			for(i=0; i<p_IFaceCount; i++){
				typeIFace = dspParserCheckType::GetTypeFromFullName(pinfo->GetInterface(i));
				p_Interfaces[i] = new dspParserCheckInterface(p_natErrNode, typeIFace);
				if(!p_Interfaces[i]) DSTHROW(dueOutOfMemory);
				typeIFace = NULL;
			}
		}
		// now store the class for later
		p_ClassNew = nativeClass;
	}catch( ... ){
		if(typeIFace) delete typeIFace;
		if(p_TypeParent) delete p_TypeParent;
		if(p_TypeClass) delete p_TypeClass;
		if(p_TypeBase) delete p_TypeBase;
		if(p_Interfaces){
			for(int i=0; i<p_IFaceCount; i++){
				if(p_Interfaces[i]) delete p_Interfaces[i];
			}
			delete [] p_Interfaces;
		}
		throw;
	}
}
dspParserCheckClass::~dspParserCheckClass(){
	if(p_TypeParent) delete p_TypeParent;
	if(p_TypeClass) delete p_TypeClass;
	if(p_TypeBase) delete p_TypeBase;
	if(p_Interfaces){
		for(int i=0; i<p_IFaceCount; i++){
			if(p_Interfaces[i]) delete p_Interfaces[i];
		}
		delete [] p_Interfaces;
	}
	if(p_Functions){
		for(int i=0; i<p_FuncCount; i++) delete p_Functions[i];
		delete [] p_Functions;
	}
	if(p_Constants){
		for(int i=0; i<p_ConstCount; i++) delete p_Constants[i];
		delete [] p_Constants;
	}
	if(!p_ready && p_ClassNew) delete p_ClassNew;
	if(p_natErrNode) delete p_natErrNode;
}

// management
void dspParserCheckClass::CreateClass(dspParserCheck *ParserCheck){
	if(!ParserCheck) DSTHROW(dueInvalidParam);
	dsEngine *vEngine = ParserCheck->GetEngine();
	dsClass *clsBase=NULL, *clsParent=NULL;
	dspBaseNode *errNode = p_Node ? (dspBaseNode*)p_Node : (dspBaseNode*)p_natErrNode;
	int i, j;
	// create class if not already done
	if(!p_ClassNew){
		p_ClassNew = new dsClass(p_Node->GetName(), p_Node->GetClassType(), p_Node->GetTypeMods());
		if(!p_ClassNew) DSTHROW(dueOutOfMemory);
	}
	// check class type and base class type
	if(p_TypeParent){
		if(p_natErrNode){
			ParserCheck->CreateNS(p_natErrNode, p_TypeParent);
		}
		clsParent = ParserCheck->GetClassFromType(p_TypeParent, NULL);
		if(!clsParent) return;
	}
	if(p_TypeBase){
		clsBase = ParserCheck->GetClassFromType(p_TypeBase, clsParent);
		if(!clsBase) return;
	}
	// check if base class is fixed and thus can not be extended
	if(clsBase && (clsBase->GetTypeModifiers() & DSTM_FIXED)){
		ParserCheck->ErrorClassIsFixed(errNode, p_TypeBase); DSTHROW(dseInvalidSyntax);
	}
	// try finding all interfaces listed
	for(i=0; i<p_IFaceCount; i++){
		if(!p_Interfaces[i]->GetClass()){
			p_Interfaces[i]->SetClass( ParserCheck->GetClassFromType(p_Interfaces[i]->GetType(), clsParent) );
			if(!p_Interfaces[i]->GetClass()) return;
		}
	}
	// check if no interface is duplicate and if they are really interfaces
	for(i=0; i<p_IFaceCount; i++){
		for(j=0; j<p_IFaceCount; j++){
			if(i == j) continue;
			if(p_Interfaces[i]->GetClass()->IsSubClassOf(p_Interfaces[j]->GetClass())) return;
		}
		if(p_Interfaces[i]->GetClass()->GetClassType() != DSCT_INTERFACE) return;
	}
	// check if the base class is a class
	if(clsBase){
		if(clsBase->GetClassType() != DSCT_CLASS) return;
	}
	// set base class
	if(clsBase){
		p_ClassNew->SetBaseClass(clsBase);
	}else{
		p_ClassNew->SetBaseClass(vEngine->GetClassObject());
	}
	// add interfaces
	for(i=0; i<p_IFaceCount; i++){
		p_ClassNew->AddImplement(p_Interfaces[i]->GetClass());
	}
	// add class
	if(clsParent){
		clsParent->AddInnerClass(p_ClassNew);
	}else{
		vEngine->p_AddClass(p_ClassNew);
	}
	ParserCheck->GetParser()->p_AddParsedClass(p_ClassNew);
	p_ready = true;
}
void dspParserCheckClass::CreateClassMembers(dspParserCheck *ParserCheck){
	if(!ParserCheck) DSTHROW(dueInvalidParam);
	dspBaseNode *vCurNode;
	dsEngine *vEngine = ParserCheck->GetEngine();
	// create class members
	p_ClassNew->CreateClassMembers(vEngine);
	if(p_ClassNew->GetBaseClass() == vEngine->GetClassEnumeration()){
		((dsClassEnumeration*)vEngine->GetClassEnumeration())->GenerateMethods(*vEngine, *p_ClassNew);
	}
	if(!p_natErrNode){
		dspListNodeIterator vMembItr(p_Node->GetMemberList());
		while(vMembItr.HasNext()){
			vCurNode = *vMembItr.GetNext();
			switch(vCurNode->GetNodeType()){
			case ntClassFunc:
				p_CreateClassFunction(ParserCheck, (dspNodeClassFunction*)vCurNode);
				break;
			case ntClassVarList:
				p_CreateClassVariable(ParserCheck, (dspNodeClassVariableList*)vCurNode);
				break;
			}
		}
		if(ParserCheck->HasErrors()) return;
	}
	p_ClassNew->CreateStaFunc(vEngine);
	// temporary
#ifndef __NODBGPRINTF__
//	p_Node->GetInitSta()->DebugPrint(0, "");
#endif
}
bool dspParserCheckClass::CheckForErrors(dspParserCheck *ParserCheck){
	if(!ParserCheck) DSTHROW(dueInvalidParam);
//	dsClass *clsParent=p_ClassNew->GetParent();
	dsClass *clsBase=NULL, *clsParent=NULL, *clsA, *clsB;
	dspBaseNode *node = p_natErrNode ? (dspBaseNode*)p_natErrNode : p_Node;
	bool failure = false;
	int i, j;
	// sanity check. if something is wrong here we are fuxored
	if(!p_ClassNew) DSTHROW(dueInvalidAction);
	// get informations so we can do our checks
	if(p_TypeParent){
		clsParent = ParserCheck->GetClassFromType(p_TypeParent, NULL);
	}
	if(p_TypeBase){
		clsBase = ParserCheck->GetClassFromType(p_TypeBase, clsParent);
	}
	// check if the base class does not exist. this checks if
	// the class is has been found or is inside one of the scripts.
	if(!clsBase && p_TypeBase && !ParserCheck->ExistsClass(p_TypeBase)){
		ParserCheck->ErrorClassNotExist(node, p_TypeBase);
		failure = true;
	}
	// check if the base class is not an interface
	if(clsBase){
		if(clsBase->GetClassType() != DSCT_CLASS){
			ParserCheck->ErrorBaseNotSameType(node, clsBase, p_ClassNew);
			failure = true;
		}
	}
	// check if interfaces are missing
	for(i=0; i<p_IFaceCount; i++){
		if(!p_Interfaces[i]->GetClass()){
			ParserCheck->ErrorClassNotExist(p_Interfaces[i]->GetNode(), p_Interfaces[i]->GetType());
			failure = true;
		}
	}
	// check if an interface is not an interface in fact
	for(i=0; i<p_IFaceCount; i++){
		if(!p_Interfaces[i]->GetClass()) continue;
		if(p_Interfaces[i]->GetClass()->GetClassType() != DSCT_INTERFACE){
			ParserCheck->ErrorNotInterface(p_Interfaces[i]->GetNode(), p_Interfaces[i]->GetClass());
			failure = true;
		}
	}
	// check if no interface is duplicate
	for(i=0; i<p_IFaceCount; i++){
		clsA = p_Interfaces[i]->GetClass();
		if(!clsA) continue;
		for(j=i+1; j<p_IFaceCount; j++){
			clsB = p_Interfaces[j]->GetClass();
			if(!clsB) continue;
			if(clsA->IsSubClassOf(clsB)){
				ParserCheck->ErrorDupImplement(p_Interfaces[i]->GetNode(), p_ClassNew, clsB);
				failure = true;
			}else if(clsB->IsSubClassOf(clsA)){
				ParserCheck->ErrorDupImplement(p_Interfaces[j]->GetNode(), p_ClassNew, clsA);
				failure = true;
			}
		}
	}
	// return what we found
	return failure;
}
void dspParserCheckClass::CheckFunctionCode(dspParserCheck *ParserCheck){
	if(!ParserCheck) DSTHROW(dueInvalidParam);
	dspParserCheckFunction *vCheckFunc;
	dspNodeClassFunction *vFuncNode;
	dsFunction *vFunc;
	int i;
	// only if a class and not native
	if(p_ClassNew->GetClassType() != DSCT_CLASS) return;
	if(p_natErrNode) return;
	// check init function if present
	if(p_ClassNew->GetInitStaFunc()){
		dspNodeStaList **vStaNode = p_Node->GetInitSta();
		dspParserCheckFunction vCheckFunc(p_Script->GetScript(), p_ClassNew->GetInitStaFunc());
		dspParserCheckCode vCheckCode(ParserCheck, &vCheckFunc);
		(*vStaNode)->CheckCode(&vCheckCode, (dspBaseNode**)vStaNode);
		p_ClassNew->GetInitStaFunc()->SetLocVarSize(vCheckCode.GetMaxLocVarCount());
	}
	// check function code for errors
	for(i=0; i<p_FuncCount; i++){
		// prepare for check
		vCheckFunc = p_Functions[i];
		dspParserCheckCode vCheckCode(ParserCheck, vCheckFunc);
		vFuncNode = vCheckFunc->GetFuncNode();
		vFunc = vCheckFunc->GetFunction();
		// ignore abstract functions, they have no body
		if(vFunc->GetTypeModifiers() & DSTM_ABSTRACT) continue;
		// check function code
		if(!vFuncNode->CheckCode(&vCheckCode, ( dspBaseNode** )&vFuncNode)) continue;
		vFunc->SetLocVarSize(vCheckCode.GetMaxLocVarCount());
		// check for missing return statement
		if(vFunc->GetType()->GetPrimitiveType() != DSPT_VOID){
			if(!vCheckCode.GetHasReturn()){
				ParserCheck->ErrorMissingReturn(vFuncNode, vFunc->GetName(), vFunc->GetOwnerClass()->GetName());
			}
		}
	}
}
void dspParserCheckClass::CheckConstants(dspParserCheck *ParserCheck){
	if(!ParserCheck) DSTHROW(dueInvalidParam);
	dspParserCheckConstant *vCheckConst;
	int i;
	// for class and interface only
	if( p_ClassNew->GetClassType() != DSCT_CLASS && p_ClassNew->GetClassType() != DSCT_INTERFACE ) return;
	if(p_natErrNode) return;
	// check constants for errors
	for(i=0; i<p_ConstCount; i++){
		// prepare for check
		vCheckConst = p_Constants[i];
		if(!vCheckConst->GetConstant()){
			vCheckConst->CreateConstant(p_ClassNew, ParserCheck);
		}
	}
}
dsConstant *dspParserCheckClass::CheckConstant(dspParserCheck *ParserCheck, const char *Name){
	if(!ParserCheck || !Name) DSTHROW(dueInvalidParam);
	dspParserCheckConstant *vCheckConst;
	dspNodeClassVariable *vVarNode;
	int i;
	// for class and interface only
	if( p_ClassNew->GetClassType() != DSCT_CLASS && p_ClassNew->GetClassType() != DSCT_INTERFACE ) return NULL;
	if(p_natErrNode) return NULL;
	// check constants for errors
	for(i=0; i<p_ConstCount; i++){
		vCheckConst = p_Constants[i];
		vVarNode = vCheckConst->GetVarNode();
		if(strcmp(vVarNode->GetName(), Name) == 0){
			if(!vCheckConst->GetConstant()){
				vCheckConst->CreateConstant(p_ClassNew, ParserCheck);
			}
			return vCheckConst->GetConstant();
		}
	}
	return NULL;
}

void dspParserCheckClass::CompileFunctionCode(dspParserCheck *ParserCheck){
	if(!ParserCheck) DSTHROW(dueInvalidParam);
	dspParserCheckFunction *vCheckFunc;
	dspNodeClassFunction *vFuncNode;
	int i;
	// only if a class and not native
	if(p_ClassNew->GetClassType() != DSCT_CLASS) return;
	if(p_natErrNode) return;
	// compile
//	try{
		// compile init function if present
		if(p_ClassNew->GetInitStaFunc()){
			dspNodeStaList *vStaNode = *p_Node->GetInitSta();
			dspParserCompileCode vCompileCode(ParserCheck, p_ClassNew, p_ClassNew->GetInitStaFunc()->GetByteCode());
			vStaNode->CompileCode(&vCompileCode);
			dsByteCode::sCode code;
			code.code = dsByteCode::ebcRet;
			vCompileCode.AddCode( code, vStaNode->GetLineNum() );
#ifndef __NODBGPRINTF__
			// debug
//			printf("[INIT]\n");
//			p_ClassNew->GetInitStaFunc()->GetByteCode()->DebugPrint();
#endif
		}
		// compile function code for errors
		for(i=0; i<p_FuncCount; i++){
			vCheckFunc = p_Functions[i];
			vFuncNode = vCheckFunc->GetFuncNode();
			dspParserCompileCode vCompileCode(ParserCheck, vCheckFunc);
			vFuncNode->CompileCode(&vCompileCode); // false on error
#ifndef __NODBGPRINTF__
			// debug
			/*
			if(strcmp(vCheckFunc->GetFunction()->GetName(), "functionName") == 0 &&
			strcmp(vCheckFunc->GetFunction()->GetOwnerClass()->GetName(), "className") == 0){
				printf("[FUNCTION] %s\n", vCheckFunc->GetFunction()->GetName());
				vCheckFunc->GetFunction()->GetByteCode()->DebugPrint();
			}
			*/
#endif
		}
//	catch( ... ){
//		
//	}
}

void dspParserCheckClass::OptimizeFunctionCode( dspParserCheck &parserCheck ){
	if( p_ClassNew->GetClassType() != DSCT_CLASS || p_natErrNode ){
		return;
	}
	
	int i;
	for( i=0; i<p_FuncCount; i++ ){
		dspOptimizeFunction( *p_Functions[ i ] ).Optimize( parserCheck );
	}
}

void dspParserCheckClass::CompileConstants(dspParserCheck *ParserCheck){
	if(!ParserCheck) DSTHROW(dueInvalidParam);
	dspNodeStaList *vInitStaList = NULL;
	const char *vClassName = p_ClassNew->GetName();
	dspParserCheckConstant *vCheckConst;
	dspNodeClassVariable *vVarNode;
	dsVariable *vVariable=NULL;
	bool vSuccess = true;
	int i;
	// for class and interface only
	if( p_ClassNew->GetClassType() != DSCT_CLASS && p_ClassNew->GetClassType() != DSCT_INTERFACE ) return;
	if(p_natErrNode) return;
	// compile
	vInitStaList = *p_Node->GetInitSta();
	try{
		for(i=0; i<p_ConstCount; i++){
			vCheckConst = p_Constants[i];
			if(!vCheckConst->GetConstant()){
				vVarNode = vCheckConst->GetVarNode();
				if(!(vVariable = new dsVariable(p_ClassNew, vVarNode->GetName(), vCheckConst->GetType(), vCheckConst->GetTypeMods()))) DSTHROW(dueOutOfMemory);
				if( ! p_ClassNew->FindVariable( vVariable->GetName(), false ) ){
					vVarNode->CreateInit(vInitStaList, vVariable);
					p_ClassNew->AddVariable(vVariable);
				}else{
					ParserCheck->ErrorDupVariable(vVarNode, vClassName, vVariable->GetName());
					vSuccess = false;
				}
				vVariable = NULL;
			}
		}
		if(!vSuccess) DSTHROW(dseInvalidSyntax);
		
	}catch( const duException &e ){
		if(vVariable) delete vVariable;
		if(e.IsNamed("InvalidSyntax")) throw;
		
	}catch( ... ){
		if(vVariable) delete vVariable;
		throw;
	}
}
// information
bool dspParserCheckClass::MatchesClass(const char *Name, dspParserCheckType *TypeParent) const{
	if(!Name) DSTHROW(dueInvalidParam);
	const char *ourName = p_ClassNew ? p_ClassNew->GetName() : p_Node->GetName();
	if(TypeParent){
		if(!p_TypeParent || !p_TypeParent->MatchesType(TypeParent)) return false;
	}else{
		if(p_TypeParent) return false;
	}
	return strcmp(ourName, Name) == 0;
}
// private functions
void dspParserCheckClass::p_AddFunction(dspParserCheckFunction *Function){
	if(!Function) DSTHROW(dueInvalidParam);
	dspParserCheckFunction **vNewArray = new dspParserCheckFunction*[p_FuncCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_Functions){
		for(int i=0; i<p_FuncCount; i++) vNewArray[i] = p_Functions[i];
		delete [] p_Functions;
	}
	vNewArray[p_FuncCount] = Function;
	p_Functions = vNewArray;
	p_FuncCount++;
}
void dspParserCheckClass::p_AddConstant(dspParserCheckConstant *Constant){
	if(!Constant) DSTHROW(dueInvalidParam);
	dspParserCheckConstant **vNewArray = new dspParserCheckConstant*[p_ConstCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_Constants){
		for(int i=0; i<p_ConstCount; i++) vNewArray[i] = p_Constants[i];
		delete [] p_Constants;
	}
	vNewArray[p_ConstCount] = Constant;
	p_Constants = vNewArray;
	p_ConstCount++;
}
void dspParserCheckClass::p_CreateClassVariable(dspParserCheck *ParserCheck, dspNodeClassVariableList *VarNode){
	dsClass *vTypeClass;
	dsVariable *vVariable=NULL;
	const char *vClassName = p_ClassNew->GetName();
	dspNodeClassVariable *vCurVar;
	dspNodeStaList *vInitStaList = *p_Node->GetInitSta();
	dspParserCheckConstant *vCheckConst=NULL;
	bool vSuccess = true;
	dspListNodeIterator vVarItr(VarNode->GetVarList());
	try{
		vCurVar = *(dspNodeClassVariable**)vVarItr.GetNext();
		vTypeClass = ParserCheck->GetClassFromNode(VarNode->GetTypeNode(), p_ClassNew);
		// check for invalid type
		if(vTypeClass->GetPrimitiveType() == DSPT_VOID){
			ParserCheck->ErrorInvVarTypeVoid(VarNode->GetTypeNode(), vCurVar->GetName()); DSTHROW(dseInvalidSyntax);
		}
		// create all variables
		vVarItr.Rewind();
		while(vVarItr.HasNext()){
			vCurVar = *(dspNodeClassVariable**)vVarItr.GetNext();
			// check if this variable is a constant
			if( (VarNode->GetTypeMods()&DSTM_FIXED)==DSTM_FIXED && 
				vTypeClass->GetPrimitiveType()>=DSPT_BYTE &&
				vTypeClass->GetPrimitiveType()<=DSPT_FLOAT)
			{
				if(!(vCheckConst = new dspParserCheckConstant(vCurVar, vTypeClass, VarNode->GetTypeMods()))) DSTHROW(dueOutOfMemory);
				p_AddConstant(vCheckConst); vCheckConst = NULL;
			}else{
				// create variable
				if(!(vVariable = new dsVariable(p_ClassNew, vCurVar->GetName(), vTypeClass, VarNode->GetTypeMods()))) DSTHROW(dueOutOfMemory);
				// check if the variable can be added to this class
				if( p_ClassNew->FindVariable( vVariable->GetName(), false ) || p_ClassNew->FindConstant( vVariable->GetName(), false ) ){
					ParserCheck->ErrorDupVariable(vCurVar, vClassName, vVariable->GetName());
					vSuccess = false;
				}else{
					// check if the init value is correct
					if(vVariable->GetTypeModifiers() & DSTM_STATIC){
						if(!vCurVar->GetInitNode()){
							ParserCheck->ErrorNoStaVarInit(vCurVar, vClassName); DSTHROW(dseInvalidSyntax);
						}
						vCurVar->CreateInit(vInitStaList, vVariable);
					}else{
						if(vCurVar->GetInitNode()){
							ParserCheck->ErrorInvalidVarInit(vCurVar, vClassName); DSTHROW(dseInvalidSyntax);
						}
					}
					p_ClassNew->AddVariable(vVariable);
				}
			}
			vVariable = NULL;
		}
		if(!vSuccess) DSTHROW(dseInvalidSyntax);
		
	}catch( const duException &e ){
		if(vVariable) delete vVariable;
		if(e.IsNamed("InvalidSyntax")) throw;
		
	}catch( ... ){
		if(vVariable) delete vVariable;
		throw;
	}
}
void dspParserCheckClass::p_CreateClassFunction(dspParserCheck *ParserCheck, dspNodeClassFunction *FuncNode){
	const char *vUnOps[] = {"++","--","+","-","!","~"};
	const char *vBinOps[] = {"+","-","*","/","%","<<",">>","&","|","^","<",">",
		"<=",">=","*=","/=","%=","+=","-=","<<=",">>=","&=","|=","^=","==","!="};
	dspParserCheckFunction *vFunction=NULL;
	dsFunction *vDSFunction=NULL;
	dsFunction *vDSSubFunc;
	dsSignature *vDSSignature;
	const char *vClassName = p_ClassNew->GetName();
	const char *vFuncName;
	int vArgCount;
	try{
		// create function
		if(!(vFunction = new dspParserCheckFunction(p_Script->GetScript(), FuncNode))) DSTHROW(dueOutOfMemory);
		vFunction->CreateFunction(p_ClassNew, ParserCheck);
		vDSFunction = vFunction->GetFunction();
		vDSSignature = vFunction->GetSignature();
		vFuncName = vDSFunction->GetName();
		// check if the function can be added to this class/interface
		if(p_ClassNew->GetClassType() == DSCT_CLASS){
			if(vDSFunction->GetFuncType() == DSFT_OPERATOR){
				vArgCount = vDSSignature->GetCount();
				if( !(p_MatchesOp(vFuncName, &vUnOps[0], 6) && (vArgCount == 0)) &&
					!(p_MatchesOp(vFuncName, &vBinOps[0], 26) && (vArgCount == 1)) ){
					ParserCheck->ErrorInvOpArgCount(vFunction->GetFuncNode(), vFuncName, vClassName); DSTHROW(dseInvalidSyntax);
				}
//				if(p_ClassNew->FindFunction(vDSFunction, false)){
//					ParserCheck->ErrorDupOperator(vFunction->GetFuncNode(), vFuncName, vClassName); DSTHROW(dseInvalidSyntax);
//				}
			}else{
				if(vDSFunction->GetFuncType() == DSFT_DESTRUCTOR){
					if(vDSSignature->GetCount() > 0){
						ParserCheck->ErrorInvDestrSig(vFunction->GetFuncNode(), vFuncName); DSTHROW(dseInvalidSyntax);
					}
					if(p_ClassNew->GetDestructor()){
						ParserCheck->ErrorDupDestructor(vFunction->GetFuncNode(), vClassName); DSTHROW(dseInvalidSyntax);
					}
				}
			}
			if( ( vDSSubFunc = p_ClassNew->FindFunction(vDSFunction, true) ) ){
				if(p_ClassNew->IsEqualTo(vDSSubFunc->GetOwnerClass())){
					ParserCheck->ErrorDupFunction(vFunction->GetFuncNode(), vFuncName, vClassName); DSTHROW(dseInvalidSyntax);
				}
				if(!vDSFunction->GetType()->IsEqualTo(vDSSubFunc->GetType())){
					ParserCheck->ErrorInvDupFuncSig(vFunction->GetFuncNode(), vFuncName, vDSSubFunc->GetOwnerClass()->GetName()); DSTHROW(dseInvalidSyntax);
				}
			}
		}else if(p_ClassNew->GetClassType() == DSCT_INTERFACE){
			if(p_ClassNew->FindFunction(vDSFunction, false)){
				ParserCheck->ErrorDupFunction(vFunction->GetFuncNode(), vFuncName, vClassName); DSTHROW(dseInvalidSyntax);
			}
		}
		// add function to this class
		p_ClassNew->AddFunction(vDSFunction); vDSFunction=NULL;
		p_AddFunction(vFunction);
		// output debug info
//		printf("[FUNC] Function '%s' created (type '%s')\n", FuncNode->GetName(), FuncNode->GetName());
	}catch( ... ){
		if(vFunction) delete vFunction;
		if(vDSFunction) delete vDSFunction;
//		if(e.IsNamed("InvalidSyntax")) throw;
		throw;
	}
}

bool dspParserCheckClass::p_MatchesOp(const char *CheckOp, const char **OpList, int Count){
	for(int i=0; i<Count; i++){
		if(strcmp(CheckOp, OpList[i]) == 0) return true;
	}
	return false;
}

