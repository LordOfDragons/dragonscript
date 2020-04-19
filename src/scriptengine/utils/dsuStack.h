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
#ifndef _CSTACKH_
#define _CSTACKH_

// includes
#include "dsuArrayable.h"

// class dsuStack
class dsuStack{
private:
	dsuArrayable **p_Objects;
	int p_StackSize;
	int p_StackPointer;
	int p_GrowBy;
public:
	// constructor, destructor
	dsuStack();
	dsuStack(int InitSize, int GrowBy = 10);
	~dsuStack();
	// management
	void Clear();
	void Push(dsuArrayable *Object);
	dsuArrayable *Pop();
	dsuArrayable *Top();
	dsuArrayable *Get(int Index) const;
	void Remove(int Count);
	void SetCapacity(int Size);
	void SetGrowBy(int GrowBy);
	inline bool CanPop() const{ return p_StackPointer > 0; }
	inline int GetSize() const{ return p_StackPointer; }
	inline int GetCapacity() const{ return p_StackSize; }
};

// end of include only once
#endif

