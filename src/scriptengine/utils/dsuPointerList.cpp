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
#include "../dragonscript_config.h"
#include "dsuPointerList.h"
#include "../exceptions.h"

// class dsuPointerList
/////////////////
// constructor, destructor
dsuPointerList::dsuPointerList(){
	p_Pointers = NULL;
	p_Count = 0;
	p_EffCount = 0;
}
dsuPointerList::~dsuPointerList(){
	if(p_Pointers) delete [] p_Pointers;
}
// management
void dsuPointerList::Resize(int Size){
	if(Size < 0) DSTHROW(dueInvalidParam);
	void **vNewArray;
	int i;
	// do nothing if size stays the same
	if(Size == p_Count) return;
	// grow array if size is bigger than the the array capacity
	if(Size > p_EffCount){
		vNewArray = new void*[Size];
		if(!vNewArray) DSTHROW(dueOutOfMemory);
		for(i=0; i<p_Count; i++) vNewArray[i] = p_Pointers[i];
		if(p_Pointers) delete [] p_Pointers;
		p_Pointers = vNewArray;
		p_EffCount = Size;
	}
	// store new size
	p_Count = Size;
}
void dsuPointerList::Add(void *Pointer){
	Resize(p_Count + 1);
	p_Pointers[p_Count-1] = Pointer;
}
void dsuPointerList::Insert(void *Pointer, int Position){
	if((Position < 0) || (Position > p_Count)) DSTHROW(dueOutOfBoundary);
	Resize(p_Count + 1);
	for(int i=p_Count-1; i>Position; i--) p_Pointers[i] = p_Pointers[i-1];
	p_Pointers[Position] = Pointer;
}
void dsuPointerList::Remove(int Position){
	if((Position < 0) || (Position >= p_Count)) DSTHROW(dueOutOfBoundary);
	for(int i=Position; i<p_Count-1; i++) p_Pointers[i] = p_Pointers[i+1];
	Resize(p_Count - 1);
}
void dsuPointerList::RemoveAll(){
	Resize(0);
}
void *dsuPointerList::GetPointer(int Position) const{
	if((Position < 0) || (Position >= p_Count)) DSTHROW(dueOutOfBoundary);
	return p_Pointers[Position];
}
void dsuPointerList::SetPointer(int Position, void *Pointer){
	if((Position < 0) || (Position >= p_Count)) DSTHROW(dueOutOfBoundary);
	p_Pointers[Position] = Pointer;
}
int dsuPointerList::FindPointer(void *Pointer) const{
	for(int i=0; i<p_Count; i++){
		if(p_Pointers[i] == Pointer) return i;
	}
	return -1;
}
void *dsuPointerList::operator[](int Position) const{
	if((Position < 0) || (Position >= p_Count)) DSTHROW(dueOutOfBoundary);
	return p_Pointers[Position];
}


void dsuPointerList::AppendFrom(dsuPointerList *plist){
	if(!plist) DSTHROW(dueInvalidParam);
	Resize(p_Count + plist->p_Count);
	for(int i=0; i<plist->p_Count; i++){
		p_Pointers[p_Count+i] = plist->p_Pointers[i];
		plist->p_Pointers[i] = NULL;
	}
	p_Count += plist->p_Count;
	plist->p_Count = 0;
}
