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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../dragonscript_config.h"
#include "dspNodes.h"
#include "dspScanner.h"
#include "../dsEngine.h"
#include "../dsScriptSource.h"
#include "../dsBaseEngineManager.h"
#include "../exceptions.h"

// definitions
//#define DSP_READBUFSIZE		10
#define DSP_READBUFSIZE		4096
#define DSP_TOKENBUFSIZE	512
#define DSP_STRINGBUFSIZE	512
#define DSP_MSGBUFSIZE		250

// keywords
static const char *p_KeywordList[] = {
	"end", "if", "elif", "else",
	"while", "for", "to", "downto", "step", "break", "continue", "select", "case",
	"cast", "castable", "typeof",
	"return", "try", "catch", "throw",
	"requires", "pin",
	"private", "protected", "public", "fixed", "abstract", "static",
	"class", "interface", "namespace",
	"extends", "implements",
	"var", "func", "enum",
	"and", "or", "not", "block"
};
static const int p_KeywordCount = 40;

// NOTE padded to 3-character length with 0-characters to simplify the test
//      below since it expects all operator strings to be 3-length.
static const char *p_OperatorList[] = {
	"(\0\0", ")\0\0", "=\0\0", "*=\0", "/=\0",
	"%=\0", "+=\0", "-=\0", "<<=", ">>=",
	"&=\0", "|=\0", "^=\0", "&\0\0", "|\0\0",
	"^\0\0", ":\0\0", "<<\0", ">>\0", "<\0\0",
	">\0\0", "<=\0", ">=\0", "==\0", "!=\0",
	"*\0\0", "/\0\0", "%\0\0", "+\0\0", "-\0\0",
	"++\0", "--\0", "~\0\0", ".\0\0", ",\0\0",
	"\n\0\0"
};
static const int p_OperatorCount = 36;

// dspScanner
///////////////
// constructor, destructor
dspScanner::dspScanner(dsEngine *Engine){
	if(!Engine) DSTHROW(dueInvalidParam);
	p_Engine = Engine;
	p_Source = NULL;
	if(!(p_ReadBuf = new char[DSP_READBUFSIZE])) DSTHROW(dueOutOfMemory);
	p_ReadBufLen = 0;
	p_ReadBufPos = 0;
	if(!(p_TokenBuf = new char[DSP_TOKENBUFSIZE])) DSTHROW(dueOutOfMemory);
	p_TokenBuf[0] = '\0';
	p_TokenBufLen = DSP_TOKENBUFSIZE;
	p_TokenBufPos = 0;
	if(!(p_StrBuf = new char[DSP_STRINGBUFSIZE])) DSTHROW(dueOutOfMemory);
	p_StrBuf[0] = '\0';
	p_StrBufLen = DSP_STRINGBUFSIZE;
	p_StrBufPos = 0;
	p_CurChar = '\0';
	p_NextChar = '\0';
	p_LineNum = 1;
	p_CharNum = 1;
	p_ErrCount = 0;
	p_WarnCount = 0;
	p_NeedsNL = true;
	if(!(p_MsgBuf = new char[DSP_MSGBUFSIZE+1])) DSTHROW(dueOutOfMemory);
}
dspScanner::~dspScanner(){
	delete [] p_ReadBuf;
	delete [] p_StrBuf;
	delete [] p_TokenBuf;
	delete [] p_MsgBuf;
}
// scanning management
void dspScanner::SetScript(dsScriptSource *Script){
	if(!Script) DSTHROW(dueInvalidParam);
	// reset variables
	p_Source = Script;
	p_ReadBufPos = 0;
	p_ClearToken();
	p_LineNum = 1;
	p_CharNum = 1;
	p_ErrCount = 0;
	p_WarnCount = 0;
	p_NeedsNL = true;
	// prepare source and read first data
	p_Source->Open();
	p_ReadBufLen = p_Source->ReadData(p_ReadBuf, DSP_READBUFSIZE);
	p_CurChar = (p_ReadBufLen > 0) ? p_ReadBuf[p_ReadBufPos++] : '\0';
	p_NextChar = (p_ReadBufLen > 1) ? p_ReadBuf[p_ReadBufPos++] : '\0';
}
void dspScanner::ClearScript(){
	if(p_Source) p_Source->Close();
	p_ReadBufPos = 0;
	p_ReadBufLen = 0;
	p_ClearToken();
	p_CurChar = '\0';
	p_NextChar = '\0';
	p_Source = NULL;
	p_NeedsNL = true;
}
dspBaseNode *dspScanner::GetNextNode(){
	dspBaseNode *vNode = NULL;
	while(!vNode && p_CurChar!='\0'){
		// skip blanks
		p_SkipBlanks();
		if(p_CurChar == '\0') break;
		// check cur character
		p_RefLineNum = p_LineNum; p_RefCharNum = p_CharNum;
		if(p_CurChar == '0'){
			if(p_NextChar == 'b'){
				vNode = p_ScanBinaryInt();
			}else if(p_NextChar == 'o'){
				vNode = p_ScanOctalInt();
			}else if(p_NextChar == 'h'){
				vNode = p_ScanHexalInt();
			}else{
				vNode = p_ScanIntOrFloat();
			}
		}else if(p_CurChar>='1' && p_CurChar<='9'){
			vNode = p_ScanIntOrFloat();
		}else if( ((p_CurChar>='A') && (p_CurChar<='Z')) || ((p_CurChar>='a') && (p_CurChar<='z')) || (p_CurChar == '_') ){
			vNode = p_ScanIdent();
		}else if(p_CurChar == '\''){
			vNode = p_ScanByteOrByteInt();
		}else if(p_CurChar == '"'){
			vNode = p_ScanString();
		}else if(p_CurChar == '/'){
			if(p_NextChar == '/'){
				vNode = p_ScanComment();
			}else if(p_NextChar == '*'){
				vNode = p_ScanInlineComment();
			}else{
				vNode = p_ScanOperator();
			}
		}else{
			vNode = p_ScanOperator();
		}
	}
	// add one newline at the end of a file to avoid parser errors
	if(!vNode && p_NeedsNL){
		vNode = new dspNodeOperator(p_Source, p_RefLineNum, p_RefCharNum, "NL");
		if(!vNode) DSTHROW(dueOutOfMemory);
		p_NeedsNL = false;
	}
	return vNode;
}
// private functions
void dspScanner::p_AdvanceChars(int Count, bool Skip){
	do{
		if(!Skip) p_AddTokenChar(p_CurChar);
		p_CurChar = p_NextChar; p_CharNum++;
		if(p_ReadBufPos==p_ReadBufLen && p_ReadBufLen>0){
			p_ReadBufLen = p_Source->ReadData(p_ReadBuf, DSP_READBUFSIZE);
			p_ReadBufPos = 0;
		}
		if(p_ReadBufLen == 0){
			p_NextChar = '\0';
		}else{
			p_NextChar = p_ReadBuf[p_ReadBufPos++];
		}
	}while(--Count > 0);
}
void dspScanner::p_AdvanceLine(){
	p_LineNum++; p_CharNum = 1;
}
void dspScanner::p_AddTokenChar(char Char){
	if(p_TokenBufPos == p_TokenBufLen-1){
		char *vNewBuf = new char[p_TokenBufLen + DSP_TOKENBUFSIZE];
		if(!vNewBuf) DSTHROW(dueOutOfMemory);
		#ifdef OS_W32_VS
			strncpy_s(vNewBuf, p_TokenBufLen + 1, p_TokenBuf, p_TokenBufLen);
		#else
			strncpy(vNewBuf, p_TokenBuf, p_TokenBufLen);
		#endif
		delete [] p_TokenBuf;
		p_TokenBuf = vNewBuf;
		p_TokenBufLen += DSP_TOKENBUFSIZE;
	}
	p_TokenBuf[p_TokenBufPos++] = Char;
	p_TokenBuf[p_TokenBufPos] = '\0';
}
void dspScanner::p_AddStringChar(char Char){
	if(p_StrBufPos == p_StrBufLen-1){
		char *vNewBuf = new char[p_StrBufLen+DSP_STRINGBUFSIZE];
		if(!vNewBuf) DSTHROW(dueOutOfMemory);
		#ifdef OS_W32_VS
			strncpy_s(vNewBuf, p_StrBufLen + 1, p_StrBuf, p_StrBufLen);
		#else
			strncpy(vNewBuf, p_StrBuf, p_StrBufLen);
		#endif
		delete [] p_StrBuf;
		p_StrBuf = vNewBuf;
		p_StrBufLen += DSP_STRINGBUFSIZE;
	}
	p_StrBuf[p_StrBufPos++] = Char;
	p_StrBuf[p_StrBufPos] = '\0';
}
void dspScanner::p_ClearToken(){
	p_TokenBuf[0] = '\0';
	p_TokenBufPos = 0;
	p_StrBuf[0] = '\0';
	p_StrBufPos = 0;
}
dspBaseNode *dspScanner::p_ScanBinaryInt(){
	dspBaseNode *vNode = NULL;
	p_ClearToken(); p_AdvanceChars(2);
	if(!(vNode = new dspNodeByte(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf, p_ParseBinaryInt()))) DSTHROW(dueOutOfMemory);
	return vNode;
}
dspBaseNode *dspScanner::p_ScanOctalInt(){
	dspBaseNode *vNode = NULL;
	p_ClearToken(); p_AdvanceChars(2);
	if(!(vNode = new dspNodeInt(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf, p_ParseOctalInt()))) DSTHROW(dueOutOfMemory);
	return vNode;
}
dspBaseNode *dspScanner::p_ScanHexalInt(){
	dspBaseNode *vNode = NULL;
	p_ClearToken(); p_AdvanceChars(2);
	if(!(vNode = new dspNodeInt(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf, p_ParseHexalInt()))) DSTHROW(dueOutOfMemory);
	return vNode;
}
dspBaseNode *dspScanner::p_ScanIntOrFloat(){
	dspBaseNode *vNode = NULL;
	bool vHasPoint = false;
	p_ClearToken();
	while(true){
		if(p_CurChar == '.'){
			if(vHasPoint) break;
			vHasPoint = true;
		}else if(!(p_CurChar>='0' && p_CurChar<='9')) break;
		p_AdvanceChars();
	}
	if(vHasPoint){
		if(!(vNode = new dspNodeFloat(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf, strtof(p_TokenBuf,NULL)))) DSTHROW(dueOutOfMemory);
	}else{
		if(!(vNode = new dspNodeInt(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf, (int)strtol(p_TokenBuf,NULL,10)))) DSTHROW(dueOutOfMemory);
	}
	return vNode;
}
dspBaseNode *dspScanner::p_ScanByteOrByteInt(){
	dspBaseNode *vNode = NULL;
	int i, vIntVal=0;
	p_ClearToken(); p_AdvanceChars();
	for(i=0; i<4; i++){
		if(p_CurChar == '\'') break;
		vIntVal <<= 8; vIntVal += p_ParseByte();
	}
	p_AdvanceChars();
	if(i > 1){
		if(!(vNode = new dspNodeInt(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf, vIntVal))) DSTHROW(dueOutOfMemory);
	}else{
		if(!(vNode = new dspNodeByte(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf, (byte)vIntVal))) DSTHROW(dueOutOfMemory);
	}
	return vNode;
}
dspBaseNode *dspScanner::p_ScanString(){
	dspBaseNode *vNode = NULL;
	p_ClearToken(); p_AdvanceChars();
	while(p_CurChar != '"'){
		if(p_CurChar == '\n'){
			const char * const message = "String literal spawning multiple lines found.";
			const int size = ( int )strlen( message );
			#ifdef OS_W32_VS
				strncpy_s( p_MsgBuf, DSP_MSGBUFSIZE, message, size );
			#else
				strncpy(p_MsgBuf, message, size + 1);
			#endif
			p_MsgBuf[ size ] = 0;
			p_Engine->GetEngineManager()->OutputWarning(p_MsgBuf, dsEngine::pwStringMulLineSpawn, p_Source, p_RefLineNum, p_RefCharNum);
			p_WarnCount++; p_AdvanceChars(); p_AdvanceLine();
		}else{
			p_AddStringChar(p_ParseByte());
		}
	}
	if(!(vNode = new dspNodeString(p_Source, p_RefLineNum, p_RefCharNum, p_StrBuf))) DSTHROW(dueOutOfMemory);
	p_AdvanceChars();
	return vNode;
}
dspBaseNode *dspScanner::p_ScanIdent(){
	dspBaseNode *vNode = NULL;
	p_ClearToken();
	while((p_CurChar>='A' && p_CurChar<='Z') || (p_CurChar>='a' && p_CurChar<='z') || (p_CurChar>='0' && p_CurChar<='9') || p_CurChar=='_'){
		p_AdvanceChars();
	}
	if(strcmp(p_TokenBuf, "true") == 0){
		if(!(vNode = new dspNodeBool(p_Source, p_RefLineNum, p_RefCharNum, true))) DSTHROW(dueOutOfMemory);
	}else if(strcmp(p_TokenBuf, "false") == 0){
		if(!(vNode = new dspNodeBool(p_Source, p_RefLineNum, p_RefCharNum, false))) DSTHROW(dueOutOfMemory);
	}else if(strcmp(p_TokenBuf, "null") == 0){
		if(!(vNode = new dspNodeNull(p_Source, p_RefLineNum, p_RefCharNum))) DSTHROW(dueOutOfMemory);
	}else if(strcmp(p_TokenBuf, "this") == 0){
		if(!(vNode = new dspNodeThis(p_Source, p_RefLineNum, p_RefCharNum))) DSTHROW(dueOutOfMemory);
	}else if(strcmp(p_TokenBuf, "super") == 0){
		if(!(vNode = new dspNodeSuper(p_Source, p_RefLineNum, p_RefCharNum))) DSTHROW(dueOutOfMemory);
	}else{
		for(int i=0; i<p_KeywordCount; i++){
			if(strcmp(p_TokenBuf, p_KeywordList[i]) == 0){
				if(!(vNode = new dspNodeKeyword(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf))) DSTHROW(dueOutOfMemory);
				return vNode;
			}
		}
		if(!(vNode = new dspNodeIdent(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf))) DSTHROW(dueOutOfMemory);
	}
	return vNode;
}
dspBaseNode *dspScanner::p_ScanComment(){
	p_AdvanceChars(2,true);
	while(p_CurChar!='\n' && p_CurChar!='\0'){
		p_AdvanceChars(1,true);
	}
	return NULL;
}
dspBaseNode *dspScanner::p_ScanInlineComment(){
	p_AdvanceChars(2,true);
	while(!(p_CurChar=='*' && p_NextChar=='/')){
		if(p_CurChar == '\0'){
			const char * const message = "Unclosed inline comment found.";
			const int size = ( int )strlen( message );
			#ifdef OS_W32_VS
				strncpy_s( p_MsgBuf, DSP_MSGBUFSIZE, message, size );
			#else
				strncpy(p_MsgBuf, message, size + 1);
			#endif
			p_MsgBuf[ size ] = 0;
			p_Engine->GetEngineManager()->OutputWarning(p_MsgBuf, dsEngine::pwUnclosedInlineComment, p_Source, p_RefLineNum, p_RefCharNum);
			p_WarnCount++; break;
		}else if(p_CurChar == '\n'){
			p_AdvanceChars(1,true); p_AdvanceLine();
		}else{
			p_AdvanceChars(1,true);
		}
	}
	p_AdvanceChars(2,true);
	return NULL;
}
dspBaseNode *dspScanner::p_ScanOperator(){
	dspBaseNode *vNode = NULL;
	char vCheckOp[4];
	bool partialMatch = false;
	int i, j, vOpIndex = -1;
	p_ClearToken();
	for(i=0; i<3; i++){
		vCheckOp[i] = p_CurChar; vCheckOp[i+1] = '\0';
		partialMatch = false;
		for(j=0; j<p_OperatorCount; j++){
			if(strncmp(p_OperatorList[j], vCheckOp, i+1) == 0){
				if(p_OperatorList[j][i+1] == '\0'){
					vOpIndex = j;
					break;
				}else{
					partialMatch = true;
				}
			}
		}
		if(j==p_OperatorCount && !partialMatch) break;
		p_AdvanceChars();
	}
	if(vOpIndex == -1){
		p_SkipInvalidChars();
	}else{
		// a bit hacky but for the moment it will work
		if(p_OperatorList[vOpIndex][0] == '\n'){
			if(!(vNode = new dspNodeOperator(p_Source, p_RefLineNum, p_RefCharNum, "NL"))) DSTHROW(dueOutOfMemory);
			p_AdvanceLine();
		}else if(p_OperatorList[vOpIndex][0] == ':'){
			if(!(vNode = new dspNodeOperator(p_Source, p_RefLineNum, p_RefCharNum, "NL"))) DSTHROW(dueOutOfMemory);
		}else{
			if(!(vNode = new dspNodeOperator(p_Source, p_RefLineNum, p_RefCharNum, p_TokenBuf))) DSTHROW(dueOutOfMemory);
		}
	}
	return vNode;
}
char dspScanner::p_ParseByte(){
	char vChar=0;
	if(p_CurChar == '\\'){
		if(p_NextChar>='0' && p_NextChar<='7'){
			p_AdvanceChars(); vChar = p_ParseOctalInt();
		}else if(p_NextChar == 'h'){
			p_AdvanceChars(2); vChar = (char)p_ParseHexalInt(2);
		}else{
			if(p_NextChar == 'a') vChar = '\a';
			else if(p_NextChar == 'b') vChar = '\b';
			else if(p_NextChar == 'f') vChar = '\f';
			else if(p_NextChar == 'n') vChar = '\n';
			else if(p_NextChar == 'r') vChar = '\r';
			else if(p_NextChar == 't') vChar = '\t';
			else if(p_NextChar == 'v') vChar = '\v';
			else vChar = p_NextChar;
			p_AdvanceChars(2);
		}
	}else{
		vChar = p_CurChar; p_AdvanceChars();
	}
	return vChar;
}
byte dspScanner::p_ParseBinaryInt(){
	byte vByteVal = 0;
	for(int i=0; i<8; i++){
		if(!(p_CurChar=='0' || p_CurChar=='1')) break;
		vByteVal <<= 1; vByteVal += p_CurChar-'0';
		p_AdvanceChars();
	}
	return vByteVal;
}
int dspScanner::p_ParseOctalInt(){
	int vIntVal = 0;
	for(int i=0; i<3; i++){
		if(!(p_CurChar>='0' && p_CurChar<='7')) break;
		vIntVal <<= 3; vIntVal += p_CurChar-'0';
		p_AdvanceChars();
	}
	return vIntVal;
}
int dspScanner::p_ParseHexalInt(int Len){
	int i, vAddInt, vIntVal = 0;
	for(i=0; i<Len; i++){
		if(p_CurChar>='0' && p_CurChar<='9') vAddInt = p_CurChar - '0';
		else if(p_CurChar>='a' && p_CurChar<='f') vAddInt = 10 + (p_CurChar - 'a');
		else if(p_CurChar>='A' && p_CurChar<='F') vAddInt = 10 + (p_CurChar - 'A');
		else break;
		vIntVal <<= 4; vIntVal += vAddInt; p_AdvanceChars();
	}
	return vIntVal;
}
void dspScanner::p_SkipBlanks(){
	while( (p_CurChar>=0x09 && p_CurChar<=0x0d) || p_CurChar==0x20 || p_CurChar=='\\' ){
		if(p_CurChar == '\\'){
			p_ClearToken(); p_AdvanceChars();
			if(p_CurChar == '\r') p_AdvanceChars(); // nasty, skip carrier returns
			if(p_CurChar != '\n'){
				p_SkipInvalidChars();
			}else{
				p_AdvanceChars(1,true); p_AdvanceLine();
			}
		}else if(p_CurChar == '\n'){
			break;
		}else{
			p_AdvanceChars(1,true);
		}
	}
}
void dspScanner::p_SkipInvalidChars(){
	int i;
	p_ClearToken();
	while(true){
		if((p_CurChar>=0x09 && p_CurChar<=0x0d) || p_CurChar==0x20) break;
		for(i=0; i<p_OperatorCount; i++){
			if(p_OperatorList[i][0] == p_CurChar) break;
		}
		if(i < p_OperatorCount) break;
		p_AdvanceChars();
	}
	snprintf(p_MsgBuf, DSP_MSGBUFSIZE, "Invalid token '%s' found.", p_TokenBuf);
	p_Engine->GetEngineManager()->OutputError(p_MsgBuf, dsEngine::peInvalidToken, p_Source, p_RefLineNum, p_RefCharNum);
	p_ErrCount++;
}

