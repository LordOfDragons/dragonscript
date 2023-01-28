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
#include "../objects/dsByteCode.h"

// byte node
//////////////
dspNodeByte::dspNodeByte(dsScriptSource *Source, int LineNum, int CharNum, const char *Token, byte Value) : dspBaseNode(ntByte,Source,LineNum,CharNum) {
	p_Value = Value;
	const int size = ( int )strlen( Token );
	if(!(p_Token = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Token, size + 1, Token, size );
	#else
		strncpy(p_Token, Token, size);
	#endif
	p_Token[ size ] = 0;
}
dspNodeByte::dspNodeByte(dspBaseNode *RefNode, byte Value) : dspBaseNode(ntByte,RefNode) {
	p_Value = Value;
	if(!(p_Token = new char[5])) DSTHROW(dueInvalidParam);
	snprintf(p_Token, 5, "%i", Value);
}
dspNodeByte::~dspNodeByte(){ delete [] p_Token; }
const char *dspNodeByte::GetTerminalString(){ return (const char *)p_Token; }
bool dspNodeByte::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	CheckCode->SetTypeObject(CheckCode->GetEngine()->GetClassByte(), true);
	return true;
}
void dspNodeByte::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCodeCByte code;
	code.code = dsByteCode::ebcCByte;
	code.value = p_Value;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeByte::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Byte = %i", (int)p_Value);
	p_PostPrintDebug();
}
#endif

// boolean node
/////////////////
dspNodeBool::dspNodeBool(dsScriptSource *Source, int LineNum, int CharNum, bool Value) : dspBaseNode(ntBool,Source,LineNum,CharNum) {
	p_Value = Value;
}
dspNodeBool::dspNodeBool(dspBaseNode *RefNode, bool Value) : dspBaseNode(ntBool,RefNode) {
	p_Value = Value;
}
const char *dspNodeBool::GetTerminalString(){ return p_Value ? "true" : "false"; }
bool dspNodeBool::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	CheckCode->SetTypeObject(CheckCode->GetEngine()->GetClassBool(), true);
	return true;
}
void dspNodeBool::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCode code;
	code.code = p_Value ? dsByteCode::ebcTrue : dsByteCode::ebcFalse;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeBool::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Boolean = %s", p_Value ? "true" : "false");
	p_PostPrintDebug();
}
#endif

// integer node
/////////////////
dspNodeInt::dspNodeInt(dsScriptSource *Source, int LineNum, int CharNum, const char *Token, int Value) : dspBaseNode(ntInt,Source,LineNum,CharNum) {
	p_Value = Value;
	const int size = ( int )strlen( Token );
	if(!(p_Token = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Token, size + 1, Token, size );
	#else
		strncpy(p_Token, Token, size);
	#endif
	p_Token[ size ] = 0;
}
dspNodeInt::dspNodeInt(dspBaseNode *RefNode, int Value) : dspBaseNode(ntInt,RefNode) {
	p_Value = Value;
	if(!(p_Token = new char[20])) DSTHROW(dueInvalidParam);
	snprintf(p_Token, 20, "%i", Value);
}
dspNodeInt::~dspNodeInt(){ delete [] p_Token; }
const char *dspNodeInt::GetTerminalString(){ return (const char *)p_Token; }
bool dspNodeInt::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	CheckCode->SetTypeObject(CheckCode->GetEngine()->GetClassInt(), true);
	return true;
}
void dspNodeInt::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCodeCInt code;
	code.code = dsByteCode::ebcCInt;
	code.value = p_Value;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeInt::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Integer = %i", p_Value);
	p_PostPrintDebug();
}
#endif

// float node
///////////////
dspNodeFloat::dspNodeFloat(dsScriptSource *Source, int LineNum, int CharNum, const char *Token, float Value) : dspBaseNode(ntFloat,Source,LineNum,CharNum) {
	p_Value = Value;
	const int size = ( int )strlen( Token );
	if(!(p_Token = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Token, size + 1, Token, size );
	#else
		strncpy(p_Token, Token, size);
	#endif
	p_Token[ size ] = 0;
}
dspNodeFloat::dspNodeFloat(dspBaseNode *RefNode, float Value) : dspBaseNode(ntFloat,RefNode) {
	p_Value = Value;
	if(!(p_Token = new char[20])) DSTHROW(dueInvalidParam);
	snprintf(p_Token, 20, "%f", Value);
}
dspNodeFloat::~dspNodeFloat(){ delete [] p_Token; }
const char *dspNodeFloat::GetTerminalString(){ return (const char *)p_Token; }
bool dspNodeFloat::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	CheckCode->SetTypeObject(CheckCode->GetEngine()->GetClassFloat(), true);
	return true;
}
void dspNodeFloat::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCodeCFloat code;
	code.code = dsByteCode::ebcCFlt;
	code.value = p_Value;
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeFloat::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Float = %f", p_Value);
	p_PostPrintDebug();
}
#endif

// string node
////////////////
dspNodeString::dspNodeString(dsScriptSource *Source, int LineNum, int CharNum, const char *String) : dspBaseNode(ntString,Source,LineNum,CharNum) {
	if(!String) DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( String );
	if(!(p_String = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_String, size + 1, String, size );
	#else
		strncpy(p_String, String, size);
	#endif
	p_String[ size ] = 0;
}
dspNodeString::~dspNodeString(){ delete [] p_String; }
const char *dspNodeString::GetTerminalString(){ return p_String; }
bool dspNodeString::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	if(!CheckCode || !ppThisNode || !*ppThisNode) DSTHROW(dueInvalidParam);
	CheckCode->SetTypeObject(CheckCode->GetEngine()->GetClassString(), true);
	return true;
}
void dspNodeString::CompileCode(dspParserCompileCode *CompileCode){
	if(!CompileCode) DSTHROW(dueInvalidParam);
	dsByteCode::sCodeCString code;
	code.code = dsByteCode::ebcCStr;
	code.index = CompileCode->GetEngine()->AddConstString( p_String );
	CompileCode->AddCode( code, GetLineNum() );
}
#ifndef __NODBGPRINTF__
void dspNodeString::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- String = '%s'", p_String);
	p_PostPrintDebug();
}
#endif

