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
#include <string.h>
#include "../dragonscript_config.h"
#include "dspNodes.h"
#include "dspParserCheckCode.h"
#include "dspParserCompileCode.h"
#include "../dsEngine.h"
#include "../dsScriptSource.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsFunction.h"
#include "../objects/dsFuncList.h"
#include "../objects/dsSignature.h"

// function argument node
///////////////////////////
dspNodeFuncArg::dspNodeFuncArg(dspBaseNode *RefNode, const char *Name, dspBaseNode *Type, dspBaseNode *DefValue) : dspBaseNode(ntFuncArg,RefNode){
	if(!Name || !Type) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size);
	#endif
	p_Name[ size ] = 0;
	p_Type = Type; p_DefVal = DefValue;
}
dspNodeFuncArg::~dspNodeFuncArg(){
	delete [] p_Name; delete p_Type; if(p_DefVal) delete p_DefVal;
}
#ifndef __NODBGPRINTF__
void dspNodeFuncArg::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Function Argument '%s'", p_Name);
	p_PostPrintDebug();
	p_Type->DebugPrint(Level+1, "(type) ");
	if(p_DefVal) p_DefVal->DebugPrint(Level+1, "(defval) ");
}
#endif


// class init constructor call
////////////////////////////////
dspNodeInitConstrCall::dspNodeInitConstrCall(dspBaseNode *RefNode) : dspBaseNode(ntInitConstrCall,RefNode){
	p_DSClsFunc = NULL;
	if(RefNode->GetNodeType() == ntThis){
		p_Mode = true;
	}else{ // super and all the rest
		p_Mode = false;
	}
}
dspNodeInitConstrCall::~dspNodeInitConstrCall(){
}
void dspNodeInitConstrCall::AddArgument(dspBaseNode *Argument){
	p_Args.AddNode(Argument);
}
bool dspNodeInitConstrCall::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsThis, *vClsSuper, *vClsLookIn;
	bool vSuccess=true;
	int vSigID;
	// build signature
	dsSignature vFuncSig, *vFoundFuncSig;
	dsFuncList *vFuncList;
	dspBaseNode **vCurNode;
	dspListNodeIterator vArgItr(&p_Args);
	while(vArgItr.HasNext()){
		vCurNode = vArgItr.GetNext();
		if((*vCurNode)->CheckCode(CheckCode, vCurNode)){
			if(CheckCode->IsObject()){
				vFuncSig.AddParameter(CheckCode->GetTypeObject());
			}else{
				CheckCode->ErrorInvFuncArg(*vCurNode);
				vSuccess = false;
			}
		}else vSuccess = false;
	}
	if(!vSuccess) return false;
	// determine function to call
	vClsThis = CheckCode->GetOwnerClass();
	vClsSuper = vClsThis->GetBaseClass();
	vClsLookIn = p_Mode ? vClsThis : vClsSuper;
	vFuncList = vClsLookIn->GetFuncList();
	p_DSClsFunc = vFuncList->FindBestFunction(vClsLookIn, DSFUNC_CONSTRUCTOR, &vFuncSig);
	if(!p_DSClsFunc){
		CheckCode->ErrorInvFuncSig(this, vClsLookIn, DSFUNC_CONSTRUCTOR, &vFuncSig); return false;
	}
	// check for access rights
	if(!(CheckCode->CanAccessFunction(vClsThis, vClsThis, p_DSClsFunc))){
		CheckCode->ErrorNoAccessClsFunc(this, p_DSClsFunc);
		p_DSClsFunc = NULL; return false;
	}
	// auto-cast arguments to match selected function
	vArgItr.Rewind();
	vFoundFuncSig = p_DSClsFunc->GetSignature();
	vSigID = 0;
	while(vArgItr.HasNext()){
		vCurNode = vArgItr.GetNext();
		if(!CheckCode->AutoNodeCast(vCurNode, vFuncSig.GetParameter(vSigID), vFoundFuncSig->GetParameter(vSigID))) return false;
		vSigID++;
	}
	// set infos and go home
	CheckCode->SetTypeObject(p_DSClsFunc->GetType(), false);
	return true;
}
void dspNodeInitConstrCall::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCode code;
	// process arguments in reverse order
	dspBaseNode **vRevNodeList = NULL;
	int i, vNodeCount = 0;
	vNodeCount = p_Args.GetNodeCount();
	try{
		if(!(vRevNodeList = new dspBaseNode*[vNodeCount])) DSTHROW(dueOutOfMemory);
		dspListNodeIterator vArgItr(&p_Args);
		for(i=vNodeCount-1; i>=0; i--){
			vRevNodeList[i] = *vArgItr.GetNext();
		}
		for(i=0; i<vNodeCount; i++){
			vRevNodeList[i]->CompileCode(CompileCode);
			code.code = dsByteCode::ebcPush;
			CompileCode->AddCode( code, GetLineNum() );
		}
		delete [] vRevNodeList;
	}catch( ... ){
		if(vRevNodeList) delete [] vRevNodeList;
		throw;
	}
	code.code = p_Mode ? dsByteCode::ebcThis : dsByteCode::ebcSuper;
	CompileCode->AddCode( code, GetLineNum() );
	
	dsByteCode::sCodeDirectCall codeDCall;
	codeDCall.code = dsByteCode::ebcDCall;
	codeDCall.function = p_DSClsFunc;
	CompileCode->AddCode( codeDCall, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeInitConstrCall::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Init Constructor Call");
	p_PostPrintDebug();
	dspListNodeIterator vArgItr(&p_Args);
	while(vArgItr.HasNext()){
		(*vArgItr.GetNext())->DebugPrint(Level+1, "(arg) ");
	}
}
#endif


// class function node
////////////////////////
dspNodeClassFunction::dspNodeClassFunction( dspBaseNode *RefNode, const char *Name,
int FuncType, int TypeMods, dspBaseNode *Type) :
dspBaseNode( ntClassFunc, RefNode )
{
	if(!Name) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size);
	#endif
	p_Name[ size ] = 0;
	if(Type){
		p_Type = Type;
	}else{
		if(!(p_Type = new dspNodeMembVar(RefNode, "void"))) DSTHROW(dueOutOfMemory);
	}
	p_FuncType = FuncType; p_TypeMods = TypeMods;
	p_InitConstr = NULL;
	p_DSType = NULL;
	p_Flags = 0;
	if(!(p_Sta = new dspNodeEmptySta(RefNode))) DSTHROW(dueOutOfMemory);
}
dspNodeClassFunction::~dspNodeClassFunction(){
	delete [] p_Name; delete p_Sta; if(p_Type) delete p_Type;
	if(p_InitConstr) delete p_InitConstr;
}
void dspNodeClassFunction::AddArgument(dspNodeFuncArg *Argument){
	p_Args.AddNode(Argument);
}
void dspNodeClassFunction::SetStatement(dspBaseNode *Statement){
	if(!Statement) DSTHROW(dueInvalidParam);
	delete p_Sta; p_Sta = Statement;
}
void dspNodeClassFunction::SetDSType(dsClass *Class){
	if(!Class) DSTHROW(dueInvalidParam);
	p_DSType = Class;
}
void dspNodeClassFunction::SetInitConstrCall(dspNodeInitConstrCall *Init){
	if(p_FuncType != DSFT_CONSTRUCTOR) DSTHROW(dueInvalidParam);
	if(!p_InitConstr) delete p_InitConstr;
	p_InitConstr = Init;
}
bool dspNodeClassFunction::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode) DSTHROW(dueInvalidParam);
	dsFuncList *vFuncList;
	dsFunction *vConstrFunc=NULL;
	dsSignature vFuncSig;
//	const char *vConstrName=NULL;
	dsClass *vRefClass;
	// check init function if constructor
	if(p_FuncType == DSFT_CONSTRUCTOR){
		vRefClass = CheckCode->GetOwnerClass()->GetBaseClass();
//		if(!vRefClass->IsEqualTo(CheckCode->GetEngine()->GetClassObject())){
			if(!p_InitConstr){
				vFuncList = vRefClass->GetFuncList();
				vConstrFunc = vFuncList->FindBestFunction(vRefClass, DSFUNC_CONSTRUCTOR, &vFuncSig);
				/*
				for(int i=0; i<vFuncList->GetCount(); i++){
					vCurFunc = vFuncList->GetFunction(i);
					if(vCurFunc->GetFuncType() != DSFT_CONSTRUCTOR) continue;
					if(vCurFunc->GetSignature()->GetCount() > 0) continue;
					if(vConstrName){
						CheckCode->ErrorAmbigDefConstrFunc(this, vRefClass->GetName()); return false;
					}else{
						vConstrName = vCurFunc->GetName();
					}
				}
				if(!vConstrName){
					CheckCode->ErrorNoDefConstrFunc(this, vRefClass->GetName()); return false;
				}
				*/
				if(!vConstrFunc){
					CheckCode->ErrorNoDefConstrFunc(this, vRefClass->GetName()); return false;
				}
				if(!(p_InitConstr = new dspNodeInitConstrCall(this))) DSTHROW(dueOutOfMemory);
			}
			if(!p_InitConstr->CheckCode(CheckCode, (dspBaseNode**)&p_InitConstr)) return false;
//		}
	}
	
	// check body
	if(!p_Sta->CheckCode(CheckCode, &p_Sta)) return false;
	// add return statement if void function and none existing already
	if(p_DSType->GetPrimitiveType()==DSPT_VOID && !CheckCode->GetHasReturn()) p_Flags = 1;
	// finished
	return true;
}
void dspNodeClassFunction::CompileCode(dspParserCompileCode *CompileCode){
	int vCount = CompileCode->GetLocVarCount();
	if(p_InitConstr) p_InitConstr->CompileCode(CompileCode);
	p_Sta->CompileCode(CompileCode);
	CompileCode->DecLocVarCount( vCount, GetLineNum() );
	if(p_Flags == 1){
		dsByteCode::sCode code;
		code.code = dsByteCode::ebcRet;
		CompileCode->AddCode( code, GetLineNum() );
	}
	CompileCode->BuildByteCode();
}
#ifndef __NODBGPRINTF__
void dspNodeClassFunction::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	if(p_FuncType == DSFT_FUNCTION){
		printf("- Class Function '%s' (TypeModifiers %i)", p_Name, p_TypeMods);
	}else if(p_FuncType == DSFT_CONSTRUCTOR){
		printf("- Class Constructor '%s' (TypeModifiers %i)", p_Name, p_TypeMods);
	}else if(p_FuncType == DSFT_DESTRUCTOR){
		printf("- Class Destructor '%s' (TypeModifiers %i)", p_Name, p_TypeMods);
	}else{
		printf("- Class Operator '%s' (TypeModifiers %i)", p_Name, p_TypeMods);
	}
	p_PostPrintDebug();
	if(p_Type) p_Type->DebugPrint(Level+1, "(type) ");
	dspListNodeIterator vArgItr(&p_Args);
	while(vArgItr.HasNext()){
		(*vArgItr.GetNext())->DebugPrint(Level+1, "(arg) ");
	}
	if(p_InitConstr) p_InitConstr->DebugPrint(Level+1, "(init) ");
	p_Sta->DebugPrint(Level+1, "(body) ");
}
#endif


// class variable node
////////////////////////
dspNodeClassVariableList::dspNodeClassVariableList(dspBaseNode *RefNode, int TypeMods, dspBaseNode *Type) : dspBaseNode(ntClassVarList,RefNode){
	if(!Type) DSTHROW(dueInvalidParam);
	p_Type = Type; p_TypeMods = TypeMods;
	pTypeClass = NULL;
}
dspNodeClassVariableList::~dspNodeClassVariableList(){
	delete p_Type;
}
void dspNodeClassVariableList::AddVariable(dspNodeClassVariable *Variable){
	p_Vars.AddNode(Variable);
}
#ifndef __NODBGPRINTF__
void dspNodeClassVariableList::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Class Variables (TypeModifiers %i)", p_TypeMods);
	p_PostPrintDebug();
	p_Type->DebugPrint(Level+1, "(type) ");
	dspListNodeIterator vVarItr(&p_Vars);
	while(vVarItr.HasNext()){
		(*vVarItr.GetNext())->DebugPrint(Level+1, "");
	}
}
#endif

dspNodeClassVariable::dspNodeClassVariable(dspNodeIdent *IdentNode, dspBaseNode *Init) : dspBaseNode(ntClassVar,IdentNode){
	const int size = ( int )strlen( IdentNode->GetName() );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, IdentNode->GetName(), size );
	#else
		strncpy(p_Name, IdentNode->GetName(), size);
	#endif
	p_Name[ size ] = 0;
	p_Init = Init;
	pTypeClass = NULL;
}
dspNodeClassVariable::~dspNodeClassVariable(){
	delete [] p_Name;
	if(p_Init) delete p_Init;
}
void dspNodeClassVariable::CreateInit(dspNodeStaList *StaList, dsVariable *Variable){
	if(!StaList || !Variable || !p_Init) DSTHROW(dueInvalidParam);
	dspNodeSetStaVar *vSetStaVar=NULL;
	try{
		if(!(vSetStaVar = new dspNodeSetStaVar(p_Init, Variable))) DSTHROW(dueOutOfMemory);
		p_Init = NULL;
		StaList->AddNode(vSetStaVar);
	}catch( ... ){
		if(vSetStaVar) delete vSetStaVar;
		throw;
	}
}
bool dspNodeClassVariable::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !pTypeClass) DSTHROW(dueInvalidParam);
	dsClass *vInitType;
	// check init value
	if(p_Init){
		if(!(vInitType = CheckCode->CheckForObject(&p_Init))) return false;
		if(!CheckCode->AutoNodeCast(&p_Init, vInitType, pTypeClass)) return false;
	}
	// finished
	return true;
}
#ifndef __NODBGPRINTF__
void dspNodeClassVariable::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Name '%s'", p_Name);
	p_PostPrintDebug();
	if(p_Init) p_Init->DebugPrint(Level+1, "(init) ");
}
#endif


// class enumeration node
///////////////////////////
dspNodeEnumeration::dspNodeEnumeration(dspBaseNode *RefNode, const char *Name, int TypeMods) :
dspNodeClass( RefNode, Name, DSCT_CLASS, TypeMods ){
}
dspNodeEnumeration::~dspNodeEnumeration(){
}
#ifndef __NODBGPRINTF__
void dspNodeEnumeration::DebugPrint(int Level, const char *Prefix){
	dspNodeClass::DebugPrint(Level, Prefix);
}
#endif


// class node
///////////////
dspNodeClass::dspNodeClass(dspBaseNode *RefNode, const char *Name, int ClassType, int TypeMods) : dspBaseNode(ntClass,RefNode){
	if(!Name) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size);
	#endif
	p_Name[ size ] = 0;
	p_BaseClass = NULL;
	p_ClsType = ClassType;
	p_TypeMods = TypeMods;
	if( ClassType == DSCT_INTERFACE ) p_TypeMods |= DSTM_ABSTRACT;
	if(!(p_InitSta = new dspNodeStaList(RefNode))) DSTHROW(dueOutOfMemory);
}
dspNodeClass::~dspNodeClass(){
	delete [] p_Name; if(p_BaseClass) delete p_BaseClass;
	delete p_InitSta;
}
void dspNodeClass::SetBaseClass(dspBaseNode *Class){
	if(p_BaseClass) delete p_BaseClass;
	p_BaseClass = Class;
}
void dspNodeClass::AddImplClass(dspBaseNode *Class){
	p_ImplClasses.AddNode(Class);
}
void dspNodeClass::AddMember(dspBaseNode *Member){
	p_Members.AddNode(Member);
}
#ifndef __NODBGPRINTF__
void dspNodeClass::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	if(p_ClsType == DSCT_INTERFACE){
		printf("- Interface '%s' (TypeModifiers %i", p_Name, p_TypeMods);
	}else{
		printf("- Class '%s' (TypeModifiers %i", p_Name, p_TypeMods);
	}
	p_PostPrintDebug();
	if(p_BaseClass) p_BaseClass->DebugPrint(Level+1, "(base-class) ");
	dspListNodeIterator vImplItr(&p_ImplClasses);
	while(vImplItr.HasNext()){
		(*vImplItr.GetNext())->DebugPrint(Level+1, "(implements) ");
	}
	dspListNodeIterator vMembItr(&p_Members);
	while(vMembItr.HasNext()){
		(*vMembItr.GetNext())->DebugPrint(Level+1, "(member) ");
	}
}
#endif


// pin node
/////////////
dspNodePin::dspNodePin(dspBaseNode *RefNode, dspBaseNode *Namespace) : dspBaseNode(ntPin,RefNode){
	p_Namespace = Namespace;
}
dspNodePin::~dspNodePin(){
	if(p_Namespace) delete p_Namespace;
}
#ifndef __NODBGPRINTF__
void dspNodePin::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Pin");
	p_PostPrintDebug();
	if(p_Namespace) p_Namespace->DebugPrint(Level+1, "");
}
#endif


// namespace node
///////////////////
dspNodeNamespace::dspNodeNamespace(dspBaseNode *RefNode, dspBaseNode *Namespace) : dspBaseNode(ntNamespace,RefNode){
	p_Namespace = Namespace;
}
dspNodeNamespace::~dspNodeNamespace(){
	if(p_Namespace) delete p_Namespace;
}
#ifndef __NODBGPRINTF__
void dspNodeNamespace::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Namespace");
	p_PostPrintDebug();
	if(p_Namespace) p_Namespace->DebugPrint(Level+1, "");
}
#endif


// option node
dspNodeOption::dspNodeOption(dspBaseNode *RefNode, const char *Key, const char *Value) : dspBaseNode(ntOption,RefNode){
	if(!Key || Key[0]=='\0' || !Value) DSTHROW(dueInvalidParam);

	int size = ( int )strlen( Key );
	if(!(p_Key = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Key, size + 1, Key, size );
	#else
		strncpy(p_Key, Key, size);
	#endif
	p_Key[ size ] = 0;

	size = ( int )strlen( Value );
	if(!(p_Value = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Value, size + 1, Value, size );
	#else
		strncpy(p_Value, Value, size);
	#endif
	p_Value[ size ] = 0;
}
dspNodeOption::~dspNodeOption(){
	delete [] p_Key; delete [] p_Value;
}
#ifndef __NODBGPRINTF__
void dspNodeOption::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Option '%s' = '%s'", p_Key, p_Value);
	p_PostPrintDebug();
}
#endif


// requires node
//////////////////
dspNodeRequires::dspNodeRequires(dspBaseNode *RefNode, const char *Package) : dspBaseNode(ntRequires,RefNode){
	if(!Package) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Package );
	if(!(p_Package = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Package, size + 1, Package, size );
	#else
		strncpy(p_Package, Package, size);
	#endif
	p_Package[ size ] = 0;
}
dspNodeRequires::~dspNodeRequires(){
	delete [] p_Package;
}
#ifndef __NODBGPRINTF__
void dspNodeRequires::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Requires Package '%s'", p_Package);
	p_PostPrintDebug();
}
#endif


// includes node
//////////////////
dspNodeIncludes::dspNodeIncludes(dspBaseNode *RefNode, const char *Package) : dspBaseNode(ntIncludes,RefNode){
	if(!Package) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Package );
	if(!(p_Package = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Package, size + 1, Package, size );
	#else
		strncpy(p_Package, Package, size);
	#endif
	p_Package[ size ] = 0;
}
dspNodeIncludes::~dspNodeIncludes(){
	delete [] p_Package;
}
#ifndef __NODBGPRINTF__
void dspNodeIncludes::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Includes Package '%s'", p_Package);
	p_PostPrintDebug();
}
#endif


// script node
////////////////
dspNodeScript::dspNodeScript(dsScriptSource *Source) : dspBaseNode(ntStaList,Source,1,1){ }
dspNodeScript::~dspNodeScript(){ }
void dspNodeScript::AddNode(dspBaseNode *Node){
	p_Nodes.AddNode(Node);
}
#ifndef __NODBGPRINTF__
void dspNodeScript::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Script '%s'", GetSource()->GetName());
	p_PostPrintDebug();
	dspListNodeIterator vNodeItr(&p_Nodes);
	while(vNodeItr.HasNext()){
		(*vNodeItr.GetNext())->DebugPrint(Level+1, "");
	}
}
#endif

