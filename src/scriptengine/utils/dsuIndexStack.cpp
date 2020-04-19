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
#include "dsuIndexStack.h"
#include "../exceptions.h"

// class dsuIndexStack
//////////////////////////
// constructor, destructor
dsuIndexStack::dsuIndexStack(){
	p_Indices = NULL;
	p_StackSize = 0;
	p_StackPointer = 0;
	p_GrowBy = 10;
}
dsuIndexStack::dsuIndexStack(int InitSize, int GrowBy){
	if(InitSize < 0) DSTHROW(dueInvalidParam);
	if(GrowBy < 1) DSTHROW(dueInvalidParam);
	p_Indices = NULL;
	p_StackSize = InitSize;
	if(p_StackSize > 0){
		if(!(p_Indices = new int[p_StackSize])) DSTHROW(dueOutOfMemory);
	}
	p_StackPointer = 0;
	p_GrowBy = GrowBy;
}
dsuIndexStack::~dsuIndexStack(){
	if(p_Indices) delete [] p_Indices;
}
// management
void dsuIndexStack::Clear(){
	p_StackPointer = 0;
}
void dsuIndexStack::Push(int Index){
	int *vNewStack;
	int i;
	// enlarge stack if too small
	if(p_StackPointer == p_StackSize){
		if(!(vNewStack = new int[p_StackSize+p_GrowBy])) DSTHROW(dueOutOfMemory);
		for(i=0; i<p_StackPointer; i++) vNewStack[i] = p_Indices[i];
		if(p_Indices) delete [] p_Indices;
		p_Indices = vNewStack;
		p_StackSize += p_GrowBy;
	}
	// put index on stack
	p_Indices[p_StackPointer] = Index;
	p_StackPointer++;
}
int dsuIndexStack::Pop(){
	if(p_StackPointer == 0) DSTHROW(dueStackEmpty);
	p_StackPointer--;
	return p_Indices[p_StackPointer];
}
int dsuIndexStack::Top(){
	if(p_StackPointer == 0) DSTHROW(dueStackEmpty);
	return p_Indices[p_StackPointer-1];
}
int dsuIndexStack::Get(int Position) const{
	if((Position < 0) || (Position >= p_StackPointer)) DSTHROW(dueOutOfBoundary);
	return p_Indices[p_StackPointer-1-Position];
}
void dsuIndexStack::Remove(int Count){
	if(Count > p_StackPointer) DSTHROW(dueInvalidParam);
	p_StackPointer -= Count;
}
void dsuIndexStack::SetCapacity(int Size){
	if(Size < p_StackSize) return;
	int *vNewStack;
	if(!(vNewStack = new int[Size])) DSTHROW(dueOutOfMemory);
	for(int i=0; i<p_StackPointer; i++) vNewStack[i] = p_Indices[i];
	if(p_Indices) delete [] p_Indices;
	p_Indices = vNewStack;
	p_StackSize = Size;
}
void dsuIndexStack::SetGrowBy(int GrowBy){
	if(GrowBy < 1) DSTHROW(dueInvalidParam);
	p_GrowBy = GrowBy;
}

