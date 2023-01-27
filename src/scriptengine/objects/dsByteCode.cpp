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
#include <memory.h>
#include "../dragonscript_config.h"
#include "dsClass.h"
#include "dsFunction.h"
#include "dsFuncList.h"
#include "dsVariable.h"
#include "dsByteCode.h"
#include "../exceptions.h"

// class dsByteCode
/////////////////////
// constructor, destructor
dsByteCode::dsByteCode(){
	p_Data = NULL;
	p_Length = 0;
	p_DebugInfos = NULL;
	p_DebugCount = 0;
	p_DebugSize = 0;
}
dsByteCode::~dsByteCode(){
	Clear();
}



// Management
///////////////

void dsByteCode::Clear(){
	if(p_DebugInfos){
		delete [] p_DebugInfos;
		p_DebugInfos = NULL;
		p_DebugCount = 0;
		p_DebugSize = 0;
	}
	if(p_Data){
		delete [] p_Data;
		p_Data = NULL;
		p_Length = 0;
	}
}

void dsByteCode::AddCode( const sCode &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CODE );
}

void dsByteCode::AddCode( const sCodeCByte &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CBYTE );
}

void dsByteCode::AddCode( const sCodeCInt &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CINT );
}

void dsByteCode::AddCode( const sCodeCFloat &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CFLOAT );
}

void dsByteCode::AddCode( const sCodeCString &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CSTRING );
}

void dsByteCode::AddCode( const sCodeNull &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_NULL );
}

void dsByteCode::AddCode( const sCodeClassVar &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CLASSVAR );
}

void dsByteCode::AddCode( const sCodeParam &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_PARAM );
}

void dsByteCode::AddCode( const sCodeLocalVar &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_LOCVAR );
}

void dsByteCode::AddCode( const sCodeAllocLocalVar &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_ALV );
}

void dsByteCode::AddCode( const sCodeFreeLocalVars &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_FLV );
}

void dsByteCode::AddCode( const sCodeJumpByte &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JBYTE );
}

void dsByteCode::AddCode( const sCodeJumpShort &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JSHORT );
}

void dsByteCode::AddCode( const sCodeJumpLong &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JLONG );
}

void dsByteCode::AddCode( const sCodeJumpCaseByte &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JCBYTE );
}

void dsByteCode::AddCode( const sCodeJumpCaseShort &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JCSHORT );
}

void dsByteCode::AddCode( const sCodeJumpCaseLong &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JCLONG );
}

void dsByteCode::AddCode( const sCodeJumpCaseStaticByte &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JCSBYTE );
}

void dsByteCode::AddCode( const sCodeJumpCaseStaticShort &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JCSSHORT );
}

void dsByteCode::AddCode( const sCodeJumpCaseStaticLong &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JCSLONG );
}

void dsByteCode::AddCode( const sCodeJumpExcepByte &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JEBYTE );
}

void dsByteCode::AddCode( const sCodeJumpExcepShort &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JESHORT );
}

void dsByteCode::AddCode( const sCodeJumpExcepLong &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_JELONG );
}

void dsByteCode::AddCode( const sCodeCast &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CAST );
}

void dsByteCode::AddCode( const sCodeCall &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_CALL );
}

void dsByteCode::AddCode( const sCodeDirectCall &code ){
	p_AddData( &code, sizeof( code ), DS_BCSTRIDE_DCALL );
}

void dsByteCode::AddByteCode( dsByteCode *byteCode ){
	if( ! byteCode){
		DSTHROW( dueInvalidParam );
	}
	
	const int oldSize = p_Length;
	if( byteCode->p_Length > 0 ){
		p_AddData( byteCode->p_Data, byteCode->p_Length );
	}
	
	// add debug infos
	if( byteCode->p_DebugCount > 0 ){
		if( p_DebugCount + byteCode->p_DebugCount >= p_DebugSize ){
			const int newSize = p_DebugCount + byteCode->p_DebugCount;
			sDebugInfo * const newArray = new sDebugInfo[ newSize ];
			if( p_DebugInfos ){
				memcpy( newArray, p_DebugInfos, sizeof( sDebugInfo ) * p_DebugSize );
				delete [] p_DebugInfos;
			}
			p_DebugInfos = newArray;
			p_DebugSize = newSize;
		}
		
		int i;
		for( i=0; i<byteCode->p_DebugCount; i++ ){
			p_DebugInfos[ p_DebugCount + i ].line = byteCode->p_DebugInfos[ i ].line;
			p_DebugInfos[ p_DebugCount + i ].cp = oldSize + byteCode->p_DebugInfos[ i ].cp;
		}
		p_DebugCount += byteCode->p_DebugCount;
	}
}


// debugging
void dsByteCode::AddDebugLine(int line){
	if(line < 1) DSTHROW(dueInvalidParam);
	// determine last line and if we need to add another record
	int lastLine = 1, cp = p_Length;
	if(p_DebugInfos) lastLine = p_DebugInfos[p_DebugCount-1].line;
	if(line <= lastLine) return;
	// add record
	if(p_DebugCount == p_DebugSize){
		int newSize = p_DebugSize * 3 / 2 + 1;
		sDebugInfo *newArray = new sDebugInfo[newSize];
		if(!newArray) DSTHROW(dueOutOfMemory);
		if(p_DebugInfos){
			memcpy(newArray, p_DebugInfos, sizeof(sDebugInfo)*p_DebugSize);
			delete [] p_DebugInfos;
		}
		p_DebugInfos = newArray;
		p_DebugSize = newSize;
	}
	p_DebugInfos[p_DebugCount].line = line;
	p_DebugInfos[p_DebugCount].cp = cp;
	p_DebugCount++;
}
int dsByteCode::GetDebugLine(int cp){
	if(cp < 0) DSTHROW(dueInvalidParam);
//	int left=0; right=p_DebugCount-1, middle;
	int line = 1;
	if(!p_DebugInfos) return 1;
	// determine line whose cp is closest to the searching cp but less
	/*
	while(right - left > 0){
		
		
		if(cp == p_DebugInfos[left].cp){
			return p_DebugInfos[left].line;
		}else if(cp == p_DebugInfos[right].cp){
			return p_DebugInfos[right].line;
		}else{
			middle = left + (right - left) / 2;
			if(cp < p_DebugInfos[left].cp){
			right = left + (right - left) / 2;
		}
	}
	return p_DebugInfos[left].line;
	*/
	line = p_DebugInfos[0].line;
	for(int i=1; i<p_DebugCount; i++){
		if(p_DebugInfos[i].cp > cp) break;
		line = p_DebugInfos[i].line;
	}
	return line;
}
#ifndef __NODBGPRINTF__
void dsByteCode::DebugPrint(){
	printf( "[ByteCode]\n" );
	int cp = 0;
	
	while( cp < p_Length ){
		const int cpc = cp;
		const sCode &code = *( ( const sCode* )( p_Data + cp ) );
		
		switch( code.code ){
		case ebcNop:
			cp += DS_BCSTRIDE_CODE;
			printf( "NOOP (%i)\n", cpc );
			break;
			
		case ebcCByte:{
			const sCodeCByte &codeCByte = ( const sCodeCByte & )code;
			cp += DS_BCSTRIDE_CBYTE;
			printf( "CBYTE (%i): %i\n", cpc, codeCByte.value );
			}break;
			
		case ebcCInt:{
			const sCodeCInt &codeCInt = ( const sCodeCInt & )code;
			cp += DS_BCSTRIDE_CINT;
			printf( "CINT (%i): %i\n", cpc, codeCInt.value );
			}break;
			
		case ebcCFlt:{
			const sCodeCFloat &codeCFlt = ( const sCodeCFloat & )code;
			cp += DS_BCSTRIDE_CFLOAT;
			printf( "CFLT (%i): %f\n", cpc, codeCFlt.value );
			}break;
			
		case ebcCStr:{
			const sCodeCString &codeCStr = ( const sCodeCString & )code;
			cp += DS_BCSTRIDE_CSTRING;
			printf( "CSTR (%i): %i\n", cpc, codeCStr.index );
			}break;
			
		case ebcTrue:
			cp += DS_BCSTRIDE_CODE;
			printf( "TRUE (%i)\n", cpc );
			break;
			
		case ebcFalse:
			cp += DS_BCSTRIDE_CODE;
			printf( "FALSE (%i)\n", cpc );
			break;
			
		case ebcNull:{
			const sCodeNull &codeNull = ( const sCodeNull & )code;
			cp += DS_BCSTRIDE_NULL;
			printf( "NULL (%i): %s\n", cpc, codeNull.type->GetName() );
			}break;
			
		case ebcThis:
			cp += DS_BCSTRIDE_CODE;
			printf( "THIS (%i)\n", cpc );
			break;
			
		case ebcSuper:
			cp += DS_BCSTRIDE_CODE;
			printf( "SUPER (%i)\n", cpc );
			break;
			
		case ebcClsVar:{
			const sCodeClassVar &codeClsVar = ( const sCodeClassVar & )code;
			cp += DS_BCSTRIDE_CLASSVAR;
			printf( "CLSVAR (%i): %s.%s\n", cpc, codeClsVar.variable->GetOwnerClass()->GetName(), codeClsVar.variable->GetName() );
			}break;
			
		case ebcParam:{
			const sCodeParam &codeParam = ( const sCodeParam & )code;
			cp += DS_BCSTRIDE_PARAM;
			printf( "PARAM (%i): %i\n", cpc, codeParam.index );
			}break;
			
		case ebcLocVar:{
			const sCodeLocalVar &codeLocVar = ( const sCodeLocalVar & )code;
			cp += DS_BCSTRIDE_LOCVAR;
			printf( "LOCVAR (%i): %i\n", cpc, codeLocVar.index );
			}break;
			
		case ebcPush:
			cp += DS_BCSTRIDE_CODE;
			printf( "ADDARG (%i)\n", cpc );
			break;
			
		case ebcOpInc:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPINC (%i)\n", cpc );
			break;
			
		case ebcOpDec:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPDEC (%i)\n", cpc );
			break;
			
		case ebcOpMin:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPMIN (%i)\n", cpc );
			break;
			
		case ebcOpNot:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPNOT (%i)\n", cpc );
			break;
			
		case ebcOpInv:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPINV (%i)\n", cpc );
			break;
			
		case ebcOpMul:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPMUL (%i)\n", cpc );
			break;
			
		case ebcOpDiv:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPDIV (%i)\n", cpc );
			break;
			
		case ebcOpMod:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPMOD (%i)\n", cpc );
			break;
			
		case ebcOpAdd:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPADD (%i)\n", cpc );
			break;
			
		case ebcOpSub:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPSUB (%i)\n", cpc );
			break;
			
		case ebcOpLS:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPLS (%i)\n", cpc );
			break;
			
		case ebcOpRS:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPRS (%i)\n", cpc );
			break;
			
		case ebcOpLe:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPLE (%i)\n", cpc );
			break;
			
		case ebcOpGr:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPGR (%i)\n", cpc );
			break;
			
		case ebcOpLEq:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPLEQ (%i)\n", cpc );
			break;
			
		case ebcOpGEq:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPGEQ (%i)\n", cpc );
			break;
			
		case ebcOpEq:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPEQ (%i)\n", cpc );
			break;
			
		case ebcOpNEq:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPNEQ (%i)\n", cpc );
			break;
			
		case ebcOpAnd:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPAND (%i)\n", cpc );
			break;
			
		case ebcOpOr:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPOR (%i)\n", cpc );
			break;
			
		case ebcOpXor:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPXOR (%i)\n", cpc );
			break;
			
		case ebcOpAss:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPASS (%i)\n", cpc );
			break;
			
		case ebcOpMulA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPMULA (%i)\n", cpc );
			break;
			
		case ebcOpDivA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPDIVA (%i)\n", cpc );
			break;
			
		case ebcOpModA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPMODA (%i)\n", cpc );
			break;
			
		case ebcOpAddA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPADDA (%i)\n", cpc );
			break;
			
		case ebcOpSubA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPSUBA (%i)\n", cpc );
			break;
			
		case ebcOpLSA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPLSA (%i)\n", cpc );
			break;
			
		case ebcOpRSA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPRSA (%i)\n", cpc );
			break;
			
		case ebcOpAndA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPANDA (%i)\n", cpc );
			break;
			
		case ebcOpOrA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPORA (%i)\n", cpc );
			break;
			
		case ebcOpXorA:
			cp += DS_BCSTRIDE_CODE;
			printf( "OPXSORA (%i)\n", cpc );
			break;
			
		case ebcPInc:
			cp += DS_BCSTRIDE_CODE;
			printf( "PINC (%i)\n", cpc );
			break;
			
		case ebcPDec:
			cp += DS_BCSTRIDE_CODE;
			printf( "PDEC (%i)\n", cpc );
			break;
			
		case ebcALV:{
			const sCodeAllocLocalVar &codeALV = ( const sCodeAllocLocalVar & )code;
			cp += DS_BCSTRIDE_ALV;
			printf( "ALV (%i): %s\n", cpc, codeALV.type->GetName() );
			}break;
			
		case ebcFLV:{
			const sCodeFreeLocalVars &codeFLV = ( const sCodeFreeLocalVars & )code;
			cp += DS_BCSTRIDE_FLV;
			printf( "FLV (%i): %i\n", cpc, codeFLV.count );
			}break;
			
		case ebcRet:
			cp += DS_BCSTRIDE_CODE;
			printf( "RET (%i)\n", cpc );
			break;
			
		case ebcJMPB:{
			const sCodeJumpByte &codeJump = ( const sCodeJumpByte & )code;
			cp += DS_BCSTRIDE_JBYTE;
			printf( "JMPB (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJMPS:{
			const sCodeJumpShort &codeJump = ( const sCodeJumpShort & )code;
			cp += DS_BCSTRIDE_JSHORT;
			printf( "JMPS (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJMPL:{
			const sCodeJumpShort &codeJump = ( const sCodeJumpShort & )code;
			cp += DS_BCSTRIDE_JLONG;
			printf( "JMPL (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJEQB:{
			const sCodeJumpByte &codeJump = ( const sCodeJumpByte & )code;
			cp += DS_BCSTRIDE_JBYTE;
			printf( "JEQB (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJEQS:{
			const sCodeJumpShort &codeJump = ( const sCodeJumpShort & )code;
			cp += DS_BCSTRIDE_JSHORT;
			printf( "JEQS (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJEQL:{
			const sCodeJumpShort &codeJump = ( const sCodeJumpShort & )code;
			cp += DS_BCSTRIDE_JLONG;
			printf( "JEQL (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJNEB:{
			const sCodeJumpByte &codeJump = ( const sCodeJumpByte & )code;
			cp += DS_BCSTRIDE_JBYTE;
			printf( "JNEB (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJNES:{
			const sCodeJumpShort &codeJump = ( const sCodeJumpShort & )code;
			cp += DS_BCSTRIDE_JSHORT;
			printf( "JNES (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJNEL:{
			const sCodeJumpShort &codeJump = ( const sCodeJumpShort & )code;
			cp += DS_BCSTRIDE_JLONG;
			printf( "JNEL (%i): %i(cp=%i)\n", cpc, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJCEB:{
			const sCodeJumpCaseByte &codeJump = ( const sCodeJumpCaseByte & )code;
			cp += DS_BCSTRIDE_JCBYTE;
			printf( "JCEB (%i): val=%i, %i(cp=%i)\n", cpc, codeJump.caseValue, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJCES:{
			const sCodeJumpCaseShort &codeJump = ( const sCodeJumpCaseShort & )code;
			cp += DS_BCSTRIDE_JCSHORT;
			printf( "JCES (%i): val=%i, %i(cp=%i)\n", cpc, codeJump.caseValue, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJCEL:{
			const sCodeJumpCaseLong &codeJump = ( const sCodeJumpCaseLong & )code;
			cp += DS_BCSTRIDE_JCLONG;
			printf( "JCEL (%i): val=%i, %i(cp=%i)\n", cpc, codeJump.caseValue, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJCESB:{
			const sCodeJumpCaseStaticByte &codeJump = ( const sCodeJumpCaseStaticByte & )code;
			cp += DS_BCSTRIDE_JCSBYTE;
			printf( "JCESB (%i): val=%p, %i(cp=%i)\n", cpc, codeJump.caseValue, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJCESS:{
			const sCodeJumpCaseStaticShort &codeJump = ( const sCodeJumpCaseStaticShort & )code;
			cp += DS_BCSTRIDE_JCSSHORT;
			printf( "JCESS (%i): val=%p, %i(cp=%i)\n", cpc, codeJump.caseValue, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJCESL:{
			const sCodeJumpCaseStaticLong &codeJump = ( const sCodeJumpCaseStaticLong & )code;
			cp += DS_BCSTRIDE_JCSLONG;
			printf( "JCESL (%i): val=%p, %i(cp=%i)\n", cpc, codeJump.caseValue, codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJOEB:{
			const sCodeJumpExcepByte &codeJump = ( const sCodeJumpExcepByte & )code;
			cp += DS_BCSTRIDE_JEBYTE;
			printf( "JOEB (%i): %s, %i(cp=%i)\n", cpc, codeJump.type->GetName(), codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJOES:{
			const sCodeJumpExcepShort &codeJump = ( const sCodeJumpExcepShort & )code;
			cp += DS_BCSTRIDE_JESHORT;
			printf( "JOES (%i): %s, %i(cp=%i)\n", cpc, codeJump.type->GetName(), codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcJOEL:{
			const sCodeJumpExcepLong &codeJump = ( const sCodeJumpExcepLong & )code;
			cp += DS_BCSTRIDE_JELONG;
			printf( "JOEL (%i): %s, %i(cp=%i)\n", cpc, codeJump.type->GetName(), codeJump.offset, cp + codeJump.offset );
			}break;
			
		case ebcCast:{
			const sCodeCast &codeCast = ( const sCodeCast & )code;
			cp += DS_BCSTRIDE_CAST;
			printf( "CAST (%i): %s\n", cpc, codeCast.type->GetName() );
			}break;
			
		case ebcCaTo:{
			const sCodeCast &codeCast = ( const sCodeCast & )code;
			cp += DS_BCSTRIDE_CAST;
			printf( "CATO (%i): %s\n", cpc, codeCast.type->GetName() );
			}break;
			
		case ebcTypO:{
			const sCodeCast &codeCast = ( const sCodeCast & )code;
			cp += DS_BCSTRIDE_CAST;
			printf( "TYPO (%i): %s\n", cpc, codeCast.type->GetName() );
			}break;
			
		case ebcCall:{
			const sCodeCall &codeCall = ( const sCodeCall & )code;
			cp += DS_BCSTRIDE_CALL;
			printf( "CALL (%i): index %i\n", cpc, codeCall.index );
			}break;
			
		case ebcDCall:{
			const sCodeDirectCall &codeDCall = ( const sCodeDirectCall & )code;
			cp += DS_BCSTRIDE_DCALL;
			printf( "DCALL (%i): %s.%s\n", cpc, codeDCall.function->GetOwnerClass()->GetName(), codeDCall.function->GetName() );
			}break;
			
		case ebcCCall:{
			const sCodeDirectCall &codeDCall = ( const sCodeDirectCall & )code;
			cp += DS_BCSTRIDE_DCALL;
			printf( "CCALL (%i): %s.%s\n", cpc, codeDCall.function->GetOwnerClass()->GetName(), codeDCall.function->GetName() );
			}break;
			
		case ebcBTB:
			cp += DS_BCSTRIDE_CODE;
			printf( "BTB (%i)\n", cpc );
			break;
			
		case ebcETB:
			cp += DS_BCSTRIDE_CODE;
			printf( "ETB (%i)\n", cpc );
			break;
			
		case ebcThrow:
			cp += DS_BCSTRIDE_CODE;
			printf( "DSTHROW (%i)\n", cpc);
			break;
			
		case ebcReThrow:
			cp += DS_BCSTRIDE_CODE;
			printf( "REDSTHROW (%i)\n", cpc );
			break;
			
		case ebcBlock:{
			//const sCodeDirectCall &codeDCall = ( const sCodeDirectCall & )code;
			cp += DS_BCSTRIDE_DCALL;
			printf( "BLOCK (%i)\n", cpc );
			}break;
			
		case ebcCVar:{
			const sCodeLocalVar &codeLocVar = ( const sCodeLocalVar & )code;
			cp += DS_BCSTRIDE_LOCVAR;
			printf( "CVAR (%i): %i\n", cpc, codeLocVar.index );
			}break;
			
		default:
			DSTHROW( dueInvalidParam );
		}
	}
	
	printf( "[DebugInfos]\n" );
	for( int l=0; l<p_DebugCount; l++ ){
		printf( "line %i => cp %i\n", p_DebugInfos[ l ].line, p_DebugInfos[ l ].cp );
	}
}
#endif

// private functions
void dsByteCode::p_AddData(void *Data, int Size){
	byte *vNewData = NULL;
	if(!(vNewData = new byte[p_Length+Size])) DSTHROW(dueOutOfMemory);
	if(p_Length > 0) memcpy(vNewData, p_Data, p_Length);
	memcpy(vNewData+p_Length, Data, Size);
	if(p_Data) delete [] p_Data;
	p_Data = vNewData;
	p_Length += Size;
}

void dsByteCode::p_AddData( const void *data, int size, int advance ){
	byte * const newData = new byte[ p_Length + advance ];
	if( p_Length > 0 ){
		memcpy( newData, p_Data, p_Length );
	}
	
	memcpy( newData + p_Length, data, size);
	if( advance > size ){
		memset( newData + p_Length + size, '\xff', advance - size );
	}
	
	if( p_Data ){
		delete [] p_Data;
	}
	p_Data = newData;
	p_Length += advance;
}

