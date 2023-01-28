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
#ifndef _DSCLASS_H_
#define _DSCLASS_H_

// includes
#include "../dsDefinitions.h"
#include "../utils/dsuArrayable.h"

// predefinitions
class dsEngine;
class dsRealObject;
class dsImplClass;
class dsValue;
class dsVariable;
class dsConstant;
class dsFunction;
class dsFuncList;
class dsBaseClassDef;
class dsSignature;
class dsEnumeration;
class dsRunTime;
class dsuArray;
class dsuString;
class dsClassParserInfo;

// class dsClass
class DS_DLL_EXPORT dsClass : public dsuArrayable {
friend class dsEngine;
friend class dsFunction;
private:
	char *p_Name; // class name
	dsClass *p_Parent; // parent class
	dsClass *p_BaseClass; // base class
	dsuArray *p_Implements; // dsImplClass*
	dsuArray *p_InnerClasses; // dsClass*
	dsuArray *p_Variables; // dsVariable*
	dsuArray *p_Constants; // dsConstant*
	dsuArray *p_LocFuncs; // dsFunction*
	dsuArray *p_BlockFuncs; // dsFunction*
	dsFuncList *p_FuncList;
	dsFunction *p_InitStaFunc;
	dsFunction *p_FirstConstrFunc, *p_DestrFunc;
	dsClassParserInfo *p_parserInfo; // infos for parse for native classes only
	int p_Size; // size of class object
	int p_BaseSize; // size of base class
	int p_NativeSize; // size of data needed by native class
	int p_TypeModifiers;
	int p_PrimitiveType;
	int p_ClassType;
public:
	// constructor, destructor
	dsClass(const char *Name, int ClassType, int TypeModifiers=DSTM_PUBLIC, int PrimitiveType=DSPT_OBJECT);
	virtual ~dsClass();
	// object management
	void Destructor(dsRunTime *RT, dsValue *This);
	void InitVariables(dsRealObject *This);
	void FreeVariables(dsRunTime *RT, dsValue *This);
	// implements
	void AddImplement(dsClass *Class);
	int GetImplementsCount() const;
	dsClass *GetImplement(int Index) const;
	int GetImplFuncBase(int Index) const;
	int FindImplement(const char *Name, bool All = true) const;
	int GetImplFuncBase(dsClass *Class) const;
	bool ExistsImplement(dsClass *Class) const;
	// inner class management
	void AddInnerClass(dsClass *Class);
	int GetInnerClassCount() const;
	dsClass *GetInnerClass(int Index) const;
	dsClass *GetInnerClass(const char *Name) const;
	// variables
	void AddVariable(dsVariable *Variable);
	int GetVariableCount() const;
	dsVariable *GetVariable(int Index) const;
	dsVariable *FindVariable(const char *Name, bool All = true);
	// constants
	void AddConstant(dsConstant *Constant);
	int GetConstantCount() const;
	dsConstant *GetConstant(int Index) const;
	dsConstant *FindConstant(const char *Name, bool All = true);
	// local functions
	void AddFunction(dsFunction *Function);
	int GetFunctionCount() const;
	dsFunction *GetFunction(int Index) const;
	inline dsFuncList *GetFuncList() const{ return p_FuncList; }
	dsFunction *FindFunction(const char *Name, dsSignature *Signature, bool All = true);
	dsFunction *FindFunction(dsFunction *Function, bool All = true);
	inline dsFunction *GetFirstConstructor() const{ return p_FirstConstrFunc; }
	inline dsFunction *GetDestructor() const{ return p_DestrFunc; }
	int GetFunctionNameType(const char *Name);
	bool HasConstructors() const;
	// block functions (internal use only)
	void AddBlockFunction(dsFunction *Function);
	int GetBlockFunctionCount() const;
	dsFunction *GetBlockFunction(int Index) const;
	// parser info for native classes only
	inline dsClassParserInfo *GetParserInfo() const{ return p_parserInfo; }
	// base class
	void CallBaseClassConstructor(dsRunTime *RT, dsValue *This, dsFunction *Function, int ArgCount);
	inline dsClass *GetBaseClass() const{ return p_BaseClass; }
	void SetBaseClass(dsClass *BaseClass);
	// static variables
	void FreeStatics(dsRunTime *RT);
	virtual void InitStatics(dsRunTime *RT);
	void CreateStaFunc(dsEngine *engine);
	inline dsFunction *GetInitStaFunc() const{ return p_InitStaFunc; }
	// class
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dsClass *GetParent() const{ return p_Parent; }
	void GetFullName(dsuString *FullName) const;
	int GetFullName(char *buffer, int len);
	inline int GetClassType() const{ return p_ClassType; }
	inline int GetTypeModifiers() const{ return p_TypeModifiers; }
	inline int GetPrimitiveType() const{ return p_PrimitiveType; }
	inline int SizeOf() const{ return p_Size; }
	bool IsEqualTo(dsClass *Class) const;
	bool CastableTo(dsClass *Type);
	int GetSubClassLevel(dsClass *Class) const;
	bool IsSubClassOf(dsClass *Class) const;
	void CalcMemberOffsets();
	void UpdateFuncList();

	 /* create all class members like native functions, constants
	  * or variables. only for native classes.
	  */
	virtual void CreateClassMembers(dsEngine *engine);
	
	// debugging
	void PrintClass(bool descent) const;
protected:
	void p_FillFuncList(dsFuncList *FuncList, int base, bool All);
	void p_SetNativeDataSize(int Size);
	void *p_GetNativeData(void *This) const;
	bool p_HasAbstractFunc(dsFunction *Function);
	void p_SetParent(dsClass *Parent);
	void p_RemoveInnerClass(dsClass *Class);
};

// end of include only once
#endif

