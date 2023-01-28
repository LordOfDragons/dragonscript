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
#include "../objects/dsFunction.h"
#include "../objects/dsFuncList.h"
#include "../objects/dsSignature.h"

// operator node
//////////////////
dspNodeOperator::dspNodeOperator(dsScriptSource *Source, int LineNum, int CharNum, const char *Operator) : dspBaseNode(ntOperator,Source,LineNum,CharNum){
	if(!Operator) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Operator );
	if(!(p_Op = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Op, size + 1, Operator, size );
	#else
		strncpy(p_Op, Operator, size);
	#endif
	p_Op[ size ] = 0;
}
dspNodeOperator::~dspNodeOperator(){
	delete [] p_Op;
}
const char *dspNodeOperator::GetTerminalString(){ return (const char *)p_Op; }
#ifndef __NODBGPRINTF__
void dspNodeOperator::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Operator '%s'", p_Op);
	p_PostPrintDebug();
}
#endif

// unary operation node
/////////////////////////
dspNodeUnaryOperation::dspNodeUnaryOperation(dspNodeOperator *OpNode, dspBaseNode *Obj) : dspBaseNode(ntUnaryOp,OpNode){
	if(!Obj) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( OpNode->GetOperator() );
	if(!(p_Op = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Op, size + 1, OpNode->GetOperator(), size );
	#else
		strncpy(p_Op, OpNode->GetOperator(), size);
	#endif
	p_Op[ size ] = 0;
	p_Obj = Obj;
	p_ObjClass = NULL;
	p_FuncID = -11;
}
dspNodeUnaryOperation::dspNodeUnaryOperation(dspBaseNode *RefNode, const char *Op, dspBaseNode *Obj) : dspBaseNode(ntUnaryOp,RefNode){
	if(!Obj || !Op) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Op );
	if(!(p_Op = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Op, size + 1, Op, size );
	#else
		strncpy(p_Op, Op, size);
	#endif
	p_Op[ size ] = 0;
	p_Obj = Obj;
	p_ObjClass = NULL;
	p_FuncID = -1;
}
dspNodeUnaryOperation::~dspNodeUnaryOperation(){
	delete [] p_Op; delete p_Obj;
}
dspBaseNode *dspNodeUnaryOperation::Simplify(){
	dspBaseNode *vNewNode=NULL;
	int vIntVal=0;
	float vFltVal=0;
	try{
		if(p_Obj->GetNodeType() == ntByte){
			vIntVal = ((dspNodeByte*)p_Obj)->GetValue();
		}else if(p_Obj->GetNodeType() == ntInt){
			vIntVal = ((dspNodeInt*)p_Obj)->GetValue();
		}else if(p_Obj->GetNodeType() == ntFloat){
			vFltVal = ((dspNodeFloat*)p_Obj)->GetValue();
		}else{
			return NULL;
		}
		if(p_Obj->GetNodeType()==ntByte || p_Obj->GetNodeType()==ntInt){
			if(strcmp(p_Op, "+") == 0){
				if(!(vNewNode = new dspNodeInt(p_Obj, vIntVal))) DSTHROW(dueOutOfMemory);
			}else if(strcmp(p_Op, "-") == 0){
				if(!(vNewNode = new dspNodeInt(p_Obj, -vIntVal))) DSTHROW(dueOutOfMemory);
			}else if(strcmp(p_Op, "~") == 0){
				if(!(vNewNode = new dspNodeInt(p_Obj, ~vIntVal))) DSTHROW(dueOutOfMemory);
			}
		}else if(p_Obj->GetNodeType() == ntFloat){
			if(strcmp(p_Op, "+") == 0){
				if(!(vNewNode = new dspNodeFloat(p_Obj, vFltVal))) DSTHROW(dueOutOfMemory);
			}else if(strcmp(p_Op, "-") == 0){
				if(!(vNewNode = new dspNodeFloat(p_Obj, -vFltVal))) DSTHROW(dueOutOfMemory);
			}
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
bool dspNodeUnaryOperation::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsFuncList *vFuncList;
	dsFunction *vOpFunc;
	dspBaseNode *vSimpleNode;
	dsClass *vRetType;
	dsSignature vFuncSig;
	if(!(p_ObjClass = CheckCode->CheckForObject(&p_Obj))) return false;
	if(p_ObjClass->GetPrimitiveType() == DSPT_OBJECT){
		vFuncList = p_ObjClass->GetFuncList();
		vOpFunc = vFuncList->FindBestFunction(NULL, p_Op, &vFuncSig);
		if(!vOpFunc){
			CheckCode->ErrorInvOpSig(this, p_ObjClass, p_Op, NULL); return false;
		}
		p_FuncID = vFuncList->GetIndexOf(vOpFunc);
		vRetType = vOpFunc->GetType();
	}else if(p_ObjClass->GetPrimitiveType() == DSPT_NULL){
		CheckCode->ErrorInvPrimOp(this, "null", p_Op); return false;
	}else{
		if(strcmp(p_Op,"++")==0 || strcmp(p_Op,"--")==0){
			if(CheckCode->GetIsObjConst()){
				CheckCode->ErrorConstObj(this, p_Op); return false;
			}
		}
		vRetType = CheckCode->GetUnOpReturnType(p_ObjClass, p_Op);
		if(!vRetType){
			CheckCode->ErrorInvPrimOp(this, p_ObjClass->GetName(), p_Op); return false;
		}
	}
	if( ( vSimpleNode = Simplify() ) ){
		CheckCode->SetTypeObject(vRetType, true);
		delete *ppThisNode; *ppThisNode = vSimpleNode;
	}else{
		CheckCode->SetTypeObject(vRetType, strcmp(p_Op,"++")!=0 && strcmp(p_Op,"--")!=0);
	}
	return true;
}
void dspNodeUnaryOperation::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Obj->CompileCode(CompileCode);
	if(p_ObjClass->GetPrimitiveType() == DSPT_OBJECT){
		dsByteCode::sCodeCall code;
		code.code = dsByteCode::ebcCall;
		code.index = p_FuncID;
		CompileCode->AddCode( code, GetLineNum() );
	}else{
		dsByteCode::sCode code;
		if(strcmp(p_Op, "++") == 0){
			code.code = dsByteCode::ebcOpInc;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "--") == 0){
			code.code = dsByteCode::ebcOpDec;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "-") == 0){
			code.code = dsByteCode::ebcOpMin;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "~") == 0){
			code.code = dsByteCode::ebcOpInv;
			CompileCode->AddCode( code, GetLineNum() );
		}
	}
}
#ifndef __NODBGPRINTF__
void dspNodeUnaryOperation::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Unary Operation '%s'", p_Op);
	p_PostPrintDebug();
	p_Obj->DebugPrint(Level+1, "");
}
#endif

// post increment node
////////////////////////
dspNodePostIncrement::dspNodePostIncrement(dspBaseNode *RefNode, dspBaseNode *Obj) : dspBaseNode(ntPostInc,RefNode){
	if(!Obj) DSTHROW(dueInvalidParam);
	p_Obj = Obj;
	p_ObjClass = NULL;
}
dspNodePostIncrement::~dspNodePostIncrement(){
	if(p_Obj) delete p_Obj;
}
bool dspNodePostIncrement::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vRetType;
	if(!(p_ObjClass = CheckCode->CheckForObject(&p_Obj))) return false;
	if(p_ObjClass->GetPrimitiveType() > DSPT_FLOAT){
		CheckCode->ErrorInvPrimOp(this, p_ObjClass->GetName(), "(post)++"); return false;
	}
	vRetType = CheckCode->GetUnOpReturnType(p_ObjClass, "++");
	if(!vRetType){
		CheckCode->ErrorInvPrimOp(this, p_ObjClass->GetName(), "(post)++"); return false;
	}
	CheckCode->SetTypeObject(vRetType, true);
	return true;
}
void dspNodePostIncrement::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Obj->CompileCode(CompileCode);
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcPInc;
	CompileCode->AddCode( code, GetLineNum() );
}
dspBaseNode *dspNodePostIncrement::Reduce(){
	dspNodeUnaryOperation *vNodeUnOp = NULL;
	dspBaseNode *vNodeObj = NULL;
	try{
		if(p_Obj->GetNodeType() == ntPostInc){
			vNodeObj = ((dspNodePostIncrement*)p_Obj)->Reduce();
			delete p_Obj; p_Obj = vNodeObj; vNodeObj = NULL;
		}else if(p_Obj->GetNodeType() == ntPostDec){
			vNodeObj = ((dspNodePostDecrement*)p_Obj)->Reduce();
			delete p_Obj; p_Obj = vNodeObj; vNodeObj = NULL;
		}
		if(!(vNodeUnOp = new dspNodeUnaryOperation(this, "++", p_Obj))) DSTHROW(dueOutOfMemory);
		p_Obj = NULL;
	}catch( ... ){
		if(vNodeObj) delete vNodeObj;
		if(vNodeUnOp) delete vNodeUnOp;
	}
	return vNodeUnOp;
}
#ifndef __NODBGPRINTF__
void dspNodePostIncrement::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Post Increment");
	p_PostPrintDebug();
	if(p_Obj) p_Obj->DebugPrint(Level+1, "");
}
#endif

// post decrement node
////////////////////////
dspNodePostDecrement::dspNodePostDecrement(dspBaseNode *RefNode, dspBaseNode *Obj) : dspBaseNode(ntPostDec,RefNode){
	if(!Obj) DSTHROW(dueInvalidParam);
	p_Obj = Obj;
	p_ObjClass = NULL;
}
dspNodePostDecrement::~dspNodePostDecrement(){
	delete p_Obj;
}
bool dspNodePostDecrement::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vRetType;
	if(!(p_ObjClass = CheckCode->CheckForObject(&p_Obj))) return false;
	if(p_ObjClass->GetPrimitiveType() > DSPT_FLOAT){
		CheckCode->ErrorInvPrimOp(this, p_ObjClass->GetName(), "(post)--"); return false;
	}
	vRetType = CheckCode->GetUnOpReturnType(p_ObjClass, "--");
	if(!vRetType){
		CheckCode->ErrorInvPrimOp(this, p_ObjClass->GetName(), "(post)--"); return false;
	}
	CheckCode->SetTypeObject(vRetType, true);
	return true;
}
void dspNodePostDecrement::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Obj->CompileCode(CompileCode);
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcPDec;
	CompileCode->AddCode( code, GetLineNum() );
}
dspBaseNode *dspNodePostDecrement::Reduce(){
	dspNodeUnaryOperation *vNodeUnOp = NULL;
	dspBaseNode *vNodeObj = NULL;
	try{
		if(p_Obj->GetNodeType() == ntPostInc){
			vNodeObj = ((dspNodePostIncrement*)p_Obj)->Reduce();
			delete p_Obj; p_Obj = vNodeObj; vNodeObj = NULL;
		}else if(p_Obj->GetNodeType() == ntPostDec){
			vNodeObj = ((dspNodePostDecrement*)p_Obj)->Reduce();
			delete p_Obj; p_Obj = vNodeObj; vNodeObj = NULL;
		}
		if(!(vNodeUnOp = new dspNodeUnaryOperation(this, "--", p_Obj))) DSTHROW(dueOutOfMemory);
		p_Obj = NULL;
	}catch( ... ){
		if(vNodeObj) delete vNodeObj;
		if(vNodeUnOp) delete vNodeUnOp;
	}
	return vNodeUnOp;
}
#ifndef __NODBGPRINTF__
void dspNodePostDecrement::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Post Decrement");
	p_PostPrintDebug();
	p_Obj->DebugPrint(Level+1, "");
}
#endif

// binary operation node
//////////////////////////
dspNodeBinaryOperation::dspNodeBinaryOperation(dspNodeOperator *OpNode, dspBaseNode *Obj, dspBaseNode *Arg) : dspBaseNode(ntBinaryOp,OpNode){
	if(!Obj || !Arg) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( OpNode->GetOperator() );
	if(!(p_Op = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Op, size + 1, OpNode->GetOperator(), size );
	#else
		strncpy(p_Op, OpNode->GetOperator(), size);
	#endif
	p_Op[ size ] = 0;
	p_Obj = Obj; p_Arg = Arg;
	p_ObjClass = p_ArgClass = NULL;
	p_FuncID = -1;
}
dspNodeBinaryOperation::dspNodeBinaryOperation(dspBaseNode *RefNode, const char *Op, dspBaseNode *Obj, dspBaseNode *Arg) : dspBaseNode(ntBinaryOp,RefNode){
	if(!Op || !Obj || !Arg) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Op );
	if(!(p_Op = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Op, size + 1, Op, size );
	#else
		strncpy(p_Op, Op, size);
	#endif
	p_Op[ size ] = 0;
	p_Obj = Obj; p_Arg = Arg;
	p_ObjClass = p_ArgClass = NULL;
	p_FuncID = -1;
}
dspNodeBinaryOperation::~dspNodeBinaryOperation(){
	delete [] p_Op; delete p_Obj; delete p_Arg;
}
dspBaseNode *dspNodeBinaryOperation::Simplify(){
	dspBaseNode *vNewNode = NULL;
	float vFltVal1 = 0.0f;
	float vFltVal2 = 0.0f;
	int vIntVal1 = 0;
	int vIntVal2 = 0;
	
	try{
		// convert first value to int and float
		if( p_Obj->GetNodeType() == ntByte ){
			vIntVal1 = ( int )( ( ( dspNodeByte* )p_Obj )->GetValue() );
			vFltVal1 = ( float )vIntVal1;
			
		}else if( p_Obj->GetNodeType() == ntBool ){
			vIntVal1 = ( ( ( dspNodeBool* )p_Obj )->GetValue() != 0 );
			vFltVal1 = ( ( ( dspNodeBool* )p_Obj )->GetValue() != 0.0f );
			
		}else if( p_Obj->GetNodeType() == ntInt ){
			vIntVal1 = ( ( dspNodeInt* )p_Obj )->GetValue();
			vFltVal1 = ( float )vIntVal1;
			
		}else if( p_Obj->GetNodeType() == ntFloat ){
			vFltVal1 = ( ( dspNodeFloat* )p_Obj )->GetValue();
			vIntVal1 = ( int )vFltVal1;
			
		}else{
			return NULL;
		}
		
		// convert second value to int and float
		if( p_Arg->GetNodeType() == ntByte ){
			vIntVal2 = ( int )( ( ( dspNodeByte* )p_Arg )->GetValue() );
			vFltVal2 = ( float )vIntVal2;
			
		}else if( p_Arg->GetNodeType() == ntBool ){
			vIntVal2 = ( ( ( dspNodeBool* )p_Arg )->GetValue() != 0 );
			vFltVal2 = ( ( ( dspNodeBool* )p_Arg )->GetValue() != 0.0f );
			
		}else if( p_Arg->GetNodeType() == ntInt ){
			vIntVal2 = ( ( dspNodeInt* )p_Arg )->GetValue();
			vFltVal2 = ( float )vIntVal2;
			
		}else if( p_Arg->GetNodeType() == ntFloat ){
			vFltVal2 = ( ( dspNodeFloat* )p_Arg )->GetValue();
			vIntVal2 = ( int )vFltVal2;
			
		}else{
			return NULL;
		}
		
		// apply operation using the matching values
		if( p_Obj->GetNodeType() == ntFloat || p_Arg->GetNodeType() == ntFloat ){
			if( strcmp( p_Op, "*" ) == 0 ){
				vNewNode = new dspNodeFloat( p_Obj, vFltVal1 * vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "/" ) == 0 ){
				if( vFltVal2 == 0.0f ){
					return NULL;
				}
				
				vNewNode = new dspNodeFloat( p_Obj, vFltVal1 / vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "+" ) == 0 ){
				vNewNode = new dspNodeFloat( p_Obj, vFltVal1 + vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "-" ) == 0 ){
				vNewNode = new dspNodeFloat( p_Obj, vFltVal1 - vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "<" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vFltVal1 < vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, ">" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vFltVal1 > vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "<=" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vFltVal1 <= vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, ">=" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vFltVal1 >= vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "==" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vFltVal1 == vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "!=" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vFltVal1 != vFltVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
			}
			
		}else{
			if( strcmp( p_Op, "*" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 * vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "/" ) == 0 ){
				if( vIntVal2 == 0 ){
					return NULL;
				}
				
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 / vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "%" ) == 0 ){
				if( vIntVal2 == 0 ){
					return NULL;
				}
				
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 % vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "+" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 + vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "-" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 - vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "<<" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 << vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, ">>" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 >> vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "<" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vIntVal1 < vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, ">" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vIntVal1 > vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "<=" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vIntVal1 <= vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, ">=" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vIntVal1 >= vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "==" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vIntVal1 == vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "!=" ) == 0 ){
				vNewNode = new dspNodeBool( p_Obj, vIntVal1 != vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "&" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 & vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "|" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 | vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
				
			}else if( strcmp( p_Op, "^" ) == 0 ){
				vNewNode = new dspNodeInt( p_Obj, vIntVal1 ^ vIntVal2 );
				if( ! vNewNode ){
					DSTHROW( dueOutOfMemory );
				}
			}
		}
		
	}catch( ... ){
		if( vNewNode ){
			delete vNewNode;
			throw;
		}
	}
	return vNewNode;
}

bool dspNodeBinaryOperation::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	const char *vAssOps[] = {"=","*=","/=","%=","+=","-=","<<=",">>=","&=","|=","^="};
	bool isAssignOp = false;
	dsFuncList *vFuncList;
	dsFunction *vOpFunc;
	dspBaseNode *vSimpleNode;
	dsClass *vRetType;
	dsSignature vFuncSig;
	bool vIsLValueConst;
	
	if(!(p_ObjClass = CheckCode->CheckForObject(&p_Obj))) return false;
	vIsLValueConst = CheckCode->GetIsObjConst();
	if(!(p_ArgClass = CheckCode->CheckForObject(&p_Arg))) return false;
	if( CheckCode->MatchesOperators( p_Op, &vAssOps[ 0 ], 11 ) ){
		isAssignOp = true;
		if( vIsLValueConst ){
			CheckCode->ErrorConstObj( this, p_Op );
			return false;
		}
	}
	
	if(p_ObjClass->GetPrimitiveType() == DSPT_OBJECT){
		if(strcmp(p_Op, "=") == 0){
			if(!CheckCode->AutoNodeCast(&p_Arg, p_ArgClass, p_ObjClass)) return false;
			vRetType = p_ArgClass = p_ObjClass;
		}else if(strcmp(p_Op,"==")==0 || strcmp(p_Op,"!=")==0){
			if(p_ArgClass->GetPrimitiveType() < DSPT_NULL){
				CheckCode->ErrorInvTypeCast(p_Arg, p_ArgClass, p_ObjClass); return false;
			}
			vRetType = CheckCode->GetEngine()->GetClassBool();
		}else{
			vFuncList = p_ObjClass->GetFuncList();
			vFuncSig.AddParameter( p_ArgClass );
			vOpFunc = vFuncList->FindBestFunction(NULL, p_Op, &vFuncSig);
			if(!vOpFunc){
				CheckCode->ErrorInvOpSig(this, p_ObjClass, p_Op, p_ArgClass); return false;
			}
			p_FuncID = vFuncList->GetIndexOf(vOpFunc);
			vRetType = vOpFunc->GetType();
			if( ! CheckCode->AutoNodeCast( &p_Arg, p_ArgClass, vOpFunc->GetSignature()->GetParameter( 0 ) ) ) return false;
		}
		
	}else if(p_ObjClass->GetPrimitiveType() == DSPT_NULL){
		if(strcmp(p_Op,"==")==0 || strcmp(p_Op,"!=")==0){
			if(p_ArgClass->GetPrimitiveType() < DSPT_NULL){
				CheckCode->ErrorInvTypeCast(p_Arg, p_ArgClass, p_ObjClass); return false;
			}
			vRetType = CheckCode->GetEngine()->GetClassBool();
		}else{
			CheckCode->ErrorInvPrimOp(this, "null", p_Op); return false;
		}
//		CheckCode->ErrorInvPrimOp(this, "null", p_Op); return false;
		
	}else{
		if( isAssignOp ){
			if( ! CheckCode->AutoNodeCast( &p_Arg, p_ArgClass, p_ObjClass ) ){
				return false;
			}
			
			p_ArgClass = p_ObjClass;
			
		}else{
			vSimpleNode = Simplify();
			
			if( vSimpleNode ){
				delete *ppThisNode;
				*ppThisNode = vSimpleNode;
				
				return vSimpleNode->CheckCode( CheckCode, ppThisNode );
				
			}else{
				if( ! CheckCode->AutoNodeCastBinOp( &p_Obj, &p_ObjClass, &p_Arg, &p_ArgClass ) ){
					return false;
				}
			}
		}
		
		vRetType = CheckCode->GetBinOpReturnType( p_ObjClass, p_Op );
		
		if( ! vRetType ){
			CheckCode->ErrorInvPrimOp( this, p_ObjClass->GetName(), p_Op );
			return false;
		}
	}
	
	CheckCode->SetTypeObject( vRetType, ! ( vRetType->GetPrimitiveType() == DSPT_OBJECT ) );
	
	return true;
}
void dspNodeBinaryOperation::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCode code;
	
	p_Arg->CompileCode(CompileCode);
	code.code = dsByteCode::ebcPush;
	CompileCode->AddCode( code, GetLineNum() );
	p_Obj->CompileCode(CompileCode);
	if(p_ObjClass->GetPrimitiveType() == DSPT_OBJECT){
		if(strcmp(p_Op, "=") == 0){
			code.code = dsByteCode::ebcOpAss;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "==") == 0){
			code.code = dsByteCode::ebcOpEq;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "!=") == 0){
			code.code = dsByteCode::ebcOpNEq;
			CompileCode->AddCode( code, GetLineNum() );
		}else{
			dsByteCode::sCodeCall codeCall;
			codeCall.code = dsByteCode::ebcCall;
			codeCall.index = p_FuncID;
			CompileCode->AddCode( codeCall, GetLineNum() );
		}
	}else{
		if(strcmp(p_Op, "*") == 0){
			code.code = dsByteCode::ebcOpMul;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "/") == 0){
			code.code = dsByteCode::ebcOpDiv;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "%") == 0){
			code.code = dsByteCode::ebcOpMod;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "+") == 0){
			code.code = dsByteCode::ebcOpAdd;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "-") == 0){
			code.code = dsByteCode::ebcOpSub;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "<<") == 0){
			code.code = dsByteCode::ebcOpLS;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, ">>") == 0){
			code.code = dsByteCode::ebcOpRS;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "<") == 0){
			code.code = dsByteCode::ebcOpLe;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, ">") == 0){
			code.code = dsByteCode::ebcOpGr;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "<=") == 0){
			code.code = dsByteCode::ebcOpLEq;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, ">=") == 0){
			code.code = dsByteCode::ebcOpGEq;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "==") == 0){
			code.code = dsByteCode::ebcOpEq;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "!=") == 0){
			code.code = dsByteCode::ebcOpNEq;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "&") == 0){
			code.code = dsByteCode::ebcOpAnd;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "|") == 0){
			code.code = dsByteCode::ebcOpOr;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "^") == 0){
			code.code = dsByteCode::ebcOpXor;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "=") == 0){
			code.code = dsByteCode::ebcOpAss;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "*=") == 0){
			code.code = dsByteCode::ebcOpMulA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "/=") == 0){
			code.code = dsByteCode::ebcOpDivA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "%=") == 0){
			code.code = dsByteCode::ebcOpModA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "+=") == 0){
			code.code = dsByteCode::ebcOpAddA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "-=") == 0){
			code.code = dsByteCode::ebcOpSubA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "<<=") == 0){
			code.code = dsByteCode::ebcOpLSA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, ">>=") == 0){
			code.code = dsByteCode::ebcOpRSA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "&=") == 0){
			code.code = dsByteCode::ebcOpAndA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "|=") == 0){
			code.code = dsByteCode::ebcOpOrA;
			CompileCode->AddCode( code, GetLineNum() );
		}else if(strcmp(p_Op, "^=") == 0){
			code.code = dsByteCode::ebcOpXorA;
			CompileCode->AddCode( code, GetLineNum() );
		}
	}
}
#ifndef __NODBGPRINTF__
void dspNodeBinaryOperation::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Binary Operation '%s'", p_Op);
	p_PostPrintDebug();
	p_Obj->DebugPrint(Level+1, "(op1) ");
	p_Arg->DebugPrint(Level+1, "(op2) ");
}
#endif


// inline if node
///////////////////
dspNodeInlineIf::dspNodeInlineIf(dspBaseNode *RefNode, dspBaseNode *Condition, dspBaseNode *IfPart, dspBaseNode *ElsePart) : dspBaseNode(ntInlineIf,RefNode){
	if(!Condition || !IfPart || !ElsePart) DSTHROW(dueInvalidParam);
	p_Cond = Condition; p_If = IfPart; p_Else = ElsePart;
}
dspNodeInlineIf::~dspNodeInlineIf(){
	delete p_Cond;
	if(p_If) delete p_If;
	if(p_Else) delete p_Else;
}
bool dspNodeInlineIf::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dspBaseNode *vSimpleNode;
	dsClass *vClsBool = CheckCode->GetEngine()->GetClassBool();
	dsClass *vCondType, *vIfType, *vElseType;
	if(!(vCondType = CheckCode->CheckForObject(&p_Cond))) return false;
	if(!CheckCode->AutoNodeCast(&p_Cond, vCondType, vClsBool)) return false;
	if(!(vIfType = CheckCode->CheckForObject(&p_If))) return false;
	if(!(vElseType = CheckCode->CheckForObject(&p_Else))) return false;
	if(!CheckCode->AutoNodeCast(&p_Else, vElseType, vIfType)) return false;
	if(p_Cond->GetNodeType() == ntBool){
		if( ((dspNodeBool*)p_Cond)->GetValue() ){
			vSimpleNode = p_If; p_If = NULL;
		}else{
			vSimpleNode = p_Else; p_Else = NULL;
		}
		delete *ppThisNode; *ppThisNode = vSimpleNode;
	}
	CheckCode->SetTypeObject(vIfType, true);
	return true;
}
void dspNodeInlineIf::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	int vBlockBegin = CompileCode->GetCurOffset();
	// compile condition
	p_Cond->CompileCode(CompileCode);
	CompileCode->AddJump(dspParserCompileCode::ettElse, dsByteCode::ebcJNEB);
	// compile if block
	p_If->CompileCode(CompileCode);
	CompileCode->AddJump(dspParserCompileCode::ettIfEnd, dsByteCode::ebcJMPB);
	// compile else block
	CompileCode->AddTarget(dspParserCompileCode::ettElse, vBlockBegin, CompileCode->GetCurOffset());
	p_Else->CompileCode(CompileCode);
	CompileCode->AddTarget(dspParserCompileCode::ettIfEnd, vBlockBegin, CompileCode->GetCurOffset());
}
#ifndef __NODBGPRINTF__
void dspNodeInlineIf::DebugPrint(int Level, const char *Prefix){
	if(!p_If || !p_Else) DSTHROW(dueInvalidParam);
	p_PrePrintDebug(Level, Prefix);
	printf("- Inline if");
	p_PostPrintDebug();
	p_Cond->DebugPrint(Level+1, "(cond) ");
	p_If->DebugPrint(Level+1, "(if) ");
	p_Else->DebugPrint(Level+1, "(else) ");
}
#endif


// logical and node
/////////////////////
dspNodeLogicalAnd::dspNodeLogicalAnd(dspBaseNode *RefNode, dspBaseNode *Condition1, dspBaseNode *Condition2) : dspBaseNode(ntLogicalAnd,RefNode){
	if(!Condition1 || !Condition2) DSTHROW(dueInvalidParam);
	p_Cond1 = Condition1; p_Cond2 = Condition2;
}
dspNodeLogicalAnd::~dspNodeLogicalAnd(){
	delete p_Cond1; if(p_Cond2) delete p_Cond2;
}
bool dspNodeLogicalAnd::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsBool = CheckCode->GetEngine()->GetClassBool();
	dspBaseNode *vSimpleNode;
	dsClass *vCondType;
	if(!(vCondType = CheckCode->CheckForObject(&p_Cond1))) return false;
	if(!CheckCode->AutoNodeCast(&p_Cond1, vCondType, vClsBool)) return false;
	if(!(vCondType = CheckCode->CheckForObject(&p_Cond2))) return false;
	if(!CheckCode->AutoNodeCast(&p_Cond2, vCondType, vClsBool)) return false;
	if(p_Cond1->GetNodeType() == ntBool){
		if( ((dspNodeBool*)p_Cond1)->GetValue() ){
			vSimpleNode = p_Cond2; p_Cond2 = NULL;
		}else{
			if(!(vSimpleNode = new dspNodeBool(this, false))) DSTHROW(dueOutOfMemory);
		}
		delete *ppThisNode; *ppThisNode = vSimpleNode;
	}
	CheckCode->SetTypeObject(vClsBool, true);
	return true;
}
void dspNodeLogicalAnd::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	int vBlockBegin = CompileCode->GetCurOffset();
	// compile first condition
	p_Cond1->CompileCode(CompileCode);
	CompileCode->AddJump(dspParserCompileCode::ettEnd, dsByteCode::ebcJNEB);
	// compile second condition
	p_Cond2->CompileCode(CompileCode);
	CompileCode->AddTarget(dspParserCompileCode::ettEnd, vBlockBegin, CompileCode->GetCurOffset());
}
#ifndef __NODBGPRINTF__
void dspNodeLogicalAnd::DebugPrint(int Level, const char *Prefix){
	if(!p_Cond2) DSTHROW(dueInvalidParam);
	p_PrePrintDebug(Level, Prefix);
	printf("- Logical And");
	p_PostPrintDebug();
	p_Cond1->DebugPrint(Level+1, "(cond1) ");
	p_Cond2->DebugPrint(Level+1, "(cond2) ");
}
#endif


// logical or node
////////////////////
dspNodeLogicalOr::dspNodeLogicalOr(dspBaseNode *RefNode, dspBaseNode *Condition1, dspBaseNode *Condition2) : dspBaseNode(ntLogicalOr,RefNode){
	if(!Condition1 || !Condition2) DSTHROW(dueInvalidParam);
	p_Cond1 = Condition1; p_Cond2 = Condition2;
}
dspNodeLogicalOr::~dspNodeLogicalOr(){
	delete p_Cond1; delete p_Cond2;
}
bool dspNodeLogicalOr::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsBool = CheckCode->GetEngine()->GetClassBool();
	dspBaseNode *vSimpleNode;
	dsClass *vCondType;
	if(!(vCondType = CheckCode->CheckForObject(&p_Cond1))) return false;
	if(!CheckCode->AutoNodeCast(&p_Cond1, vCondType, vClsBool)) return false;
	if(!(vCondType = CheckCode->CheckForObject(&p_Cond2))) return false;
	if(!CheckCode->AutoNodeCast(&p_Cond2, vCondType, vClsBool)) return false;
	if(p_Cond1->GetNodeType() == ntBool){
		if( ((dspNodeBool*)p_Cond1)->GetValue() ){
			if(!(vSimpleNode = new dspNodeBool(this, true))) DSTHROW(dueOutOfMemory);
		}else{
			vSimpleNode = p_Cond2; p_Cond2 = NULL;
		}
		delete *ppThisNode; *ppThisNode = vSimpleNode;
	}
	CheckCode->SetTypeObject(vClsBool, true);
	return true;
}
void dspNodeLogicalOr::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	int vBlockBegin = CompileCode->GetCurOffset();
	// compile first condition
	p_Cond1->CompileCode(CompileCode);
	CompileCode->AddJump(dspParserCompileCode::ettEnd, dsByteCode::ebcJEQB);
	// compile second condition
	p_Cond2->CompileCode(CompileCode);
	CompileCode->AddTarget(dspParserCompileCode::ettEnd, vBlockBegin, CompileCode->GetCurOffset());
}
#ifndef __NODBGPRINTF__
void dspNodeLogicalOr::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Logical Or");
	p_PostPrintDebug();
	p_Cond1->DebugPrint(Level+1, "(cond1) ");
	p_Cond2->DebugPrint(Level+1, "(cond2) ");
}
#endif


// logical not node
/////////////////////
dspNodeLogicalNot::dspNodeLogicalNot(dspBaseNode *RefNode, dspBaseNode *Condition) : dspBaseNode(ntLogicalOr,RefNode){
	if(!Condition) DSTHROW(dueInvalidParam);
	p_Cond = Condition;
}
dspNodeLogicalNot::~dspNodeLogicalNot(){
	delete p_Cond;
}
bool dspNodeLogicalNot::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vClsBool = CheckCode->GetEngine()->GetClassBool();
	dspBaseNode *vSimpleNode;
	dsClass *vCondType;
	if(!(vCondType = CheckCode->CheckForObject(&p_Cond))) return false;
	if(!CheckCode->AutoNodeCast(&p_Cond, vCondType, vClsBool)) return false;
	if(p_Cond->GetNodeType() == ntBool){
		if(!(vSimpleNode = new dspNodeBool(this, ! ((dspNodeBool*)p_Cond)->GetValue()) )) DSTHROW(dueOutOfMemory);
		delete *ppThisNode; *ppThisNode = vSimpleNode;
	}
	CheckCode->SetTypeObject(vClsBool, true);
	return true;
}
void dspNodeLogicalNot::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Cond->CompileCode(CompileCode);
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcOpNot;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeLogicalNot::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Logical Not");
	p_PostPrintDebug();
	p_Cond->DebugPrint(Level+1, "(cond) ");
}
#endif


// castto node
////////////////
dspNodeCastTo::dspNodeCastTo(dspBaseNode *RefNode, dspBaseNode *Object, dspBaseNode *Type) : dspBaseNode(ntCastTo,RefNode){
	if(!Object || !Type) DSTHROW(dueInvalidParam);
	p_Obj = Object; p_Type = Type; p_DSClass = NULL;
}
dspNodeCastTo::~dspNodeCastTo(){
	delete p_Obj; delete p_Type;
}

dspBaseNode *dspNodeCastTo::Simplify(){
	dspBaseNode *newNode = NULL;
	dspNodeMembVar *membVarNode;
	float fltVal;
	int intVal;
	
	try{
		if( p_Obj->GetNodeType() == ntByte ){
			intVal = ( ( dspNodeByte* )p_Obj )->GetValue();
			fltVal = ( float )intVal;
			
		}else if( p_Obj->GetNodeType() == ntBool ){
			intVal = ( ( dspNodeBool* )p_Obj )->GetValue();
			fltVal = ( float )intVal;
			
		}else if( p_Obj->GetNodeType() == ntInt ){
			intVal = ( ( dspNodeInt* )p_Obj )->GetValue();
			fltVal = ( float )intVal;
			
		}else if(p_Obj->GetNodeType() == ntFloat){
			fltVal = ( ( dspNodeFloat* )p_Obj )->GetValue();
			intVal = ( int )fltVal;
			
		}else{
			return NULL;
		}
		
		if( p_Type->GetNodeType() == ntMembVar ){
			membVarNode = ( dspNodeMembVar* )p_Type;
			
			if( ! membVarNode->GetRealObject() ){
				if( strcmp( membVarNode->GetName(), "byte" ) == 0 ){
					newNode = new dspNodeByte( p_Obj, ( byte )intVal );
					if( ! newNode ){
						DSTHROW( dueOutOfMemory );
					}
					
				}else if( strcmp( membVarNode->GetName(), "bool" ) == 0 ){
					newNode = new dspNodeBool( p_Obj, ( bool )intVal );
					if( ! newNode ){
						DSTHROW( dueOutOfMemory );
					}
					
				}else if( strcmp( membVarNode->GetName(), "int" ) == 0 ){
					newNode = new dspNodeInt( p_Obj, intVal );
					if( ! newNode ){
						DSTHROW( dueOutOfMemory );
					}
					
				}else if( strcmp( membVarNode->GetName(), "float" ) == 0 ){
					newNode = new dspNodeFloat( p_Obj, fltVal );
					if( ! newNode ){
						DSTHROW(dueOutOfMemory);
					}
				}
			}
		}
		
	}catch( ... ){
		if( newNode){
			delete newNode;
		}
		throw;
	}
	
	return newNode;
}

bool dspNodeCastTo::CheckCode( dspParserCheckCode *checkCode, dspBaseNode **ppThisNode ){
	if( ! checkCode || ! ppThisNode || ! *ppThisNode ){
		DSTHROW( dueInvalidParam );
	}
	
	dspBaseNode *simpleNode;
	dsClass *objType;
	
	objType = checkCode->CheckForObject( &p_Obj );
	if( ! objType ){
		return false;
	}
	
	p_DSClass = checkCode->CheckForClass( &p_Type );
	if( ! p_DSClass ){
		return false;
	}
	
	simpleNode = Simplify();
	
	checkCode->SetTypeObject( p_DSClass, ! ( p_DSClass->GetPrimitiveType() == DSPT_OBJECT ) );
	
	if( simpleNode ){
		delete *ppThisNode;
		*ppThisNode = simpleNode;
	}
	
	return true;
}

void dspNodeCastTo::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Obj->CompileCode(CompileCode);
	dsByteCode::sCodeCast code;
	code.code = dsByteCode::ebcCast;
	code.type = p_DSClass;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeCastTo::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- CastTo");
	p_PostPrintDebug();
	p_Obj->DebugPrint(Level+1, "(obj) ");
	p_Type->DebugPrint(Level+1, "(type) ");
}
#endif


// castableto node
////////////////////
dspNodeCastableTo::dspNodeCastableTo(dspBaseNode *RefNode, dspBaseNode *Object, dspBaseNode *Type) : dspBaseNode(ntCastableTo,RefNode){
	if(!Object || !Type) DSTHROW(dueInvalidParam);
	p_Obj = Object; p_Type = Type; p_DSClass = NULL;
}
dspNodeCastableTo::~dspNodeCastableTo(){
	delete p_Obj; delete p_Type;
}
dspBaseNode *dspNodeCastableTo::Simplify(){
	dspBaseNode *vNewNode=NULL;
	dspNodeMembVar *vMembVarNode;
	try{
		if( p_Obj->GetNodeType()==ntByte || p_Obj->GetNodeType()==ntBool ||
			p_Obj->GetNodeType()==ntInt || p_Obj->GetNodeType()==ntFloat )
		{
			if(p_Type->GetNodeType() == ntMembVar){
				vMembVarNode = (dspNodeMembVar*)p_Type;
				if(!vMembVarNode->GetRealObject()){
					if( strcmp(vMembVarNode->GetName(), "byte")==0 ||
						strcmp(vMembVarNode->GetName(), "bool")==0 ||
						strcmp(vMembVarNode->GetName(), "int")==0 ||
						strcmp(vMembVarNode->GetName(), "float")==0 )
					{
						if(!(vNewNode = new dspNodeBool(p_Obj, true))) DSTHROW(dueOutOfMemory);
					}else{
						if(!(vNewNode = new dspNodeBool(p_Obj, false))) DSTHROW(dueOutOfMemory);
					}
				}
			}
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
bool dspNodeCastableTo::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dspBaseNode *vSimpleNode;
	if(!CheckCode->CheckForObject(&p_Obj)) return false;
	if(!(p_DSClass = CheckCode->CheckForClass(&p_Type))) return false;
	if( ( vSimpleNode = Simplify() ) ){
		delete *ppThisNode; *ppThisNode = vSimpleNode;
	}
	CheckCode->SetTypeObject(CheckCode->GetEngine()->GetClassBool(), true);
	return true;
}
void dspNodeCastableTo::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Obj->CompileCode(CompileCode);
	dsByteCode::sCodeCast code;
	code.code = dsByteCode::ebcCaTo;
	code.type = p_DSClass;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeCastableTo::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- CastableTo");
	p_PostPrintDebug();
	p_Obj->DebugPrint(Level+1, "(obj) ");
	p_Type->DebugPrint(Level+1, "(type) ");
}
#endif


// istypeof node
//////////////////
dspNodeIsTypeOf::dspNodeIsTypeOf(dspBaseNode *RefNode, dspBaseNode *Object, dspBaseNode *Type) : dspBaseNode(ntIsTypeOf,RefNode){
	if(!Object || !Type) DSTHROW(dueInvalidParam);
	p_Obj = Object; p_Type = Type; p_DSClass = NULL;
}
dspNodeIsTypeOf::~dspNodeIsTypeOf(){
	delete p_Obj; delete p_Type;
}
dspBaseNode *dspNodeIsTypeOf::Simplify(){
	dspBaseNode *vNewNode=NULL;
	dspNodeMembVar *vMembVarNode;
	try{
		if(p_Type->GetNodeType() == ntMembVar){
			vMembVarNode = (dspNodeMembVar*)p_Type;
			if(!vMembVarNode->GetRealObject()){
				if(p_Obj->GetNodeType() == ntByte){
					if(!(vNewNode = new dspNodeBool(p_Obj, strcmp(vMembVarNode->GetName(), "byte")==0))) DSTHROW(dueOutOfMemory);
				}else if(p_Obj->GetNodeType() == ntBool){
					if(!(vNewNode = new dspNodeBool(p_Obj, strcmp(vMembVarNode->GetName(), "bool")==0))) DSTHROW(dueOutOfMemory);
				}else if(p_Obj->GetNodeType() == ntInt){
					if(!(vNewNode = new dspNodeBool(p_Obj, strcmp(vMembVarNode->GetName(), "int")==0))) DSTHROW(dueOutOfMemory);
				}else if(p_Obj->GetNodeType() == ntFloat){
					if(!(vNewNode = new dspNodeBool(p_Obj, strcmp(vMembVarNode->GetName(), "float")==0))) DSTHROW(dueOutOfMemory);
				}
			}
		}
	}catch( ... ){
		if(vNewNode) delete vNewNode;
		throw;
	}
	return vNewNode;
}
bool dspNodeIsTypeOf::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dspBaseNode *vSimpleNode;
	if(!CheckCode->CheckForObject(&p_Obj)) return false;
	if(!(p_DSClass = CheckCode->CheckForClass(&p_Type))) return false;
	if( ( vSimpleNode = Simplify() ) ){
		delete *ppThisNode; *ppThisNode = vSimpleNode;
	}
	CheckCode->SetTypeObject(CheckCode->GetEngine()->GetClassBool(), true);
	return true;
}
void dspNodeIsTypeOf::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	p_Obj->CompileCode(CompileCode);
	dsByteCode::sCodeCast code;
	code.code = dsByteCode::ebcTypO;
	code.type = p_DSClass;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeIsTypeOf::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- IsTypeOf");
	p_PostPrintDebug();
	p_Obj->DebugPrint(Level+1, "(obj) ");
	p_Type->DebugPrint(Level+1, "(type) ");
}
#endif

