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



// includes
#include <stdlib.h>
#include "../../config.h"
#include "dsuArray.h"
#include "../exceptions.h"

// class dsuArray
/////////////////
// constructor, destructor
dsuArray::dsuArray(){
	p_Objects = NULL;
	p_ObjCount = 0;
	p_EffObjCount = 0;
}
dsuArray::~dsuArray(){
	if(p_Objects){
		for(int i=0; i<p_ObjCount; i++){
			if(p_Objects[i]) delete p_Objects[i];
		}
		delete [] p_Objects;
	}
}
// management
void dsuArray::SetCapacity(int Capacity){
	if(Capacity < p_ObjCount) DSTHROW(dueInvalidParam);
	int i;
	// resize array
	if(Capacity != p_EffObjCount){
		dsuArrayable **vNewArray = new dsuArrayable*[Capacity];
		if(!Capacity) DSTHROW(dueOutOfMemory);
		for(i=0; i<p_ObjCount; i++) vNewArray[i] = p_Objects[i];
		for(; i<Capacity; i++) vNewArray[i] = NULL;
		if(p_Objects) delete [] p_Objects;
		p_Objects = vNewArray;
		p_EffObjCount = Capacity;
	}
}
void dsuArray::Resize(int Size){
	if(Size < 0) DSTHROW(dueInvalidParam);
	dsuArrayable **vNewArray;
	int i;
	// do nothing if size stays the same
	if(Size == p_ObjCount) return;
	// free objects if size is smaller than the object count
	for(i=Size; i<p_ObjCount; i++){
		if(p_Objects[i]){
			delete p_Objects[i];
			p_Objects[i] = NULL;
		}
	}
	// grow array if size is bigger than the the array capacity
	if(Size > p_EffObjCount){
		vNewArray = new dsuArrayable*[Size];
		if(!vNewArray) DSTHROW(dueOutOfMemory);
		for(i=0; i<p_ObjCount; i++) vNewArray[i] = p_Objects[i];
		for(; i<Size; i++) vNewArray[i] = NULL;
		if(p_Objects) delete [] p_Objects;
		p_Objects = vNewArray;
		p_EffObjCount = Size;
	}
	// store new size
	p_ObjCount = Size;
}
void dsuArray::Add(dsuArrayable *Object){
	Resize(p_ObjCount + 1);
	p_Objects[p_ObjCount-1] = Object;
}
void dsuArray::Insert(dsuArrayable *Object, int Index){
	if((Index < 0) || (Index > p_ObjCount)) DSTHROW(dueOutOfBoundary);
	Resize(p_ObjCount + 1);
	for(int i=p_ObjCount-1; i>Index; i--) p_Objects[i] = p_Objects[i-1];
	p_Objects[Index] = Object;
}
void dsuArray::Remove(int Index){
	if((Index < 0) || (Index >= p_ObjCount)) DSTHROW(dueOutOfBoundary);
	if(p_Objects[Index]) delete p_Objects[Index];
	for(int i=Index; i<p_ObjCount-1; i++) p_Objects[i] = p_Objects[i+1];
	p_Objects[p_ObjCount-1] = NULL;
	Resize(p_ObjCount - 1);
}
void dsuArray::RemoveAll(){
	Resize(0);
}

dsuArrayable *dsuArray::GetObject( int Index ) const{
	if( ( Index < 0 ) || ( Index >= p_ObjCount ) ){
		DSTHROW( dueOutOfBoundary );
	}
	return p_Objects[ Index ];
}

dsuArrayable *dsuArray::TakeOutObject(int Index){
	if((Index < 0) || (Index >= p_ObjCount)) DSTHROW(dueOutOfBoundary);
	dsuArrayable *vObj = p_Objects[Index];
	p_Objects[Index] = NULL;
	return vObj;
}
void dsuArray::SetObject(int Index, dsuArrayable *Object){
	if((Index < 0) || (Index >= p_ObjCount)) DSTHROW(dueOutOfBoundary);
	if(Object != p_Objects[Index]){
		if(p_Objects[Index]) delete p_Objects[Index];
		p_Objects[Index] = Object;
	}
}
int dsuArray::FindObject(dsuArrayable *Object) const{
	for(int i=0; i<p_ObjCount; i++){
		if(p_Objects[i] == Object) return i;
	}
	return -1;
}
dsuArrayable *dsuArray::operator[](int Index) const{
	if((Index < 0) || (Index >= p_ObjCount)) DSTHROW(dueOutOfBoundary);
	return p_Objects[Index];
}

void dsuArray::AppendFrom(dsuArray *array){
	if(!array) DSTHROW(dueInvalidParam);
	Resize(p_ObjCount + array->p_ObjCount);
	for(int i=0; i<array->p_ObjCount; i++){
		p_Objects[p_ObjCount+i] = array->p_Objects[i];
		array->p_Objects[i] = NULL;
	}
	p_ObjCount += array->p_ObjCount;
	array->p_ObjCount = 0;
}
