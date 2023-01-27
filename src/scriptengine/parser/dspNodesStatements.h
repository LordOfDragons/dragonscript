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
#ifndef _DSPNODESSTATEMENTS_H_
#define _DSPNODESSTATEMENTS_H_

// member nodes
class DS_DLL_EXPORT dspNodeMembVar : public dspBaseNode{
private:
	dspBaseNode *p_Obj;
	char *p_Name;
	dsClass *p_DSClass;
	int p_DSLocVarIndex;
	dsVariable *p_DSClsVar;
	dsConstant *p_DSClsConst;
	int p_ParamID;
	int pContextVariable;
	
public:
	dspNodeMembVar(dspNodeIdent *Member, dspBaseNode *Obj);
	dspNodeMembVar(dspBaseNode *RefNode, const char *Name);
	~dspNodeMembVar();
	inline const char *GetName() const{ return p_Name; }
	inline dspBaseNode *GetRealObject() const{ return p_Obj; }
	inline dsVariable *GetClassVariable() const{ return p_DSClsVar; }
	inline int GetParameterID() const{ return p_ParamID; }
	inline int GetContextVariable() const{ return pContextVariable; }
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

class DS_DLL_EXPORT dspNodeMembFunc : public dspBaseNode{
private:
	dspBaseNode *p_Obj;
	char *p_Name;
	dspListNodes p_Args;
	dsFunction *p_DSClsFunc;
	int p_FuncID;
	bool p_SuperCall;
public:
	dspNodeMembFunc(dspNodeIdent *Member, dspBaseNode *Obj);
	~dspNodeMembFunc();
	inline dspListNodes *GetArgumentList(){ return &p_Args; }
	void AddArgument(dspBaseNode *Argument);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// empty statement node
class DS_DLL_EXPORT dspNodeEmptySta : public dspBaseNode{
public:
	dspNodeEmptySta(dspBaseNode *RefNode);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// if-else node
class DS_DLL_EXPORT dspNodeIfElse : public dspBaseNode{
private:
	dspBaseNode *p_Cond;
	dspBaseNode *p_If;
	dspBaseNode *p_Else;
	dspBaseNode *pSimpleBlock;
public:
	dspNodeIfElse(dspBaseNode *RefNode, dspBaseNode *Condition, dspBaseNode *IfPart, dspBaseNode *ElsePart);
	~dspNodeIfElse();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// select-case nodes
class DS_DLL_EXPORT dspNodeCase : public dspBaseNode{
private:
	dspListNodes p_ValNodes;
	dspBaseNode *p_Sta;
	int p_IntValCount;
	int *p_IntValues;
	int p_StaValCount;
	dsVariable **p_StaValues;
public:
	dspNodeCase(dspBaseNode *RefNode);
	~dspNodeCase();
	void AddValue(dspBaseNode *Value);
	void SetStatement(dspBaseNode *Sta);
	bool MatchesValues(int Value);
	bool MatchesValues(dspNodeCase *Case, int *Value);
	inline int GetValueCount() const{ return p_IntValCount; }
	int GetValue(int Index) const;
	bool MatchesStaticValues(dsVariable *Value);
	bool MatchesStaticValues(dspNodeCase *Case, dsVariable **Value);
	inline int GetStaticValueCount() const{ return p_StaValCount; }
	dsVariable *GetStaticValue(int Index) const;
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCodeCond(dspParserCompileCode *CompileCode, int Num);
	void CompileCodeSta(dspParserCompileCode *CompileCode, bool Jump);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
private:
	void p_AddIntValue(int Value);
	void p_AddStaticValue(dsVariable *Value);
};

class DS_DLL_EXPORT dspNodeSelect : public dspBaseNode{
private:
	dspBaseNode *p_Cond;
	dspListNodes p_Cases;
	dspBaseNode *p_StaElse;
public:
	dspNodeSelect(dspBaseNode *RefNode, dspBaseNode *Condition);
	~dspNodeSelect();
	void AddCase(dspNodeCase *Case);
	void SetCaseElse(dspBaseNode *StaElse);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
private:
	bool p_MatchesCases(dspParserCheckCode *CheckCode, dspNodeCase *Case, int Count);
};

// while node
class DS_DLL_EXPORT dspNodeWhile : public dspBaseNode{
private:
	dspBaseNode *p_Cond, *p_Sta;
public:
	dspNodeWhile(dspBaseNode *RefNode, dspBaseNode *Condition, dspBaseNode *Statement);
	~dspNodeWhile();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// do while node
class DS_DLL_EXPORT dspNodeDoWhile : public dspBaseNode{
private:
	dspBaseNode *p_Cond, *p_Sta;
public:
	dspNodeDoWhile(dspBaseNode *RefNode, dspBaseNode *Condition, dspBaseNode *Statement);
	~dspNodeDoWhile();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// for nodes
class DS_DLL_EXPORT dspNodeForUp : public dspBaseNode{
private:
	dspBaseNode *p_Var, *p_From, *p_To, *p_Step, *p_Sta;
public:
	dspNodeForUp(dspBaseNode *RefNode, dspBaseNode *Var, dspBaseNode *From, dspBaseNode *To, dspBaseNode *Step, dspBaseNode *Sta);
	~dspNodeForUp();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

class DS_DLL_EXPORT dspNodeForDown : public dspBaseNode{
private:
	dspBaseNode *p_Var, *p_From, *p_To, *p_Step, *p_Sta;
public:
	dspNodeForDown(dspBaseNode *RefNode, dspBaseNode *Var, dspBaseNode *From, dspBaseNode *To, dspBaseNode *Step, dspBaseNode *Sta);
	~dspNodeForDown();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// break node
class DS_DLL_EXPORT dspNodeBreak : public dspBaseNode{
public:
	dspNodeBreak(dspBaseNode *RefNode);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// continue node
class DS_DLL_EXPORT dspNodeContinue : public dspBaseNode{
public:
	dspNodeContinue(dspBaseNode *RefNode);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// block node
class DS_DLL_EXPORT dspNodeBlock : public dspBaseNode{
private:
	dspListNodes p_Args;
	dspBaseNode *p_Sta;
	dsFunction *p_Func;
	bool p_NeedsReturn;
	
	int pCVarCount;
	short *pCVars;
	
public:
	dspNodeBlock(dspBaseNode *RefNode);
	~dspNodeBlock();
	void AddArgument(dspNodeFuncArg *Argument);
	void SetStatement(dspBaseNode *Statement);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
	
private:
	void pAddCVar( int accessNumber );
};

// variable declaration nodes
class DS_DLL_EXPORT dspNodeVarDefList : public dspBaseNode{
private:
	dspBaseNode *p_Type;
	dspListNodes p_Vars;
	dsClass *p_DSClass;
public:
	dspNodeVarDefList(dspBaseNode *RefNode, dspBaseNode *Type);
	~dspNodeVarDefList();
	inline dsClass *GetDSType() const{ return p_DSClass; }
	void AddVariable(dspNodeVarDef *Node);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

class DS_DLL_EXPORT dspNodeVarDef : public dspBaseNode{
private:
	char *p_Name;
	dspBaseNode *p_Init;
	dsClass *pTypeClass;
public:
	dspNodeVarDef(dspNodeIdent *IdentNode, dspBaseNode *Init);
	~dspNodeVarDef();
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dspBaseNode *GetInit() const{ return p_Init; }
	inline void SetTypeClass( dsClass *sclass ){ pTypeClass = sclass; }
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// return node
class DS_DLL_EXPORT dspNodeReturn : public dspBaseNode{
private:
	dspBaseNode *p_RetVal;
public:
	dspNodeReturn(dspBaseNode *RefNode, dspBaseNode *RetVal);
	~dspNodeReturn();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
	inline dspBaseNode *GetReturnValue() const{ return p_RetVal; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// throw node
class DS_DLL_EXPORT dspNodeThrow : public dspBaseNode{
private:
	dspBaseNode *p_Expr;
public:
	dspNodeThrow(dspBaseNode *RefNode, dspBaseNode *Expression);
	~dspNodeThrow();
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// try-catch nodes
class DS_DLL_EXPORT dspNodeCatch : public dspBaseNode{
private:
	dspBaseNode *p_CatchType;
	char *p_CatchName;
	dspBaseNode *p_Sta;
	dsClass *p_DSType;
public:
	dspNodeCatch(dspBaseNode *RefNode, dspBaseNode *Type, const char *VarName, dspBaseNode *Sta);
	~dspNodeCatch();
	inline const char *GetCatchName() const{ return (const char *)p_CatchName; }
	inline dsClass *GetDSType() const{ return p_DSType; }
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

class DS_DLL_EXPORT dspNodeTry : public dspBaseNode{
private:
	dspBaseNode *p_Sta;
	dspListNodes p_Catches;
public:
	dspNodeTry(dspBaseNode *RefNode, dspBaseNode *Sta);
	~dspNodeTry();
	void AddCatch(dspNodeCatch *Node);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
private:
	dspNodeCatch *p_FindCatch(dsClass *Type);
};

// statement list node
class DS_DLL_EXPORT dspNodeStaList : public dspBaseNode{
private:
	dspListNodes p_Nodes;
public:
	dspNodeStaList(dspBaseNode *RefNode);
	~dspNodeStaList();
	void AddNode(dspBaseNode *Node);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
	inline dspListNodes &GetNodes(){ return p_Nodes; }
	inline const dspListNodes &GetNodes() const{ return p_Nodes; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// end of include only once
#endif

