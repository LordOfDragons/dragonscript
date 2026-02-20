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



#ifndef _DSBYTECODE_H_
#define _DSBYTECODE_H_

#include "../dsDefinitions.h"


/**
 * Byte code.
 */
class DS_DLL_EXPORT dsByteCode{
public:
	enum eByteCodes{
		ebcNop, // 0, no operation
		ebcCByte, // 1, const byte value
		ebcCInt, // 2, const int value
		ebcCFlt, // 3, const float value
		ebcCStr, // 4, const string value
		ebcTrue, // 5, const bool value true
		ebcFalse, // 6, const bool value false
		ebcNull, // 7, null object
		ebcThis, // 8, this object
		ebcSuper, // 9, super object
		ebcClsVar, // 10, class variable
		ebcParam, // 11, function parameter
		ebcLocVar, // 12, local variable
		ebcPush, // 13, push value onto stack
		ebcPop, // 14, pop value from stack
		ebcOpInc, // 15, primitive increment
		ebcOpDec, // 16, primitive decrement
		ebcOpMin, // 17, primitive minus
		ebcOpNot, // 18, primitive not
		ebcOpInv, // 19, primitive inverse
		ebcOpMul, // 20, Primitive Multiply Operation
		ebcOpDiv, // 21, Primitive Division Operation
		ebcOpMod, // 22, Primitive Modulus Operation
		ebcOpAdd, // 23, Primitive Add Operation
		ebcOpSub, // 24, Primitive Subtract Operation
		ebcOpLS, // 25, Primitive Left Shift Operation
		ebcOpRS, // 26, Primitive Right Shift Operation
		ebcOpLe, // 27, Primitive Less Comparission
		ebcOpGr, // 28, Primitive Greater Comparission
		ebcOpLEq, // 29, Primitive Less Or Equal Comparission
		ebcOpGEq, // 30, Primitive Greater Or Equal Comparission
		ebcOpEq, // 31, Primitive Equal Comparission
		ebcOpNEq, // 32, Primitive Not Equal Comparission
		ebcOpAnd, // 33, Primitive And Operation
		ebcOpOr, // 34, Primitive Or Operation
		ebcOpXor, // 35, Primitive Xor Operation
		ebcOpAss, // 36, Primitive Assign Operation
		ebcOpMulA, // 37, Primitive Multiply Assign Operation
		ebcOpDivA, // 38, Primitive Division Assign Operation
		ebcOpModA, // 39, Primitive Modulate Assign Operation
		ebcOpAddA, // 40, Primitive Add Assign Operation
		ebcOpSubA, // 41, Primitive Subtract Assign Operation
		ebcOpLSA, // 42, Primitive Left Shift Assign Operation
		ebcOpRSA, // 43, Primitive Right Shift Assign Operation
		ebcOpAndA, // 44, Primitive And Assign Operation
		ebcOpOrA, // 45, Primitive Or Assign Operation
		ebcOpXorA, // 46, Primitive Xor Assign Operation
		ebcPInc, // 47, Post increment
		ebcPDec, // 48, Post decrement
		ebcALV, // 49, Add local variable
		ebcFLV, // 50, Free local variables
		ebcRet, // 51, Return from function
		ebcJMPB, // 52, Unconditional Jump (byte)
		ebcJMPS, // 53, Unconditional Jump (short)
		ebcJMPL, // 54, Unconditional Jump (long)
		ebcJEQB, // 55, Jump If Equal (byte)
		ebcJEQS, // 56, Jump If Equal (short)
		ebcJEQL, // 57, Jump If Equal (long)
		ebcJNEB, // 58, Jump If Not Equal (byte)
		ebcJNES, // 59, Jump If Not Equal (short)
		ebcJNEL, // 60, Jump If Not Equal (long)
		ebcJCEB, // 61, Jump If Case Equal (byte)
		ebcJCES, // 62, Jump If Case Equal (short)
		ebcJCEL, // 63, Jump If Case Equal (long)
		ebcJCESB, // 64, Jump If Case Static Equal (byte)
		ebcJCESS, // 65, Jump If Case Static Equal (short)
		ebcJCESL, // 66, Jump If Case Static Equal (long)
		ebcJOEB, // 67, Jump On Exception Equal (byte)
		ebcJOES, // 68, Jump On Exception Equal (short)
		ebcJOEL, // 69, Jump On Exception Equal (long)
		ebcCast, // 70, Cast Operation
		ebcCaTo, // 71, CastableTo Operation
		ebcTypO, // 72, IsTypeOf Operation
		ebcCall, // 73, Call Function (late binding)
		ebcDCall, // 74, Direct Call Function
		ebcCCall, // 75, Constructor Call Function
		ebcBTB, // 76, Begin Try Block
		ebcETB, // 77, End Try Block
		ebcThrow, // 78, throws an exception
		ebcReThrow, // 79, rethrows the last exception
		ebcBlock, // 80, creates a new block
		ebcCVar, // 81, retrieves a context variable
	};
	
	/**
	 * \brief Base code structure all other structures have to fit.
	 * \details Used by all code not using a separate struct below.
	 */
	struct DS_DLL_EXPORT sCode{
		byte code;
	};
	
	/**
	 * \brief Byte constant code struct.
	 * \details Used by ebcCByte.
	 */
	struct DS_DLL_EXPORT sCodeCByte{
		byte code;
		byte value;
	};
	
	/**
	 * \brief Integer constant code struct.
	 * \details Used by ebcCInt.
	 */
	struct DS_DLL_EXPORT sCodeCInt{
		byte code;
		int value;
	};
	
	/**
	 * \brief Floating point constant code struct.
	 * \details Used by ebcCFlt.
	 */
	struct DS_DLL_EXPORT sCodeCFloat{
		byte code;
		float value;
	};
	
	/**
	 * \brief String constant code struct.
	 * \details Used by ebcCStr.
	 */
	struct DS_DLL_EXPORT sCodeCString{
		byte code;
		int index;
	};
	
	/**
	 * \brief Null code struct.
	 * \details Used by ebcNull.
	 */
	struct DS_DLL_EXPORT sCodeNull{
		byte code;
		dsClass *type;
	};
	
	/**
	 * \brief Class variable code struct.
	 * \details Used by ebcClsVar.
	 */
	struct DS_DLL_EXPORT sCodeClassVar{
		byte code;
		dsVariable *variable;
	};
	
	/**
	 * \brief Function parameter code struct.
	 * \details Used by ebcParam.
	 */
	struct DS_DLL_EXPORT sCodeParam{
		byte code;
		byte index;
	};
	
	/**
	 * \brief Local variable code struct.
	 * \details Used by ebcLocVar, ebcCVar.
	 */
	struct DS_DLL_EXPORT sCodeLocalVar{
		byte code;
		short index;
	};
	
	/**
	 * \brief Allocate local variable code struct.
	 * \details Used by ebcALV.
	 */
	struct DS_DLL_EXPORT sCodeAllocLocalVar{
		byte code;
		dsClass *type;
	};
	
	/**
	 * \brief Free local variables code struct.
	 * \details Used by ebcFLV.
	 */
	struct DS_DLL_EXPORT sCodeFreeLocalVars{
		byte code;
		short count;
	};
	
	/**
	 * \brief Jump short distance code struct.
	 * \details Used by ebcJMPB, ebcJEQB, ebcJNEB.
	 */
	struct DS_DLL_EXPORT sCodeJumpByte{
		byte code;
		signed char offset;
	};
	
	/**
	 * \brief Jump middle distance code struct.
	 * \details Used by ebcJMPS, ebcJEQS, ebcJNES.
	 */
	struct DS_DLL_EXPORT sCodeJumpShort{
		byte code;
		short offset;
	};
	
	/**
	 * \brief Jump long distance code struct.
	 * \details Used by ebcJMPL, ebcJEQL, ebcJNEL.
	 */
	struct DS_DLL_EXPORT sCodeJumpLong{
		byte code;
		int offset;
	};
	
	/**
	 * \brief Jump case short distance code struct.
	 * \details Used by ebcJCEB.
	 */
	struct DS_DLL_EXPORT sCodeJumpCaseByte{
		byte code;
		signed char offset;
		int caseValue;
	};
	
	/**
	 * \brief Jump case medium distance code struct.
	 * \details Used by ebcJCES.
	 */
	struct DS_DLL_EXPORT sCodeJumpCaseShort{
		byte code;
		short offset;
		int caseValue;
	};
	
	/**
	 * \brief Jump case long distance code struct.
	 * \details Used by ebcJCEL.
	 */
	struct DS_DLL_EXPORT sCodeJumpCaseLong{
		byte code;
		int offset;
		int caseValue;
	};
	
	/**
	 * \brief Jump case static short distance code struct.
	 * \details Used by ebcJCESB.
	 */
	struct DS_DLL_EXPORT sCodeJumpCaseStaticByte{
		byte code;
		signed char offset;
		dsVariable *caseValue;
	};
	
	/**
	 * \brief Jump case static medium distance code struct.
	 * \details Used by ebcJCES.
	 */
	struct DS_DLL_EXPORT sCodeJumpCaseStaticShort{
		byte code;
		short offset;
		dsVariable *caseValue;
	};
	
	/**
	 * \brief Jump case static long distance code struct.
	 * \details Used by ebcJCEL.
	 */
	struct DS_DLL_EXPORT sCodeJumpCaseStaticLong{
		byte code;
		int offset;
		dsVariable *caseValue;
	};
	
	/**
	 * \brief Jump exception short distance code struct.
	 * \details Used by ebcJOEB.
	 */
	struct DS_DLL_EXPORT sCodeJumpExcepByte{
		byte code;
		byte offset;
		dsClass *type;
	};
	
	/**
	 * \brief Jump exception medium distance code struct.
	 * \details Used by ebcJOES.
	 */
	struct DS_DLL_EXPORT sCodeJumpExcepShort{
		byte code;
		short offset;
		dsClass *type;
	};
	
	/**
	 * \brief Jump exception long distance code struct.
	 * \details Used by ebcJOEL.
	 */
	struct DS_DLL_EXPORT sCodeJumpExcepLong{
		byte code;
		int offset;
		dsClass *type;
	};
	
	/**
	 * \brief Type casting code struct.
	 * \details Used by ebcCast, ebcCaTo, ebcTypO.
	 */
	struct DS_DLL_EXPORT sCodeCast{
		byte code;
		dsClass *type;
	};
	
	/**
	 * \brief Function call code struct.
	 * \details Used by ebcCall.
	 */
	struct DS_DLL_EXPORT sCodeCall{
		byte code;
		int index;
	};
	
	/**
	 * \brief Direct, constructor or block function call code struct.
	 * \details Used by ebcDCall, ebcCCall, ebcBlock.
	 */
	struct DS_DLL_EXPORT sCodeDirectCall{
		byte code;
		dsFunction *function;
	};
	
	
	
private:
	struct sDebugInfo{
		int line;
		int cp;
	};
	
private:
	byte *p_Data;
	int p_Length;
	
	// debugging
	sDebugInfo *p_DebugInfos;
	int p_DebugCount, p_DebugSize;
	
public:
	// constructor, destructor
	dsByteCode();
	~dsByteCode();
	
	// management
	inline int GetLength() const{ return p_Length; }
	inline void *GetPointer() const{ return p_Data; }
	
	/** \brief Clear byte code. */
	void Clear();
	
	/** \brief Add code. */
	void AddCode( const sCode &code );
	
	/** \brief Add byte constant code. */
	void AddCode( const sCodeCByte &code );
	
	/** \brief Add integer constant code. */
	void AddCode( const sCodeCInt &code );
	
	/** \brief Add floating point constant code. */
	void AddCode( const sCodeCFloat &code );
	
	/** \brief Add string constant code. */
	void AddCode( const sCodeCString &code );
	
	/** \brief Add null code. */
	void AddCode( const sCodeNull &code );
	
	/** \brief Add class variable code. */
	void AddCode( const sCodeClassVar &code );
	
	/** \brief Add parameter code. */
	void AddCode( const sCodeParam &code );
	
	/** \brief Add local variable code. */
	void AddCode( const sCodeLocalVar &code );
	
	/** \brief Add allocate local variable code. */
	void AddCode( const sCodeAllocLocalVar &code );
	
	/** \brief Add free local variables code. */
	void AddCode( const sCodeFreeLocalVars &code );
	
	/** \brief Add byte jump code. */
	void AddCode( const sCodeJumpByte &code );
	
	/** \brief Add short jump code. */
	void AddCode( const sCodeJumpShort &code );
	
	/** \brief Add long jump code. */
	void AddCode( const sCodeJumpLong &code );
	
	/** \brief Add byte case jump code. */
	void AddCode( const sCodeJumpCaseByte &code );
	
	/** \brief Add short case jump code. */
	void AddCode( const sCodeJumpCaseShort &code );
	
	/** \brief Add long case jump code. */
	void AddCode( const sCodeJumpCaseLong &code );
	
	/** \brief Add byte case static jump code. */
	void AddCode( const sCodeJumpCaseStaticByte &code );
	
	/** \brief Add short case static jump code. */
	void AddCode( const sCodeJumpCaseStaticShort &code );
	
	/** \brief Add long case static jump code. */
	void AddCode( const sCodeJumpCaseStaticLong &code );
	
	/** \brief Add byte exception jump code. */
	void AddCode( const sCodeJumpExcepByte &code );
	
	/** \brief Add short exception jump code. */
	void AddCode( const sCodeJumpExcepShort &code );
	
	/** \brief Add long exception jump code. */
	void AddCode( const sCodeJumpExcepLong &code );
	
	/** \brief Add cast code. */
	void AddCode( const sCodeCast &code );
	
	/** \brief Add call code. */
	void AddCode( const sCodeCall &code );
	
	/** \brief Add direct call code. */
	void AddCode( const sCodeDirectCall &code );
	
	/** \brief Add byte code. */
	void AddByteCode( dsByteCode *byteCode );
	
	// debugging
	void AddDebugLine(int line);
	int GetDebugLine(int cp);
	
	#ifndef __NODBGPRINTF__
	void DebugPrint();
	#endif
	
private:
	void p_AddData(void *Data, int Size);
	void p_AddData( const void *data, int size, int advance );
};

/**
 * \brief Base code structure stride all other structures have to fit.
 * \details Used by all code not using a separate struct below.
 */
constexpr size_t DS_BCSTRIDE_CODE = dsByteCodeStride<dsByteCode::sCode>();

/**
 * \brief Byte constant code stride.
 * \details Used by ebcCByte.
 */
constexpr size_t DS_BCSTRIDE_CBYTE = dsByteCodeStride<dsByteCode::sCodeCByte>();

/**
 * \brief Integer constant code stride.
 * \details Used by ebcCInt.
 */
constexpr size_t DS_BCSTRIDE_CINT = dsByteCodeStride<dsByteCode::sCodeCInt>();

/**
 * \brief Floating point constant code stride.
 * \details Used by ebcCFlt.
 */
constexpr size_t DS_BCSTRIDE_CFLOAT = dsByteCodeStride<dsByteCode::sCodeCFloat>();

/**
 * \brief String constant code stride.
 * \details Used by ebcCStr.
 */
constexpr size_t DS_BCSTRIDE_CSTRING = dsByteCodeStride<dsByteCode::sCodeCString>();

/**
 * \brief Null code stride.
 * \details Used by ebcNull.
 */
constexpr size_t DS_BCSTRIDE_NULL = dsByteCodeStride<dsByteCode::sCodeNull>();

/**
 * \brief Class variable code stride.
 * \details Used by ebcClsVar.
 */
constexpr size_t DS_BCSTRIDE_CLASSVAR = dsByteCodeStride<dsByteCode::sCodeClassVar>();

/**
 * \brief Function parameter code stride.
 * \details Used by ebcParam.
 */
constexpr size_t DS_BCSTRIDE_PARAM = dsByteCodeStride<dsByteCode::sCodeParam>();

/**
 * \brief Local variable code stride.
 * \details Used by ebcLocVar, ebcCVar.
 */
constexpr size_t DS_BCSTRIDE_LOCVAR = dsByteCodeStride<dsByteCode::sCodeLocalVar>();

/**
 * \brief Allocate local variable code stride.
 * \details Used by ebcALV.
 */
constexpr size_t DS_BCSTRIDE_ALV = dsByteCodeStride<dsByteCode::sCodeAllocLocalVar>();

/**
 * \brief Free local variables code stride.
 * \details Used by ebcFLV.
 */
constexpr size_t DS_BCSTRIDE_FLV = dsByteCodeStride<dsByteCode::sCodeFreeLocalVars>();

/**
 * \brief Jump short distance code struct.
 * \details Used by ebcJMPB, ebcJEQB, ebcJNEB.
 */
constexpr size_t DS_BCSTRIDE_JBYTE = dsByteCodeStride<dsByteCode::sCodeJumpByte>();

/**
 * \brief Jump middle distance code stride.
 * \details Used by ebcJMPS, ebcJEQS, ebcJNES.
 */
constexpr size_t DS_BCSTRIDE_JSHORT = dsByteCodeStride<dsByteCode::sCodeJumpShort>();

/**
 * \brief Jump long distance code struct.
 * \details Used by ebcJMPL, ebcJEQL, ebcJNEL.
 */
constexpr size_t DS_BCSTRIDE_JLONG = dsByteCodeStride<dsByteCode::sCodeJumpLong>();

/**
 * \brief Jump case short distance code stride.
 * \details Used by ebcJCEB.
 */
constexpr size_t DS_BCSTRIDE_JCBYTE = dsByteCodeStride<dsByteCode::sCodeJumpCaseByte>();

/**
 * \brief Jump case medium distance code stride.
 * \details Used by ebcJCES.
 */
constexpr size_t DS_BCSTRIDE_JCSHORT = dsByteCodeStride<dsByteCode::sCodeJumpCaseShort>();

/**
 * \brief Jump case long distance code stride.
 * \details Used by ebcJCEL.
 */
constexpr size_t DS_BCSTRIDE_JCLONG = dsByteCodeStride<dsByteCode::sCodeJumpCaseLong>();

/**
 * \brief Jump case static short distance code stride.
 * \details Used by ebcJCESB.
 */
constexpr size_t DS_BCSTRIDE_JCSBYTE = dsByteCodeStride<dsByteCode::sCodeJumpCaseStaticByte>();

/**
 * \brief Jump case static medium distance code stride.
 * \details Used by ebcJCESS.
 */
constexpr size_t DS_BCSTRIDE_JCSSHORT = dsByteCodeStride<dsByteCode::sCodeJumpCaseStaticShort>();

/**
 * \brief Jump case static long distance code stride.
 * \details Used by ebcJCEL.
 */
constexpr size_t DS_BCSTRIDE_JCSLONG = dsByteCodeStride<dsByteCode::sCodeJumpCaseStaticLong>();

/**
 * \brief Jump exception short distance code stride.
 * \details Used by ebcJOEB.
 */
constexpr size_t DS_BCSTRIDE_JEBYTE = dsByteCodeStride<dsByteCode::sCodeJumpExcepByte>();

/**
 * \brief Jump exception medium distance code stride.
 * \details Used by ebcJOES.
 */
constexpr size_t DS_BCSTRIDE_JESHORT = dsByteCodeStride<dsByteCode::sCodeJumpExcepShort>();

/**
 * \brief Jump exception long distance code stride.
 * \details Used by ebcJOEL.
 */
constexpr size_t DS_BCSTRIDE_JELONG = dsByteCodeStride<dsByteCode::sCodeJumpExcepLong>();

/**
 * \brief Type casting code stride.
 * \details Used by ebcCast, ebcCaTo, ebcTypO.
 */
constexpr size_t DS_BCSTRIDE_CAST = dsByteCodeStride<dsByteCode::sCodeCast>();

/**
 * \brief Function call code stride.
 * \details Used by ebcCall.
 */
constexpr size_t DS_BCSTRIDE_CALL = dsByteCodeStride<dsByteCode::sCodeCall>();

/**
 * \brief Direct, constructor or block function call code stride.
 * \details Used by ebcDCall, ebcCCall, ebcBlock.
 */
constexpr size_t DS_BCSTRIDE_DCALL = dsByteCodeStride<dsByteCode::sCodeDirectCall>();


#endif
