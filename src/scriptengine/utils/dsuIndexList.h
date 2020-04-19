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
#ifndef _CINDEXLIST_H_
#define _CINDEXLIST_H_

// includes
#include "dsuArrayable.h"

// class dsuIndexList
// ----------------
// Manages a list of indices.
class dsuIndexList : public dsuArrayable {
private:
	int *p_Indices;
	int p_EffIndCount;
	int p_IndCount;
public:
	// constructor, destructor
	dsuIndexList();
	~dsuIndexList();
	// management
	inline int Length() const{ return p_IndCount; }
	inline int Capacity() const{ return p_EffIndCount; }
	void Resize(int Size);
	void Add(int Index);
	void Insert(int Index, int Position);
	void Remove(int Position);
	void RemoveAll();
	int GetIndex(int Position) const;
	void SetIndex(int Position, int Index);
	int FindIndex(int Index) const;
	inline bool ExistsIndex(int Index) const{ return FindIndex(Index) != -1; }
	int operator[](int Position) const;
};

// end of include only once
#endif

