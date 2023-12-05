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
#ifndef _DSFUNCTION_H_
#define _DSFUNCTION_H_

// includes
#include "../dsDefinitions.h"
#include "../utils/dsuArrayable.h"

// predefinitions
class dsResult;
class dsRealObject;
class dsRunTime;
class dsClass;
class dsByteCode;
class dsSignature;
class dsValue;
class dsFunctionOptimized;

// class dsFunction
class DS_DLL_EXPORT dsFunction : public dsuArrayable {
private:
	dsClass *p_OwnerClass;
	char *p_Name;
	dsClass *p_Type;
	dsSignature *p_Signature;
	dsByteCode *p_ByteCode; // byte code script
	int p_FuncType, p_TypeModifiers;
	int p_LocVarSize;
	bool p_HasRetVal;
	dsFunctionOptimized *pOptimized;
	
public:
	// constructor, destructor
	dsFunction(dsClass *OwnerClass, const char *Name, int FuncType, int TypeModifiers, dsClass *Type);
	virtual ~dsFunction();
	// management
	bool IsEqualTo(dsFunction *Function) const;
	inline int GetFuncType() const{ return p_FuncType; }
	inline dsClass *GetOwnerClass() const{ return p_OwnerClass; }
	inline dsSignature *GetSignature() const{ return p_Signature; }
	inline void MakeAbstract(){ p_TypeModifiers |= DSTM_ABSTRACT; }
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dsClass *GetType() const{ return p_Type; }
	inline dsByteCode *GetByteCode() const{ return p_ByteCode; }
	inline int GetTypeModifiers() const{ return p_TypeModifiers; }
	inline int GetLocVarSize() const{ return p_LocVarSize; }
	inline bool HasRetVal() const{ return p_HasRetVal; }
	void SetLocVarSize(int Size);
	
	/** \brief Optimized function or NULL. */
	inline dsFunctionOptimized *GetOptimized() const{ return pOptimized; }
	
	/**
	 * \brief Set optimized function or NULL.
	 * 
	 * If \em function is different from the stored optimized function deletes the existing
	 * optimized function if not NULL.
	 */
	void SetOptimized( dsFunctionOptimized *optimized );
	
	// run function
	virtual void RunFunction( dsRunTime *rt, dsValue *myself );
	
	/** \brief Check if a type mod combination is valid for a function. */
	static bool AreTypeModsValid( int functionType, int typeModifiers );
	
protected:
	// helper functions for native implementators
	void *p_GetNativeData(dsValue *This) const;
	void p_AddParameter(dsClass *type);
	bool p_IsObjOfType(dsValue *obj, dsClass *type) const;
};

// end of include only once
#endif

