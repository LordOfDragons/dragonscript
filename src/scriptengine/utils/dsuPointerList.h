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
#ifndef _CPOINTERLIST_H_
#define _CPOINTERLIST_H_

// includes
#include "dsuArrayable.h"

// class dsuPointerList
// --------------------
// Manages a list of pointers.
class dsuPointerList : public dsuArrayable {
private:
	void **p_Pointers;
	int p_EffCount;
	int p_Count;
public:
	// constructor, destructor
	dsuPointerList();
	~dsuPointerList();
	// management
	inline int Length() const{ return p_Count; }
	inline int Capacity() const{ return p_EffCount; }
	void Resize(int Size);
	void Add(void *Pointer);
	void Insert(void *Pointer, int Position);
	void Remove(int Position);
	void RemoveAll();
	void *GetPointer(int Position) const;
	void SetPointer(int Position, void *Pointer);
	int FindPointer(void *Pointer) const;
	inline bool ExistsPointer(void *Pointer) const{ return FindPointer(Pointer) != -1; }
	void *operator[](int Position) const;
	void AppendFrom(dsuPointerList *plist);
};

// end of include only once
#endif

