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
#include "../../config.h"
#include "dspNodes.h"
#include "dspNodesScript.h"
#include "dspParserCheck.h"
#include "dspParserCheckCode.h"
#include "dspParserCompileCode.h"
#include "dspParserCheckFunction.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsConstant.h"
#include "../objects/dsVariable.h"
#include "../objects/dsFunction.h"
#include "../objects/dsFuncList.h"
#include "../objects/dsSignature.h"
#include "../objects/dsLocalVariable.h"


// Member Nodes
/////////////////

// dspNodeMembVar

dspNodeMembVar::dspNodeMembVar( dspNodeIdent *member, dspBaseNode *obj ) : dspBaseNode( ntMembVar, member ){
	p_Name = new char[ strlen( member->GetName() ) + 1 ];
	if( ! p_Name ) DSTHROW( dueOutOfMemory );
	strcpy( p_Name, member->GetName() );
	
	p_Obj = obj;
	p_DSClass = NULL;
	p_DSClsVar = NULL;
	p_DSClsConst = NULL;
	p_ParamID = -1;
	pContextVariable = -1;
	p_DSLocVarIndex = -1;
}

dspNodeMembVar::dspNodeMembVar( dspBaseNode *refnode, const char *name ) : dspBaseNode( ntMembVar, refnode ){
	if( ! name || name[ 0 ] == '\0' ) DSTHROW( dueInvalidParam );
	
	p_Name = new char[ strlen( name ) + 1 ];
	if( ! p_Name ) DSTHROW( dueOutOfMemory );
	strcpy( p_Name, name );
	
	p_Obj = NULL;
	p_DSClass = NULL;
	p_DSClsVar = NULL;
	p_DSClsConst = NULL;
	p_ParamID = -1;
	pContextVariable = -1;
	p_DSLocVarIndex = -1;
}

dspNodeMembVar::~dspNodeMembVar(){
	if( p_Name ) delete [] p_Name;
	if( p_Obj ) delete p_Obj;
}

bool dspNodeMembVar::CheckCode( dspParserCheckCode *checkCode, dspBaseNode **ppThisNode ){
	if( ! checkCode || ! ppThisNode || ! *ppThisNode ) DSTHROW( dueInvalidParam );
	
	dspParserCheckFunction *checkFunc = checkCode->GetCheckFunc();
	dspBaseNode *newNode = NULL;
	dsClass *refClass;
	
	if( p_Obj ){
		if( ! p_Obj->CheckCode( checkCode, &p_Obj ) ){
			return false;
		}
		
		if( checkCode->IsClass() ){
			refClass = checkCode->GetTypeClass();
			p_DSClass = checkCode->GetParserCheck()->GetClassFromName( p_Name, refClass );
			
		}else{
			refClass = checkCode->GetTypeObject();
		}
		
	}else{
		refClass = checkCode->GetOwnerClass();
		p_DSClass = checkCode->GetParserCheck()->GetClassFromName( p_Name, refClass );
		p_DSLocVarIndex = checkCode->FindLocalVariable( p_Name, false );
		pContextVariable = checkCode->AbsoluteIndexOfBlockCVarNamed( p_Name, false );
		
		if( checkFunc ){
			p_ParamID = checkFunc->FindArgument( p_Name );
		}
	}
	
	p_DSClsVar = refClass->FindVariable( p_Name, true );
	p_DSClsConst = checkCode->FindClassConstant( refClass, p_Name );
	
	if( p_ParamID != -1 ){
		if( ! checkFunc ) DSTHROW( dueInvalidParam );
		
		checkCode->SetTypeObject( checkFunc->GetSignature()->GetParameter( p_ParamID ), false );
		
	}else if( p_DSLocVarIndex != -1 ){
		checkCode->SetTypeObject( checkCode->GetLocalVariable( p_DSLocVarIndex, false )->GetType(), false );
		
	}else if( pContextVariable != -1 ){
		checkCode->SetTypeObject( checkCode->GetBlockCVarAbsoluteAt( pContextVariable, false )->GetType(), false );
		
	}else if( p_DSClsConst ){
		if( p_DSClsConst->GetType()->GetPrimitiveType() == DSPT_BYTE ){
			newNode = new dspNodeByte( this, p_DSClsConst->GetByte() );
			
		}else if( p_DSClsConst->GetType()->GetPrimitiveType() == DSPT_BOOL ){
			newNode = new dspNodeBool( this, p_DSClsConst->GetBool() );
			
		}else if( p_DSClsConst->GetType()->GetPrimitiveType() == DSPT_INT ){
			newNode = new dspNodeInt( this, p_DSClsConst->GetInt() );
			
		}else if( p_DSClsConst->GetType()->GetPrimitiveType() == DSPT_FLOAT ){
			newNode = new dspNodeFloat( this, p_DSClsConst->GetFloat() );
		}
		if( ! newNode ) DSTHROW( dueOutOfMemory );
		
		checkCode->SetTypeObject( p_DSClsConst->GetType(), true );
		delete *ppThisNode;
		*ppThisNode = newNode;
		
	}else if( p_DSClsVar ){
		if( p_Obj ){
//			if(refClass->GetPrimitiveType() < DSPT_OBJECT){
//			if(p_Obj->GetNodeType() == ntString){
//				checkCode->ErrorNoMembSelAllowed(this, refClass->GetName());
//				p_DSClsVar = NULL; return false;
//			}
			if( p_DSClsVar->GetTypeModifiers() & DSTM_STATIC ){
				delete p_Obj;
				p_Obj = NULL;
				
			}else{
//				if(checkFunc && (checkCode->GetOwnerFunction()->GetTypeModifiers() & DSTM_STATIC)){
				if( ! checkCode->IsObject() ){
					checkCode->ErrorVarNotStatic( this, p_DSClsVar );
					p_DSClsVar = NULL;
					return false;
				}
			}
			
		}else{
			if( p_DSClsVar->GetTypeModifiers() & DSTM_STATIC ){
				checkCode->SetTypeClass( checkCode->GetOwnerClass() );
				
			}else{
				if( checkFunc && ! ( checkCode->GetOwnerFunction()->GetTypeModifiers() & DSTM_STATIC ) ){
					p_Obj = new dspNodeThis( this );
					if( ! p_Obj ) DSTHROW( dueOutOfMemory );
					
					checkCode->SetTypeObject( checkCode->GetOwnerClass(), false );
					
				}else{
					checkCode->ErrorVarNotStatic( this, p_DSClsVar );
					p_DSClsVar = NULL;
					return false;
				}
//				if(checkFunc){
//					checkCode->SetTypeObject(checkCode->GetOwnerClass(), checkCode->GetOwnerFunction()->GetTypeModifiers() & DSTM_FIXED);
//					checkCode->SetTypeObject(checkCode->GetOwnerClass(), false);
//				}else{
//					checkCode->SetTypeObject(checkCode->GetOwnerClass(), true);
//				}
			}
		}
		
		if( ! ( p_DSClsVar->GetTypeModifiers() & DSTM_STATIC ) ){
			if( checkCode->IsClass() ){
				checkCode->ErrorVarNotStatic( this, p_DSClsVar );
				p_DSClsVar = NULL;
				return false;
			}
		}
		
		if( ! ( checkCode->CanAccessVariable( refClass, checkCode->GetOwnerClass(), p_DSClsVar ) ) ){
			checkCode->ErrorNoAccessClsVar( this, p_DSClsVar );
			p_DSClsVar = NULL;
			return false;
		}
		
		checkCode->SetTypeObject( p_DSClsVar->GetType(), checkCode->GetIsObjConst() || ( p_DSClsVar->GetTypeModifiers() & DSTM_FIXED ) );
		
		if( ( p_DSClsVar->GetTypeModifiers() & DSTM_STATIC ) == DSTM_STATIC ){
			checkCode->SetStaticClassVariable( p_DSClsVar );
		}
		
	}else if( p_DSClass ){
		checkCode->SetTypeClass( p_DSClass );
		
	}else{
		checkCode->ErrorInvClassMemberName( this, refClass, p_Name );
		return false;
	}
	
	return true;
}

void dspNodeMembVar::CompileCode( dspParserCompileCode *compileCode ){
	if( ! compileCode ) DSTHROW( dueInvalidParam );
	
	if( p_ParamID != -1 ){
		dsByteCode::sCodeParam code;
		code.code = dsByteCode::ebcParam;
		code.index = ( byte )p_ParamID;
		compileCode->AddCode( code, GetLineNum() );
		
	}else if( p_DSLocVarIndex != -1 ){
		dsByteCode::sCodeLocalVar code;
		code.code = dsByteCode::ebcLocVar;
		code.index = ( short )p_DSLocVarIndex;
		compileCode->AddCode( code, GetLineNum() );
		
	}else if( pContextVariable != -1 ){
		dsByteCode::sCodeLocalVar code;
		code.code = dsByteCode::ebcCVar;
		code.index = ( short )pContextVariable;
		compileCode->AddCode( code, GetLineNum() );
		
//	}else if(p_DSClsConst){
//		// do nothing
		
	}else if( p_DSClsVar ){
		if( p_Obj ){
			p_Obj->CompileCode( compileCode );
		}
		
		dsByteCode::sCodeClassVar code;
		code.code = dsByteCode::ebcClsVar;
		code.variable = p_DSClsVar;
		compileCode->AddCode( code, GetLineNum() );
		
//	}else if(p_DSClass){
//		// do nothing
	}
}
#ifndef __NODBGPRINTF__
void dspNodeMembVar::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Member Variable '%s'", p_Name);
	p_PostPrintDebug();
	if(p_Obj) p_Obj->DebugPrint(level+1, "");
}
#endif


// dspNodeMembFunc
dspNodeMembFunc::dspNodeMembFunc(dspNodeIdent *Member, dspBaseNode *Obj) : dspBaseNode(ntSuper,Member){
	if(!(p_Name = new char[strlen(Member->GetName())+1])) DSTHROW(dueOutOfMemory);
	strcpy(p_Name, Member->GetName()); p_Obj = Obj;
	p_DSClsFunc = NULL;
	p_FuncID = -1;
	p_SuperCall = false;
}
dspNodeMembFunc::~dspNodeMembFunc(){
	delete [] p_Name; if(p_Obj) delete p_Obj;
}
void dspNodeMembFunc::AddArgument(dspBaseNode *argument){
	p_Args.AddNode(argument);
}
bool dspNodeMembFunc::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *refClass;
	int vSigID;
	// build signature
	dsSignature vFuncSig, *vFoundFuncSig;
	dsFuncList *vFuncList;
	dspBaseNode **vCurNode;
	dspListNodeIterator vArgItr(&p_Args);
	while(vArgItr.HasNext()){
		vCurNode = vArgItr.GetNext();
		if((*vCurNode)->CheckCode(checkCode, vCurNode)){
			if(checkCode->IsObject()){
				vFuncSig.AddParameter(checkCode->GetTypeObject());
			}else{
				checkCode->ErrorInvFuncArg(*vCurNode); return false;
			}
		}else return false;
	}
	// determine function to call
	if(p_Obj){
		if(!p_Obj->CheckCode(checkCode, &p_Obj)) return false;
		if(checkCode->IsClass()){
			refClass = checkCode->GetTypeClass();
		}else{
			refClass = checkCode->GetTypeObject();
		}
	}else{
		refClass = checkCode->GetOwnerClass();
	}
	vFuncList = refClass->GetFuncList();
	if(strcmp(p_Name, DSFUNC_CONSTRUCTOR) == 0){
		p_DSClsFunc = vFuncList->FindBestFunction(refClass, p_Name, &vFuncSig);
	}else{
		p_DSClsFunc = vFuncList->FindBestFunction(NULL, p_Name, &vFuncSig);
	}
	if(p_DSClsFunc){
		// determine return type
		if(p_Obj){
//			if(refClass->GetPrimitiveType() < DSPT_OBJECT){
//			if(p_Obj->GetNodeType() == ntString){
//				checkCode->ErrorNoMembSelAllowed(this, refClass->GetName()); 
//				p_DSClsFunc = NULL; return false;
//			}
			if(p_DSClsFunc->GetTypeModifiers() & DSTM_STATIC ||
			p_DSClsFunc->GetFuncType() == DSFT_CONSTRUCTOR){
				delete p_Obj; p_Obj = NULL;
			}else{
				if(!checkCode->IsObject()){
					checkCode->ErrorFuncNotStatic(this, p_DSClsFunc);
					p_DSClsFunc = NULL; return false;
				}
			}
		}else{
			if(p_DSClsFunc->GetFuncType() == DSFT_FUNCTION){
				if(p_DSClsFunc->GetTypeModifiers() & DSTM_STATIC){
					checkCode->SetTypeClass(checkCode->GetOwnerClass());
				}else{
					if(!(p_Obj = new dspNodeThis(this))) DSTHROW(dueOutOfMemory);
					if(checkCode->GetCheckFunc()){
						checkCode->SetTypeObject(checkCode->GetOwnerClass(), false/*, checkCode->GetOwnerFunction()->GetTypeModifiers() & DSTM_FIXED*/);
					}else{
						checkCode->SetTypeObject(checkCode->GetOwnerClass(), true);
					}
				}
			}else if(p_DSClsFunc->GetFuncType() == DSFT_CONSTRUCTOR){
				checkCode->SetTypeClass(checkCode->GetOwnerClass());
			}else if(p_DSClsFunc->GetFuncType() == DSFT_DESTRUCTOR){
				checkCode->ErrorNoDirectDestrCall(this, p_DSClsFunc);
				p_DSClsFunc = NULL; return false;
			}else DSTHROW(dueInvalidAction);
		}
		// auto-cast arguments to match selected function
		vArgItr.Rewind();
		vFoundFuncSig = p_DSClsFunc->GetSignature();
		vSigID = 0;
		while(vArgItr.HasNext()){
			vCurNode = vArgItr.GetNext();
			if(!checkCode->AutoNodeCast(vCurNode, vFuncSig.GetParameter(vSigID), vFoundFuncSig->GetParameter(vSigID))) return false;
			vSigID++;
		}
		// check for correct calling (access, static)
		if(p_DSClsFunc->GetFuncType() != DSFT_CONSTRUCTOR){
			if(!(p_DSClsFunc->GetTypeModifiers() & DSTM_STATIC)){
				if(checkCode->IsClass()){
					checkCode->ErrorFuncNotStatic(this, p_DSClsFunc);
					p_DSClsFunc = NULL; return false;
				}
			}
			if(!(checkCode->CanAccessFunction(refClass, checkCode->GetOwnerClass(), p_DSClsFunc))){
				checkCode->ErrorNoAccessClsFunc(this, p_DSClsFunc);
				p_DSClsFunc = NULL; return false;
			}
			if(p_DSClsFunc->GetType()->GetPrimitiveType() == DSPT_VOID){
				checkCode->SetTypeNothing();
			}else{
				checkCode->SetTypeObject(p_DSClsFunc->GetType(), false);
			}
		}else{
			if(checkCode->IsObject()){
				checkCode->ErrorConstrNotOnClass(this, p_DSClsFunc);
				p_DSClsFunc = NULL; return false;
			}else if( ! checkCode->IsClass() ){
				DSTHROW( dueInvalidAction );
			}
			if(checkCode->GetTypeClass()->GetTypeModifiers() & DSTM_ABSTRACT){
				checkCode->ErrorNoNewOnAbstractClass(this, checkCode->GetTypeClass());
				p_DSClsFunc = NULL; return false;
			}
			if(!(checkCode->CanAccessFunction(refClass, checkCode->GetOwnerClass(), p_DSClsFunc))){
				checkCode->ErrorNoAccessClsFunc(this, p_DSClsFunc);
				p_DSClsFunc = NULL; return false;
			}
			checkCode->SetTypeObject(p_DSClsFunc->GetOwnerClass(), false);
		}
		// finish
		p_FuncID = vFuncList->GetIndexOf(p_DSClsFunc);
		p_SuperCall = checkCode->GetSuperCall();
		checkCode->SetSuperCall(false);
	}else{
		checkCode->ErrorInvFuncSig(this, refClass, p_Name, &vFuncSig); return false;
	}
	return true;
}
void dspNodeMembFunc::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	int i, vNodeCount = 0;
	vNodeCount = p_Args.GetNodeCount();
	dspBaseNode **vArgNodes = new dspBaseNode*[vNodeCount];
	if(!vArgNodes) DSTHROW(dueOutOfMemory);
	try{
		dspListNodeIterator vArgItr(&p_Args);
		for(i=vNodeCount-1; i>=0; i--) vArgNodes[i] = *vArgItr.GetNext();
		for(i=0; i<vNodeCount; i++){
			vArgNodes[i]->CompileCode(compileCode);
			dsByteCode::sCode code;
			code.code = dsByteCode::ebcPush;
			compileCode->AddCode( code, GetLineNum() );
		}
		delete [] vArgNodes;
	}catch( ... ){
		delete [] vArgNodes; throw;
	}
	if(!p_DSClsFunc){
		DebugPrint(0, "(ohnoes) ");
		DSTHROW(dueInvalidAction);
	}
	if(p_DSClsFunc->GetFuncType() == DSFT_FUNCTION){
		if(p_Obj) p_Obj->CompileCode(compileCode);
//		if(p_DSClsFunc->GetTypeModifiers() & DSTM_ABSTRACT){
		if(p_SuperCall || (p_DSClsFunc->GetTypeModifiers() & DSTM_STATIC)){
			dsByteCode::sCodeDirectCall code;
			code.code = dsByteCode::ebcDCall;
			code.function = p_DSClsFunc;
			compileCode->AddCode( code, GetLineNum() );
		}else{
			dsByteCode::sCodeCall code;
			code.code = dsByteCode::ebcCall;
			code.index = p_FuncID;
			compileCode->AddCode( code, GetLineNum() );
		}
	}else if(p_DSClsFunc->GetFuncType() == DSFT_CONSTRUCTOR){
		dsByteCode::sCodeDirectCall code;
		code.code = dsByteCode::ebcCCall;
		code.function = p_DSClsFunc;
		compileCode->AddCode( code, GetLineNum() );
	}
}
#ifndef __NODBGPRINTF__
void dspNodeMembFunc::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Member Function '%s'", p_Name);
	p_PostPrintDebug();
	if(p_Obj) p_Obj->DebugPrint(level+1, "");
	dspListNodeIterator vArgItr(&p_Args);
	while(vArgItr.HasNext()){
		(*vArgItr.GetNext())->DebugPrint(level+1, "(arg) ");
	}
}
#endif


// empty statement node
/////////////////////////
dspNodeEmptySta::dspNodeEmptySta(dspBaseNode *refnode) : dspBaseNode(ntEmptySta,refnode) { }
bool dspNodeEmptySta::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	return true;
}
void dspNodeEmptySta::CompileCode(dspParserCompileCode *compileCode){ }
#ifndef __NODBGPRINTF__
void dspNodeEmptySta::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Empty statement");
	p_PostPrintDebug();
}
#endif


// if-else node
/////////////////
dspNodeIfElse::dspNodeIfElse( dspBaseNode *refnode, dspBaseNode *Condition, dspBaseNode *IfPart, dspBaseNode *ElsePart ) :
dspBaseNode( ntIfElse, refnode ),
p_Cond( NULL ),
p_If( NULL ),
p_Else( NULL ),
pSimpleBlock( NULL )
{
	if( ! Condition || ! IfPart ){
		DSTHROW( dueInvalidParam );
	}
	
	p_Cond = Condition;
	p_If = IfPart;
	p_Else = ElsePart;
}

dspNodeIfElse::~dspNodeIfElse(){
	if( p_Cond ){
		delete p_Cond;
	}
	if( p_If ){
		delete p_If;
	}
	if( p_Else ){
		delete p_Else;
	}
	if( pSimpleBlock ){
		delete pSimpleBlock;
	}
}

bool dspNodeIfElse::CheckCode( dspParserCheckCode *checkCode, dspBaseNode **ppThisNode ){
	if( ! checkCode || ! ppThisNode || ! *ppThisNode ){
		DSTHROW( dueInvalidParam );
	}
	
	dsClass * const vCondType = checkCode->CheckForObject( &p_Cond );
	if( ! vCondType ){
		return false;
	}
	
	dsClass * const vClsBool = checkCode->GetEngine()->GetClassBool();
	if( ! checkCode->AutoNodeCast( &p_Cond, vCondType, vClsBool ) ){
		return false;
	}
	
	// optimize if condition is a static boolean value
	if( p_Cond->GetNodeType() == ntBool ){
		if( ( ( dspNodeBool* )p_Cond )->GetValue() ){
			dspParserCheckCode ifCheckCode( checkCode );
			if( ! p_If->CheckCode( &ifCheckCode, &p_If ) ){
				return false;
			}
			if( ifCheckCode.GetHasReturn() ){
				checkCode->SetHasReturn( true );
			}
			
			pSimpleBlock = p_If;
			p_If = NULL;
			
		}else{
			if( p_Else ){
				dspParserCheckCode elseCheckCode( checkCode );
				if( ! p_Else->CheckCode( &elseCheckCode, &p_Else ) ){
					return false;
				}
				if( elseCheckCode.GetHasReturn() ){
					checkCode->SetHasReturn( true );
				}
				
				pSimpleBlock = p_Else;
				p_Else = NULL;
				
			}else{
				pSimpleBlock = new dspNodeEmptySta( this );
				if( ! pSimpleBlock ){
					DSTHROW(dueOutOfMemory);
				}
			}
		}
		
		checkCode->SetTypeNothing();
		return true;
	}
	
	// if case. ifCheckCode has to be scoped due to destructor side-effect on checkCode
	bool ifHasReturn = false;
	{
	dspParserCheckCode ifCheckCode( checkCode );
	if( ! p_If->CheckCode( &ifCheckCode, &p_If ) ){
		return false;
	}
	ifHasReturn = ifCheckCode.GetHasReturn();
	}
	
	// else case. elseCheckCode has to be scoped due to destructor side-effect on checkCode
	bool elseHasReturn = false;
	if( p_Else ){
		dspParserCheckCode elseCheckCode( checkCode );
		if( ! p_Else->CheckCode( &elseCheckCode, &p_Else ) ){
			return false;
		}
		elseHasReturn = elseCheckCode.GetHasReturn();
	}
	
	if( ifHasReturn && elseHasReturn ){
		checkCode->SetHasReturn( true );
	}
	checkCode->SetTypeNothing();
	
	return true;
}

void dspNodeIfElse::CompileCode( dspParserCompileCode *compileCode ){
	if( ! compileCode ){
		DSTHROW( dueInvalidParam );
	}
	
	const int blockBegin = compileCode->GetCurOffset();
	int locVarCount;
	
	if( pSimpleBlock ){
		locVarCount = compileCode->GetLocVarCount();
		pSimpleBlock->CompileCode( compileCode );
		compileCode->DecLocVarCount( locVarCount, GetLineNum() );
		
	}else if( p_Else ){
		// compile condition
		p_Cond->CompileCode( compileCode );
		compileCode->AddJump( dspParserCompileCode::ettElse, dsByteCode::ebcJNEB );
		
		// compile if block
		locVarCount = compileCode->GetLocVarCount();
		p_If->CompileCode( compileCode );
		compileCode->DecLocVarCount( locVarCount, GetLineNum() );
		compileCode->AddJump( dspParserCompileCode::ettIfEnd, dsByteCode::ebcJMPB );
		
		// compile else block
		compileCode->AddTarget( dspParserCompileCode::ettElse, blockBegin, compileCode->GetCurOffset() );
		locVarCount = compileCode->GetLocVarCount();
		p_Else->CompileCode( compileCode );
		compileCode->DecLocVarCount( locVarCount, GetLineNum() );
		
	}else{
		// compile condition
		p_Cond->CompileCode( compileCode );
		compileCode->AddJump( dspParserCompileCode::ettIfEnd, dsByteCode::ebcJNEB );
		
		// compile if block
		locVarCount = compileCode->GetLocVarCount();
		p_If->CompileCode( compileCode );
		compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	}
	
	compileCode->AddTarget( dspParserCompileCode::ettIfEnd, blockBegin, compileCode->GetCurOffset() );
}

#ifndef __NODBGPRINTF__
void dspNodeIfElse::DebugPrint( int level, const char *prefix ){
	p_PrePrintDebug( level, prefix );
	printf( "- If-Else statement" );
	p_PostPrintDebug();
	p_Cond->DebugPrint( level + 1, "(cond) " );
	if( pSimpleBlock ){
		pSimpleBlock->DebugPrint( level + 1, "(simple) " );
	}
	if( p_If ){
		p_If->DebugPrint(level+1, "(if) ");
	}
	if( p_Else ){
		p_Else->DebugPrint(level+1, "(else) ");
	}
}
#endif


// select-case nodes
//////////////////////
dspNodeCase::dspNodeCase(dspBaseNode *refnode) : dspBaseNode(ntCase,refnode){
	if(!(p_Sta = new dspNodeEmptySta(refnode))) DSTHROW(dueOutOfMemory);
	p_IntValCount = 0;
	p_IntValues = NULL;
	p_StaValCount = 0;
	p_StaValues = NULL;
}
dspNodeCase::~dspNodeCase(){
	delete p_Sta;
	if(p_IntValues) delete [] p_IntValues;
	if(p_StaValues) delete [] p_StaValues;
}
void dspNodeCase::AddValue(dspBaseNode *Value){
	if(!Value) DSTHROW(dueInvalidParam);
	p_ValNodes.AddNode(Value);
}
void dspNodeCase::SetStatement(dspBaseNode *Sta){
	if(!Sta) DSTHROW(dueInvalidParam);
	if(p_Sta) delete p_Sta;
	p_Sta = Sta;
}
bool dspNodeCase::MatchesValues(int Value){
	for(int i=0; i<p_IntValCount; i++){
		if(Value == p_IntValues[i]) return true;
	}
	return false;
}
bool dspNodeCase::MatchesValues(dspNodeCase *Case, int *Value){
	if(!Case || !Value) DSTHROW(dueInvalidParam);
	int i, j;
	for(i=0; i<p_IntValCount; i++){
		for(j=0; j<Case->p_IntValCount; j++){
			if(p_IntValues[i] == Case->p_IntValues[j]){
				*Value = p_IntValues[i]; return true;
			}
		}
	}
	return false;
}
int dspNodeCase::GetValue(int Index) const{
	if(Index<0 || Index>=p_IntValCount) DSTHROW(dueOutOfBoundary);
	return p_IntValues[Index];
}

bool dspNodeCase::MatchesStaticValues( dsVariable *Value ){
	for(int i=0; i<p_StaValCount; i++){
		if(Value == p_StaValues[i]) return true;
	}
	return false;
}

bool dspNodeCase::MatchesStaticValues( dspNodeCase *Case, dsVariable **Value ){
	if(!Case || !Value) DSTHROW(dueInvalidParam);
	int i, j;
	for(i=0; i<p_StaValCount; i++){
		for(j=0; j<Case->p_StaValCount; j++){
			if(p_StaValues[i] == Case->p_StaValues[j]){
				*Value = p_StaValues[i]; return true;
			}
		}
	}
	return false;
}

dsVariable *dspNodeCase::GetStaticValue( int Index ) const{
	if(Index<0 || Index>=p_StaValCount) DSTHROW(dueOutOfBoundary);
	return p_StaValues[Index];
}

bool dspNodeCase::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vValType;
	dspBaseNode **vCurNode;
	int vValue;
	// check values
	dspListNodeIterator vValItr(&p_ValNodes);
	while(vValItr.HasNext()){
		dsVariable *staValue = NULL;
		vCurNode = vValItr.GetNext();
		if(!(vValType = checkCode->CheckForObject(vCurNode))) return false;
		switch((*vCurNode)->GetNodeType()){
		case ntByte:
			vValue = ((dspNodeByte*)*vCurNode)->GetValue(); break;
		case ntInt:
			vValue = ((dspNodeInt*)*vCurNode)->GetValue(); break;
		default:
			if(checkCode->GetStaticClassVariable()){
				staValue = checkCode->GetStaticClassVariable();
			}else{
				checkCode->ErrorCaseValNotScalar(*vCurNode); return false;
			}
		}
		if(staValue){
			if(MatchesStaticValues(staValue)){
				checkCode->ErrorDupCaseValue(*vCurNode); return false;
			}
			p_AddStaticValue(staValue);
		}else{
			if(MatchesValues(vValue)){
				checkCode->ErrorDupCaseValue(*vCurNode, vValue); return false;
			}
			p_AddIntValue(vValue);
		}
	}
	// check statement
	if(!p_Sta->CheckCode(checkCode, &p_Sta)) return false;
	// finished
	return true;
}
void dspNodeCase::CompileCodeCond(dspParserCompileCode *compileCode, int Num){
	if(!compileCode) DSTHROW(dueInvalidParam);
	// compile condition
	int i;
	for(i=0; i<p_IntValCount; i++){
		compileCode->AddJumpInt( dspParserCompileCode::ettCase0+Num, dsByteCode::ebcJCEB, p_IntValues[i] );
	}
	for(i=0; i<p_StaValCount; i++){
		compileCode->AddJumpPtr( dspParserCompileCode::ettCase0+Num, dsByteCode::ebcJCESB, p_StaValues[i] );
	}
}
void dspNodeCase::CompileCodeSta(dspParserCompileCode *compileCode, bool Jump){
	if(!compileCode) DSTHROW(dueInvalidParam);
	// compile statement
	int locVarCount = compileCode->GetLocVarCount();
	p_Sta->CompileCode(compileCode);
	compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	if(Jump) compileCode->AddJump(dspParserCompileCode::ettEnd, dsByteCode::ebcJMPB);
}
#ifndef __NODBGPRINTF__
void dspNodeCase::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Case statement");
	p_PostPrintDebug();
	dspListNodeIterator vValItr(&p_ValNodes);
	while(vValItr.HasNext()){
		(*vValItr.GetNext())->DebugPrint(level+1, "(value) ");
	}
	p_Sta->DebugPrint(level+1, "");
}
#endif
void dspNodeCase::p_AddIntValue(int Value){
	int *vArray = new int[p_IntValCount+1];
	if(!vArray) DSTHROW(dueOutOfMemory);
	for(int i=0; i<p_IntValCount; i++) vArray[i] = p_IntValues[i];
	vArray[p_IntValCount] = Value;
	if(p_IntValues) delete [] p_IntValues;
	p_IntValues = vArray;
	p_IntValCount++;
}

void dspNodeCase::p_AddStaticValue( dsVariable *Value ){
	dsVariable **vArray = new dsVariable*[p_StaValCount+1];
	if(!vArray) DSTHROW(dueOutOfMemory);
	for(int i=0; i<p_StaValCount; i++) vArray[i] = p_StaValues[i];
	vArray[p_StaValCount] = Value;
	if(p_StaValues) delete [] p_StaValues;
	p_StaValues = vArray;
	p_StaValCount++;
}


dspNodeSelect::dspNodeSelect(dspBaseNode *refnode, dspBaseNode *Condition) : dspBaseNode(ntSelect,refnode){
	if(!Condition) DSTHROW(dueInvalidParam);
	p_Cond = Condition;
	p_StaElse = NULL;
}
dspNodeSelect::~dspNodeSelect(){
	delete p_Cond; if(p_StaElse) delete p_StaElse;
}
void dspNodeSelect::AddCase(dspNodeCase *Case){
	if(!Case) DSTHROW(dueInvalidParam);
	p_Cases.AddNode(Case);
}
void dspNodeSelect::SetCaseElse(dspBaseNode *StaElse){
	if(p_StaElse) delete p_StaElse;
	p_StaElse = StaElse;
}
bool dspNodeSelect::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsInt = checkCode->GetEngine()->GetClassInt();
	dsClass *vCondType;
	int vCount=0;
	bool hasReturn = true;
	// check select object
	if(!(vCondType = checkCode->CheckForObject(&p_Cond))) return false;
	if(vCondType->GetPrimitiveType() != DSPT_OBJECT){
		if(!checkCode->AutoNodeCast(&p_Cond, vCondType, vClsInt)) return false;
	}
	// check cases
	dspBaseNode **vCurNode;
	dspListNodeIterator vCaseItr(&p_Cases);
	while(vCaseItr.HasNext()){
		vCurNode = vCaseItr.GetNext();
		dspParserCheckCode vCaseCC(checkCode); // scope it, has destructor side-effect
		vCaseCC.SetCanUseBreak(true);
		if(!(*vCurNode)->CheckCode(&vCaseCC, vCurNode)) return false;
		if(!vCaseCC.GetHasReturn()) hasReturn = false;
		if(p_MatchesCases(checkCode, *(dspNodeCase**)vCurNode, vCount)) return false;
		vCount++;
	}
	// check else
	if(p_StaElse){
		dspParserCheckCode vElseCC(checkCode); // scope it, has destructor side-effect
		vElseCC.SetCanUseBreak(true);
		if(!p_StaElse->CheckCode(&vElseCC, &p_StaElse)) return false;
		if(!vElseCC.GetHasReturn()) hasReturn = false;
	}else{
		hasReturn = false;
	}
	// check for return
	if( vCount > 0 && hasReturn ) checkCode->SetHasReturn(true);
	// finished
	return true;
}
void dspNodeSelect::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	int blockBegin = compileCode->GetCurOffset();
	int i, vCaseNum = 0, locVarCount;
	
	// break support
	const int gotoLocVarCount = compileCode->GetGotoLocVarCount();
	compileCode->SetGotoLocVarCount( compileCode->GetLocVarCount() );
	
	// compile condition
	p_Cond->CompileCode(compileCode);
	
	// compile case conditions
	dspListNodeIterator vCaseItr(&p_Cases);
	while(vCaseItr.HasNext()){
		(*(dspNodeCase**)vCaseItr.GetNext())->CompileCodeCond(compileCode, vCaseNum);
		vCaseNum++;
	}
	
	// compile else case
	if(p_StaElse){
		locVarCount = compileCode->GetLocVarCount();
		p_StaElse->CompileCode(compileCode);
		compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	}		
	compileCode->AddJump(dspParserCompileCode::ettEnd, dsByteCode::ebcJMPB);
	
	// compile case blocks
	vCaseItr.Rewind();
	for(i=0; i<vCaseNum; i++){
		compileCode->AddTarget(dspParserCompileCode::ettCase0+i, blockBegin, compileCode->GetCurOffset());
		(*(dspNodeCase**)vCaseItr.GetNext())->CompileCodeSta(compileCode, i<vCaseNum-1);
	}
	
	// end
	compileCode->SetGotoLocVarCount( gotoLocVarCount );
	
	compileCode->AddTarget(dspParserCompileCode::ettEnd, blockBegin, compileCode->GetCurOffset());
}
#ifndef __NODBGPRINTF__
void dspNodeSelect::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Select statement");
	p_PostPrintDebug();
	dspListNodeIterator vCaseItr(&p_Cases);
	while(vCaseItr.HasNext()){
		(*vCaseItr.GetNext())->DebugPrint(level+1, "");
	}
	if(p_StaElse) p_StaElse->DebugPrint(level+1, "(else) ");
}
#endif
bool dspNodeSelect::p_MatchesCases(dspParserCheckCode *checkCode, dspNodeCase *Case, int Count){
	dspNodeCase *vCurNode;
	int vValue;
	dspListNodeIterator vCaseItr(&p_Cases);
	for(int i=0; i<Count; i++){
		vCurNode = *(dspNodeCase**)vCaseItr.GetNext();
		if(vCurNode->MatchesValues(Case, &vValue)){
			checkCode->ErrorDupCaseValue(vCurNode, vValue);
			return true;
		}
	}
	return false;
}


// while node
///////////////
dspNodeWhile::dspNodeWhile(dspBaseNode *refnode, dspBaseNode *Condition, dspBaseNode *statement) : dspBaseNode(ntWhile,refnode){
	if(!Condition || !statement) DSTHROW(dueInvalidParam);
	p_Cond = Condition; p_Sta = statement;
}
dspNodeWhile::~dspNodeWhile(){
	delete p_Cond; delete p_Sta;
}
bool dspNodeWhile::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsBool = checkCode->GetEngine()->GetClassBool();
	dsClass *vCondType;
	if(!(vCondType = checkCode->CheckForObject(&p_Cond))) return false;
	if(!checkCode->AutoNodeCast(&p_Cond, vCondType, vClsBool)) return false;
	dspParserCheckCode vStaCC(checkCode); // scope it, has destructor side-effect
	vStaCC.SetCanUseBreak(true); vStaCC.SetCanUseContinue(true);
	if(!p_Sta->CheckCode(&vStaCC, &p_Sta)) return false;
	// NOTE while can never be hasReturn even if body is hasReturn since condition can be false in the very first run
	checkCode->SetTypeNothing();
	return true;
}
void dspNodeWhile::CompileCode( dspParserCompileCode *compileCode ){
	if( ! compileCode ) DSTHROW( dueInvalidParam );
	int blockBegin = compileCode->GetCurOffset();
	int gotoLocVarCount;
	int locVarCount;
	
	gotoLocVarCount = compileCode->GetGotoLocVarCount();
	compileCode->SetGotoLocVarCount( compileCode->GetLocVarCount() );
	
	// compile condition
	p_Cond->CompileCode( compileCode );
	compileCode->AddJump( dspParserCompileCode::ettEnd, dsByteCode::ebcJNEB );
	
	// compile statement
	locVarCount = compileCode->GetLocVarCount();
	p_Sta->CompileCode( compileCode );
	compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	compileCode->AddJump( dspParserCompileCode::ettNext, dsByteCode::ebcJMPB );
	
	// set targets
	compileCode->SetGotoLocVarCount( gotoLocVarCount );
	
	compileCode->AddTarget(dspParserCompileCode::ettCont, blockBegin, blockBegin);
	compileCode->AddTarget(dspParserCompileCode::ettNext, blockBegin, blockBegin);
	compileCode->AddTarget(dspParserCompileCode::ettEnd, blockBegin, compileCode->GetCurOffset());
}
#ifndef __NODBGPRINTF__
void dspNodeWhile::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- While statement");
	p_PostPrintDebug();
	p_Cond->DebugPrint(level+1, "(cond) ");
	p_Sta->DebugPrint(level+1, "(sta) ");
}
#endif


// do while node
//////////////////
dspNodeDoWhile::dspNodeDoWhile(dspBaseNode *refnode, dspBaseNode *Condition, dspBaseNode *statement) : dspBaseNode(ntDoWhile,refnode){
	if(!Condition || !statement) DSTHROW(dueInvalidParam);
	p_Cond = Condition; p_Sta = statement;
}
dspNodeDoWhile::~dspNodeDoWhile(){
	delete p_Cond; delete p_Sta;
}
bool dspNodeDoWhile::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsBool = checkCode->GetEngine()->GetClassBool();
	dsClass *vCondType;
	if(!(vCondType = checkCode->CheckForObject(&p_Cond))) return false;
	if(!checkCode->AutoNodeCast(&p_Cond, vCondType, vClsBool)) return false;
	dspParserCheckCode vStaCC(checkCode); // scope it, has destructor side-effect
	vStaCC.SetCanUseBreak(true); vStaCC.SetCanUseContinue(true);
	if(!p_Sta->CheckCode(&vStaCC, &p_Sta)) return false;
	if(vStaCC.GetHasReturn()) checkCode->SetHasReturn(true); // NOTE correct since body is always run before condition is checked for the first time
	checkCode->SetTypeNothing();
	return true;
}
void dspNodeDoWhile::CompileCode( dspParserCompileCode *compileCode ){
	if( ! compileCode ) DSTHROW( dueInvalidParam );
	int blockBegin = compileCode->GetCurOffset();
	int gotoLocVarCount;
	int vContPoint, locVarCount;
	
	gotoLocVarCount = compileCode->GetGotoLocVarCount();
	compileCode->SetGotoLocVarCount( compileCode->GetLocVarCount() );
	
	// compile statement
	locVarCount = compileCode->GetLocVarCount();
	p_Sta->CompileCode( compileCode );
	compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	
	// compile condition
	vContPoint = compileCode->GetCurOffset();
	p_Cond->CompileCode( compileCode );
	compileCode->AddJump( dspParserCompileCode::ettNext, dsByteCode::ebcJEQB );
	
	// set targets
	compileCode->SetGotoLocVarCount( gotoLocVarCount );
	
	compileCode->AddTarget( dspParserCompileCode::ettCont, blockBegin, vContPoint );
	compileCode->AddTarget( dspParserCompileCode::ettNext, blockBegin, blockBegin );
	compileCode->AddTarget( dspParserCompileCode::ettEnd, blockBegin, compileCode->GetCurOffset() );
}
#ifndef __NODBGPRINTF__
void dspNodeDoWhile::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Do While statement");
	p_PostPrintDebug();
	p_Cond->DebugPrint(level+1, "(cond) ");
	p_Sta->DebugPrint(level+1, "(sta) ");
}
#endif


// for nodes
//////////////
// for up node
dspNodeForUp::dspNodeForUp(dspBaseNode *refnode, dspBaseNode *Var, dspBaseNode *From, dspBaseNode *To, dspBaseNode *Step, dspBaseNode *Sta) : dspBaseNode(ntForUp,refnode){
	if(!Var || !From || !To || !Sta) DSTHROW(dueInvalidParam);
	p_Var = Var; p_From = From; p_To = To; p_Step = Step; p_Sta = Sta;
}
dspNodeForUp::~dspNodeForUp(){
	delete p_Var; delete p_From; delete p_To; delete p_Sta;
	if(p_Step) delete p_Step;
}
bool dspNodeForUp::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vLoopType, *vValType;
	int vPrimType;
	if(!(vLoopType = checkCode->CheckForObject(&p_Var))) return false;
	if(checkCode->GetIsObjConst()){
		checkCode->ErrorLoopConstObj(p_Var); return false;
	}
	vPrimType = vLoopType->GetPrimitiveType();
	if(vPrimType!=DSPT_BYTE && vPrimType!=DSPT_INT){
		checkCode->ErrorInvLoopVarType(p_Var); return false;
	}
	if(!(vValType = checkCode->CheckForObject(&p_From))) return false;
	if(!checkCode->AutoNodeCast(&p_From, vValType, vLoopType)) return false;
	if(!(vValType = checkCode->CheckForObject(&p_To))) return false;
	if(!checkCode->AutoNodeCast(&p_To, vValType, vLoopType)) return false;
	if(p_Step){
		if(!(vValType = checkCode->CheckForObject(&p_Step))) return false;
		if(!checkCode->AutoNodeCast(&p_Step, vValType, vLoopType)) return false;
	}
	dspParserCheckCode vStaCC(checkCode); // scope it, has destructor side-effect
	vStaCC.SetCanUseBreak(true); vStaCC.SetCanUseContinue(true);
	if(!p_Sta->CheckCode(&vStaCC, &p_Sta)) return false;
	// NOTE for can never be hasReturn even if body is hasReturn since condition can be false in the very first run
	checkCode->SetTypeNothing();
	return true;
}
void dspNodeForUp::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	int blockBegin, vContPoint, locVarCount;
	int gotoLocVarCount;
	dsByteCode::sCode code;
	
	gotoLocVarCount = compileCode->GetGotoLocVarCount();
	compileCode->SetGotoLocVarCount( compileCode->GetLocVarCount() );
	
	// compile init
	p_From->CompileCode( compileCode );
	code.code = dsByteCode::ebcPush;
	compileCode->AddCode( code, GetLineNum() );
	p_Var->CompileCode( compileCode );
	code.code = dsByteCode::ebcOpAss;
	compileCode->AddCode( code, GetLineNum() );
	
	// compile condition
	blockBegin = compileCode->GetCurOffset();
	p_To->CompileCode( compileCode );
	code.code = dsByteCode::ebcPush;
	compileCode->AddCode( code, GetLineNum() );
	p_Var->CompileCode( compileCode );
	code.code = dsByteCode::ebcOpGEq;
	compileCode->AddCode( code, GetLineNum() );
	compileCode->AddJump( dspParserCompileCode::ettEnd, dsByteCode::ebcJEQB );
	
	// compile statement
	locVarCount = compileCode->GetLocVarCount();
	p_Sta->CompileCode( compileCode );
	compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	
	// compile step
	vContPoint = compileCode->GetCurOffset();
	
	if( p_Step ){
		p_Step->CompileCode( compileCode );
		code.code = dsByteCode::ebcPush;
		compileCode->AddCode( code, GetLineNum() );
		p_Var->CompileCode( compileCode );
		code.code = dsByteCode::ebcOpAddA;
		compileCode->AddCode( code, GetLineNum() );
		
	}else{
		p_Var->CompileCode( compileCode );
		code.code = dsByteCode::ebcOpInc;
		compileCode->AddCode( code, GetLineNum() );
	}
	
	compileCode->AddJump( dspParserCompileCode::ettNext, dsByteCode::ebcJMPB );
	
	// set targets
	compileCode->SetGotoLocVarCount( gotoLocVarCount );
	
	compileCode->AddTarget( dspParserCompileCode::ettCont, blockBegin, vContPoint );
	compileCode->AddTarget( dspParserCompileCode::ettNext, blockBegin, blockBegin );
	compileCode->AddTarget( dspParserCompileCode::ettEnd, blockBegin, compileCode->GetCurOffset() );
}
#ifndef __NODBGPRINTF__
void dspNodeForUp::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- For statement (incrementing)");
	p_PostPrintDebug();
	p_Var->DebugPrint(level+1, "(var) ");
	p_From->DebugPrint(level+1, "(from) ");
	p_To->DebugPrint(level+1, "(to) ");
	if(p_Step) p_Step->DebugPrint(level+1, "(step) ");
	p_Sta->DebugPrint(level+1, "(sta) ");
}
#endif

// for down node
dspNodeForDown::dspNodeForDown(dspBaseNode *refnode, dspBaseNode *Var, dspBaseNode *From, dspBaseNode *To, dspBaseNode *Step, dspBaseNode *Sta) : dspBaseNode(ntForDown,refnode){
	if(!Var || !From || !To || !Sta) DSTHROW(dueInvalidParam);
	p_Var = Var; p_From = From; p_To = To; p_Step = Step; p_Sta = Sta;
}
dspNodeForDown::~dspNodeForDown(){
	delete p_Var; delete p_From; delete p_To; delete p_Step; delete p_Sta;
}
bool dspNodeForDown::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vLoopType, *vValType;
	int vPrimType;
	if(!(vLoopType = checkCode->CheckForObject(&p_Var))) return false;
	if(checkCode->GetIsObjConst()){
		checkCode->ErrorLoopConstObj(p_Var); return false;
	}
	vPrimType = vLoopType->GetPrimitiveType();
	if(vPrimType!=DSPT_BYTE && vPrimType!=DSPT_INT){
		checkCode->ErrorInvLoopVarType(p_Var); return false;
	}
	if(!(vValType = checkCode->CheckForObject(&p_From))) return false;
	if(!checkCode->AutoNodeCast(&p_From, vValType, vLoopType)) return false;
	if(!(vValType = checkCode->CheckForObject(&p_To))) return false;
	if(!checkCode->AutoNodeCast(&p_To, vValType, vLoopType)) return false;
	if(p_Step){
		if(!(vValType = checkCode->CheckForObject(&p_Step))) return false;
		if(!checkCode->AutoNodeCast(&p_Step, vValType, vLoopType)) return false;
	}
	dspParserCheckCode vStaCC(checkCode); // scope it, has destructor side-effect
	vStaCC.SetCanUseBreak(true); vStaCC.SetCanUseContinue(true);
	if(!p_Sta->CheckCode(&vStaCC, &p_Sta)) return false;
	// NOTE for can never be hasReturn even if body is hasReturn since condition can be false in the very first run
	checkCode->SetTypeNothing();
	return true;
}
void dspNodeForDown::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	int blockBegin, vContPoint, locVarCount;
	int gotoLocVarCount;
	dsByteCode::sCode code;
	
	gotoLocVarCount = compileCode->GetGotoLocVarCount();
	compileCode->SetGotoLocVarCount( compileCode->GetLocVarCount() );
	
	// compile init
	p_From->CompileCode( compileCode );
	code.code = dsByteCode::ebcPush;
	compileCode->AddCode( code, GetLineNum() );
	p_Var->CompileCode( compileCode );
	code.code = dsByteCode::ebcOpAss;
	compileCode->AddCode( code, GetLineNum() );
	
	// compile condition
	blockBegin = compileCode->GetCurOffset();
	p_To->CompileCode( compileCode );
	code.code = dsByteCode::ebcPush;
	compileCode->AddCode( code, GetLineNum() );
	p_Var->CompileCode( compileCode );
	code.code = dsByteCode::ebcOpLe;
	compileCode->AddCode( code, GetLineNum() );
	compileCode->AddJump( dspParserCompileCode::ettEnd, dsByteCode::ebcJEQB );
	
	// compile statement
	locVarCount = compileCode->GetLocVarCount();
	p_Sta->CompileCode( compileCode );
	compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	
	// compile step
	vContPoint = compileCode->GetCurOffset();
	
	if( p_Step ){
		p_Step->CompileCode( compileCode );
		code.code = dsByteCode::ebcPush;
		compileCode->AddCode( code, GetLineNum() );
		p_Var->CompileCode( compileCode );
		code.code = dsByteCode::ebcOpSubA;
		compileCode->AddCode( code, GetLineNum() );
		
	}else{
		p_Var->CompileCode( compileCode );
		code.code = dsByteCode::ebcOpDec;
		compileCode->AddCode( code, GetLineNum() );
	}
	
	compileCode->AddJump( dspParserCompileCode::ettNext, dsByteCode::ebcJMPB );
	
	// set targets
	compileCode->SetGotoLocVarCount( gotoLocVarCount );
	
	compileCode->AddTarget( dspParserCompileCode::ettCont, blockBegin, vContPoint );
	compileCode->AddTarget( dspParserCompileCode::ettNext, blockBegin, blockBegin );
	compileCode->AddTarget( dspParserCompileCode::ettEnd, blockBegin, compileCode->GetCurOffset() );
}
#ifndef __NODBGPRINTF__
void dspNodeForDown::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- For statement (decrementing)");
	p_PostPrintDebug();
	p_Var->DebugPrint(level+1, "(var) ");
	p_From->DebugPrint(level+1, "(from) ");
	p_To->DebugPrint(level+1, "(to) ");
	if(p_Step) p_Step->DebugPrint(level+1, "(step) ");
	p_Sta->DebugPrint(level+1, "(sta) ");
}
#endif


// break node
///////////////
dspNodeBreak::dspNodeBreak(dspBaseNode *refnode) : dspBaseNode(ntBreak,refnode){ }
bool dspNodeBreak::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if( ! checkCode || ! ppThisNode || ! *ppThisNode ) DSTHROW( dueInvalidParam );
	
	if( ! checkCode->GetCanUseBreak() ){
		checkCode->ErrorBreakOutside( this );
		return false;
	}
	
	checkCode->SetTypeNothing();
	
	return true;
}

void dspNodeBreak::CompileCode(dspParserCompileCode *compileCode){
	if( ! compileCode ) DSTHROW( dueInvalidParam );
	int gotoLocVarCount = compileCode->GetGotoLocVarCount();
	int locVarCount = compileCode->GetLocVarCount();
	
	if( gotoLocVarCount == -1 ) DSTHROW( dueInvalidAction ); // sanity check
	
	if( locVarCount > gotoLocVarCount ){
		dsByteCode::sCodeFreeLocalVars code;
		code.code = dsByteCode::ebcFLV;
		code.count = ( short )( locVarCount - gotoLocVarCount );
		compileCode->AddCode( code, GetLineNum() );
	}
	
	compileCode->AddJump( dspParserCompileCode::ettEnd, dsByteCode::ebcJMPB );
}

#ifndef __NODBGPRINTF__
void dspNodeBreak::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Break statement");
	p_PostPrintDebug();
}
#endif


// continue node
//////////////////
dspNodeContinue::dspNodeContinue(dspBaseNode *refnode) : dspBaseNode(ntContinue,refnode){ }
bool dspNodeContinue::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if( ! checkCode || ! ppThisNode || ! *ppThisNode ) DSTHROW( dueInvalidParam );
	
	if( ! checkCode->GetCanUseContinue() ){
		checkCode->ErrorContinueOutside( this );
		return false;
	}
	
	checkCode->SetTypeNothing();
	
	return true;
}

void dspNodeContinue::CompileCode(dspParserCompileCode *compileCode){
	if( ! compileCode ) DSTHROW( dueInvalidParam );
	int gotoLocVarCount = compileCode->GetGotoLocVarCount();
	int locVarCount = compileCode->GetLocVarCount();
	
	if( gotoLocVarCount == -1 ) DSTHROW( dueInvalidAction ); // sanity check
	
	if( locVarCount > gotoLocVarCount ){
		dsByteCode::sCodeFreeLocalVars code;
		code.code = dsByteCode::ebcFLV;
		code.count = ( short )( locVarCount - gotoLocVarCount );
		compileCode->AddCode( code, GetLineNum() );
	}
	
	compileCode->AddJump( dspParserCompileCode::ettCont, dsByteCode::ebcJMPB );
}

#ifndef __NODBGPRINTF__
void dspNodeContinue::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Continue statement");
	p_PostPrintDebug();
}
#endif



// Block Node
///////////////

dspNodeBlock::dspNodeBlock( dspBaseNode *refnode ) : dspBaseNode( ntBlock, refnode ){
	p_Func = NULL;
	p_NeedsReturn = true;
	pCVarCount = 0;
	pCVars = NULL;
	p_Sta = new dspNodeEmptySta( refnode );
	if( ! p_Sta ) DSTHROW( dueOutOfMemory );
}

dspNodeBlock::~dspNodeBlock(){
	if( pCVars ) delete [] pCVars;
	delete p_Sta;
}

void dspNodeBlock::AddArgument( dspNodeFuncArg *argument ){
	p_Args.AddNode( argument );
}

void dspNodeBlock::SetStatement( dspBaseNode *statement ){
	if( ! statement ) DSTHROW( dueInvalidParam );
	
	delete p_Sta;
	p_Sta = statement;
}

bool dspNodeBlock::CheckCode( dspParserCheckCode *checkCode, dspBaseNode **ppThisNode ){
	if( ! checkCode || ! ppThisNode || ! * ppThisNode ) DSTHROW( dueInvalidParam );
	
	dspParserCheckFunction *checkFunc;
	dsClass *clsObj = checkCode->GetEngine()->GetClassObject();
	dsClass *clsBlock = checkCode->GetEngine()->GetClassBlock();
	dspParserCheck *parserCheck = checkCode->GetParserCheck();
	dsClass *ownerClass = checkCode->GetOwnerClass();
	dsLocalVariable *locvar, *cvar = NULL;
	int v, variableCount;
	dsSignature *sig;
	dsClass *typeClass;
	dspNodeFuncArg *argNode;
	const char *argName;
	int funcType;
	
	// create function
	funcType = DSTM_PRIVATE;
	if( checkCode->GetOwnerFunction()->GetTypeModifiers() & DSTM_STATIC ){
		funcType |= DSTM_STATIC;
	}
	
	p_Func = new dsFunction( ownerClass, "<block>", DSFT_FUNCTION, funcType, clsObj );
	if( ! p_Func ) DSTHROW( dueOutOfMemory );
	
	ownerClass->AddBlockFunction( p_Func );
	dspParserCheckFunction staCheckFunc( checkCode->GetCheckFunc()->GetScript(), p_Func );
	staCheckFunc.SetFunction( p_Func );
	
	// add arguments
	sig = p_Func->GetSignature();
	dspListNodeIterator argItr( &p_Args );
	
	while( argItr.HasNext() ){
		argNode = ( dspNodeFuncArg* )*argItr.GetNext();
		typeClass = parserCheck->GetClassFromNode( argNode->GetTypeNode(), ownerClass );
		staCheckFunc.AddArgName( argNode->GetName() );
		sig->AddParameter( typeClass );
	}
	
	// create new code checker
	dspParserCheckCode staCheckCode( parserCheck, &staCheckFunc ); // scope it, has destructor side-effect
	staCheckCode.SetCCBlock( checkCode );
	
	// add context variables
	try{
		checkFunc = checkCode->GetCheckFunc();
		if( checkFunc ){
			variableCount = checkFunc->GetArgumentCount();
			for( v=0; v<variableCount; v++ ){
				argName = checkFunc->GetArgumentAt( v );
				
				cvar = new dsLocalVariable( clsBlock, argName, checkFunc->GetSignature()->GetParameter( v ) );
				if( ! cvar ) DSTHROW( dueOutOfMemory );
				staCheckCode.AddBlockCVar( cvar );
				cvar = NULL;
				
				pAddCVar( -( v + 1 ) );
			}
		}
		
		variableCount = checkCode->GetLocVarBase() + checkCode->GetLocVarCount();
		for( v=0; v<variableCount; v++ ){
			locvar = checkCode->GetLocalVariable( v, false );
			
			cvar = new dsLocalVariable( clsBlock, locvar->GetName(), locvar->GetType() );
			if( ! cvar ) DSTHROW( dueOutOfMemory );
			staCheckCode.AddBlockCVar( cvar );
			cvar = NULL;
			
			pAddCVar( v );
		}
		
	}catch( ... ){
		if( cvar ) delete cvar;
		throw;
	}
	/*
	printf( "DEBUG: block ( line %i ) has %i context variables ( base %i ).\n", GetLineNum(), staCheckCode.GetBlockCVarCount(), staCheckCode.GetBlockCVarBase() );
	if( pCVarCount != staCheckCode.GetBlockCVarCount() ) DSTHROW( dueInvalidParam );
	
	for( v=0; v<staCheckCode.GetBlockCVarCount(); v++ ){
		cvar = staCheckCode.GetBlockCVarAt( v );
		printf( "- n( %s ) t( %s ) AN( %i )\n", cvar->GetName(), cvar->GetType()->GetName(), pCVars[ v ] );
	}
	*/
	// check body
	if( ! p_Sta->CheckCode( &staCheckCode, &p_Sta ) ) return false;
	p_NeedsReturn = ! staCheckCode.GetHasReturn();
	
	// update the function
	p_Func->SetLocVarSize( staCheckCode.GetMaxLocVarCount() );
	
	// set return type
	checkCode->SetTypeObject( clsBlock, true );
	return true;
}

void dspNodeBlock::CompileCode( dspParserCompileCode *compileCode ){
	if( ! compileCode ) DSTHROW( dueInvalidParam );
	
	dspParserCheck *parserCheck = compileCode->GetParserCheck();
	dsClass *ownerClass = compileCode->GetOwnerClass();
	dsClass *clsObj = compileCode->GetEngine()->GetClassObject();
	
	// add command
	dsByteCode::sCodeDirectCall codeBlock;
	codeBlock.code = dsByteCode::ebcBlock;
	codeBlock.function = p_Func;
	compileCode->AddCode( codeBlock, GetLineNum() );
	
	// add context variables
	/*compileCode->AddShort( ( short )pCVarCount );
	
	try{
		ccwalker = compileCode;
		while( ccwalker ){
			variableCount = ccwalker->GetLocVarCount();
			for( v=0; v<variableCount; v++ ){
				locvar = ccwalker->GetLocalVariableAt( v );
				if( staCheckCode.IndexOfBlockCVarNamed( locvar->GetName() ) == -1 ){
					newCVar = new dsLocalVariable( clsBlock, locvar->GetName(), locvar->GetType() );
					if( ! newCVar ) DSTHROW( dueOutOfMemory );
					staCheckCode.AddBlockCVar( newCVar );
					newCVar = NULL;
				}
			}
			
			checkFunc = ccwalker->GetCheckFunc();
			if( checkFunc ){
				variableCount = checkFunc->GetArgumentCount();
				for( v=0; v<variableCount; v++ ){
					argName = checkFunc->GetArgumentAt( v );
					
					if( staCheckCode.IndexOfBlockCVarNamed( argName ) == -1 ){
						newCVar = new dsLocalVariable( clsBlock, argName, checkFunc->GetSignature()->GetParameter( v ) );
						if( ! newCVar ) DSTHROW( dueOutOfMemory );
						staCheckCode.AddBlockCVar( newCVar );
						newCVar = NULL;
					}
				}
			}
			
			if( ccwalker->GetCCBlock() ){
				ccwalker = ccwalker->GetCCBlock();
				
			}else{
				ccwalker = ccwalker->GetPrecedor();
			}
		}
		
	}catch( ... ){
		if( newCVar ) delete newCVar;
		throw;
	}*/
	
	// compile the statement block
	dspParserCompileCode staCompileCode( parserCheck, ownerClass, p_Func->GetByteCode() );
	staCompileCode.SetCCBlock( compileCode );
	p_Sta->CompileCode( &staCompileCode );
	if( p_NeedsReturn ){
		dsByteCode::sCode code;
		
		staCompileCode.DecLocVarCount( 0, GetLineNum() );
		dsByteCode::sCodeNull codeNull;
		codeNull.code = dsByteCode::ebcNull;
		codeNull.type = clsObj;
		staCompileCode.AddCode( codeNull, GetLineNum() );
		
		code.code = dsByteCode::ebcPush;
		staCompileCode.AddCode( code, GetLineNum() );
		
		code.code = dsByteCode::ebcRet;
		staCompileCode.AddCode( code, GetLineNum() );
	}
	
	staCompileCode.BuildByteCode();
	
	/*
	printf( "\nblock at %i\n", GetLineNum() );
	p_Func->GetByteCode()->DebugPrint();
	printf( "end-block at %i\n\n", GetLineNum() );
	*/
}

#ifndef __NODBGPRINTF__
void dspNodeBlock::DebugPrint( int level, const char *prefix ){
	p_PrePrintDebug( level, prefix );
	printf( "- Block statement" );
	p_PostPrintDebug();
}
#endif

void dspNodeBlock::pAddCVar( int accessNumber ){
	short *newArray = new short[ pCVarCount + 1 ];
	if( ! newArray ) DSTHROW( dueOutOfMemory );
	if( pCVars ){
		memcpy( newArray, pCVars, sizeof( short ) * pCVarCount );
		delete [] pCVars;
	}
	pCVars = newArray;
	pCVars[ pCVarCount++ ] = ( short )accessNumber;
}



// variable declaration nodes
///////////////////////////////
dspNodeVarDefList::dspNodeVarDefList(dspBaseNode *refnode, dspBaseNode *Type) : dspBaseNode(ntVarDefList,refnode){
	if(!Type) DSTHROW(dueInvalidParam);
	p_Type = Type; p_DSClass = NULL;
}
dspNodeVarDefList::~dspNodeVarDefList(){
	delete p_Type;
}
void dspNodeVarDefList::AddVariable(dspNodeVarDef *Node){
	p_Vars.AddNode(Node);
}
bool dspNodeVarDefList::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dspParserCheck *vPCheck = checkCode->GetParserCheck();
	dspNodeVarDef *vCurNode;
	dsLocalVariable *vNewLocVar = NULL;
	bool vSuccess = true;
	// check type
	dspListNodeIterator vVarItr(&p_Vars);
	p_DSClass = vPCheck->GetClassFromNode(p_Type, checkCode->GetOwnerClass());
	if(p_DSClass->GetPrimitiveType() == DSPT_VOID){
		vCurNode = *(dspNodeVarDef**)vVarItr.GetNext();
		vPCheck->ErrorInvLocVarTypeVoid(p_Type, vCurNode->GetName()); return false;
	}
	
	// create all variables. for each variable check first the init code if present before
	// adding the local variable. if done the other way round block context variables are
	// resolved with the wrong number of context variables
	while( vVarItr.HasNext() ){
		vCurNode = *( dspNodeVarDef** )vVarItr.GetNext();
		if( checkCode->FindLocalVariable( vCurNode->GetName(), true ) == -1 ){
			vCurNode->SetTypeClass( p_DSClass );
			if( ! vCurNode->CheckCode( checkCode, ( dspBaseNode** )&vCurNode ) ){
				vSuccess = false;
			}
			
			try{
				vNewLocVar = new dsLocalVariable( checkCode->GetOwnerClass(), vCurNode->GetName(), p_DSClass );
				if( ! vNewLocVar ){
					DSTHROW( dueOutOfMemory );
				}
				
				checkCode->AddLocalVariable( vNewLocVar );
				vNewLocVar = NULL;
				
			}catch( ... ){
				if( vNewLocVar ){
					delete vNewLocVar;
				}
				throw;
			}
			
		}else{
			vPCheck->ErrorDupLocVar( vCurNode );
			vSuccess = false;
		}
	}
	
	// finished
	return vSuccess;
}
void dspNodeVarDefList::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	dspListNodeIterator vVarItr(&p_Vars);
	while(vVarItr.HasNext()){
		( *( ( dspNodeVarDef** )vVarItr.GetNext() ) )->CompileCode(compileCode );
	}
}
#ifndef __NODBGPRINTF__
void dspNodeVarDefList::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Variable declarations");
	p_PostPrintDebug();
	p_Type->DebugPrint(level+1, "(type) ");
	dspListNodeIterator vVarItr(&p_Vars);
	while(vVarItr.HasNext()){
		(*vVarItr.GetNext())->DebugPrint(level+1, "");
	}
}
#endif


dspNodeVarDef::dspNodeVarDef(dspNodeIdent *IdentNode, dspBaseNode *Init) : dspBaseNode(ntVarDef,IdentNode){
	if(!(p_Name = new char[strlen(IdentNode->GetName())+1])) DSTHROW(dueOutOfMemory);
	strcpy(p_Name, IdentNode->GetName());
	p_Init = Init;
	pTypeClass = NULL;
}
dspNodeVarDef::~dspNodeVarDef(){
	delete [] p_Name; if(p_Init) delete p_Init;
}
bool dspNodeVarDef::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !pTypeClass) DSTHROW(dueInvalidParam);
	dsClass *vInitType;
	if(p_Init){
		if(!(vInitType = checkCode->CheckForObject(&p_Init))) return false;
		if(!checkCode->AutoNodeCast(&p_Init, vInitType, pTypeClass)) return false;
	}
	return true;
}
void dspNodeVarDef::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode || !pTypeClass) DSTHROW(dueInvalidParam);
	if(p_Init){
		p_Init->CompileCode(compileCode);
		dsByteCode::sCode code;
		code.code = dsByteCode::ebcPush;
		compileCode->AddCode( code, GetLineNum() );
	}
	
	dsByteCode::sCodeAllocLocalVar codeALV;
	codeALV.code = dsByteCode::ebcALV;
	codeALV.type = pTypeClass;
	compileCode->AddCode( codeALV, GetLineNum() );
	compileCode->IncLocVarCount();
	
	if(p_Init){
		dsByteCode::sCode code;
		code.code = dsByteCode::ebcOpAss;
		compileCode->AddCode( code, GetLineNum() );
	}
}
#ifndef __NODBGPRINTF__
void dspNodeVarDef::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Name '%s'", p_Name);
	p_PostPrintDebug();
	if(p_Init) p_Init->DebugPrint(level+1, "(init) ");
}
#endif


// return node
////////////////
dspNodeReturn::dspNodeReturn(dspBaseNode *refnode, dspBaseNode *RetVal) : dspBaseNode(ntReturn,refnode){
	p_RetVal = RetVal;
}
dspNodeReturn::~dspNodeReturn(){
	if(p_RetVal) delete p_RetVal;
}
bool dspNodeReturn::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !checkCode->GetCheckFunc() || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsFunction *vFunc = checkCode->GetOwnerFunction();
	dsClass *vFuncType = vFunc->GetType();
	dsClass *vRetType;
	checkCode->SetHasReturn(true);
	if(vFuncType->GetPrimitiveType() == DSPT_VOID){
		if(p_RetVal){
			checkCode->ErrorReturnOnVoid(this, vFunc); return false;
		}
	}else{
		if(!p_RetVal){
			checkCode->ErrorNoReturnValue(this, vFunc); return false;
		}
		if(!(vRetType = checkCode->CheckForObject(&p_RetVal))) return false;
		if(!checkCode->AutoNodeCast(&p_RetVal, vRetType, vFuncType)) return false;
	}
	checkCode->SetTypeNothing();
	return true;
}
void dspNodeReturn::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	if(p_RetVal){
		p_RetVal->CompileCode(compileCode);
		dsByteCode::sCode code;
		code.code = dsByteCode::ebcPush;
		compileCode->AddCode( code, GetLineNum() );
	}
	if(compileCode->GetLocVarCount() > 0){
		dsByteCode::sCodeFreeLocalVars code;
		code.code = dsByteCode::ebcFLV;
		code.count = (short)compileCode->GetLocVarCount();
		compileCode->AddCode( code, GetLineNum() );
	}
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcRet;
	compileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeReturn::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Return statement");
	p_PostPrintDebug();
	if(p_RetVal) p_RetVal->DebugPrint(level+1,"");
}
#endif


// throw node
///////////////
dspNodeThrow::dspNodeThrow(dspBaseNode *refnode, dspBaseNode *Expression) : dspBaseNode(ntThrow,refnode){
	p_Expr = Expression;
}
dspNodeThrow::~dspNodeThrow(){
	if(p_Expr) delete p_Expr;
}
bool dspNodeThrow::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsExcep = checkCode->GetEngine()->GetClassException();
	dsClass *vThrowType;
	checkCode->SetHasReturn(true); // NOTE throw counts as return
	if(p_Expr){
		if(!(vThrowType = checkCode->CheckForObject(&p_Expr))) return false;
		if(!vThrowType->CastableTo(vClsExcep)){
			checkCode->ErrorThrowNotException(p_Expr); return false;
		}
	}else{
		if(!checkCode->GetCanUseRethrow()){
			checkCode->ErrorRethrowOutside(this); return false;
		}
	}
	checkCode->SetTypeNothing();
	return true;
}
void dspNodeThrow::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCode code;
	if(p_Expr){
		p_Expr->CompileCode(compileCode);
		code.code = dsByteCode::ebcThrow;
	}else{
		code.code = dsByteCode::ebcReThrow;
	}
	compileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeThrow::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Throw statement");
	p_PostPrintDebug();
	if(p_Expr) p_Expr->DebugPrint(level+1,"");
}
#endif


// try-catch nodes
////////////////////
dspNodeCatch::dspNodeCatch(dspBaseNode *refnode, dspBaseNode *Type, const char *VarName, dspBaseNode *Sta) : dspBaseNode(ntCatch,refnode){
	if(!Type || !VarName || VarName[0]=='\0' || !Sta) DSTHROW(dueInvalidParam);
	if(!(p_CatchName = new char[strlen(VarName)+1])) DSTHROW(dueOutOfMemory);
	strcpy(p_CatchName, VarName);
	p_CatchType = Type; p_Sta = Sta;
	p_DSType = NULL;
}
dspNodeCatch::~dspNodeCatch(){
	delete p_CatchType; delete [] p_CatchName; delete p_Sta;
}
bool dspNodeCatch::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsExcep = checkCode->GetEngine()->GetClassException();
	dspParserCheck *vPCheck = checkCode->GetParserCheck();
	dsLocalVariable *vNewLocVar = NULL;
	// check type
	p_DSType = vPCheck->GetClassFromNode(p_CatchType, checkCode->GetOwnerClass());
	if(!p_DSType->CastableTo(vClsExcep)){
		checkCode->ErrorInvExceptionType(p_CatchType, p_DSType->GetName()); return false;
	}
	// check code block
	dspParserCheckCode vStaCC(checkCode); // scope it, has destructor side-effect
	try{
		if(!(vNewLocVar = new dsLocalVariable(checkCode->GetOwnerClass(), p_CatchName, p_DSType))) DSTHROW(dueOutOfMemory);
		vStaCC.AddLocalVariable(vNewLocVar); vNewLocVar = NULL;
	}catch( ... ){
		if(vNewLocVar) delete vNewLocVar;
		throw;
	}
	vStaCC.SetCanUseRethrow(true);
	if(!p_Sta->CheckCode(&vStaCC, &p_Sta)) return false;
	if(vStaCC.GetHasReturn()) checkCode->SetHasReturn(true);
	checkCode->SetTypeNothing();
	return true;
}
void dspNodeCatch::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	int locVarCount;
	// compile statement
	locVarCount = compileCode->GetLocVarCount();
	compileCode->IncLocVarCount(); // var added my engine
	p_Sta->CompileCode(compileCode);
	compileCode->DecLocVarCount( locVarCount, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeCatch::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Catch statement (var = %s)", p_CatchName);
	p_PostPrintDebug();
	p_CatchType->DebugPrint(level+1, "(type) ");
	p_Sta->DebugPrint(level+1, "");
}
#endif


dspNodeTry::dspNodeTry(dspBaseNode *refnode, dspBaseNode *Sta) : dspBaseNode(ntTry,refnode){
	if(!Sta) DSTHROW(dueInvalidParam);
	p_Sta = Sta;
}
dspNodeTry::~dspNodeTry(){
	delete p_Sta;
}
void dspNodeTry::AddCatch(dspNodeCatch *Node){
	p_Catches.AddNode(Node);
}
bool dspNodeTry::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dspNodeCatch **vCurNode;
	dsClass *vCatchType;
	bool vSuccess=true, vOldHasReturn=checkCode->GetHasReturn(), vHasReturn;
	
	// check statement
	{ // vStaCC has to be scoped since destructor has side-effect
	dspParserCheckCode vStaCC(checkCode);
	if(!p_Sta->CheckCode(&vStaCC, &p_Sta)) return false;
	vHasReturn = vStaCC.GetHasReturn();
	checkCode->SetTypeNothing();
	}
	
	// check all catch blocks
	dspListNodeIterator vCatchItr(&p_Catches);
	if(!vCatchItr.HasNext()){
		checkCode->ErrorMissingCatch(this); return false;
	}
	while(vCatchItr.HasNext()){
		vCurNode = (dspNodeCatch**)vCatchItr.GetNext();
		checkCode->SetHasReturn(false);
		if(!(*vCurNode)->CheckCode(checkCode, (dspBaseNode**)vCurNode)){
			vSuccess = false;
		}else{
			if(!checkCode->GetHasReturn()) vHasReturn = false;
			vCatchType = (*vCurNode)->GetDSType();
			if(p_FindCatch(vCatchType) != *vCurNode){
				checkCode->ErrorDupCatch(*vCurNode, vCatchType->GetName()); vSuccess = false;
			}
		}
	}
	// finished
	checkCode->SetHasReturn(vOldHasReturn | vHasReturn);
	return vSuccess;
}
void dspNodeTry::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	dspNodeCatch *vCurNode;
	int blockBegin=compileCode->GetCurOffset(), vCount=0, locVarCount;
	// begin catch block
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcBTB;
	compileCode->AddCode( code, GetLineNum() );
	// compile catch jumps
	dspListNodeIterator vCatchItr(&p_Catches);
	while(vCatchItr.HasNext()){
		vCurNode = *(dspNodeCatch**)vCatchItr.GetNext();
		compileCode->AddJumpPtr( dspParserCompileCode::ettCase0 + vCount, dsByteCode::ebcJOEB, vCurNode->GetDSType() );
		vCount++;
	}
	// compile try block
	locVarCount = compileCode->GetLocVarCount();
	p_Sta->CompileCode(compileCode);
	compileCode->DecLocVarCount( locVarCount, GetLineNum() );
	code.code = dsByteCode::ebcETB;
	compileCode->AddCode( code, GetLineNum() );
	compileCode->AddJump(dspParserCompileCode::ettEnd, dsByteCode::ebcJMPB);
	// compile catch blocks
	vCatchItr.Rewind();
	for(int i=0; i<vCount; i++){
		vCurNode = *(dspNodeCatch**)vCatchItr.GetNext();
		compileCode->AddTarget(dspParserCompileCode::ettCase0 + i, blockBegin, compileCode->GetCurOffset());
		vCurNode->CompileCode(compileCode);
		if(i < vCount - 1){
			compileCode->AddJump(dspParserCompileCode::ettEnd, dsByteCode::ebcJMPB);
		}
	}
	// set targets
	compileCode->AddTarget(dspParserCompileCode::ettEnd, blockBegin, compileCode->GetCurOffset());
}
#ifndef __NODBGPRINTF__
void dspNodeTry::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Try Block");
	p_PostPrintDebug();
	p_Sta->DebugPrint(level+1, "(sta) ");
	dspListNodeIterator vCatchItr(&p_Catches);
	while(vCatchItr.HasNext()){
		(*vCatchItr.GetNext())->DebugPrint(level+1, "");
	}
}
#endif
dspNodeCatch *dspNodeTry::p_FindCatch(dsClass *Type){
	dspListNodeIterator vCatchItr(&p_Catches);
	dspNodeCatch *vCurNode;
	while(vCatchItr.HasNext()){
		vCurNode = *(dspNodeCatch**)vCatchItr.GetNext();
		if(Type->IsEqualTo(vCurNode->GetDSType())) return vCurNode;
	}
	return NULL;
}


// statement list node
////////////////////////
dspNodeStaList::dspNodeStaList(dspBaseNode *refnode) : dspBaseNode(ntStaList,refnode) { }
dspNodeStaList::~dspNodeStaList(){ }
void dspNodeStaList::AddNode(dspBaseNode *Node){
	p_Nodes.AddNode(Node);
}
bool dspNodeStaList::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
//	dspParserCheckCode vSLCheckCode(checkCode);
	bool vSuccess=true, vHasReturn=false;
	dspBaseNode **vCurNode;
	dspListNodeIterator vNodeItr(&p_Nodes);
	while(vNodeItr.HasNext()){
		vCurNode = vNodeItr.GetNext();
		vHasReturn = checkCode->GetHasReturn();
		if(!(*vCurNode)->CheckCode(checkCode, vCurNode)) vSuccess = false;
		if(vHasReturn && !checkCode->GetHasUnreachableCode()){
			checkCode->WarnUnreachableCode(*vCurNode);
			checkCode->SetHasUnreachableCode(true);
		}
	}
//	if(vSLCheckCode.GetHasReturn()) checkCode->SetHasReturn(true);
	return vSuccess;
}
void dspNodeStaList::CompileCode(dspParserCompileCode *compileCode){
	dspListNodeIterator vNodeItr(&p_Nodes);
	while(vNodeItr.HasNext()){
		(*vNodeItr.GetNext())->CompileCode(compileCode);
	}
}
#ifndef __NODBGPRINTF__
void dspNodeStaList::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- statement List");
	p_PostPrintDebug();
	dspListNodeIterator vNodeItr(&p_Nodes);
	while(vNodeItr.HasNext()){
		(*vNodeItr.GetNext())->DebugPrint(level+1, "");
	}
}
#endif

