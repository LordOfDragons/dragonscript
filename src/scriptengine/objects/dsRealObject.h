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
#ifndef _DSOBJECT_H_
#define _DSOBJECT_H_

// includes

// predefinitions
class dsClass;

// class dsRealObject
class dsRealObject{
friend class dsRunTime;
friend class dsGarbageCollector;
private:
	dsClass *p_Type;
	int p_RefCount;
	int p_WeakRefCount;
	dsRealObject *p_Prev, *p_Next;
	//char *p_Data;
	char p_Data[]; // c-trick for free-size structs
public:
	// management
	inline dsClass *GetType() const{ return p_Type; }
	inline char *GetBuffer(){ return (char*)&p_Data; }
	inline int GetRefCount() const{ return p_RefCount; }
	inline int GetWeakRefCount() const{ return p_WeakRefCount; }
	// garbage control
	inline dsRealObject *GetPrevious() const{ return p_Prev; }
	inline dsRealObject *GetNext() const{ return p_Next; }
private:
//	dsRealObject(dsClass *Type);
	dsRealObject();
	~dsRealObject();
	void Init(dsClass *Type);
	void Clear();
	inline void IncRefCount(){ p_RefCount++; }
	inline void DecRefCount(){ p_RefCount--; }
	inline void IncWeakRefCount(){ p_WeakRefCount++; }
	inline void DecWeakRefCount(){ p_WeakRefCount--; }
	inline void ResetRefCount(int count){ p_RefCount = count; }
	inline void SetPrevious(dsRealObject *prev){ p_Prev = prev; }
	inline void SetNext(dsRealObject *next){ p_Next = next; }
};

// end of include only once
#endif

