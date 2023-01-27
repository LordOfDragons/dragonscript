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
#ifndef _DSPPARSER_H_
#define _DSPPARSER_H_

// includes

// predefinitions
class dsEngine;
class dsScriptSource;
class dsPackage;
class dspScanner;
class dspBaseNode;
class dspListNodes;
class dspNodeIdent;
class dspNodeString;
class dspNodeOperator;
class dspNodeCase;
class dspNodeCatch;
class dspNodeClassVariableList;
class dspNodeClassVariable;
class dspNodeClassFunction;
class dspNodeFuncArg;
class dspNodeEnumEntry;
class dspNodeEnumeration;
class dspNodeClass;
class dspNodeError;
class dspNodeNamespace;
class dspNodePin;
class dspNodeRequires;
class dspNodeScript;
class dspTypeModifiers;

// class dspParser
class DS_DLL_EXPORT dspParser{
friend class dsEngine;
friend class dspParserCheck;
friend class dspParserCheckClass;
private:
	dsEngine *p_Engine;
	dsPackage *p_Package;
	dspScanner *p_Scanner;
	dspBaseNode **p_NodeQueue;
	int p_NodeQueueSize, p_NodeQueueEffSize;
	dspNodeScript **p_ScriptNodes;
	int p_ScriptNodeCount;
	int p_ErrCount;
	int p_WarnCount;
	int p_LastErrLine;
	char *p_MsgBuf;
public:
	// constructor, destructor
	dspParser(dsEngine *Engine);
	~dspParser();
	// parsing
	void Clear();
	bool ParsePackage(dsPackage *Package);
	// engine
	inline dsEngine *GetEngine() const{ return p_Engine; }
	inline dsPackage *GetPackage() const{ return p_Package; }
	// status functions
	inline int GetErrorCount() const{ return p_ErrCount; }
	inline int GetWarningCount() const{ return p_WarnCount; }
	inline void IncErrCount(){ p_ErrCount++; }
	inline void IncWarnCount(){ p_WarnCount++; }
	// script node informations
	inline int GetScriptNodeCount() const{ return p_ScriptNodeCount; }
	dspNodeScript *GetScriptNode(int Index) const;
private:
	// node queue management
	bool p_QueueNextNode();
	void p_DropNode();
	dspBaseNode *p_PopNode();
	dspBaseNode *p_PeekNode(int Index);
	void p_ClearQueue();
	// matching functions
	bool p_MatchesConst(dspBaseNode *Node);
	bool p_MatchesType(dspBaseNode *Node, int Type);
	bool p_MatchesIdent(dspBaseNode *Node, const char *Ident);
	bool p_MatchesKeyword(dspBaseNode *Node, const char *Keyword);
	bool p_MatchesOperator(dspBaseNode *Node, const char *Operator);
	bool p_MatchesOperatorList(dspBaseNode *Node, const char **Ops, int Count);
	void p_HasToBeOperator(const char *Operator);
	void p_HasToBeKeyword(const char *Keyword);
	void p_HasToBeBlockEnd(const char *Keyword);
	void p_DropEmptyLines();
	// error functions
	void p_ErrorUnexpEOF();
	void p_ErrorUnexpNode(const char *Expected, dspBaseNode *Found);
	void p_ErrorInvName(const char *ErrName, dspBaseNode *Node, int ErrID);
	void p_ErrorInvOperator(const char *ErrName, dspBaseNode *Node, int ErrID);
	void p_ErrorInvTypeMod(dspBaseNode *Node);
	void p_ErrorIncompTypeMod(dspBaseNode *Node, const char *TypeMod);
	void p_ErrorNonEmptyIFFunc(dspBaseNode *Node);
	void p_ErrorUnexpectedToken( dspBaseNode *node );
	// warning functions
	void p_WarnDupTypeMod(dspBaseNode *Node);
	// parsing functions
	dspNodeScript *p_ParseScript(dsScriptSource *Script);
	dspNodeIdent *p_ParseName(const char *ErrName, int ErrID);
	dspNodeString *p_ParseString(const char *ErrName, int ErrID);
	dspNodeNamespace *p_ParseNamespace();
	dspBaseNode *p_ParseRequires();
	dspNodePin *p_ParsePin();
	dspNodeClass *p_ParseAnyClass(dspTypeModifiers *TypeMods, bool onlyPub);
	dspNodeClass *p_ParseClass(dspTypeModifiers *TypeMods, bool onlyPub);
	dspNodeClass *p_ParseInterface(dspTypeModifiers *TypeMods, bool onlyPub);
	dspTypeModifiers *p_ParseTypeModifiers();
	dspBaseNode *p_ParseClassMemberDef(dspTypeModifiers *TypeMods);
	dspNodeClassFunction *p_ParseClassFunction(dspTypeModifiers *TypeMods);
	dspNodeClassVariableList *p_ParseClassVariableList(dspTypeModifiers *TypeMods);
	dspNodeClassVariable *p_ParseClassVariable();
	dspBaseNode *p_ParseInterfaceMemberDef(dspTypeModifiers *TypeMods);
	dspNodeClassFunction *p_ParseInterfaceFuncDef(dspTypeModifiers *TypeMods);
	dspNodeFuncArg *p_ParseFuncArg();
	dspNodeEnumeration *p_ParseEnumeration(dspTypeModifiers *TypeMods, bool onlyPub);
	dspBaseNode *p_ParseSta();
	dspBaseNode *p_ParseGroupedSta(const char *end="end", const char *enda1=NULL, const char *enda2=NULL);
	dspBaseNode *p_ParseIfElse(bool isElif=false);
	dspBaseNode *p_ParseSelect();
	dspNodeCase *p_ParseCase();
	dspBaseNode *p_ParseWhile();
	dspBaseNode *p_ParseFor();
	dspBaseNode *p_ParseBreak();
	dspBaseNode *p_ParseContinue();
	dspBaseNode *p_ParseThrow();
	dspBaseNode *p_ParseVarDefList();
	dspBaseNode *p_ParseVarDef();
	dspBaseNode *p_ParseExpr();
	dspBaseNode *p_ParseObject();
	dspBaseNode *p_ParseBaseObject();
	dspBaseNode *p_ParseBlockExpr();
	dspBaseNode *p_ParseType();
	dspBaseNode *p_ParseRetVal();
	dspBaseNode *p_ParseTry();
	dspNodeCatch *p_ParseCatch();
	dspBaseNode *p_ParseClassMember(dspBaseNode *Object);
	dspBaseNode *p_ParseGroupedExpr();
	dspBaseNode *p_ParseUnaryOperation();
	dspBaseNode *p_ParseSpecOperation(dspBaseNode *Object);
	dspBaseNode *p_ParseMulOperation(dspBaseNode *Object);
	dspBaseNode *p_ParseAddOperation(dspBaseNode *Object);
	dspBaseNode *p_ParseBitOperation(dspBaseNode *Object);
	dspBaseNode *p_ParseCompOperation(dspBaseNode *Object);
	dspBaseNode *p_ParseLogCompOperation(dspBaseNode *Object);
	dspBaseNode *p_ParseAssignOperation(dspBaseNode *Object);
	dspBaseNode *p_ParseInlineIfOperation(dspBaseNode *Object);
	void p_ParseFuncArgs(dspListNodes *ArgList);
	void p_SkipToOperator(const char *Operator);
	void p_SkipToOperators(const char **OperatorList, int Count);
	void p_SkipToOpWithBreak(const char *SkipOp, const char *BreakOp);
	void p_SkipToOpWithBreakKW(const char *SkipOp, const char *BreakKW);
	void p_SkipToKeyword(const char *Keyword);
	void p_CheckTypeMods(dspTypeModifiers *TypeMods, int allowed);
	// add class for dspParserCheckClass
	void p_AddParsedClass(dsClass *Class);
};

// end of include only once
#endif

