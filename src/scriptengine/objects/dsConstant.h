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
#ifndef _DSCONSTANT_H_
#define _DSCONSTANT_H_

// includes
#include "dsValueData.h"
#include "../utils/dsuArrayable.h"

// predefinitions
class dsClass;
class dsRealObject;

// class dsConstant
class dsConstant : public dsuArrayable {
private:
	char *p_Name;
	dsClass *p_Type;
	dsValueData p_Data;
public:
	// constructor, destructor
	dsConstant(const char *Name, dsClass *Type);
	dsConstant(const char *Name, dsClass *Type, byte InitValue);
	dsConstant(const char *Name, dsClass *Type, bool InitValue);
	dsConstant(const char *Name, dsClass *Type, int InitValue);
	dsConstant(const char *Name, dsClass *Type, float InitValue);
	~dsConstant();
	// management
	inline const char *GetName() const{ return (const char*)p_Name; }
	inline dsClass *GetType() const{ return p_Type; }
	// data retrieval
	inline byte GetByte() const{ return p_Data.GetByte(); }
	inline bool GetBool() const{ return p_Data.GetBool(); }
	inline int GetInt() const{ return p_Data.GetInt(); }
	inline float GetFloat() const{ return p_Data.GetFloat(); }
private:
	void p_Empty();
};

// end of include only once
#endif

