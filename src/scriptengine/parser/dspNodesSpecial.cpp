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
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsVariable.h"
#include "../objects/dsConstant.h"

// auto cast node
dspNodeAutoCast::dspNodeAutoCast(dspBaseNode *Obj, dsClass *Type) : dspBaseNode(ntAutoCast,Obj){
	if(!Type) DSTHROW(dueInvalidParam);
	p_Obj = Obj; p_Type = Type;
}
dspNodeAutoCast::~dspNodeAutoCast(){
	delete p_Obj;
}
bool dspNodeAutoCast::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	return true;
}
void dspNodeAutoCast::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Obj->CompileCode(CompileCode);
	dsByteCode::sCodeCast code;
	code.code = dsByteCode::ebcCast;
	code.type = p_Type;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeAutoCast::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Auto Cast '%s'", p_Type->GetName());
	p_PostPrintDebug();
	p_Obj->DebugPrint(Level+1, "");
}
#endif


// assign init value to static variable
dspNodeSetStaVar::dspNodeSetStaVar(dspBaseNode *Init, dsVariable *Variable) : dspBaseNode(ntSetStaVar,Init){
	if(!Variable) DSTHROW(dueInvalidParam);
	p_Init = Init; p_Var = Variable; p_InitClass = NULL;
}
dspNodeSetStaVar::~dspNodeSetStaVar(){
	delete p_Init;
}
bool dspNodeSetStaVar::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vVarClass=p_Var->GetType();
	if(!(p_InitClass = CheckCode->CheckForObject(&p_Init))) return false;
//	if(vVarClass->GetPrimitiveType() == DSPT_OBJECT){
		if(!CheckCode->AutoNodeCast(&p_Init, p_InitClass, vVarClass)) return false;
		p_InitClass = vVarClass;
//	}else{
//		vInitNode = CheckCode->AutoNodeCast(p_Init, p_InitClass, vVarClass);
//		if(!vInitNode) return false;
//		p_Init = vInitNode; p_InitClass = vVarClass;
//	}
	CheckCode->SetTypeObject(vVarClass, false);
	return true;
}
void dspNodeSetStaVar::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Init->CompileCode(CompileCode);
	
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcPush;
	CompileCode->AddCode( code, GetLineNum() );
	
	//code.code = dsByteCode::ebcThis;
	//CompileCode->AddCode( code, GetLineNum() );
	
	dsByteCode::sCodeClassVar codeClsVar;
	codeClsVar.code = dsByteCode::ebcClsVar;
	codeClsVar.variable = p_Var;
	CompileCode->AddCode( codeClsVar, GetLineNum() );
	
	code.code = dsByteCode::ebcOpAss;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeSetStaVar::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Set Static Variable '%s'", p_Var->GetName());
	p_PostPrintDebug();
	if(p_Init) p_Init->DebugPrint(Level+1, "(init) ");
}
#endif


// enumeration value
dspNodeEnumValue::dspNodeEnumValue(dspBaseNode *RefNode, const char *name, int order) : dspBaseNode(ntEnumValue,RefNode){
	if(!name) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( name );
	if(!(pName = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( pName, size + 1, name, size );
	#else
		strncpy(pName, name, size + 1);
	#endif
	pName[ size ] = 0;
	pOrder = order;
}
dspNodeEnumValue::~dspNodeEnumValue(){
	delete [] pName;
}
bool dspNodeEnumValue::CheckCode( dspParserCheckCode *checkCode, dspBaseNode **ppThisNode ){
	if( ! checkCode || ! ppThisNode || ! *ppThisNode ){
		DSTHROW( dueInvalidParam );
	}
	
	checkCode->SetTypeObject( checkCode->GetOwnerClass(), true );
	return true;
}
void dspNodeEnumValue::CompileCode( dspParserCompileCode *compileCode ){
	if( ! compileCode ){
		DSTHROW( dueInvalidParam );
	}
	
	// int order
	dsByteCode::sCodeCInt codeOrder;
	codeOrder.code = dsByteCode::ebcCInt;
	codeOrder.value = pOrder;
	compileCode->AddCode( codeOrder, GetLineNum() );
	
	dsByteCode::sCode codePush;
	codePush.code = dsByteCode::ebcPush;
	compileCode->AddCode( codePush, GetLineNum() );
	
	// string name
	dsByteCode::sCodeCString codeName;
	codeName.code = dsByteCode::ebcCStr;
	codeName.index = compileCode->GetEngine()->AddConstString( pName );
	compileCode->AddCode( codeName, GetLineNum() );
	
	compileCode->AddCode( codePush, GetLineNum() );
	
	// constructor call
	dsByteCode::sCodeDirectCall codeCCall;
	codeCCall.code = dsByteCode::ebcCCall;
	codeCCall.function = compileCode->GetOwnerClass()->GetFirstConstructor();
	compileCode->AddCode( codeCCall, GetLineNum() );
}

#ifndef __NODBGPRINTF__
void dspNodeEnumValue::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Enumeration %s = %d", pName, pOrder);
	p_PostPrintDebug();
}
#endif

