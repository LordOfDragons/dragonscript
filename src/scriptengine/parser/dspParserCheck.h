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
#ifndef _DSPPARSERCHECK_H_
#define _DSPPARSERCHECK_H_

// includes

// predefinitions
class dsClass;
class dsScriptSource;
class dsEngine;
class dsFunction;
class dsVariable;
class dsConstant;
class dspParser;
class dspBaseNode;
class dspNodeClass;
class dspNodeScript;
class dspNodeVarDef;
class dspNodeClassFunction;
class dspNodeClassVariable;
class dspParserCheckType;
class dspParserCheckClass;
class dspParserCheckScript;

// class dspParserCheck
class dspParserCheck{
private:
	dspParser *p_Parser;
	dspParserCheckClass **p_Classes;
	int p_ClassCount;
	dspParserCheckScript **p_Scripts;
	int p_ScriptCount;
	dspParserCheckScript *p_curScript;
	char *p_MsgBuf;
public:
	// constructor, destructor
	dspParserCheck(dspParser *Parser);
	~dspParserCheck();
	// management
	void Clear();
	void ProcessScript(dspNodeScript *Script);
	void CheckScripts();
	void CompileScripts();
	void OptimizeScripts();
	dsClass *GetClassFromNode(dspBaseNode *Node, dsClass *BaseClass);
	dsClass *GetClassFromName(const char *Name, dsClass *BaseClass);
	dsClass *GetClassFromType(dspParserCheckType *Type, dsClass *BaseClass);
	void CreateNS(dspBaseNode *refNode, dspParserCheckType *type);
	// information
	inline dspParser *GetParser() const{ return p_Parser; }
	dsEngine *GetEngine() const;
	bool HasErrors() const;
	dsConstant *FindClassConstant(dsClass *OwnerClass, const char *ConstName);
	// error functions
	void ErrorClassExists(dspNodeClass *NodeClass, dspParserCheckType *TypeParent);
	void ErrorInheritanceLoop(dspParserCheckClass *Class);
	void ErrorClassNotExist(dspBaseNode *Node, dspParserCheckType *Type);
	void ErrorClassIsFixed(dspBaseNode *Node, dspParserCheckType *Type);
	void ErrorDupFunction(dspBaseNode *Node, const char *FuncName, const char *ClassName);
	void ErrorDupDestructor(dspBaseNode *Node, const char *ClassName);
	void ErrorDupOperator(dspBaseNode *Node, const char *Op, const char *ClassName);
	void ErrorInvDupFuncSig(dspBaseNode *Node, const char *FuncName, const char *ClassName);
	void ErrorInvOpArgCount(dspBaseNode *Node, const char *Op, const char *ClassName);
	void ErrorInvDestrSig(dspBaseNode *Node, const char *Name);
	void ErrorDupVariable(dspBaseNode *Node, const char *ClassName, const char *VarName);
	void ErrorDupConstant(dspBaseNode *Node, const char *ClassName, const char *ConstName);
	void ErrorInvVarTypeVoid(dspBaseNode *Node, const char *VarName);
	void ErrorInvLocVarTypeVoid(dspBaseNode *Node, const char *VarName);
	void ErrorDupLocVar(dspNodeVarDef *Node);
	void ErrorNoStaVarInit(dspNodeClassVariable *Node, const char *ClassName);
	void ErrorInvalidVarInit(dspNodeClassVariable *Node, const char *ClassName);
	void ErrorMissingReturn(dspBaseNode *Node, const char *FuncName, const char *ClassName);
	void ErrorNotInterface(dspBaseNode *Node, dsClass *Class);
	void ErrorBaseNotSameType(dspBaseNode *Node, dsClass *Base, dsClass *Class);
	void ErrorDupImplement(dspBaseNode *Node, dsClass *Class, dsClass *ImplClass);
	void ErrorFuncNotImplemented(dspBaseNode *Node, dsClass *OwnerClass, dsFunction *Function);
	void ErrorNSMemberNotNS(dspBaseNode *Node, dsClass *OwnerNS, const char *MemberClass);
	void ErrorConstLoop(dspNodeClassVariable *Node, const char *ClassName);
	void ErrorPackageNotFound(dspBaseNode *Node, const char *Package);
	void ErrorFuncInvTypeMods( dspBaseNode *node, const char *className, const char *functionName );
	void WarnPackageEmpty(dspBaseNode *Node, const char *Package);
	// class checks
	bool ExistsClass(const char *Name, dspParserCheckType *TypeParent);
	bool ExistsClass(dspParserCheckType *TypeClass);
private:
	// check functions
	void p_AddClass(dspParserCheckClass *Class);
	void p_RemoveClass(int Index);
	void p_ProcessClass(dspNodeClass *Class, dspParserCheckType *ParentType);
	dspParserCheckClass *p_FindClass(const char *Name, dspParserCheckType *TypeParent);
	void p_CheckInheritance();
	void p_AddScript(dspParserCheckScript *Script);
};

// end of include only once
#endif

