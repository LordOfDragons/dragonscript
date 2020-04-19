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
#ifndef _DSPNODESCONSTANTS_H_
#define _DSPNODESCONSTANTS_H_

// byte node
class dspNodeByte : public dspBaseNode{
private:
	byte p_Value;
	char *p_Token;
public:
	dspNodeByte(dsScriptSource *Source, int LineNum, int CharNum, const char *Token, byte Value);
	dspNodeByte(dspBaseNode *RefNode, byte Value);
	~dspNodeByte();
	inline byte GetValue() const{ return p_Value; }
	inline void SetValue(byte Value){ p_Value = Value; }
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// boolean node
class dspNodeBool : public dspBaseNode{
private:
	bool p_Value;
public:
	dspNodeBool(dsScriptSource *Source, int LineNum, int CharNum, bool Value);
	dspNodeBool(dspBaseNode *RefNode, bool Value);
	inline bool GetValue() const{ return p_Value; }
	inline void SetValue(bool Value){ p_Value = Value; }
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// int node
class dspNodeInt : public dspBaseNode{
private:
	int p_Value;
	char *p_Token;
public:
	dspNodeInt(dsScriptSource *Source, int LineNum, int CharNum, const char *Token, int Value);
	dspNodeInt(dspBaseNode *RefNode, int Value);
	~dspNodeInt();
	inline int GetValue() const{ return p_Value; }
	inline void SetValue(int Value){ p_Value = Value; }
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// float node
class dspNodeFloat : public dspBaseNode{
private:
	float p_Value;
	char *p_Token;
public:
	dspNodeFloat(dsScriptSource *Source, int LineNum, int CharNum, const char *Token, float Value);
	dspNodeFloat(dspBaseNode *RefNode, float Value);
	~dspNodeFloat();
	inline float GetValue() const{ return p_Value; }
	inline void SetValue(float Value){ p_Value = Value; }
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// string node
class dspNodeString : public dspBaseNode{
private:
	char *p_String;
public:
	dspNodeString(dsScriptSource *Source, int LineNum, int CharNum, const char *String);
	~dspNodeString();
	inline const char *GetString() const{ return (const char *)p_String; }
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// end of include only once
#endif

