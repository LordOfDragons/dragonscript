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
#ifndef _DSPPARSERCOMPILECODE_H_
#define _DSPPARSERCOMPILECODE_H_

// includes
#include "../dsDefinitions.h"
#include "../objects/dsByteCode.h"

// predefinitions
class dsClass;
class dsEngine;
class dsVariable;
class dsFunction;
class dsByteCode;
class dsSignature;
class dsLocalVariable;
class dspBaseNode;
class dspParserCheck;
class dspParserCheckFunction;
class dspParserCompileBlock;
class dsuString;
class dsplBlock;

// class dspParserCompileCode
class DS_DLL_EXPORT dspParserCompileCode{
public:
	enum eTargetTypes{ ettElse, ettIfEnd, ettCont, ettNext, ettEnd, ettCase0 };
	enum eOffsetTypes{ eotByte, eotShort, eotInt };
private:
	dspParserCheck *p_ParserCheck;
	dspParserCheckFunction *p_CheckFunc;
	dsClass *p_OwnerClass;
	dsByteCode *p_ByteCode;
	int p_LocVarCount;
	dsplBlock *p_TopBlock, *p_TailBlock;
	int p_BlockCount;
	int *p_Targets;
	int p_TargetCount;
	dsByteCode *p_CurBC;
	int p_TotalBCPos;
	dspParserCompileCode *pCCBlock;
	int pGotoLocVarCount;
	
public:
	// constructor, destructor
	dspParserCompileCode(dspParserCheck *ParserCheck, dspParserCheckFunction *CheckFunction);
	dspParserCompileCode(dspParserCheck *ParserCheck, dsClass *OwnerClass, dsByteCode *ByteCode);
	~dspParserCompileCode();
	// information
	dsEngine *GetEngine() const;
	inline dsClass *GetOwnerClass() const{ return p_OwnerClass; }
	dsFunction *GetOwnerFunction() const;
	inline dsByteCode *GetByteCode() const{ return p_ByteCode; }
	inline dspParserCheck *GetParserCheck() const{ return p_ParserCheck; }
	inline dspParserCheckFunction *GetCheckFunc() const{ return p_CheckFunc; }
	
	/** Retrieves the compile code for the owner block or NULL. */
	inline dspParserCompileCode *GetCCBlock() const{ return pCCBlock; }
	/** Sets the compile code for the block or NULL. */
	void SetCCBlock( dspParserCompileCode *ccblock );
	
	// local variables
	inline int GetLocVarCount() const{ return p_LocVarCount; }
	inline void IncLocVarCount(){ p_LocVarCount++; }
	void DecLocVarCount( int count, int line );
	// bytecode functions
	
	/** \brief Add code. */
	void AddCode( const dsByteCode::sCode &code, int line );
	
	/** \brief Add byte constant code. */
	void AddCode( const dsByteCode::sCodeCByte &code, int line );
	
	/** \brief Add integer constant code. */
	void AddCode( const dsByteCode::sCodeCInt &code, int line );
	
	/** \brief Add floating point constant code. */
	void AddCode( const dsByteCode::sCodeCFloat &code, int line );
	
	/** \brief Add string constant code. */
	void AddCode( const dsByteCode::sCodeCString &code, int line );
	
	/** \brief Add null code. */
	void AddCode( const dsByteCode::sCodeNull &code, int line );
	
	/** \brief Add class variable code. */
	void AddCode( const dsByteCode::sCodeClassVar &code, int line );
	
	/** \brief Add parameter code. */
	void AddCode( const dsByteCode::sCodeParam &code, int line );
	
	/** \brief Add local variable code. */
	void AddCode( const dsByteCode::sCodeLocalVar &code, int line );
	
	/** \brief Add allocate local variable code. */
	void AddCode( const dsByteCode::sCodeAllocLocalVar &code, int line );
	
	/** \brief Add free local variables code. */
	void AddCode( const dsByteCode::sCodeFreeLocalVars &code, int line );
	
	/** \brief Add byte jump code. */
	void AddCode( const dsByteCode::sCodeJumpByte &code, int line );
	
	/** \brief Add short jump code. */
	void AddCode( const dsByteCode::sCodeJumpShort &code, int line );
	
	/** \brief Add long jump code. */
	void AddCode( const dsByteCode::sCodeJumpLong &code, int line );
	
	/** \brief Add byte case jump code. */
	void AddCode( const dsByteCode::sCodeJumpCaseByte &code, int line );
	
	/** \brief Add short case jump code. */
	void AddCode( const dsByteCode::sCodeJumpCaseShort &code, int line );
	
	/** \brief Add long case jump code. */
	void AddCode( const dsByteCode::sCodeJumpCaseLong &code, int line );
	
	/** \brief Add byte exception jump code. */
	void AddCode( const dsByteCode::sCodeJumpExcepByte &code, int line );
	
	/** \brief Add short exception jump code. */
	void AddCode( const dsByteCode::sCodeJumpExcepShort &code, int line );
	
	/** \brief Add long exception jump code. */
	void AddCode( const dsByteCode::sCodeJumpExcepLong &code, int line );
	
	/** \brief Add cast code. */
	void AddCode( const dsByteCode::sCodeCast &code, int line );
	
	/** \brief Add call code. */
	void AddCode( const dsByteCode::sCodeCall &code, int line );
	
	/** \brief Add direct call code. */
	void AddCode( const dsByteCode::sCodeDirectCall &code, int line );
	
	
	void BuildByteCode();
	// jump resolving functions
	inline int GetCurOffset() const{ return p_TotalBCPos; }
	void AddJump( int Type, int JumpCode );
	void AddJumpInt( int Type, int JumpCode, int JumpData );
	void AddJumpPtr( int Type, int JumpCode, void *JumpData );
	void AddTarget(int Type, int BlockBegin, int TargetOffset);
	// special unwinding functions
	inline int GetGotoLocVarCount() const{ return pGotoLocVarCount; }
	void SetGotoLocVarCount( int locVarCount );
	// error functions
	// warning functions
private:
	void p_AdjustBlocksAndTargets(int Position, int Difference);
	void p_ClearBlocksAndTargets();
};

// end of include only once
#endif

