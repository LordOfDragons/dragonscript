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
#ifndef _DSPPARSERCHECKCODE_H_
#define _DSPPARSERCHECKCODE_H_

// predefinitions
class dsClass;
class dsEngine;
class dsVariable;
class dsFunction;
class dsConstant;
class dsSignature;
class dsLocalVariable;
class dspBaseNode;
class dspParserCheck;
class dspParserCheckFunction;
class dspNodeBlock;
class dsuString;


// class dspParserCheckCode
class DS_DLL_EXPORT dspParserCheckCode{
private:
	dspParserCheck *p_ParserCheck;
	dspParserCheckCode *p_Precedor;
	dspParserCheckFunction *p_CheckFunc;
	dsClass *p_OwnerClass;
	bool p_HasReturn, p_HasUnreachCode;
	bool p_CanUseBreak, p_CanUseContinue, p_CanUseRethrow;
	bool p_IsObjConst;
	bool p_SuperCall;
	dsClass *p_TypeClass, *p_TypeObject;
	dsVariable *p_StaClsVar;
	dsLocalVariable **p_LocVars;
	int p_LocVarCount, p_MaxLocVarCount, p_LocVarBase;
	
	dspParserCheckCode *pCCBlock;
	dsLocalVariable **pBlockCVars;
	int pBlockCVarCount;
	int pBlockCVarSize;
	int pBlockCVarBase;
	
public:
	// constructor, destructor
	dspParserCheckCode(dspParserCheck *ParserCheck, dspParserCheckFunction *CheckFunction);
	dspParserCheckCode(dspParserCheck *ParserCheck, dsClass *OwnerClass);
	dspParserCheckCode(dspParserCheckCode *Precedor);
	~dspParserCheckCode();
	// information
	dsEngine *GetEngine() const;
	inline dspParserCheckCode *GetPrecedor() const{ return p_Precedor; }
	inline dsClass *GetOwnerClass() const{ return p_OwnerClass; }
	dsFunction *GetOwnerFunction() const;
	inline dspParserCheck *GetParserCheck() const{ return p_ParserCheck; }
	inline dspParserCheckFunction *GetCheckFunc() const{ return p_CheckFunc; }
	inline bool GetHasReturn() const{ return p_HasReturn; }
	inline bool GetHasUnreachableCode() const{ return p_HasUnreachCode; }
	inline bool GetCanUseBreak() const{ return p_CanUseBreak; }
	inline bool GetCanUseContinue() const{ return p_CanUseContinue; }
	inline bool GetCanUseRethrow() const{ return p_CanUseRethrow; }
	inline bool GetSuperCall() const{ return p_SuperCall; }
	inline dsClass *GetTypeClass() const{ return p_TypeClass; }
	inline dsClass *GetTypeObject() const{ return p_TypeObject; }
	inline dsVariable *GetStaticClassVariable() const{ return p_StaClsVar; }
	inline bool GetIsObjConst() const{ return p_IsObjConst; }
	inline void SetIsObjConst(bool IsConst){ p_IsObjConst = IsConst; }
	inline bool IsClass() const{ return p_TypeClass != NULL; }
	inline bool IsObject() const{ return p_TypeObject != NULL; }
	inline bool IsNothing() const{ return !p_TypeClass && !p_TypeObject; }
	// management
	inline void SetHasReturn(bool HasReturn){ p_HasReturn = HasReturn; }
	inline void SetHasUnreachableCode(bool HasUnreachCode){ p_HasUnreachCode = HasUnreachCode; }
	inline void SetCanUseBreak(bool CanUseBreak){ p_CanUseBreak = CanUseBreak; }
	inline void SetCanUseContinue(bool CanUseContinue){ p_CanUseContinue = CanUseContinue; }
	inline void SetCanUseRethrow(bool CanUseRethrow){ p_CanUseRethrow = CanUseRethrow; }
	inline void SetSuperCall(bool SuperCall){ p_SuperCall = SuperCall; }
	void SetTypeClass(dsClass *Class);
	void SetTypeObject(dsClass *Class, bool IsConst);
	void SetStaticClassVariable( dsVariable *variable );
	void SetTypeNothing();
	void ResetState();
	void CopyState(dspParserCheckCode *CheckCode);
	dsClass *Class(const char *Name);
	
	/** Retrieves the check code for the owner block or NULL. */
	inline dspParserCheckCode *GetCCBlock() const{ return pCCBlock; }
	/** Sets the check code for the block or NULL. */
	void SetCCBlock( dspParserCheckCode *ccblock );
	/** Retrieves the number of block context variables. */
	inline int GetBlockCVarCount() const{ return pBlockCVarCount; }
	/** Retrieves the block context variable with the given index. */
	dsLocalVariable *GetBlockCVarAt( int index ) const;
	/** Retrieves the block context variable with the given index. */
	dsLocalVariable *GetBlockCVarAbsoluteAt( int index, bool onlyLocal ) const;
	/** Retrieves the index of the last block context variable with the given name. */
	int IndexOfBlockCVarNamed( const char *name ) const;
	/** Retrieves the index of the last block context variable with the given name. */
	int AbsoluteIndexOfBlockCVarNamed( const char *name, bool onlyLocal ) const;
	/** Adds a block context variable. */
	void AddBlockCVar( dsLocalVariable *variable );
	/** Removes all block context variables. */
	void RemoveAllBlockCVars();
	/** Retrieves the block context variable base. */
	inline int GetBlockCVarBase() const{ return pBlockCVarBase; }
	
	// checking functions
	dsClass *CheckForClass(dspBaseNode **pNode);
	dsClass *CheckForObject(dspBaseNode **pNode);
	// nodes helper functions
	bool AutoNodeCast(dspBaseNode **pNode, dsClass *FromType, dsClass *ToType);
	bool AutoNodeCastBinOp( dspBaseNode **op1Node, dsClass **op1Type, dspBaseNode **op2Node, dsClass **op2Type );
	dsFunction *FindBestOperator(dsClass *OwnerClass, const char *Op, dsClass *ArgType);
	dsClass *GetUnOpReturnType(dsClass *Type, const char *Op);
	dsClass *GetBinOpReturnType(dsClass *Type, const char *Op/*, dsClass *ArgType*/);
	bool CanAccessVariable(dsClass *ObjectClass, dsClass *CallerClass, dsVariable *Variable);
	bool CanAccessFunction(dsClass *ObjectClass, dsClass *CallerClass, dsFunction *Function);
	bool MatchesOperators(const char *CheckOp, const char **Ops, int Count);
	dsConstant *FindClassConstant(dsClass *OwnerClass, const char *ConstName);
	// local variables
	void ClearLocalVariables();
	inline int GetLocVarCount() const{ return p_LocVarCount; }
	void AddLocalVariable(dsLocalVariable *Variable);
	int FindLocalVariable(const char *Name, bool OnlyLocal) const;
	dsLocalVariable *GetLocalVariable(int Index, bool OnlyLocal) const;
	dsLocalVariable *GetLocalVariableAt( int index ) const;
	inline int GetMaxLocVarCount() const{ return p_MaxLocVarCount; }
	inline int GetLocVarBase() const{ return p_LocVarBase; }
	// error functions
	void ErrorVarNotStatic(dspBaseNode *Node, dsVariable *Variable);
	void ErrorFuncNotStatic(dspBaseNode *Node, dsFunction *Function);
	void ErrorFuncStatic(dspBaseNode *Node, dsFunction *Function);
	void ErrorFuncNotConst(dspBaseNode *Node, dsFunction *Function);
	void ErrorInvClassMemberName(dspBaseNode *Node, dsClass *Class, const char *Name);
	void ErrorInvTypeCast(dspBaseNode *Node, dsClass *FromType, dsClass *ToType);
	void ErrorInvFuncArg(dspBaseNode *Node);
	void ErrorInvFuncSig(dspBaseNode *Node, dsClass *OwnerClass, const char *Name, dsSignature *Sig);
	void ErrorInvOpSig(dspBaseNode *Node, dsClass *OwnerClass, const char *Op, dsClass *Arg);
	void ErrorInvObject(dspBaseNode *Node);
	void ErrorInvClass(dspBaseNode *Node);
	void ErrorInvPrimOp(dspBaseNode *Node, const char *OwnerClass, const char *Op);
	void ErrorNoAccessClsVar(dspBaseNode *Node, dsVariable *Variable);
	void ErrorNoAccessClsFunc(dspBaseNode *Node, dsFunction *Function);
	void ErrorNoMembSelAllowed(dspBaseNode *Node, const char *ClassName);
	void ErrorKeywordInStaticFunc( dspBaseNode *node, dsFunction *func, const char *keyword );
	void ErrorConstObj(dspBaseNode *Node, const char *Op);
	void ErrorLoopConstObj(dspBaseNode *Node);
	void ErrorInvLoopVarType(dspBaseNode *Node);
	void ErrorNoDirectDestrCall(dspBaseNode *Node, dsFunction *Function);
	void ErrorConstrNotOnClass(dspBaseNode *Node, dsFunction *Function);
	void ErrorDiffFuncNameType(dspBaseNode *Node, dsFunction *Function);
	void ErrorReturnOnVoid(dspBaseNode *Node, dsFunction *Function);
	void ErrorNoReturnValue(dspBaseNode *Node, dsFunction *Function);
	void ErrorBreakOutside(dspBaseNode *Node);
	void ErrorContinueOutside(dspBaseNode *Node);
	void ErrorRethrowOutside(dspBaseNode *Node);
	void ErrorNoConstrFunc(dspBaseNode *Node, dsFunction *Function);
	void ErrorAmbigDefConstrFunc(dspBaseNode *Node, const char *OwnerClass);
	void ErrorNoDefConstrFunc(dspBaseNode *Node, const char *OwnerClass);
	void ErrorCaseValNotScalar(dspBaseNode *Node);
	void ErrorDupCaseValue(dspBaseNode *Node);
	void ErrorDupCaseValue(dspBaseNode *Node, int Value);
	void ErrorInvEnumValue(dspBaseNode *Node, const char *ConstName);
	void ErrorInvExceptionType(dspBaseNode *Node, const char *ClassName);
	void ErrorMissingCatch(dspBaseNode *Node);
	void ErrorDupCatch(dspBaseNode *Node, const char *ClassName);
	void ErrorThrowNotException(dspBaseNode *Node);
	void ErrorNoNewOnAbstractClass(dspBaseNode *Node, dsClass *Type);
	// warning functions
	void WarnUnreachableCode(dspBaseNode *Node);
private:
	void p_ErrorFuncSig(dsuString *String, const char *Name, dsSignature *Sig);
	void p_ErrorOpSig(dsuString *String, const char *Op, dsClass *Arg);
	void p_ErrorFuncType(dsuString *String, int FuncType);
};

// end of include only once
#endif

