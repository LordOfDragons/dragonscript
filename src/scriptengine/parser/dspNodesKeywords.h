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
#ifndef _DSPNODESKEYWORDS_H_
#define _DSPNODESKEYWORDS_H_

// keyword node
class DS_DLL_EXPORT dspNodeKeyword : public dspBaseNode{
private:
	char *p_Keyword;
public:
	dspNodeKeyword(dsScriptSource *Source, int LineNum, int CharNum, const char *Keyword);
	~dspNodeKeyword();
	inline const char *GetKeyword() const{ return (const char *)p_Keyword; }
	const char *GetTerminalString();
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// null object node
class DS_DLL_EXPORT dspNodeNull : public dspBaseNode{
private:
	dsClass *pType;
public:
	dspNodeNull(dsScriptSource *Source, int LineNum, int CharNum);
	dspNodeNull(dspBaseNode *RefNode, dsClass *Type);
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// this object node
class DS_DLL_EXPORT dspNodeThis : public dspBaseNode{
public:
	dspNodeThis(dsScriptSource *Source, int LineNum, int CharNum);
	dspNodeThis(dspBaseNode *RefNode);
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// super object node
class DS_DLL_EXPORT dspNodeSuper : public dspBaseNode{
public:
	dspNodeSuper(dsScriptSource *Source, int LineNum, int CharNum);
	const char *GetTerminalString();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// end of include only once
#endif

