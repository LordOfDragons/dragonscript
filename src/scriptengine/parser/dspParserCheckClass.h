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
#ifndef _DSPPARSERCHECKCLASS_H_
#define _DSPPARSERCHECKCLASS_H_

// includes

// predefinitions
class dsClass;
class dsEngine;
class dsPackage;
class dspNodeClass;
class dspNodeScript;
class dspNodeClassVariable;
class dspNodeClassFunction;
class dspNodeEnumeration;
class dspNodeNatClassErr;
class dspParserCheck;
class dspParserCheckType;
class dspParserCheckFunction;
class dspParserCheckConstant;
class dspParserCheckInterface;
class dspParserCheckScript;


// class dspParserCheckClass
class dspParserCheckClass{
private:
	dspParserCheckScript *p_Script;
	dspNodeNatClassErr *p_natErrNode;
	dspNodeClass *p_Node;
	dspParserCheckType *p_TypeClass;
	dspParserCheckType *p_TypeParent;
	dspParserCheckType *p_TypeBase;
	dsClass *p_ClassNew;
	dspParserCheckFunction **p_Functions;
	dspParserCheckConstant **p_Constants;
	dspParserCheckInterface **p_Interfaces;
	int p_FuncCount;
	int p_ConstCount;
	int p_IFaceCount;
	bool p_ready;
public:
	// constructor, destructor
	dspParserCheckClass(dspParserCheckScript *Script, dspNodeClass *Node, dspParserCheckType *TypeParent, dspParserCheckType *TypeBase);
	dspParserCheckClass(dsClass *nativeClass);
	~dspParserCheckClass();
	// types
	inline dspParserCheckType *GetTypeClass() const{ return p_TypeClass; }
	inline dspParserCheckType *GetTypeParent() const{ return p_TypeParent; }
	inline dspParserCheckType *GetTypeBase() const{ return p_TypeBase; }
	// management
	inline dspParserCheckScript *GetScript() const{ return p_Script; }
	inline dspNodeClass *GetNode() const{ return p_Node; }
	inline dsClass *GetClassNew() const{ return p_ClassNew; }
	inline bool IsReady() const{ return p_ready; }
	void CreateClass(dspParserCheck *ParserCheck);
	void CreateClassMembers(dspParserCheck *ParserCheck);
	bool CheckForErrors(dspParserCheck *ParserCheck);
	void CheckFunctionCode(dspParserCheck *ParserCheck);
	void CheckConstants(dspParserCheck *ParserCheck);
	dsConstant *CheckConstant(dspParserCheck *ParserCheck, const char *Name);
	void CompileFunctionCode(dspParserCheck *ParserCheck);
	void CompileConstants(dspParserCheck *ParserCheck);
	// information
	bool MatchesClass(const char *Name, dspParserCheckType *TypeParent) const;
private:
	void p_AddFunction(dspParserCheckFunction *Function);
	void p_AddConstant(dspParserCheckConstant *Constant);
	void p_CreateClassVariable(dspParserCheck *ParserCheck, dspNodeClassVariableList *VarNode);
	void p_CreateClassFunction(dspParserCheck *ParserCheck, dspNodeClassFunction *FuncNode);
	bool p_MatchesOp(const char *CheckOp, const char **OpList, int Count);
};

// end of include only once
#endif

