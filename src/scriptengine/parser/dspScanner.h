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



// include only once
#ifndef _DSPSCANNER_H_
#define _DSPSCANNER_H_

// includes
#include "../dsDefinitions.h"

// predefinitions
class dsEngine;
class dsScriptSource;
class dspBaseNode;

// class dspScanner
class dspScanner{
private:
	dsEngine *p_Engine;
	dsScriptSource *p_Source;
	char *p_ReadBuf;
	int p_ReadBufLen;
	int p_ReadBufPos;
	char *p_TokenBuf;
	int p_TokenBufPos;
	int p_TokenBufLen;
	char *p_StrBuf;
	int p_StrBufPos;
	int p_StrBufLen;
	char p_CurChar, p_NextChar;
	int p_LineNum, p_CharNum;
	int p_RefLineNum, p_RefCharNum;
	int p_ErrCount, p_WarnCount;
	char *p_MsgBuf;
	bool p_NeedsNL;
public:
	// constructor, destructor
	dspScanner(dsEngine *Engine);
	~dspScanner();
	// scanning management
	void SetScript(dsScriptSource *Script);
	void ClearScript();
	dspBaseNode *GetNextNode();
	inline dsScriptSource *GetScript() const{ return p_Source; }
	// information
	inline int GetErrorCount() const{ return p_ErrCount; }
	inline int GetWarningCount() const{ return p_WarnCount; }
	inline int GetLineNum() const{ return p_LineNum; }
	inline int GetCharNum() const{ return p_CharNum; }
private:
	void p_AdvanceChars(int Count = 1, bool Skip = false);
	void p_AdvanceLine();
	void p_AddTokenChar(char Char);
	void p_AddStringChar(char Char);
	void p_ClearToken();
	dspBaseNode *p_ScanBinaryInt();
	dspBaseNode *p_ScanOctalInt();
	dspBaseNode *p_ScanHexalInt();
	dspBaseNode *p_ScanIntOrFloat();
	dspBaseNode *p_ScanByteOrByteInt();
	dspBaseNode *p_ScanString();
	dspBaseNode *p_ScanIdent();
	dspBaseNode *p_ScanComment();
	dspBaseNode *p_ScanInlineComment();
	dspBaseNode *p_ScanOperator();
	char p_ParseByte();
	byte p_ParseBinaryInt();
	int p_ParseOctalInt();
	int p_ParseHexalInt(int Len = 8);
	void p_SkipBlanks();
	void p_SkipInvalidChars();
};

// end of include only once
#endif

