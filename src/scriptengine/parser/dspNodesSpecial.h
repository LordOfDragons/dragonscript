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
#ifndef _DSPNODESSPECIAL_H_
#define _DSPNODESSPECIAL_H_

// auto cast node
class dspNodeAutoCast : public dspBaseNode{
private:
	dspBaseNode *p_Obj;
	dsClass *p_Type;
public:
	dspNodeAutoCast(dspBaseNode *Obj, dsClass *Type);
	~dspNodeAutoCast();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// assign init value to static variable
class dspNodeSetStaVar : public dspBaseNode{
private:
	dsVariable *p_Var;
	dspBaseNode *p_Init;
	dsClass *p_InitClass;
public:
	dspNodeSetStaVar(dspBaseNode *Init, dsVariable *Variable);
	~dspNodeSetStaVar();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// enumeration value
class dspNodeEnumValue : public dspBaseNode{
private:
	char *pName;
	int pOrder;
public:
	dspNodeEnumValue(dspBaseNode *RefNode, const char *name, int order);
	virtual ~dspNodeEnumValue();
	inline const char *GetName() const{ return pName; }
	inline int GetOrder() const{ return pOrder; }
	virtual bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	virtual void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// end of include only once
#endif

