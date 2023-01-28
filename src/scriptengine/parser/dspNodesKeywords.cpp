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
#include "dspParserCheckFunction.h"
#include "dspParserCompileCode.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsFunction.h"

// keyword node
/////////////////
dspNodeKeyword::dspNodeKeyword(dsScriptSource *source, int linenum, int charnum, const char *Keyword) : dspBaseNode(ntKeyword,source,linenum,charnum){
	if(!Keyword) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Keyword );
	if(!(p_Keyword = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Keyword, size + 1, Keyword, size );
	#else
		strncpy(p_Keyword, Keyword, size);
	#endif
	p_Keyword[ size ] = 0;
}
dspNodeKeyword::~dspNodeKeyword(){ delete [] p_Keyword; }
const char *dspNodeKeyword::GetTerminalString(){ return (const char *)p_Keyword; }
#ifndef __NODBGPRINTF__
void dspNodeKeyword::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Keyword '%s'", p_Keyword);
	p_PostPrintDebug();
}
#endif


// null object node
/////////////////////
dspNodeNull::dspNodeNull(dsScriptSource *source, int linenum, int charnum) : dspBaseNode(ntNull,source,linenum,charnum) {
	pType = NULL;
}
dspNodeNull::dspNodeNull(dspBaseNode *refnode, dsClass *Type) : dspBaseNode(ntNull,refnode) {
	pType = Type;
}
const char *dspNodeNull::GetTerminalString(){ return "null"; }
bool dspNodeNull::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	pType = checkCode->GetEngine()->GetClassObject();
	checkCode->SetTypeObject(checkCode->GetEngine()->GetClassNull(), true);
	return true;
}
void dspNodeNull::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCodeNull code;
	code.code = dsByteCode::ebcNull;
	code.type = pType;
	compileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeNull::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	if(pType){
		printf("- Null Object (type=%s)", pType->GetName());
	}else{
		printf("- Null Object (type=?)");
	}
	p_PostPrintDebug();
}
#endif


// This object Node
/////////////////////

dspNodeThis::dspNodeThis( dsScriptSource *source, int linenum, int charnum ) : dspBaseNode( ntThis, source, linenum, charnum ){
}

dspNodeThis::dspNodeThis( dspBaseNode *refnode ) : dspBaseNode( ntThis, refnode ){
}

const char *dspNodeThis::GetTerminalString(){
	return "this";
}

bool dspNodeThis::CheckCode( dspParserCheckCode *checkCode, dspBaseNode **ppThisNode ){
	if( ! checkCode || ! ppThisNode || ! *ppThisNode ) DSTHROW( dueInvalidParam );
	
	if( checkCode->GetCheckFunc() ){
		if( checkCode->GetOwnerFunction()->GetTypeModifiers() & DSTM_STATIC ){
			checkCode->ErrorKeywordInStaticFunc( this, checkCode->GetCheckFunc()->GetFunction(), "this" );
			return false;
		}
		
		checkCode->SetTypeObject( checkCode->GetOwnerClass(), false );
		
	}else{
		checkCode->SetTypeObject( checkCode->GetOwnerClass(), true );
	}
	
	return true;
}

void dspNodeThis::CompileCode( dspParserCompileCode *compileCode ){
	if( ! compileCode ) DSTHROW( dueInvalidParam );
	
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcThis;
	compileCode->AddCode( code, GetLineNum() );
}

#ifndef __NODBGPRINTF__
void dspNodeThis::DebugPrint( int level, const char *prefix ){
	p_PrePrintDebug( level, prefix );
	printf( "- This Object" );
	p_PostPrintDebug();
}
#endif



// super object node
//////////////////////

dspNodeSuper::dspNodeSuper(dsScriptSource *source, int linenum, int charnum) : dspBaseNode(ntSuper,source,linenum,charnum){
}

const char *dspNodeSuper::GetTerminalString(){
	return "super";
}

bool dspNodeSuper::CheckCode(dspParserCheckCode *checkCode, dspBaseNode **ppThisNode){
	if(!checkCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	dsClass *vOwnerClass = checkCode->GetOwnerClass();
	
	if(checkCode->GetCheckFunc()){
		if( checkCode->GetOwnerFunction()->GetTypeModifiers() & DSTM_STATIC ){
			checkCode->ErrorKeywordInStaticFunc( this, checkCode->GetCheckFunc()->GetFunction(), "super" );
			return false;
		}
		
		checkCode->SetTypeObject(vOwnerClass->GetBaseClass(), false);
		
	}else{
		checkCode->SetTypeObject(vOwnerClass->GetBaseClass(), true);
	}
	checkCode->SetSuperCall(true);
	return true;
}

void dspNodeSuper::CompileCode(dspParserCompileCode *compileCode){
	if(!compileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcSuper;
	compileCode->AddCode( code, GetLineNum() );
}

#ifndef __NODBGPRINTF__
void dspNodeSuper::DebugPrint(int level, const char *prefix){
	p_PrePrintDebug(level, prefix);
	printf("- Super Object");
	p_PostPrintDebug();
}
#endif

