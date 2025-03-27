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
#include "dspParserCheckFunction.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../dsBaseEngineManager.h"
#include "../utils/dsuString.h"
#include "../objects/dsClass.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsFunction.h"
#include "../objects/dsFuncList.h"
#include "../objects/dsVariable.h"
#include "../objects/dsSignature.h"
#include "../objects/dsLocalVariable.h"



// Conversion Table
/////////////////////

static const int p_ConvTableSize = 27;
static const char *p_ConvTableOps[] = {
	"*",        "/",        "%",        "+",        "-",        "<<",       ">>",    // 7
	"<",        ">",        "<=",       ">=",       "==",       "!=",                // 6
	"&",        "|",        "^",                                                     // 3
	"=",        "*=",       "/=",       "%=",       "+=",       "-=",                // 6
	"<<=",      ">>=",      "&=",       "|=",       "^="                             // 5
};
static const byte p_ConvTableByte[] = {
	DSPT_INT,   DSPT_BYTE,  DSPT_BYTE,  DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_BYTE,
	DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,
	DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE,
	DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE,
	DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE,  DSPT_BYTE
};
static const byte p_ConvTableBool[] = {
	DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,
	DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_BOOL,  DSPT_BOOL,
	DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,
	DSPT_BOOL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,
	DSPT_NULL,  DSPT_NULL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL
};
static const byte p_ConvTableInt[] = {
	DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,
	DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,
	DSPT_INT,   DSPT_INT,   DSPT_INT,
	DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,
	DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT,   DSPT_INT
};
static const byte p_ConvTableFloat[] = {
	DSPT_FLOAT, DSPT_FLOAT, DSPT_NULL,  DSPT_FLOAT, DSPT_FLOAT, DSPT_NULL,  DSPT_NULL,
	DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,  DSPT_BOOL,
	DSPT_NULL,  DSPT_NULL,  DSPT_NULL,
	DSPT_FLOAT, DSPT_FLOAT, DSPT_FLOAT, DSPT_NULL,  DSPT_FLOAT, DSPT_FLOAT,
	DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL,  DSPT_NULL
};



// dspParserCheckCode
///////////////////////

// Constructor, Destructor
////////////////////////////

dspParserCheckCode::dspParserCheckCode( dspParserCheck *parserCheck, dspParserCheckFunction *checkFunction ){
	if( ! parserCheck || ! checkFunction ) DSTHROW( dueInvalidParam );
	
	p_ParserCheck = parserCheck;
	p_CheckFunc = checkFunction;
	p_OwnerClass = checkFunction->GetFunction()->GetOwnerClass();
	p_Precedor = NULL;
	p_LocVars = NULL;
	p_LocVarCount = 0;
	p_MaxLocVarCount = 0;
	p_LocVarBase = 0;
	
	pCCBlock = NULL;
	pBlockCVars = NULL;
	pBlockCVarCount = 0;
	pBlockCVarSize = 0;
	pBlockCVarBase = 0;
	
	ResetState();
}

dspParserCheckCode::dspParserCheckCode( dspParserCheck *parserCheck, dsClass *ownerClass ){
	if( ! parserCheck || ! ownerClass ) DSTHROW( dueInvalidParam );
	
	p_ParserCheck = parserCheck;
	p_CheckFunc = NULL;
	p_OwnerClass = ownerClass;
	p_Precedor = NULL;
	p_LocVars = NULL;
	p_LocVarCount = 0;
	p_MaxLocVarCount = 0;
	p_LocVarBase = 0;
	
	pCCBlock = NULL;
	pBlockCVars = NULL;
	pBlockCVarCount = 0;
	pBlockCVarSize = 0;
	pBlockCVarBase = 0;
	
	ResetState();
}

dspParserCheckCode::dspParserCheckCode( dspParserCheckCode *precedor ){
	if( ! precedor ) DSTHROW( dueInvalidParam );
	
	p_ParserCheck = precedor->p_ParserCheck;
	p_CheckFunc = precedor->p_CheckFunc;
	p_OwnerClass = precedor->p_OwnerClass;
	p_Precedor = precedor;
	p_LocVars = NULL;
	p_LocVarCount = 0;
	p_MaxLocVarCount = 0;
	p_LocVarBase = precedor->p_LocVarBase + precedor->p_LocVarCount;
	
	pCCBlock = NULL;
	pBlockCVars = NULL;
	pBlockCVarCount = 0;
	pBlockCVarSize = 0;
	pBlockCVarBase = precedor->pBlockCVarBase + precedor->pBlockCVarCount;
	
	ResetState();
}

dspParserCheckCode::~dspParserCheckCode(){
	RemoveAllBlockCVars();
	if( pBlockCVars ) delete [] pBlockCVars;
	
	if( p_Precedor ){
		if( p_Precedor->p_LocVarCount + p_MaxLocVarCount > p_Precedor->p_MaxLocVarCount ){
			p_Precedor->p_MaxLocVarCount = p_Precedor->p_LocVarCount + p_MaxLocVarCount;
		}
	}
	ClearLocalVariables();
}



// Information
////////////////

dsEngine *dspParserCheckCode::GetEngine() const{
	return p_ParserCheck->GetEngine();
}

dsFunction *dspParserCheckCode::GetOwnerFunction() const{
	if( ! p_CheckFunc ) DSTHROW( dueInvalidParam );
	
	return p_CheckFunc->GetFunction();
}



// Management
///////////////

void dspParserCheckCode::SetTypeClass(dsClass *Class){
	p_TypeClass = Class; p_TypeObject = NULL;
	p_IsObjConst = false;
	p_StaClsVar = NULL;
}
void dspParserCheckCode::SetTypeObject(dsClass *Class, bool IsConst){
	p_TypeClass = NULL; p_TypeObject = Class;
	p_IsObjConst = IsConst;
	p_StaClsVar = NULL;
}
void dspParserCheckCode::SetStaticClassVariable( dsVariable *variable ){
	p_StaClsVar = variable;
}

void dspParserCheckCode::SetTypeNothing(){
	p_TypeClass = p_TypeObject = NULL;
	p_IsObjConst = false;
	p_StaClsVar = NULL;
}
void dspParserCheckCode::ResetState(){
	p_HasReturn = false;
	p_HasUnreachCode = false;
	if(p_Precedor){
		p_CanUseBreak = p_Precedor->p_CanUseBreak;
		p_CanUseContinue = p_Precedor->p_CanUseContinue;
		p_CanUseRethrow = p_Precedor->p_CanUseRethrow;
	}else{
		p_CanUseBreak = p_CanUseContinue = p_CanUseRethrow = false;
	}
	p_SuperCall = false;
	p_TypeClass = p_TypeObject = NULL;
	p_IsObjConst = false;
	p_StaClsVar = NULL;
}
void dspParserCheckCode::CopyState(dspParserCheckCode *CheckCode){
	if(!CheckCode) DSTHROW(dueInvalidParam);
	p_HasReturn = CheckCode->p_HasReturn;
	p_HasUnreachCode = CheckCode->p_HasUnreachCode;
	p_CanUseBreak = CheckCode->p_CanUseBreak;
	p_CanUseContinue = CheckCode->p_CanUseContinue;
	p_CanUseRethrow = CheckCode->p_CanUseRethrow;
	p_TypeClass = CheckCode->p_TypeClass;
	p_TypeObject = CheckCode->p_TypeObject;
	p_StaClsVar = CheckCode->p_StaClsVar;
	p_IsObjConst = CheckCode->p_IsObjConst;
}



void dspParserCheckCode::SetCCBlock( dspParserCheckCode *ccblock ){
	pCCBlock = ccblock;
	
	if( ccblock ){
		pBlockCVarBase = ccblock->GetBlockCVarBase() + ccblock->GetBlockCVarCount();
	}
}

dsLocalVariable *dspParserCheckCode::GetBlockCVarAt( int index ) const{
	if( index < 0 || index >= pBlockCVarCount ) DSTHROW( dueOutOfBoundary );
	
	return pBlockCVars[ index ];
}

dsLocalVariable *dspParserCheckCode::GetBlockCVarAbsoluteAt( int index, bool onlyLocal ) const{
	if( index - pBlockCVarBase >= pBlockCVarCount ) DSTHROW( dueOutOfBoundary );
	
	if( index < pBlockCVarBase ){
		if( onlyLocal || ( ! pCCBlock && ! p_Precedor ) ) DSTHROW( dueOutOfBoundary );
		
		if( pCCBlock ){
			return pCCBlock->GetBlockCVarAbsoluteAt( index, false );
			
		}else if( p_Precedor ){
			return p_Precedor->GetBlockCVarAbsoluteAt( index, false );
		}
		
		return NULL; // prevent compiler complaints
		
	}else{
		return pBlockCVars[ index - pBlockCVarBase ];
	}
}

int dspParserCheckCode::IndexOfBlockCVarNamed( const char *name ) const{
	if( ! name ) DSTHROW( dueInvalidParam );
	
	int i;
	
	for( i=pBlockCVarCount-1; i>=0; i-- ){
		if( strcmp( pBlockCVars[ i ]->GetName(), name ) == 0 ){
			return i;
		}
	}
	
	return -1;
}

int dspParserCheckCode::AbsoluteIndexOfBlockCVarNamed( const char *name, bool onlyLocal ) const{
	int i;
	
	/*
	printf( "AbsoluteIndexOfBlockCVarNamed(%s,%d)\n", name, onlyLocal );
	for( i=pBlockCVarCount-1; i>=0; i-- ){
		printf( "  %i: %s\n", i, pBlockCVars[i]->GetName() );
	}
	*/
	
	for( i=pBlockCVarCount-1; i>=0; i-- ){
		if( strcmp( pBlockCVars[ i ]->GetName(), name ) == 0 ){
			return pBlockCVarBase + i;
		}
	}
	
	if( ! onlyLocal ){
		if( pCCBlock ){
			return pCCBlock->AbsoluteIndexOfBlockCVarNamed( name, false );
			
		}else if( p_Precedor ){
			return p_Precedor->AbsoluteIndexOfBlockCVarNamed( name, false );
		}
	}
	
	return -1;
}

void dspParserCheckCode::AddBlockCVar( dsLocalVariable *variable ){
	if( ! variable ) DSTHROW( dueInvalidParam );
	
	if( pBlockCVarCount == pBlockCVarSize ){
		int newSize = pBlockCVarSize * 3 / 2 + 1;
		dsLocalVariable **newArray = new dsLocalVariable*[ newSize ];
		if( ! newArray ) DSTHROW( dueOutOfMemory );
		if( pBlockCVars ){
			memcpy( newArray, pBlockCVars, sizeof( dsLocalVariable* ) * pBlockCVarSize );
			delete [] pBlockCVars;
		}
		pBlockCVars = newArray;
		pBlockCVarSize = newSize;
	}
	
	pBlockCVars[ pBlockCVarCount ] = variable;
	pBlockCVarCount++;
}

void dspParserCheckCode::RemoveAllBlockCVars(){
	while( pBlockCVarCount > 0 ){
		pBlockCVarCount--;
		delete pBlockCVars[ pBlockCVarCount ];
	}
}



// checking functions
dsClass *dspParserCheckCode::CheckForClass(dspBaseNode **pNode){
	if(!pNode || !*pNode) DSTHROW(dueInvalidParam);
	if(!(*pNode)->CheckCode(this, pNode)) return NULL;
	if(!p_TypeClass){
		ErrorInvClass(*pNode);
		return NULL;
	}
	return p_TypeClass;
}
dsClass *dspParserCheckCode::CheckForObject(dspBaseNode **pNode){
	if(!pNode || !*pNode) DSTHROW(dueInvalidParam);
	if(!(*pNode)->CheckCode(this, pNode)) return NULL;
	if(!p_TypeObject){
		ErrorInvObject(*pNode);
		return NULL;
	}
	return p_TypeObject;
}



// Nodes Helper Functions
///////////////////////////

bool dspParserCheckCode::AutoNodeCast( dspBaseNode **pnode, dsClass *fromType, dsClass *toType ){
	if( ! pnode || ! *pnode || ! fromType || ! toType){
		DSTHROW( dueInvalidParam );
	}
	
	if( fromType->IsEqualTo( toType ) ){
		return true;
	}
	
	if( toType->GetPrimitiveType() == DSPT_BYTE ){
		if( fromType->GetPrimitiveType() == DSPT_INT ){
			if( ( *pnode )->GetNodeType() == ntInt ){
				const int vIntVal = ( ( dspNodeInt* )*pnode )->GetValue();
				if( vIntVal >= -255 && vIntVal <= 255 ){
					dspBaseNode *vNewNode = new dspNodeByte(*pnode, (byte)vIntVal);
					if(!vNewNode) DSTHROW(dueOutOfMemory);
					delete *pnode;
					*pnode = vNewNode;
					return true;
					
				}else{
					ErrorInvTypeCast( *pnode, fromType, toType );
					return false;
				}
				
			}else{
				ErrorInvTypeCast( *pnode, fromType, toType );
				return false;
			}
			
		}else if( fromType->GetPrimitiveType() == DSPT_FLOAT ){
			ErrorInvTypeCast( *pnode, fromType, toType );
			return false;
		}
		
	}else if( toType->GetPrimitiveType() == DSPT_BOOL ){
		if( fromType->GetPrimitiveType() == DSPT_BYTE ){
			/*
			if( ( *pnode )->GetNodeType() == ntByte ){
				dspBaseNode *vNewNode = new dspNodeBool(*pnode, ((dspNodeByte*)*pnode)->GetValue() != 0);
				if(!vNewNode) DSTHROW(dueOutOfMemory);
				delete *pnode;
				*pnode = vNewNode;
				return true;
			}
			*/
			ErrorInvTypeCast( *pnode, fromType, toType );
			return false;
			
		}else if( fromType->GetPrimitiveType() == DSPT_INT ){
			/*
			if( (*pnode)->GetNodeType()==ntInt){
				dspBaseNode *vNewNode = new dspNodeBool(*pnode, ((dspNodeInt*)*pnode)->GetValue() != 0);
				if(!vNewNode) DSTHROW(dueOutOfMemory);
				delete *pnode;
				*pnode = vNewNode;
				return true;
			}
			*/
			ErrorInvTypeCast( *pnode, fromType, toType );
			return false;
			
		}else if( fromType->GetPrimitiveType() == DSPT_FLOAT ){
			/*
			if( (*pnode)->GetNodeType()==ntFloat){
				dspBaseNode *vNewNode = new dspNodeBool(*pnode, ((dspNodeFloat*)*pnode)->GetValue() != 0);
				if(!vNewNode) DSTHROW(dueOutOfMemory);
				delete *pnode;
				*pnode = vNewNode;
				return true;
			}
			*/
			ErrorInvTypeCast( *pnode, fromType, toType );
			return false;
		}
		
	}else if( toType->GetPrimitiveType() == DSPT_INT ){
		if( fromType->GetPrimitiveType() == DSPT_BYTE ){
			if( ( *pnode )->GetNodeType() == ntByte ){
				int vByteVal = ((dspNodeByte*)*pnode)->GetValue();
				dspBaseNode *vNewNode = new dspNodeInt(*pnode, vByteVal);
				if(!vNewNode) DSTHROW(dueOutOfMemory);
				delete *pnode;
				*pnode = vNewNode;
				return true;
			}
			
		}else if( fromType->GetPrimitiveType() == DSPT_FLOAT ){
			ErrorInvTypeCast( *pnode, fromType, toType );
			return false;
		}
		
	}else if( toType->GetPrimitiveType() == DSPT_FLOAT ){
		if( fromType->GetPrimitiveType() == DSPT_INT ){
			if( ( *pnode )->GetNodeType() == ntInt ){
				int vIntVal = ((dspNodeInt*)*pnode)->GetValue();
				dspBaseNode *vNewNode = new dspNodeFloat(*pnode, (float)vIntVal);
				if(!vNewNode) DSTHROW(dueOutOfMemory);
				delete *pnode;
				*pnode = vNewNode;
				return true;
			}
			
		}else if( fromType->GetPrimitiveType() == DSPT_BYTE ){
			if( ( *pnode )->GetNodeType() == ntByte ){
				byte vByteVal = ((dspNodeByte*)*pnode)->GetValue();
				dspBaseNode *vNewNode = new dspNodeFloat(*pnode, (float)vByteVal);
				if(!vNewNode) DSTHROW(dueOutOfMemory);
				delete *pnode;
				*pnode = vNewNode;
				return true;
			}
		}
		
	}else if( toType->GetPrimitiveType() == DSPT_OBJECT ){
		if( ( *pnode )->GetNodeType() == ntNull ){
			dspBaseNode *vNewNode = new dspNodeNull(*pnode, toType);
			if(!vNewNode) DSTHROW(dueOutOfMemory);
			delete *pnode;
			*pnode = vNewNode;
			return true;
		}
	}
	
	if( ! fromType->CastableTo( toType ) ){
		ErrorInvTypeCast( *pnode, fromType, toType );
		return false;
	}
	
	dspBaseNode *vNewNode = new dspNodeAutoCast(*pnode, toType);
	if(!vNewNode) DSTHROW(dueOutOfMemory);
	*pnode = vNewNode;
	return true;
}

bool dspParserCheckCode::AutoNodeCastBinOp( dspBaseNode **op1Node, dsClass **op1Type, dspBaseNode **op2Node, dsClass **op2Type ){
	// Cast Table:
	// 
	// byte OP bool   => cast bool to byte   [ 2 to 1 ]
	// byte OP int    => cast byte to int    [ 1 to 2 ]
	// byte OP float  => cast byte to float  [ 1 to 2 ]
	// 
	// bool OP byte   => cast bool to byte   [ 1 to 2 ]
	// bool OP int    => cast bool to byte   [ 1 to 2 ]
	// bool OP float  => cast bool to float  [ 1 to 2 ]
	// 
	// int OP byte    => cast byte to int    [ 2 to 1 ]
	// int OP bool    => cast bool to int    [ 2 to 1 ]
	// int OP float   => cast int to float   [ 1 to 2 ]
	// 
	// float OP byte  => cast byte to float  [ 2 to 1 ]
	// float OP bool  => cast bool to float  [ 2 to 1 ]
	// float OP int   => cast int to float   [ 2 to 1 ]
	// 
	// object OP null => cast null to object [ 2 to 1 ]
	// null OP object => cast null to object [ 1 to 2 ]
	// 
	if( ! op1Node || ! *op1Node || ! op1Type || ! *op1Type || ! op2Node || ! *op2Node || ! op2Type || ! *op2Type ){
		DSTHROW( dueInvalidParam );
	}
	
	if( ( *op2Type )->IsEqualTo( *op1Type ) ){
		return true;
	}
	
	const int pt1 = ( *op1Type )->GetPrimitiveType();
	const int pt2 = ( *op2Type )->GetPrimitiveType();
	bool cast1to2 = false;
	
	if( pt1 == DSPT_BYTE ){
		cast1to2 = ( pt2 == DSPT_INT || pt2 == DSPT_FLOAT );
		
	}else if( pt1 == DSPT_BOOL ){
		cast1to2 = ( pt2 == DSPT_BYTE || pt2 == DSPT_INT || pt2 == DSPT_FLOAT );
		
	}else if( pt1 == DSPT_INT ){
		cast1to2 = ( pt2 == DSPT_FLOAT );
		
	}else if( ( *op1Node )->GetNodeType() == ntNull ){
		cast1to2 = ( pt2 == DSPT_OBJECT );
	}
	
	if( cast1to2 ){
		if( AutoNodeCast( op1Node, *op1Type, *op2Type ) ){
			*op1Type = *op2Type;
			return true;
		}
		
	}else{
		if( AutoNodeCast( op2Node, *op2Type, *op1Type ) ){
			*op2Type = *op1Type;
			return true;
		}
	}
	
	return false;
}

dsFunction *dspParserCheckCode::FindBestOperator(dsClass *ownerClass, const char *Op, dsClass *ArgType){
	if(!ownerClass || !Op || !ArgType) DSTHROW(dueInvalidParam);
	dsFunction *vBestFunc=NULL, *vCurFunc;
	dsSignature *vSig; dsClass *vArg;
	for(int i=0; i<ownerClass->GetFunctionCount(); i++){
		vCurFunc = ownerClass->GetFunction(i);
		if(vCurFunc->GetFuncType() != DSFT_OPERATOR) continue;
		if(strcmp(vCurFunc->GetName(), Op) != 0) continue;
		vSig = vCurFunc->GetSignature();
		if(vSig->GetCount() != 1) continue;
		vArg = vSig->GetParameter(0);
		if(ArgType->IsEqualTo(vArg)) return vCurFunc;
		if(ArgType->CastableTo(vArg)){
			if(vBestFunc) return NULL;
			vBestFunc = vCurFunc;
		}
	}
	return vBestFunc;
}
dsClass *dspParserCheckCode::GetUnOpReturnType(dsClass *Type, const char *Op){
	if(!Type || !Op) DSTHROW(dueInvalidParam);
	switch(Type->GetPrimitiveType()){
	case DSPT_BYTE:
		if(strcmp(Op,"++")==0 || strcmp(Op,"--")==0) return GetEngine()->GetClassByte();
		if(strcmp(Op,"!")==0) return GetEngine()->GetClassBool();
		if(strcmp(Op,"+")==0 || strcmp(Op,"-")==0 || strcmp(Op,"~")==0) return GetEngine()->GetClassInt();
		break;
	case DSPT_BOOL:
		if(strcmp(Op,"!")==0) return GetEngine()->GetClassBool();
		break;
	case DSPT_INT:
		if(strcmp(Op,"!")==0) return GetEngine()->GetClassBool();
		if(strcmp(Op,"++")==0 || strcmp(Op,"--")==0 || strcmp(Op,"+")==0 ||
		   strcmp(Op,"-")==0 || strcmp(Op,"~")==0) return GetEngine()->GetClassInt();
		break;
	case DSPT_FLOAT:
		if(strcmp(Op,"!")==0) return GetEngine()->GetClassBool();
		if(strcmp(Op,"++")==0 || strcmp(Op,"--")==0 ||
		   strcmp(Op,"+")==0 || strcmp(Op,"-")==0) return GetEngine()->GetClassFloat();
		break;
	}
	return NULL;
}
dsClass *dspParserCheckCode::GetBinOpReturnType(dsClass *Type, const char *Op/*, dsClass *ArgType*/){
	if( ! Type || ! Op/* || !ArgType*/ ){
		DSTHROW( dueInvalidParam );
	}
	
	int i, vRetTypeID;
	for( i=0; i<p_ConvTableSize; i++ ){
		if( strcmp( Op, p_ConvTableOps[ i ] ) == 0 ){
			break;
		}
	}
	if( i == p_ConvTableSize ){
		DSTHROW( dueInvalidParam );
	}
	
	switch( Type->GetPrimitiveType() ){
	case DSPT_BYTE:
		vRetTypeID = p_ConvTableByte[ i ];
		break;
		
	case DSPT_BOOL:
		vRetTypeID = p_ConvTableBool[ i ];
		break;
		
	case DSPT_INT:
		vRetTypeID = p_ConvTableInt[ i ];
		break;
		
	case DSPT_FLOAT:
		vRetTypeID = p_ConvTableFloat[ i ];
		break;
		
	default:
		DSTHROW( dueInvalidParam );
	}
	
	switch( vRetTypeID ){
	case DSPT_BYTE:
		return GetEngine()->GetClassByte();
		
	case DSPT_BOOL:
		return GetEngine()->GetClassBool();
		
	case DSPT_INT:
		return GetEngine()->GetClassInt();
		
	case DSPT_FLOAT:
		return GetEngine()->GetClassFloat();
		
	default:
		return NULL;
	}
}

bool dspParserCheckCode::CanAccessVariable(dsClass *ObjectClass, dsClass *CallerClass, dsVariable *Variable){
	if(!ObjectClass || !CallerClass || !Variable) DSTHROW(dueInvalidParam);
	dsClass *vOwnerClass = Variable->GetOwnerClass();
	int vLevel;
	// check for direct access (caller owns or inherits variable)
	//if(ObjectClass->GetSubClassLevel(CallerClass) != -1){
	if(CallerClass->GetSubClassLevel(ObjectClass) != -1){
		vLevel = CallerClass->GetSubClassLevel(vOwnerClass);
		if(vLevel == -1) DSTHROW(dueInvalidParam);
		if(Variable->GetTypeModifiers() & (DSTM_PUBLIC | DSTM_PROTECTED)) return true;
		return vLevel == 0;
	}
	// check for indirect access (caller neither owns nor inherits variable)
	return (Variable->GetTypeModifiers() & DSTM_PUBLIC);
}
bool dspParserCheckCode::CanAccessFunction(dsClass *ObjectClass, dsClass *CallerClass, dsFunction *Function){
	if(!ObjectClass || !CallerClass || !Function) DSTHROW(dueInvalidParam);
	dsClass *vOwnerClass = Function->GetOwnerClass();
	int vLevel;
	// check for direct access (caller owns or inherits function)
//	if(ObjectClass->GetSubClassLevel(CallerClass) != -1){
	if(CallerClass->GetSubClassLevel(ObjectClass) != -1){
		vLevel = CallerClass->GetSubClassLevel(vOwnerClass);
		if(vLevel == -1){
//			printf("[oops] %s, %s, %s\n", ObjectClass->GetName(), CallerClass->GetName(), Function->GetName());
			DSTHROW(dueInvalidParam);
//			return false;
		}
		if(Function->GetTypeModifiers() & (DSTM_PUBLIC | DSTM_PROTECTED)) return true;
		return vLevel == 0;
	}
	// check for indirect access (caller neither owns nor inherits variable)
	return (Function->GetTypeModifiers() & DSTM_PUBLIC);
}
bool dspParserCheckCode::MatchesOperators(const char *CheckOp, const char **Ops, int Count){
	if(!CheckOp || !Ops || !Count) DSTHROW(dueInvalidParam);
	for(int i=0; i<Count; i++){
		if(strcmp(CheckOp, Ops[i]) == 0) return true;
	}
	return false;
}
dsConstant *dspParserCheckCode::FindClassConstant(dsClass *ownerClass, const char *ConstName){
	if(!ownerClass || !ConstName) DSTHROW(dueInvalidParam);
	dsConstant *vConstant = ownerClass->FindConstant( ConstName, true );
	// it is bad to search constants like this in pinned classes as this is rarely ever
	// the result the user is looking for
	//if(!vConstant){
	//	vConstant = p_ParserCheck->FindClassConstant(ownerClass, ConstName);
	//}
	return vConstant;
}
// local variables
void dspParserCheckCode::ClearLocalVariables(){
	if(p_LocVars){
		for(int i=0; i<p_LocVarCount; i++) delete p_LocVars[i];
		delete [] p_LocVars; p_LocVars = NULL;
		p_LocVarCount = 0;
		p_MaxLocVarCount = 0;
	}
}
void dspParserCheckCode::AddLocalVariable(dsLocalVariable *Variable){
	if(!Variable) DSTHROW(dueInvalidParam);
	int i; const char *vNewLVName = Variable->GetName();
	// check if the variable doesn't already exists
	if(FindLocalVariable(vNewLVName, true) != -1) DSTHROW(dueInvalidParam);
	// add local variable
	dsLocalVariable **vNewArray = new dsLocalVariable*[p_LocVarCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_LocVars){
		for(i=0; i<p_LocVarCount; i++) vNewArray[i] = p_LocVars[i];
		delete [] p_LocVars;
	}
	vNewArray[p_LocVarCount] = Variable;
	p_LocVars = vNewArray;
	p_LocVarCount++;
	if(p_LocVarCount > p_MaxLocVarCount) p_MaxLocVarCount = p_LocVarCount;
}
int dspParserCheckCode::FindLocalVariable(const char *Name, bool OnlyLocal) const{
	for(int i=p_LocVarCount-1; i>=0; i--){
		if(strcmp(p_LocVars[i]->GetName(), Name) == 0) return p_LocVarBase + i;
	}
	if(!OnlyLocal && p_Precedor){
		return p_Precedor->FindLocalVariable(Name, false);
	}
	return -1;
}
dsLocalVariable *dspParserCheckCode::GetLocalVariable(int Index, bool OnlyLocal) const{
	if(Index-p_LocVarBase >= p_LocVarCount) DSTHROW(dueOutOfBoundary);
	if(Index < p_LocVarBase){
		if(OnlyLocal || !p_Precedor) DSTHROW(dueOutOfBoundary);
		return p_Precedor->GetLocalVariable(Index, false);
	}else{
		return p_LocVars[Index - p_LocVarBase];
	}
}
dsLocalVariable *dspParserCheckCode::GetLocalVariableAt( int index ) const{
	if( index >= p_LocVarCount ) DSTHROW( dueInvalidParam );
	
	return p_LocVars[ index ];
}
// error functions
void dspParserCheckCode::ErrorVarNotStatic(dspBaseNode *Node, dsVariable *Variable){
	if(!Node || !Variable) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Class variable ";
	vMsgBuf += Variable->GetName();
	vMsgBuf += " in ";
	vMsgBuf += Variable->GetOwnerClass()->GetName();
	vMsgBuf += " cannot be used on a class, use it on an object";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peVarIsNotStatic, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvClassMemberName(dspBaseNode *Node, dsClass *Class, const char *Name){
	if(!Node || !Name) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	if(!Class) Class = GetOwnerClass();
	vMsgBuf = "Class member ";
	vMsgBuf += Name;
	vMsgBuf += " does not exist in ";
	vMsgBuf += Class->GetName();
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peInvalidClassMember, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvTypeCast(dspBaseNode *Node, dsClass *FromType, dsClass *ToType){
	if(!Node || !FromType || !ToType) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Cannot cast from ";
	vMsgBuf += FromType->GetName();
	vMsgBuf += " to ";
	vMsgBuf += ToType->GetName();
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peInvalidTypeCast, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorFuncNotStatic(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Class function ";
	p_ErrorFuncSig(&vMsgBuf, Function->GetName(), Function->GetSignature());
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " cannot be used on the class itself, use it on an object";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peFuncIsNotStatic, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorFuncStatic(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Static class function ";
	p_ErrorFuncSig(&vMsgBuf, Function->GetName(), Function->GetSignature());
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " cannot be used on an object, use it on the class itself";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peFuncIsStatic, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}

void dspParserCheckCode::ErrorFuncNotConst(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Non-constant class function ";
	p_ErrorFuncSig(&vMsgBuf, Function->GetName(), Function->GetSignature());
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " cannot be used on a const object";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peFuncIsNotConst, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}

void dspParserCheckCode::ErrorInvFuncArg(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Invalid function argument specified", dsEngine::peInvalidFuncArg, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvFuncSig(dspBaseNode *Node, dsClass *ownerClass, const char *Name, dsSignature *Sig){
	if(!Node || !ownerClass || !Name || !Sig) DSTHROW(dueInvalidParam);
	int i, j;
	bool found;
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsFunction *vCurFunc;
	dsuString vMsgBuf;
	vMsgBuf = "Function ";
	p_ErrorFuncSig(&vMsgBuf, Name, Sig);
	vMsgBuf += " not found in ";
	vMsgBuf += ownerClass->GetName();
	vMsgBuf += ". Possible candidates:";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peInvalidFuncSig, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	dsFuncList *vFuncList = ownerClass->GetFuncList();
	for(i=vFuncList->GetCount()-1; i>=0; i--){
		vCurFunc = vFuncList->GetFunction(i);
		found = false;
		for(j=i-1; j>=0; j--){
			if(vCurFunc==vFuncList->GetFunction(j)){ found = true; break; }
		}
		if(!found && strcmp(vCurFunc->GetName(), Name)==0){
			vMsgBuf.Empty();
			p_ErrorFuncType(&vMsgBuf, vCurFunc->GetFuncType());
			vMsgBuf += vCurFunc->GetOwnerClass()->GetName();
			vMsgBuf += ".";
			p_ErrorFuncSig(&vMsgBuf, vCurFunc->GetName(), vCurFunc->GetSignature());
			vEngMgr->OutputErrorMore(vMsgBuf.Pointer());
		}
	}
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvOpSig(dspBaseNode *Node, dsClass *ownerClass, const char *Op, dsClass *Arg){
	if(!Node || !ownerClass || !Op) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsFunction *vCurFunc;
	dsSignature *vSig;
	dsuString vMsgBuf;
	bool found;
	int i, j;
	vMsgBuf = "Class operator ";
	p_ErrorOpSig(&vMsgBuf, Op, Arg);
	vMsgBuf += " not found in ";
	vMsgBuf += ownerClass->GetName();
	vMsgBuf += ". Possible candidates:";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peInvalidOpSig, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	dsFuncList *vFuncList = ownerClass->GetFuncList();
	for(i=vFuncList->GetCount()-1; i>=0; i--){
		vCurFunc = vFuncList->GetFunction(i);
		found = false;
		for(j=i-1; j>=0; j--){
			if(vCurFunc==vFuncList->GetFunction(j)){ found = true; break; }
		}
		if(!found && strcmp(vCurFunc->GetName(), Op)==0){
			vMsgBuf = "operator ";
			vMsgBuf += vCurFunc->GetOwnerClass()->GetName();
			vMsgBuf += ".";
			vSig = vCurFunc->GetSignature();
			if(Arg){
				if(vSig->GetCount() != 1) continue;
				p_ErrorOpSig(&vMsgBuf, vCurFunc->GetName(), vSig->GetParameter(0));
			}else{
				if(vSig->GetCount() > 0) continue;
				p_ErrorOpSig(&vMsgBuf, vCurFunc->GetName(), NULL);
			}
			vEngMgr->OutputErrorMore(vMsgBuf.Pointer());
		}
	}
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvObject(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Object expected but not found", dsEngine::peInvalidObject, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvClass(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Class expected but not found", dsEngine::peInvalidClass, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvPrimOp(dspBaseNode *Node, const char *ownerClass, const char *Op){
	if(!Node || !Op) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Operator ";
	vMsgBuf += Op;
	vMsgBuf += " is not applicable on type ";
	vMsgBuf += ownerClass;
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peInvalidPrimOp, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoAccessClsVar(dspBaseNode *Node, dsVariable *Variable){
	if(!Node || !Variable) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Cannot access class variable ";
	vMsgBuf += Variable->GetName();
	vMsgBuf += " in ";
	vMsgBuf += Variable->GetOwnerClass()->GetName();
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoAccessClsVar, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoAccessClsFunc(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Cannot access class function ";
	p_ErrorFuncSig(&vMsgBuf, Function->GetName(), Function->GetSignature());
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoAccessClsFunc, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoMembSelAllowed(dspBaseNode *Node, const char *ClassName){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Member selection is not allowed for ";
	vMsgBuf += ClassName;
	vMsgBuf += " in this context";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoMembSelAllowed, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}

void dspParserCheckCode::ErrorKeywordInStaticFunc( dspBaseNode *node, dsFunction *func, const char *keyword ){
	if( ! node || ! keyword ){
		DSTHROW( dueInvalidParam );
	}
	
	dsBaseEngineManager &engmgr = *GetEngine()->GetEngineManager();
	dsuString buf;
	buf = "Cannot use ";
	buf += keyword;
	buf += " on the static class function ";
	p_ErrorFuncSig( &buf, func->GetName(), func->GetSignature() );
	buf += " in ";
	buf += func->GetOwnerClass()->GetName();
	engmgr.OutputError( buf.Pointer(), dsEngine::peKeywordInStaticFunc, node->GetSource(), node->GetLineNum(), node->GetCharNum() );
	
	p_ParserCheck->GetParser()->IncErrCount();
	SetTypeNothing();
}

void dspParserCheckCode::ErrorConstObj(dspBaseNode *Node, const char *Op){
	if(!Node || !Op) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Cannot use operator ";
	vMsgBuf += Op;
	vMsgBuf += " on a constant object (l-value is const)";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peConstObj, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}

void dspParserCheckCode::ErrorLoopConstObj(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Cannot loop on a constant object (l-value is const)", dsEngine::peConstObj, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvLoopVarType(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Cannot loop on a non-scalar object (int/byte only allowed)", dsEngine::peInvLoopVarType, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoDirectDestrCall(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Class destructor ";
	vMsgBuf += Function->GetName();
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " cannot be called directly";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoDirectDestrCall, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorConstrNotOnClass(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Class constructor ";
	p_ErrorFuncSig(&vMsgBuf, Function->GetName(), Function->GetSignature());
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " cannot be used on an object, us it on the class itself";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peConstrNotOnClass, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorReturnOnVoid(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Return value not allowed in class function ";
	vMsgBuf += Function->GetName();
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " having type void";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peReturnOnVoid, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoReturnValue(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Missing Return value in class function ";
	vMsgBuf += Function->GetName();
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " having type ";
	vMsgBuf += Function->GetType()->GetName();
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoReturnValue, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorBreakOutside(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Cannot use break outside a loop or switch construct", dsEngine::peBreakOutside, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorContinueOutside(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Cannot use continue outside a loop construct", dsEngine::peContinueOutside, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorRethrowOutside(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Cannot use throw without an expression outside a catch-block", dsEngine::peRethrowOutside, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoConstrFunc(dspBaseNode *Node, dsFunction *Function){
	if(!Node || !Function) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Class Function ";
	p_ErrorFuncSig(&vMsgBuf, Function->GetName(), Function->GetSignature());
	vMsgBuf += " in ";
	vMsgBuf += Function->GetOwnerClass()->GetName();
	vMsgBuf += " is not a constructor function";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoConstrFunc, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorAmbigDefConstrFunc(dspBaseNode *Node, const char *ownerClass){
	if(!Node || !ownerClass) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "More than one default constructor function possible to call in ";
	vMsgBuf += ownerClass;
	vMsgBuf += " (ambigious call)";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peAmbigDefConstrFunc, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoDefConstrFunc(dspBaseNode *Node, const char *ownerClass){
	if(!Node || !ownerClass) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "No default constructor function possible to call in ";
	vMsgBuf += ownerClass;
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoDefConstrFunc, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorCaseValNotScalar(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Case value has to be a constant scalar value (byte,int)", dsEngine::peCaseValNotScalar, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorDupCaseValue(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("There exists already a case-block for this value",
		dsEngine::peDupCaseValue, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorDupCaseValue(dspBaseNode *Node, int Value){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf, vNum;
	vMsgBuf = "There exists already a case-block for the value ";
	vNum.SetInt(Value);
	vMsgBuf += vNum;
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peDupCaseValue, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvEnumValue(dspBaseNode *Node, const char *ConstName){
	if(!Node || !ConstName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Constant ";
	vMsgBuf += ConstName;
	vMsgBuf += " must have a constant integral value (int,byte)";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peInvEnumValue, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorInvExceptionType(dspBaseNode *Node, const char *ClassName){
	if(!Node || !ClassName) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Exception type ";
	vMsgBuf += ClassName;
	vMsgBuf += " has to be derived from class Exception";
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peInvExceptionType, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorMissingCatch(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Missing Catche handlers in try-catch statement", dsEngine::peMissingCatch, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorDupCatch(dspBaseNode *Node, const char *ClassName){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf, vNum;
	vMsgBuf = "There exists already a catch-block for the exception type ";
	vMsgBuf += ClassName;
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peDupCatch, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorThrowNotException(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	vEngMgr->OutputError("Only objects castable to Exception can be thrown", dsEngine::peThrowNotException, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
void dspParserCheckCode::ErrorNoNewOnAbstractClass(dspBaseNode *Node, dsClass *Type){
	if(!Node || !Type) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Cannot create instance of abstract class ";
	vMsgBuf += Type->GetName();
	vEngMgr->OutputError(vMsgBuf.Pointer(), dsEngine::peNoNewOnAbstractClass, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncErrCount(); SetTypeNothing();
}
// warning functions
void dspParserCheckCode::WarnUnreachableCode(dspBaseNode *Node){
	if(!Node || !p_CheckFunc) DSTHROW(dueInvalidParam);
	dsBaseEngineManager *vEngMgr = GetEngine()->GetEngineManager();
	dsuString vMsgBuf;
	vMsgBuf = "Unreachable code found in function ";
	vMsgBuf += GetOwnerFunction()->GetName();
	vMsgBuf += " in ";
	vMsgBuf += GetOwnerClass()->GetName();
	vEngMgr->OutputWarning(vMsgBuf.Pointer(), dsEngine::pwUnreachableCode, Node->GetSource(), Node->GetLineNum(), Node->GetCharNum());
	p_ParserCheck->GetParser()->IncWarnCount();
}
// private functions
void dspParserCheckCode::p_ErrorFuncSig(dsuString *String, const char *Name, dsSignature *Sig){
	*String += Name;
	*String += "(";
	for(int i=0; i<Sig->GetCount(); i++){
		if(i > 0) *String += ",";
		*String += Sig->GetParameter(i)->GetName();
	}
	*String += ")";
}
void dspParserCheckCode::p_ErrorOpSig(dsuString *String, const char *Op, dsClass *Arg){
	*String += Op;
	*String += "(";
	if(Arg) *String += Arg->GetName();
	*String += ")";
}
void dspParserCheckCode::p_ErrorFuncType(dsuString *String, int FuncType){
	switch(FuncType){
	case DSFT_FUNCTION:
		*String += "function "; break;
	case DSFT_CONSTRUCTOR:
		*String += "constructor "; break;
	case DSFT_DESTRUCTOR:
		*String += "destructor ";
	default:
		DSTHROW(dueInvalidParam);
	}
}

