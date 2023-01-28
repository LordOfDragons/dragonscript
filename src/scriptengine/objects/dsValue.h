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
#ifndef _DSVALUE_H_
#define _DSVALUE_H_

// includes
#include <stdlib.h>
#include "dsValueData.h"

// predefinitions
class dsClass;
class dsRealObject;

// class dsValue
class DS_DLL_EXPORT dsValue{
friend class dsMemoryManager;
friend class dsGarbageCollector;
friend class dsRunTime;
friend class dsVariable;
friend class dsClass;
friend class dsClassBlock;
private:
	dsClass *p_Type;
	dsValueData p_Data;
public:
	// management
	inline dsClass *GetType() const{ return p_Type; }
	inline dsValueData *GetValueData(){ return &p_Data; }
	void Empty();
	// data retrieval
	inline byte GetByte() const{ return p_Data.GetByte(); }
	inline bool GetBool() const{ return p_Data.GetBool(); }
	inline int GetInt() const{ return p_Data.GetInt(); }
	inline float GetFloat() const{ return p_Data.GetFloat(); }
	inline dsRealObject *GetRealObject() const{ return p_Data.GetRealObject(); }
	inline int GetFuncBase() const{ return p_Data.GetFuncBase(); }
	// special data retrieval
	const char *GetString() const;
	dsClass *GetRealType() const;
private: // for dsRunTime
	dsValue(dsClass *Type);
	dsValue(dsClass *Type, dsRealObject *obj);
	dsValue(dsValue *value);
	inline void SetValue(dsValue *Value){ p_Type=Value->p_Type; p_Data=Value->p_Data; }
	inline void SetValue(dsClass *Type, dsValueData *Data){ p_Type=Type; p_Data=*Data; }
	inline void SetByte(dsClass *Type, byte Value){ p_Type=Type; p_Data.SetByte(Value); }
	inline void SetBool(dsClass *Type, bool Value){ p_Type=Type; p_Data.SetBool(Value); }
	inline void SetInt(dsClass *Type, int Value){ p_Type=Type; p_Data.SetInt(Value); }
	inline void SetFloat(dsClass *Type, float Value){ p_Type=Type; p_Data.SetFloat(Value); }
	inline void SetRealObject(dsClass *Type, dsRealObject *Object){ p_Type=Type; p_Data.SetRealObject(Object); p_Data.SetFuncBase(0); }
	inline void SetNull(dsClass *Type){ p_Type=Type; p_Data.SetRealObject(NULL); p_Data.SetFuncBase(0); }
	inline void SetEmpty(dsClass *Type){ p_Type=Type; p_Data.SetRealObject(NULL); p_Data.SetFuncBase(0); }
	inline void SetFuncBase(int base){ p_Data.SetFuncBase(base); }
	inline void SetType(dsClass *Type){ p_Type=Type; }
};

// end of include only once
#endif

