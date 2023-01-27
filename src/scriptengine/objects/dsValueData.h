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
#ifndef _DSVALUEDATA_H_
#define _DSVALUEDATA_H_

// includes
#include "../dsDefinitions.h"

// predefinitions
class dsRealObject;

// class dsValueData
// -----------------
// special class containing the actual data of a value.
// this can be either local like a dsValue or remote like a dsVariable.
class DS_DLL_EXPORT dsValueData{
private:
	union{
		byte p_Byte;
		bool p_Bool;
		int p_Int;
		float p_Float;
		dsRealObject *p_Object;
	};
	int p_FuncBase; // for objects only
public:
	// constructor
	inline dsValueData(){ p_FuncBase=0; }
	// data retrieval
	inline byte GetByte() const{ return p_Byte; }
	inline bool GetBool() const{ return p_Bool; }
	inline int GetInt() const{ return p_Int; }
	inline float GetFloat() const{ return p_Float; }
	inline dsRealObject *GetRealObject() const{ return p_Object; }
	inline int GetFuncBase() const{ return p_FuncBase; }
	// data storing
	inline void SetByte(byte Value){ p_Byte = Value; }
	inline void SetBool(bool Value){ p_Bool = Value; }
	inline void SetInt(int Value){ p_Int = Value; }
	inline void SetFloat(float Value){ p_Float = Value; }
	inline void SetRealObject(dsRealObject *Value){ p_Object = Value; }
	inline void SetFuncBase(int base){ p_FuncBase = base; }
	// operations
	inline void IncByte(){ ++p_Byte; }
	inline void IncInt(){ ++p_Int; }
	inline void IncFloat(){ ++p_Float; }
	inline void DecByte(){ --p_Byte; }
	inline void DecInt(){ --p_Int; }
	inline void DecFloat(){ --p_Float; }
	inline void MulAByte(byte Value){ p_Byte *= Value; }
	inline void MulAInt(int Value){ p_Int *= Value; }
	inline void MulAFloat(float Value){ p_Float *= Value; }
	inline void DivAByte(byte Value){ p_Byte /= Value; }
	inline void DivAInt(int Value){ p_Int /= Value; }
	inline void DivAFloat(float Value){ p_Float /= Value; }
	inline void ModAByte(byte Value){ p_Byte %= Value; }
	inline void ModAInt(int Value){ p_Int %= Value; }
	inline void AddAByte(byte Value){ p_Byte += Value; }
	inline void AddAInt(int Value){ p_Int += Value; }
	inline void AddAFloat(float Value){ p_Float += Value; }
	inline void SubAByte(byte Value){ p_Byte -= Value; }
	inline void SubAInt(int Value){ p_Int -= Value; }
	inline void SubAFloat(float Value){ p_Float -= Value; }
	inline void LSAByte(byte Value){ p_Byte <<= Value; }
	inline void LSAInt(int Value){ p_Int <<= Value; }
	inline void RSAByte(byte Value){ p_Byte >>= Value; }
	inline void RSAInt(int Value){ p_Int >>= Value; }
	inline void AndAByte(byte Value){ p_Byte &= Value; }
	inline void AndAInt(int Value){ p_Int &= Value; }
	inline void AndABool(bool Value){ p_Bool &= Value; }
	inline void OrAByte(byte Value){ p_Byte |= Value; }
	inline void OrAInt(int Value){ p_Int |= Value; }
	inline void OrABool(bool Value){ p_Bool |= Value; }
	inline void XorAByte(byte Value){ p_Byte ^= Value; }
	inline void XorAInt(int Value){ p_Int ^= Value; }
	inline void XorABool(bool Value){ p_Bool ^= Value; }
};

// end of include only once
#endif

