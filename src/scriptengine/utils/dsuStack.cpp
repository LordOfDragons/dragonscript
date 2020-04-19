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
#include "dsuStack.h"
#include "../exceptions.h"

// class dsuStack
/////////////////
// constructor, destructor
dsuStack::dsuStack(){
	p_Objects = NULL;
	p_StackSize = 0;
	p_StackPointer = 0;
	p_GrowBy = 10;
}
dsuStack::dsuStack(int InitSize, int GrowBy){
	if(InitSize < 0) DSTHROW(dueInvalidParam);
	if(GrowBy < 1) DSTHROW(dueInvalidParam);
	p_Objects = NULL;
	p_StackSize = InitSize;
	if(p_StackSize > 0){
		p_Objects = new dsuArrayable*[p_StackSize];
		if(!p_Objects) DSTHROW(dueOutOfMemory);
		for(int i=0; i<p_StackSize; i++) p_Objects[i] = NULL;
	}
	p_StackPointer = 0;
	p_GrowBy = GrowBy;
}
dsuStack::~dsuStack(){
	if(p_Objects){
		for(int i=0; i<p_StackPointer; i++){
			if(p_Objects[i]) delete p_Objects[i];
		}
		delete [] p_Objects;
	}	
}
// management
void dsuStack::Clear(){
	for(int i=0; i<p_StackPointer; i++){
		if(p_Objects[i]){
			delete p_Objects[i];
			p_Objects[i] = NULL;
		}
	}
	p_StackPointer = 0;
}
void dsuStack::Push(dsuArrayable *Object){
	dsuArrayable **vNewStack;
	int i;
	// enlarge stack if too small
	if(p_StackPointer == p_StackSize){
		vNewStack = new dsuArrayable*[p_StackSize+p_GrowBy];
		if(!vNewStack) DSTHROW(dueOutOfMemory);
		for(i=0; i<p_StackPointer; i++) vNewStack[i] = p_Objects[i];
		for(; i<p_StackSize+p_GrowBy; i++) vNewStack[i] = NULL;
		if(p_Objects) delete [] p_Objects;
		p_Objects = vNewStack;
		p_StackSize += p_GrowBy;
	}
	// put object on stack
	p_Objects[p_StackPointer] = Object;
	p_StackPointer++;
}
dsuArrayable *dsuStack::Pop(){
	if(p_StackPointer == 0) DSTHROW(dueStackEmpty);
	dsuArrayable *vObject;
	p_StackPointer--;
	vObject = p_Objects[p_StackPointer];
	p_Objects[p_StackPointer] = NULL;
	return vObject;
}
dsuArrayable *dsuStack::Top(){
	if(p_StackPointer == 0) DSTHROW(dueStackEmpty);
	return p_Objects[p_StackPointer-1];
}
dsuArrayable *dsuStack::Get(int Index) const{
	if((Index < 0) || (Index >= p_StackPointer)) DSTHROW(dueOutOfBoundary);
	return p_Objects[p_StackPointer-1-Index];
}
void dsuStack::Remove(int Count){
	if(Count > p_StackPointer) DSTHROW(dueInvalidParam);
	dsuArrayable *vObject;
	for(int i=0; i<Count; i++){
		p_StackPointer--;
		vObject = p_Objects[p_StackPointer];
		p_Objects[p_StackPointer] = NULL;
		if(vObject) delete vObject;
	}
}
void dsuStack::SetCapacity(int Size){
	if(Size < p_StackSize) return;
	dsuArrayable **vNewStack;
	int i;
	vNewStack = new dsuArrayable*[Size];
	if(!vNewStack) DSTHROW(dueOutOfMemory);
	for(i=0; i<p_StackPointer; i++) vNewStack[i] = p_Objects[i];
	for(; i<Size; i++) vNewStack[i] = NULL;
	if(p_Objects) delete [] p_Objects;
	p_Objects = vNewStack;
	p_StackSize = Size;
}
void dsuStack::SetGrowBy(int GrowBy){
	if(GrowBy < 1) DSTHROW(dueInvalidParam);
	p_GrowBy = GrowBy;
}

