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



// includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../dragonscript_config.h"
#include "dspNodes.h"
#include "dspParser.h"
#include "dspParserCheck.h"
#include "dspParserCompileCode.h"
#include "dspParserCheckFunction.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../utils/dsuString.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsFunction.h"

// local classes
//////////////////
class dsplBlock{
private:
	int p_Type;
	int p_TargetID;
	int p_JumpCode; // JMPB, JEQB, JNEB or ACH
	int p_JumpSize; // 1(byte), 2(short), 3(long)
	int p_Offset; // from where originates the jump
	int p_JumpDataInt; // additional jump data for certain jumps
	void *p_JumpDataPtr;
	dsByteCode *p_ByteCode;
	dsplBlock *p_NextBlock;
public:
	// constructor, destructor
	dsplBlock(int Type, int JumpCode, int Offset){
		if(!(p_ByteCode = new dsByteCode)) DSTHROW(dueOutOfMemory);
		p_Type = Type; p_JumpCode = JumpCode; p_JumpSize = 1;
		p_Offset = Offset; p_NextBlock = NULL; p_TargetID = -1;
		p_JumpDataInt = 0;
		p_JumpDataPtr = NULL;
	}
	dsplBlock(int Type, int JumpCode, int JumpData, int Offset){
		if(!(p_ByteCode = new dsByteCode)) DSTHROW(dueOutOfMemory);
		p_Type = Type; p_JumpCode = JumpCode; p_JumpSize = 1;
		p_Offset = Offset; p_NextBlock = NULL; p_TargetID = -1;
		p_JumpDataInt = JumpData;
		p_JumpDataPtr = NULL;
	}
	dsplBlock(int Type, int JumpCode, void *JumpData, int Offset){
		if(!(p_ByteCode = new dsByteCode)) DSTHROW(dueOutOfMemory);
		p_Type = Type; p_JumpCode = JumpCode; p_JumpSize = 1;
		p_Offset = Offset; p_NextBlock = NULL; p_TargetID = -1;
		p_JumpDataInt = 0;
		p_JumpDataPtr = JumpData;
	}
	~dsplBlock(){ delete p_ByteCode; }
	// management
	inline int GetType() const{ return p_Type; }
	inline int GetJumpCode() const{ return p_JumpCode; }
	inline int GetJumpSize() const{ return p_JumpSize; }
	inline int GetOffset() const{ return p_Offset; }
	inline int GetTargetID() const{ return p_TargetID; }
	inline int GetJumpDataInt() const{ return p_JumpDataInt; }
	inline void *GetJumpDataPtr() const{ return p_JumpDataPtr; }
	inline dsByteCode *GetByteCode() const{ return p_ByteCode; }
	inline dsplBlock *GetNextBlock() const{ return p_NextBlock; }
	inline void SetJumpSize(int JumpSize){ p_JumpSize = JumpSize; }
	inline void SetTargetID(int TargetID){ p_TargetID = TargetID; }
	inline void SetNextBlock(dsplBlock *NextBlock){ p_NextBlock = NextBlock; }
	inline void AdjustOffset(int Diff){ p_Offset += Diff; }
};


// dspParserCompileCode
/////////////////////////

// Constructor, destructor
////////////////////////////

dspParserCompileCode::dspParserCompileCode( dspParserCheck *parseCheck, dspParserCheckFunction *checkFunction ){
	if( ! parseCheck || ! checkFunction ) DSTHROW( dueInvalidParam );
	
	p_ParserCheck = parseCheck;
	p_CheckFunc = checkFunction;
	p_OwnerClass = checkFunction->GetFunction()->GetOwnerClass();
	p_ByteCode = checkFunction->GetFunction()->GetByteCode();
	p_LocVarCount = 0;
	p_TopBlock = NULL;
	p_TailBlock = NULL;
	p_BlockCount = 0;
	p_Targets = NULL;
	p_TargetCount = 0;
	p_CurBC = p_ByteCode;
	p_TotalBCPos = 0;
	pCCBlock = NULL;
	pGotoLocVarCount = -1;
}

dspParserCompileCode::dspParserCompileCode( dspParserCheck *parseCheck, dsClass *ownerClass, dsByteCode *byteCode ){
	if( ! parseCheck || ! ownerClass || ! byteCode ) DSTHROW( dueInvalidParam );
	
	p_ParserCheck = parseCheck;
	p_CheckFunc = NULL;
	p_OwnerClass = ownerClass;
	p_ByteCode = byteCode;
	p_LocVarCount = 0;
	p_TopBlock = NULL;
	p_TailBlock = NULL;
	p_BlockCount = 0;
	p_Targets = NULL;
	p_TargetCount = 0;
	p_CurBC = p_ByteCode;
	p_TotalBCPos = 0;
	pCCBlock = NULL;
	pGotoLocVarCount = -1;
}

dspParserCompileCode::~dspParserCompileCode(){
	p_ClearBlocksAndTargets();
}



// information
dsEngine *dspParserCompileCode::GetEngine() const{
	return p_ParserCheck->GetEngine();
}
dsFunction *dspParserCompileCode::GetOwnerFunction() const{
	if(!p_CheckFunc) DSTHROW(dueInvalidParam);
	return p_CheckFunc->GetFunction();
}

void dspParserCompileCode::SetCCBlock( dspParserCompileCode *ccblock ){
	pCCBlock = ccblock;
}

// local variables
void dspParserCompileCode::DecLocVarCount( int count, int line ){
	if( count < 0 || count > p_LocVarCount ){
		DSTHROW( dueOutOfBoundary );
	}
	
	if( count == p_LocVarCount ){
		return;
	}
	
	dsByteCode::sCodeFreeLocalVars code;
	code.code = dsByteCode::ebcFLV;
	code.count = ( short )( p_LocVarCount - count );
	AddCode( code, line );
	p_LocVarCount = count;
}



// bytecode functions
void dspParserCompileCode::AddCode( const dsByteCode::sCode &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CODE;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add byte constant code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeCByte &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CBYTE;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add integer constant code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeCInt &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CINT;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add floating point constant code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeCFloat &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CFLOAT;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add string constant code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeCString &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CSTRING;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add null code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeNull &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_NULL;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add class variable code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeClassVar &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CLASSVAR;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add parameter code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeParam &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_PARAM;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add local variable code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeLocalVar &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_LOCVAR;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add allocate local variable code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeAllocLocalVar &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_ALV;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add free local variables code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeFreeLocalVars &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_FLV;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add byte jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpByte &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JBYTE;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add short jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpShort &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JSHORT;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add long jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpLong &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JLONG;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add byte case jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpCaseByte &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JCBYTE;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add short case jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpCaseShort &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JCSHORT;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add long case jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpCaseLong &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JCLONG;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add byte exception jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpExcepByte &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JEBYTE;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add short exception jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpExcepShort &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JESHORT;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add long exception jump code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeJumpExcepLong &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_JELONG;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add cast code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeCast &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CAST;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add call code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeCall &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_CALL;
	p_CurBC->AddDebugLine( line );
}

/** \brief Add direct call code. */
void dspParserCompileCode::AddCode( const dsByteCode::sCodeDirectCall &code, int line ){
	p_CurBC->AddCode( code );
	p_TotalBCPos += DS_BCSTRIDE_DCALL;
	p_CurBC->AddDebugLine( line );
}



void dspParserCompileCode::BuildByteCode(){
	dsplBlock *vCurBlock = p_TopBlock;
	int vJumpSizeDiff = 0, vJumpDist, vCorrections;
	// check if all jumps have been resolved
	// this has to be true all time otherwise we a serious bug in here
	while(vCurBlock){
		if(vCurBlock->GetTargetID() == -1) DSTHROW(dueInvalidParam);
		vCurBlock = vCurBlock->GetNextBlock();
	}
	// check if all jump sizes are matching the jump values inside
	do{
		vCorrections = 0;
		vCurBlock = p_TopBlock;
		while(vCurBlock){
			vJumpSizeDiff = 0;
			vJumpDist = p_Targets[vCurBlock->GetTargetID()] - vCurBlock->GetOffset();
			
			// check for wrong sizes
			if( vJumpDist < -32768 || vJumpDist > 32767 ){
				if( vCurBlock->GetJumpSize() == 1 ){
					vJumpSizeDiff = DS_BCSTRIDE_JLONG - DS_BCSTRIDE_JBYTE;
					vCurBlock->SetJumpSize( 4 );
					
				}else if( vCurBlock->GetJumpSize() == 2 ){
					vJumpSizeDiff = DS_BCSTRIDE_JLONG - DS_BCSTRIDE_JSHORT;
					vCurBlock->SetJumpSize( 4 );
				}
				
			}else if( vJumpDist < -128 || vJumpDist > 127 ){
				if( vCurBlock->GetJumpSize() == 1 ){
					vJumpSizeDiff = DS_BCSTRIDE_JSHORT - DS_BCSTRIDE_JBYTE;
					vCurBlock->SetJumpSize( 2 );
				}
			}
			
			// if a wrong size has been found adjust all blocks/targets
			if(vJumpSizeDiff > 0){
				p_AdjustBlocksAndTargets(vCurBlock->GetOffset(), vJumpSizeDiff);
				vCorrections++;
			}
			// next entry
			vCurBlock = vCurBlock->GetNextBlock();
		}
	}while(vCorrections > 0);
	// build bytecode
	vCurBlock = p_TopBlock;
	while(vCurBlock){
		vJumpDist = p_Targets[vCurBlock->GetTargetID()] - vCurBlock->GetOffset();
		
		// add jump
		if( vCurBlock->GetJumpSize() == 1 ){
			switch( vCurBlock->GetJumpCode() ){
			case dsByteCode::ebcJMPB:
			case dsByteCode::ebcJEQB:
			case dsByteCode::ebcJNEB:{
				dsByteCode::sCodeJumpByte code;
				code.code = vCurBlock->GetJumpCode();
				code.offset = ( signed char )vJumpDist;
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJCEB:{
				dsByteCode::sCodeJumpCaseByte code;
				code.code = vCurBlock->GetJumpCode();
				code.offset = ( signed char )vJumpDist;
				code.caseValue = vCurBlock->GetJumpDataInt();
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJCESB:{
				dsByteCode::sCodeJumpCaseStaticByte code;
				code.code = vCurBlock->GetJumpCode();
				code.offset = ( signed char )vJumpDist;
				code.caseValue = ( dsVariable* )vCurBlock->GetJumpDataPtr();
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJOEB:{
				dsByteCode::sCodeJumpExcepByte code;
				code.code = vCurBlock->GetJumpCode();
				code.offset = ( signed char )vJumpDist;
				code.type = ( dsClass* )vCurBlock->GetJumpDataPtr();
				p_ByteCode->AddCode( code );
				}break;
				
			default:
				DSTHROW( dueInvalidParam );
			}
			
		}else if( vCurBlock->GetJumpSize() == 2 ){
			switch( vCurBlock->GetJumpCode() ){
			case dsByteCode::ebcJMPB:
			case dsByteCode::ebcJEQB:
			case dsByteCode::ebcJNEB:{
				dsByteCode::sCodeJumpShort code;
				code.code = vCurBlock->GetJumpCode() + 1;
				code.offset = ( short )vJumpDist;
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJCEB:{
				dsByteCode::sCodeJumpCaseShort code;
				code.code = vCurBlock->GetJumpCode() + 1;
				code.offset = ( short )vJumpDist;
				code.caseValue = vCurBlock->GetJumpDataInt();
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJCESB:{
				dsByteCode::sCodeJumpCaseStaticShort code;
				code.code = vCurBlock->GetJumpCode() + 1;
				code.offset = ( short )vJumpDist;
				code.caseValue = ( dsVariable* )vCurBlock->GetJumpDataPtr();
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJOEB:{
				dsByteCode::sCodeJumpExcepShort code;
				code.code = vCurBlock->GetJumpCode() + 1;
				code.offset = ( short )vJumpDist;
				code.type = ( dsClass* )vCurBlock->GetJumpDataPtr();
				p_ByteCode->AddCode( code );
				}break;
				
			default:
				DSTHROW( dueInvalidParam );
			}
			
		}else{
			switch( vCurBlock->GetJumpCode() ){
			case dsByteCode::ebcJMPB:
			case dsByteCode::ebcJEQB:
			case dsByteCode::ebcJNEB:{
				dsByteCode::sCodeJumpLong code;
				code.code = vCurBlock->GetJumpCode() + 2;
				code.offset = vJumpDist;
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJCEB:{
				dsByteCode::sCodeJumpCaseLong code;
				code.code = vCurBlock->GetJumpCode() + 2;
				code.offset = vJumpDist;
				code.caseValue = vCurBlock->GetJumpDataInt();
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJCESB:{
				dsByteCode::sCodeJumpCaseStaticLong code;
				code.code = vCurBlock->GetJumpCode() + 2;
				code.offset = vJumpDist;
				code.caseValue = ( dsVariable* )vCurBlock->GetJumpDataPtr();
				p_ByteCode->AddCode( code );
				}break;
				
			case dsByteCode::ebcJOEB:{
				dsByteCode::sCodeJumpExcepLong code;
				code.code = vCurBlock->GetJumpCode() + 2;
				code.offset = vJumpDist;
				code.type = ( dsClass* )vCurBlock->GetJumpDataPtr();
				p_ByteCode->AddCode( code );
				}break;
				
			default:
				DSTHROW( dueInvalidParam );
			}
		}
		
		// add the rest of the bycode container and go on to the next block
		p_ByteCode->AddByteCode( vCurBlock->GetByteCode() );
		vCurBlock = vCurBlock->GetNextBlock();
	}
	// free all blocks and targets
	p_ClearBlocksAndTargets();
}



// Jump Resolving Functions
/////////////////////////////

void dspParserCompileCode::AddJump( int Type, int JumpCode ){
	if( (JumpCode != dsByteCode::ebcJMPB) && (JumpCode != dsByteCode::ebcJEQB) && (JumpCode != dsByteCode::ebcJNEB) ) DSTHROW(dueInvalidParam);
	int vJumpBegin = p_TotalBCPos + DS_BCSTRIDE_JBYTE;
	dsplBlock *vNewBlock = new dsplBlock(Type, JumpCode, vJumpBegin);
	if(!vNewBlock) DSTHROW(dueOutOfMemory);
	if(p_TailBlock){
		p_TailBlock->SetNextBlock(vNewBlock);
	}else{
		p_TopBlock = vNewBlock;
	}
	p_TailBlock = vNewBlock;
	p_BlockCount++;
	p_CurBC = vNewBlock->GetByteCode();
	p_TotalBCPos += DS_BCSTRIDE_JBYTE;
}

void dspParserCompileCode::AddJumpInt( int Type, int JumpCode, int JumpData ){
	if( JumpCode != dsByteCode::ebcJCEB ) DSTHROW( dueInvalidParam );
	int vJumpBegin = p_TotalBCPos + DS_BCSTRIDE_JCBYTE;
	dsplBlock *vNewBlock = new dsplBlock( Type, JumpCode, JumpData, vJumpBegin );
	
	if( ! vNewBlock ) DSTHROW( dueOutOfMemory );
	
	if( p_TailBlock ){
		p_TailBlock->SetNextBlock( vNewBlock );
	}else{
		p_TopBlock = vNewBlock;
	}
	p_TailBlock = vNewBlock;
	p_BlockCount++;
	p_CurBC = vNewBlock->GetByteCode();
	p_TotalBCPos += DS_BCSTRIDE_JCBYTE;
}

void dspParserCompileCode::AddJumpPtr( int Type, int JumpCode, void *JumpData ){
	if( JumpCode != dsByteCode::ebcJOEB && JumpCode != dsByteCode::ebcJCESB ) DSTHROW( dueInvalidParam );
	int vJumpBegin = p_TotalBCPos + DS_BCSTRIDE_JEBYTE;
	dsplBlock *vNewBlock = new dsplBlock( Type, JumpCode, JumpData, vJumpBegin );
	
	if( ! vNewBlock ) DSTHROW( dueOutOfMemory );
	
	if( p_TailBlock ){
		p_TailBlock->SetNextBlock( vNewBlock );
	}else{
		p_TopBlock = vNewBlock;
	}
	p_TailBlock = vNewBlock;
	p_BlockCount++;
	p_CurBC = vNewBlock->GetByteCode();
	p_TotalBCPos += DS_BCSTRIDE_JEBYTE;
}

void dspParserCompileCode::AddTarget( int Type, int BlockBegin, int TargetOffset ){
	// add target
	int *vNewArray = new int[p_TargetCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	for(int i=0; i<p_TargetCount; i++) vNewArray[i] = p_Targets[i];
	vNewArray[p_TargetCount] = TargetOffset;
	if(p_Targets) delete [] p_Targets;
	p_Targets = vNewArray;
	p_TargetCount++;
	// find all matching jumps and assign this target to them
	dsplBlock *vCurBlock = p_TopBlock;
	while(vCurBlock){
		if(vCurBlock->GetOffset() > BlockBegin){
			if(vCurBlock->GetType() == Type){
				if(vCurBlock->GetTargetID() == -1){
					vCurBlock->SetTargetID(p_TargetCount-1);
				}
			}
		}
		vCurBlock = vCurBlock->GetNextBlock();
	}
}



// Special unwinding functions
////////////////////////////////

void dspParserCompileCode::SetGotoLocVarCount( int locVarCount ){
	pGotoLocVarCount = locVarCount;
}



// error functions
// warning functions
// private functions
void dspParserCompileCode::p_AdjustBlocksAndTargets(int Position, int Difference){
	// adjust blocks
	dsplBlock *vCurBlock = p_TopBlock;
	while(vCurBlock){
		if(vCurBlock->GetOffset() >= Position) vCurBlock->AdjustOffset(Difference);
		vCurBlock = vCurBlock->GetNextBlock();
	}
	// adjust targets
	for(int i=0; i<p_TargetCount; i++){
		if(p_Targets[i] >= Position) p_Targets[i] += Difference;
	}
}
void dspParserCompileCode::p_ClearBlocksAndTargets(){
	if(p_TopBlock){
		dsplBlock *vNextBlock;
		while(p_TopBlock){
			vNextBlock = p_TopBlock->GetNextBlock();
			delete p_TopBlock; p_TopBlock = vNextBlock;
		}
		p_TopBlock = NULL;
		p_TailBlock = NULL;
		p_BlockCount = 0;
		p_CurBC = p_ByteCode;
		p_TotalBCPos = p_ByteCode->GetLength();
	}
	if(p_Targets){
		delete [] p_Targets;
		p_Targets = NULL;
		p_TargetCount = 0;
	}
}

