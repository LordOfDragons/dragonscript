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
#ifndef _DSPNODESSCRIPT_H_
#define _DSPNODESSCRIPT_H_

// function argument node
class dspNodeFuncArg : public dspBaseNode{
private:
	char *p_Name;
	dspBaseNode *p_Type;
	dspBaseNode *p_DefVal;
public:
	dspNodeFuncArg(dspBaseNode *RefNode, const char *Name, dspBaseNode *Type, dspBaseNode *DefValue);
	~dspNodeFuncArg();
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dspBaseNode *GetTypeNode() const{ return p_Type; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// init constructor call node
class dspNodeInitConstrCall : public dspBaseNode{
private:
	dspListNodes p_Args;
	dsFunction *p_DSClsFunc;
	bool p_Mode;
public:
	dspNodeInitConstrCall(dspBaseNode *RefNode);
	~dspNodeInitConstrCall();
	inline dspListNodes *GetArgumentList(){ return &p_Args; }
	void AddArgument(dspBaseNode *Argument);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// function node
class dspNodeClassFunction : public dspBaseNode{
private:
	char *p_Name;
	int p_FuncType, p_TypeMods, p_Flags;
	dspBaseNode *p_Type;
	dspListNodes p_Args;
	dspNodeInitConstrCall *p_InitConstr;
	dspBaseNode *p_Sta;
	dsClass *p_DSType;
public:
	dspNodeClassFunction(dspBaseNode *RefNode, const char *Name, int FuncType, int TypeMods, dspBaseNode *Type);
	~dspNodeClassFunction();
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dspBaseNode *GetTypeNode() const{ return p_Type; }
	inline dspListNodes *GetArgList(){ return &p_Args; }
	inline dspBaseNode *GetStaNode() const{ return p_Sta; }
	inline dspNodeInitConstrCall *GetInitConstrCall() const{ return p_InitConstr; }
	inline int GetFuncType() const{ return p_FuncType; }
	inline int GetTypeMods() const{ return p_TypeMods; }
	void AddArgument(dspNodeFuncArg *Argument);
	void SetStatement(dspBaseNode *Statement);
	void SetDSType(dsClass *Class);
	void SetInitConstrCall(dspNodeInitConstrCall *Init);
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// class variable nodes
class dspNodeClassVariableList : public dspBaseNode{
private:
	int p_TypeMods;
	dspBaseNode *p_Type;
	dspListNodes p_Vars;
	dsClass *pTypeClass;
public:
	dspNodeClassVariableList(dspBaseNode *RefNode, int TypeMods, dspBaseNode *Type);
	~dspNodeClassVariableList();
	inline int GetTypeMods() const{ return p_TypeMods; }
	inline dspBaseNode *GetTypeNode() const{ return p_Type; }
	inline dspListNodes *GetVarList(){ return &p_Vars; }
	inline dsClass *GetTypeClass() const{ return pTypeClass; }
	void AddVariable(dspNodeClassVariable *Variable);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

class dspNodeClassVariable : public dspBaseNode{
private:
	char *p_Name;
	dspBaseNode *p_Init;
	dsClass *pTypeClass;
public:
	dspNodeClassVariable(dspNodeIdent *IdentNode, dspBaseNode *Init);
	~dspNodeClassVariable();
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dspBaseNode *GetInitNode() const{ return p_Init; }
	void SetTypeClass( dsClass *sclass ){ pTypeClass = sclass; }
	bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	void CreateInit(dspNodeStaList *StaList, dsVariable *Variable);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};


// class node
class dspNodeClass : public dspBaseNode{
private:
	char *p_Name;
	int p_ClsType, p_TypeMods;
	dspBaseNode *p_BaseClass;
	dspListNodes p_ImplClasses;
	dspListNodes p_Members;
	dspNodeStaList *p_InitSta;
public:
	dspNodeClass(dspBaseNode *RefNode, const char *Name, int ClassType, int TypeMods);
	~dspNodeClass();
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dspBaseNode *GetBaseClassNode() const{ return p_BaseClass; }
	inline dspListNodes *GetMemberList(){ return &p_Members; }
	inline dspListNodes *GetImplList(){ return &p_ImplClasses; }
	inline dspNodeStaList **GetInitSta(){ return &p_InitSta; }
	inline int GetClassType() const{ return p_ClsType; }
	inline int GetTypeMods() const{ return p_TypeMods; }
	void SetBaseClass(dspBaseNode *Class);
	void AddImplClass(dspBaseNode *Class);
	void AddMember(dspBaseNode *Member);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// class dspNodeEnumeration
class dspNodeEnumeration : public dspNodeClass{
public:
	dspNodeEnumeration(dspBaseNode *RefNode, const char *Name, int TypeMods);
	~dspNodeEnumeration();
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// pin node
class dspNodePin : public dspBaseNode{
private:
	dspBaseNode *p_Namespace;
public:
	dspNodePin(dspBaseNode *RefNode, dspBaseNode *Namespace);
	~dspNodePin();
	inline dspBaseNode *GetNamespace() const{ return p_Namespace; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// namespace node
class dspNodeNamespace : public dspBaseNode{
private:
	dspBaseNode *p_Namespace;
public:
	dspNodeNamespace(dspBaseNode *RefNode, dspBaseNode *Namespace);
	~dspNodeNamespace();
	inline dspBaseNode *GetNamespace() const{ return p_Namespace; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// option node
class dspNodeOption : public dspBaseNode{
private:
	char *p_Key;
	char *p_Value;
public:
	dspNodeOption(dspBaseNode *RefNode, const char *Key, const char *Value);
	~dspNodeOption();
	inline const char *GetKey() const{ return (const char *)p_Key; }
	inline const char *GetValue() const{ return (const char *)p_Value; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// requires node
class dspNodeRequires : public dspBaseNode{
private:
	char *p_Package;
public:
	dspNodeRequires(dspBaseNode *RefNode, const char *Package);
	~dspNodeRequires();
	inline const char *GetPackageName() const{ return (const char *)p_Package; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// includes node
class dspNodeIncludes : public dspBaseNode{
private:
	char *p_Package;
public:
	dspNodeIncludes(dspBaseNode *RefNode, const char *Package);
	~dspNodeIncludes();
	inline const char *GetPackageName() const{ return (const char *)p_Package; }
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// script node
class dspNodeScript : public dspBaseNode{
private:
	dspListNodes p_Nodes;
public:
	dspNodeScript(dsScriptSource *Source);
	~dspNodeScript();
	inline dspListNodes *GetNodeList(){ return &p_Nodes; }
	void AddNode(dspBaseNode *Node);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// end of include only once
#endif

