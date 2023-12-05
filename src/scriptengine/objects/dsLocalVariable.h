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
#ifndef _DSLOCALVARIABLE_H_
#define _DSLOCALVARIABLE_H_

// includes
#include "../utils/dsuArrayable.h"

// prefinitions
class dsClass;
class dsFunction;

// class dsLocalVariable
class DS_DLL_EXPORT dsLocalVariable : public dsuArrayable {
private:
	char *p_Name;
	dsClass *p_Type;
	
public:
	// constructor, destructor
	dsLocalVariable(dsClass *OwnerClass, const char *Name, dsClass *Type);
	~dsLocalVariable();
	// management
	inline const char *GetName() const{ return (const char *)p_Name; }
	inline dsClass *GetType() const{ return p_Type; }
};

// end of include only once
#endif

