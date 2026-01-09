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
#ifndef _DSEXECUTIONCONTEXT_H_
#define _DSEXECUTIONCONTEXT_H_

// predefinitions
class dsEngine;
class dsClass;
class dsRealObject;
class dsValue;
class dsFunction;
class dsClassString;
class dsClassBlock;
class dsClassException;
class dsuArray;
class dsSignature;


// includes
#include "exceptions.h"

//definitions

// class dsRunTime
class DS_DLL_EXPORT dsRunTime{
friend class dsClass;
friend class dsFunction;
friend class dsVariable;
friend class dsClassArray;
friend class dsClassBlock;
private:
	// engine informations
	dsEngine *p_Engine;
	dsClass *p_ClsVoid;
	dsClass *p_ClsByte;
	dsClass *p_ClsBool;
	dsClass *p_ClsInt;
	dsClass *p_ClsFloat;
	dsClass *p_ClsObj;
	dsClass *p_ClsNull;
	dsClassString *p_ClsStr;
	dsClassBlock *p_ClsBlock;
	dsClassException *p_ClsExcep;
	// exception tracing
	dsValue *p_RaisedException;
	bool p_Locked;
	// stack
	dsValue *p_Stack;
	int p_StackSize;
	int p_StackTop;
	dsValue *p_retVal;
public:
	// constructor, destructor
	dsRunTime(dsEngine *Engine);
	~dsRunTime();
	// management
	inline dsEngine *GetEngine() const{ return p_Engine; }
	
	/** \brief Clean up freeing all object references. */
	void CleanUp();
	
	// stack management
	inline int GetStackSize() const{ return p_StackTop; }
	inline bool CanPopValue() const{ return p_StackTop > 0; }
	void PushValue(dsValue *Value);
	void PushObject(dsRealObject *Object, dsClass *Type);
	void PushObject(dsRealObject *Object);
	void PushByte(byte Value);
	void PushBool(bool Value);
	void PushInt(int Value);
	void PushFloat(float Value);
	void PushString(const char *Value);
	void PushReturnValue();
	void PopValue(dsValue *Value);
	dsValue *GetValue(int Index) const;
	void RemoveValues(int Count);
	void RemoveValuesLeaveTop(int Count);
	void ClearStack();
	int SaveStackPointer();
	void RestoreStackPointer(int savePointer);
	// weak references
	void AddWeakRef(dsRealObject *object);
	void RemoveWeakRef(dsRealObject *object);
	// value management
	dsValue *CreateValue();
	dsValue *CreateValue(dsClass *Type);
	void FreeValue(dsValue *Value);
	void CopyValue(dsValue *From, dsValue *To);
	void MoveValue(dsValue *From, dsValue *To);
	void ClearValue(dsValue *Value);
	void SetByte(dsValue *Value, byte Data);
	void SetBool(dsValue *Value, bool Data);
	void SetInt(dsValue *Value, int Data);
	void SetFloat(dsValue *Value, float Data);
	void SetString(dsValue *Value, const char *Data);
	void SetObject(dsValue *Value, dsRealObject *RealObject);
	void SetNull(dsValue *Value, dsClass *Type);
	bool EqualValue(dsValue *Val1, dsValue *Val2);
	void CreateObject(dsValue *Value, dsClass *Type, int ArgCount);
	
	/**
	 * \brief Create naked object on the stack.
	 * 
	 * Naked objects are created without calling a constructor for use by native classes
	 * requiring to build objects using special constructors.
	 */
	void CreateObjectNakedOnStack( dsClass *type );
	
	void CreateObjectNaked(dsValue *Value, dsClass *Type);
	void CreateAndPushObject(dsClass *Type, int ArgCount);
	
	/**
	 * \brief Cast value to another type.
	 * \param from Source value.
	 * \param to Target value.
	 * \param toType Target type.
	 * \note to is allowed to be the same as from.
	 */
	void CastValueTo(dsValue *from, dsValue *to, dsClass *toType);
	
	/** \brief Find function to run or \em NULL if not found. */
	dsFunction *FindRunFunction( const dsClass *objectClass, const char *funcName, int argCount );
	
	/** \brief Find function index to run or -1 if not found. */
	int FindRunFunctionIndex( const dsClass *objectClass, const char *funcName, int argCount );
	
	/** \brief Find function to run or \em NULL if not found. */
	dsFunction *FindRunFunction( dsValue *value, const char *funcName, int argCount );
	
	/** \brief Find function index to run or -1 if not found. */
	int FindRunFunctionIndex( dsValue *value, const char *funcName, int argCount );
	
	// function execution
	void RunFunction(dsValue *Value, const char *FuncName, int ArgCount);
	void RunFunction(dsValue *Value, int FuncIndex, int ArgCount);
	
	/**
	 * \brief Run function with minimal checks.
	 * 
	 * \warning Call this only if you know what you are doing. You can corrupt everything.
	 *          In particular you are responsible to ensure the values on the stack match
	 *          the function signature.
	 */
	void RunFunctionFast( dsValue *value, dsFunction *function );
	
	/**
	 * \brief Run function with minimal checks.
	 * 
	 * \warning Call this only if you know what you are doing. You can corrupt everything.
	 *          In particular you are responsible to ensure the values on the stack match
	 *          the function signature. Keep in mind that \em funcIndex is relative to the
	 *          function base of \em value.
	 */
	void RunFunctionFast( dsValue *value, int funcIndex );
	
	void RunStaticFunction(const char *Type, const char *Function, int ArgCount);
	void RunStaticFunction(dsClass *type, int FuncIndex, int ArgCount);
	bool IsStackMatchingSig(dsSignature *sig, int argCount);
	byte GetReturnByte() const;
	bool GetReturnBool() const;
	int GetReturnInt() const;
	float GetReturnFloat() const;
	const char *GetReturnString() const;
	dsValue *GetReturnValue() const;
	dsRealObject *GetReturnRealObject() const;
	// exception handling
	inline dsValue *GetRaisedException() const{ return p_RaisedException; }
	void ThrowException(dsValue *pException);
	void AddNativeTrace(dsFunction *function, int line);
	void PrintExceptionTrace();
	void ClearExceptionTrace();
private:
	void p_PrepareException(const duException &ex);
	void pPrepareUnknownException();
	void p_PrintValue(dsValue *Value);
	void p_ExecFunc(dsRealObject *This, dsClass *Class, dsFunction *Function, dsRealObject *block );
};

// end of include only once
#endif

