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
#include "dsuPointerStack.h"
#include "../exceptions.h"

// class dsuPointerStack
//////////////////////////
// constructor, destructor
dsuPointerStack::dsuPointerStack(){
	p_Objects = NULL;
	p_StackSize = 0;
	p_StackPointer = 0;
	p_GrowBy = 10;
}
dsuPointerStack::dsuPointerStack(int InitSize, int GrowBy){
	if(InitSize < 0) DSTHROW(dueInvalidParam);
	if(GrowBy < 1) DSTHROW(dueInvalidParam);
	p_Objects = NULL;
	p_StackSize = InitSize;
	if(p_StackSize > 0){
		if(!(p_Objects = new void*[p_StackSize])) DSTHROW(dueOutOfMemory);
	}
	p_StackPointer = 0;
	p_GrowBy = GrowBy;
}
dsuPointerStack::~dsuPointerStack(){
	if(p_Objects) delete [] p_Objects;
}
// management
void dsuPointerStack::Clear(){
	p_StackPointer = 0;
}
void dsuPointerStack::Push(void *Object){
	void **vNewStack;
	int i;
	// enlarge stack if too small
	if(p_StackPointer == p_StackSize){
		if(!(vNewStack = new void*[p_StackSize+p_GrowBy])) DSTHROW(dueOutOfMemory);
		for(i=0; i<p_StackPointer; i++) vNewStack[i] = p_Objects[i];
		if(p_Objects) delete [] p_Objects;
		p_Objects = vNewStack;
		p_StackSize += p_GrowBy;
	}
	// put object on stack
	p_Objects[p_StackPointer] = Object;
	p_StackPointer++;
}
void *dsuPointerStack::Pop(){
	if(p_StackPointer == 0) DSTHROW(dueStackEmpty);
	p_StackPointer--;
	return p_Objects[p_StackPointer];
}
void *dsuPointerStack::Top(){
	if(p_StackPointer == 0) DSTHROW(dueStackEmpty);
	return p_Objects[p_StackPointer-1];
}
void *dsuPointerStack::Get(int Index) const{
	if((Index < 0) || (Index >= p_StackPointer)) DSTHROW(dueOutOfBoundary);
	return p_Objects[p_StackPointer-1-Index];
}
void dsuPointerStack::Remove(int Count){
	if(Count > p_StackPointer) DSTHROW(dueInvalidParam);
	p_StackPointer -= Count;
}
void dsuPointerStack::SetCapacity(int Size){
	if(Size < p_StackSize) return;
	void **vNewStack;
	if(!(vNewStack = new void*[Size])) DSTHROW(dueOutOfMemory);
	for(int i=0; i<p_StackPointer; i++) vNewStack[i] = p_Objects[i];
	if(p_Objects) delete [] p_Objects;
	p_Objects = vNewStack;
	p_StackSize = Size;
}
void dsuPointerStack::SetGrowBy(int GrowBy){
	if(GrowBy < 1) DSTHROW(dueInvalidParam);
	p_GrowBy = GrowBy;
}

