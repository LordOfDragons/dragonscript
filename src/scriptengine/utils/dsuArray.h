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
#ifndef _CARRAY_H_
#define _CARRAY_H_

// includes
#include "dsuArrayable.h"

// class dsuArray
// ------------
// Manages a list of arrayable objects.
// Objects added to the list are managed by the array afterwards
// so don't delete them after you added/inserted them.
class dsuArray : dsuArrayable{
private:
	dsuArrayable **p_Objects;
	int p_EffObjCount;
	int p_ObjCount;
public:
	// constructor, destructor
	dsuArray();
	~dsuArray();
	// management
	inline int Length() const{ return p_ObjCount; }
	inline int Capacity() const{ return p_EffObjCount; }
	void SetCapacity(int Capacity);
	void Resize(int Size);
	void Add(dsuArrayable *Object);
	void Insert(dsuArrayable *Object, int Index);
	void Remove(int Index);
	void RemoveAll();
	dsuArrayable *GetObject(int Index) const;
	dsuArrayable *TakeOutObject(int Index);
	void SetObject(int Index, dsuArrayable *Object);
	int FindObject(dsuArrayable *Object) const;
	inline bool ExistsObject(dsuArrayable *Object) const{ return FindObject(Object) != -1; }
	dsuArrayable *operator[](int Index) const;
	void AppendFrom(dsuArray *array);
};

// end of include only once
#endif

