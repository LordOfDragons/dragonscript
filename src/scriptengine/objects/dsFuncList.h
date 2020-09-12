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
#ifndef _DSFUNCLIST_H_
#define _DSFUNCLIST_H_

// includes

// predefinitions
class dsClass;
class dsFunction;
class dsSignature;

// class dsFuncList
class dsFuncList{
private:
	dsFunction **p_Funcs;
	int p_FuncCount;
public:
	// constructor, destructor
	dsFuncList();
	~dsFuncList();
	// management
	inline int GetCount() const{ return p_FuncCount; }
	void AddFunction(dsFunction *Function);
	dsFunction *GetFunction(int Index) const;
	void ReplaceFunction( int index, dsFunction *function );
	int GetIndexOf(dsFunction *func) const;
	void Clear();
	// searching
	dsFunction *FindBestFunction(dsClass *Class, const char *Name, dsSignature *Signature, bool nullCheck=false) const;
	// debuggin
	void DebugPrint() const;
};

// end of include only once
#endif

