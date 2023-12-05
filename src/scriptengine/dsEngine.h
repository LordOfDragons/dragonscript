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
#ifndef _DSENGINE_H_
#define _DSENGINE_H_

// includes
#include "dsDefinitions.h"

// predefinitions
class dsBaseEngineManager;
class dsEnginePackages;
class dsRunTime;
class dsClass;
class dsPackage;
class dsPackageSource;
class dsuArray;
class dsMemoryManager;
class dsGarbageCollector;


// class dsEngine
class DS_DLL_EXPORT dsEngine{
friend class dspParserCheckClass;
friend class dspParserCheck;
friend class dsClass;
public:
	enum eParserWarnings{
		pwStringMulLineSpawn = 1,
		pwUnclosedInlineComment,
		pwDuplicateTypeModifier,
		pwUnreachableCode,
		pwPackageEmpty,
	};
	enum eParserErrors{
		peInvalidToken = 1,
		peUnexpectedToken,
		peUnexpectedEOF,
		peInvalidPackageName,
		peInvalidClassMember,
		peInvalidVarName,
		peInvalidFuncName,
		peInvalidFuncArgName,
		peInvalidEnumEntryName,
		peInvalidEnumName,
		peInvalidClassName,
		peInvalidBaseClassName,
		peInvalidInterfaceName,
		peInvalidExcepVarName,
		peInvalidOptionKeyName,
		peInvalidOptionValueName,
		peInvalidOperOp,
		peInvalidDestrSig,
		peInvalidVarTypeVoid,
		peInvalidLocVarTypeVoid,
		peInvalidTypeModifier,
		peIncompatibleTypeModifier,
		peClassExists,
		peClassIsFixed,
		peInheritanceLoop,
		peClassNotExist,
		peDupClassFunc,
		peDupClassDestr,
		peDupClassVar,
		peDupClassConst,
		peDupClassOp,
		peDupLocVar,
		peVarIsNotStatic,
		peFuncIsNotStatic,
		peFuncIsStatic,
		peFuncIsNotConst,
		peFuncInvTypeMods,
		peInvalidTypeCast,
		peInvalidFuncArg,
		peInvalidFuncSig,
		peInvalidOpArgCount,
		peInvalidObject,
		peInvalidClass,
		peInvalidOpSig,
		peInvalidPrimOp,
		peNoAccessClsVar,
		peNoAccessClsFunc,
		peNoMembSelAllowed,
		peKeywordInStaticFunc,
		peConstObj,
		peInvLoopVarType,
		peNoDirectDestrCall,
		peConstrNotOnClass,
		peNoStaVarInit,
		peInvalidVarInit,
		peConstLoop,
		peReturnOnVoid,
		peNoReturnValue,
		peMissingReturn,
		peBreakOutside,
		peContinueOutside,
		peRethrowOutside,
		peNoConstrFunc,
		peAmbigDefConstrFunc,
		peNoDefConstrFunc,
		peNonEmptyIFFunc,
		peNotInterface,
		peBaseNotSameType,
		peDupImplement,
		peFuncNotImplemented,
		peCaseValNotScalar,
		peDupCaseValue,
		peInvEnumValue,
		peInvExceptionType,
		peMissingCatch,
		peDupCatch,
		peNSMemberNotNS,
		peThrowNotException,
		pePackageNotFound,
		peNoNewOnAbstractClass,
	};
private:
	dsuArray *p_Classes; // dsClass*
	dsuArray *p_Packages; // dsPackage*
	dsuArray *p_ConstStrTable; // dsuString*
	dsuArray *p_PakSources; // dsPackageSource*
	dsuArray *p_SharePath; // dsuString*
	// management
	int p_LockCount;
	char *pTextBuffer;
	int pTextBufferLen;
	
	dsBaseEngineManager *p_EngineManager;
	dsEnginePackages *p_EngPkg;
	dsRunTime *p_MainRT;
	// memory stuff
	dsMemoryManager *pMemMgr;
	dsGarbageCollector *pGC;
	// shortcuts to important classes
	dsClass *pClsVoid;
	dsClass *pClsByte;
	dsClass *pClsBool;
	dsClass *pClsInt;
	dsClass *pClsFloat;
	dsClass *pClsObj;
	dsClass *pClsNull;
	dsClass *pClsStr;
	dsClass *pClsArr;
	dsClass *pClsDict;
	dsClass *pClsSet;
	dsClass *pClsEnumeration;
	dsClass *pClsBlock;
	dsClass *pClsExcep;
	dsClass *pClsTimeDate;
public:
	// constructor, destructor
	dsEngine();
	~dsEngine();
	// engine manager
	inline dsBaseEngineManager *GetEngineManager() const{ return p_EngineManager; }
	void SetEngineManager(dsBaseEngineManager *Manager);
	// package sources
	int GetPackageSourceCount() const;
	void ClearPackageSources();
	void AddDefaultPackageSources();
	void AddPackageSource(dsPackageSource *source);
	dsPackageSource *GetPackageSource(int index) const;
	dsPackageSource *FindPackageSource(const char *pakname) const;
	void RemovePackageSource(dsPackageSource *source);
	// engine management
	void Clear();
	inline dsRunTime *GetMainRunTime() const{ return p_MainRT; }
	// class management
	int GetClassCount() const;
	dsClass *GetClass(int Index) const;
	dsClass *GetClass(const char *Name) const;
	// quick class retrieval
	inline dsClass *GetClassVoid() const{ return pClsVoid; }
	inline dsClass *GetClassByte() const{ return pClsByte; }
	inline dsClass *GetClassBool() const{ return pClsBool; }
	inline dsClass *GetClassInt() const{ return pClsInt; }
	inline dsClass *GetClassFloat() const{ return pClsFloat; }
	inline dsClass *GetClassNull() const{ return pClsNull; }
	inline dsClass *GetClassObject() const{ return pClsObj; }
	inline dsClass *GetClassString() const{ return pClsStr; }
	inline dsClass *GetClassArray() const{ return pClsArr; }
	inline dsClass *GetClassDictionary() const{ return pClsDict; }
	inline dsClass *GetClassSet() const{ return pClsSet; }
	inline dsClass *GetClassEnumeration() const{ return pClsEnumeration; }
	inline dsClass *GetClassBlock() const{ return pClsBlock; }
	inline dsClass *GetClassException() const{ return pClsExcep; }
	inline dsClass *GetClassTimeDate() const{ return pClsTimeDate; }
	// memory stuf
	inline dsMemoryManager *GetMemoryManager() const{ return pMemMgr; }
	inline dsGarbageCollector *GetGarbageCollector() const{ return pGC; }
	// package management
	int GetPackageCount() const;
	dsPackage *GetPackage(int Index) const;
	dsPackage *GetPackage(const char *Name) const;
	bool ExistsPackage(const char *Name) const;
	bool AddPackage(dsPackage *Package);
	bool LoadPackage(const char *Name);
	inline dsEnginePackages *GetEnginePackages() const{ return p_EngPkg; }
	// shared path list
	int GetSharedPathCount() const;
	const char *GetSharedPath(int Index) const;
	// constant string table
	int AddConstString(const char *String);
	const char *GetConstString(int Index);
	// debugging
	void PrintClasses(bool WithInternals);
	void PrintMessage( const char *message );
	void PrintMessageFormat( const char *format, ... );
private:
	void p_CreateEngineClass();
	void pCreateExceptionClasses();
	void pCreateEngExceptionClass(const char *Name, const char *Desc);
	void p_AddClass(dsClass *Class);
	void p_RemoveClass(dsClass *Class);
	void p_BuildSharePath();
	void p_AddSharePath(const char *path);
	void pPreareTextBuffer( int requiredLength );
};

// end of include only once
#endif
