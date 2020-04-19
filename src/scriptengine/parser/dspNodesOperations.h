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
#ifndef _DSPNODESOPERATIONS_H_
#define _DSPNODESOPERATIONS_H_

// operator node
class dspNodeOperator : public dspBaseNode{
private:
	char *p_Op;
public:
	dspNodeOperator(dsScriptSource *Source, int LineNum, int CharNum, const char *Operator);
	~dspNodeOperator();
	inline const char *GetOperator() const{ return (const char *)p_Op; }
	const char *GetTerminalString();
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// unary operation node
class dspNodeUnaryOperation : public dspBaseNode{
private:
	char *p_Op;
	dspBaseNode *p_Obj;
	dsClass *p_ObjClass;
	int p_FuncID;
public:
	dspNodeUnaryOperation(dspNodeOperator *OpNode, dspBaseNode *Obj);
	dspNodeUnaryOperation(dspBaseNode *RefNode, const char *Op, dspBaseNode *Obj);
	~dspNodeUnaryOperation();
	inline const char *GetOperator() const{ return (const char *)p_Op; }
	dspBaseNode *Simplify();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// post increment node
class dspNodePostIncrement : public dspBaseNode{
private:
	dspBaseNode *p_Obj;
	dsClass *p_ObjClass;
public:
	dspNodePostIncrement(dspBaseNode *RefNode, dspBaseNode *Obj);
	~dspNodePostIncrement();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
	dspBaseNode *Reduce();
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// post decrement node
class dspNodePostDecrement : public dspBaseNode{
private:
	dspBaseNode *p_Obj;
	dsClass *p_ObjClass;
public:
	dspNodePostDecrement(dspBaseNode *RefNode, dspBaseNode *Obj);
	~dspNodePostDecrement();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
	dspBaseNode *Reduce();
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// binary operation node
class dspNodeBinaryOperation : public dspBaseNode{
private:
	char *p_Op;
	dspBaseNode *p_Obj, *p_Arg;
	dsClass *p_ObjClass, *p_ArgClass;
	int p_FuncID;
public:
	dspNodeBinaryOperation(dspNodeOperator *OpNode, dspBaseNode *Obj, dspBaseNode *Arg);
	dspNodeBinaryOperation(dspBaseNode *RefNode, const char *Op, dspBaseNode *Obj, dspBaseNode *Arg);
	~dspNodeBinaryOperation();
	inline const char *GetOperator() const{ return (const char *)p_Op; }
	dspBaseNode *Simplify();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// inline if node
class dspNodeInlineIf : public dspBaseNode{
private:
	dspBaseNode *p_Cond, *p_If, *p_Else;
public:
	dspNodeInlineIf(dspBaseNode *RefNode, dspBaseNode *Condition, dspBaseNode *IfPart, dspBaseNode *ElsePart);
	~dspNodeInlineIf();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// logical and node
class dspNodeLogicalAnd : public dspBaseNode{
private:
	dspBaseNode *p_Cond1, *p_Cond2;
public:
	dspNodeLogicalAnd(dspBaseNode *RefNode, dspBaseNode *Condition1, dspBaseNode *Condition2);
	~dspNodeLogicalAnd();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// logical or node
class dspNodeLogicalOr : public dspBaseNode{
private:
	dspBaseNode *p_Cond1, *p_Cond2;
public:
	dspNodeLogicalOr(dspBaseNode *RefNode, dspBaseNode *Condition1, dspBaseNode *Condition2);
	~dspNodeLogicalOr();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// logical not node
class dspNodeLogicalNot : public dspBaseNode{
private:
	dspBaseNode *p_Cond;
public:
	dspNodeLogicalNot(dspBaseNode *RefNode, dspBaseNode *Condition);
	~dspNodeLogicalNot();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// cast to node
class dspNodeCastTo : public dspBaseNode{
private:
	dspBaseNode *p_Obj, *p_Type;
	dsClass *p_DSClass;
public:
	dspNodeCastTo(dspBaseNode *RefNode, dspBaseNode *Object, dspBaseNode *Type);
	~dspNodeCastTo();
	inline dsClass *GetDSClass() const{ return p_DSClass; }
	dspBaseNode *Simplify();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// castable to node
class dspNodeCastableTo : public dspBaseNode{
private:
	dspBaseNode *p_Obj, *p_Type;
	dsClass *p_DSClass;
public:
	dspNodeCastableTo(dspBaseNode *RefNode, dspBaseNode *Object, dspBaseNode *Type);
	~dspNodeCastableTo();
	dspBaseNode *Simplify();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// is type of node
class dspNodeIsTypeOf : public dspBaseNode{
private:
	dspBaseNode *p_Obj, *p_Type;
	dsClass *p_DSClass;
public:
	dspNodeIsTypeOf(dspBaseNode *RefNode, dspBaseNode *Object, dspBaseNode *Type);
	~dspNodeIsTypeOf();
	dspBaseNode *Simplify();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// end of include only once
#endif

