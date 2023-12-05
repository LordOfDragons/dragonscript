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
#include "dsuIndexList.h"
#include "../exceptions.h"

// class dsuIndexList
/////////////////
// constructor, destructor
dsuIndexList::dsuIndexList(){
	p_Indices = NULL;
	p_IndCount = 0;
	p_EffIndCount = 0;
}
dsuIndexList::~dsuIndexList(){
	if(p_Indices) delete [] p_Indices;
}
// management
void dsuIndexList::Resize(int Size){
	if(Size < 0) DSTHROW(dueInvalidParam);
	int *vNewArray;
	int i;
	// do nothing if size stays the same
	if(Size == p_IndCount) return;
	// grow array if size is bigger than the the array capacity
	if(Size > p_EffIndCount){
		vNewArray = new int[Size];
		if(!vNewArray) DSTHROW(dueOutOfMemory);
		for(i=0; i<p_IndCount; i++) vNewArray[i] = p_Indices[i];
		if(p_Indices) delete [] p_Indices;
		p_Indices = vNewArray;
		p_EffIndCount = Size;
	}
	// store new size
	p_IndCount = Size;
}
void dsuIndexList::Add(int Index){
	Resize(p_IndCount + 1);
	p_Indices[p_IndCount-1] = Index;
}
void dsuIndexList::Insert(int Index, int Position){
	if((Position < 0) || (Position > p_IndCount)) DSTHROW(dueOutOfBoundary);
	Resize(p_IndCount + 1);
	for(int i=p_IndCount-1; i>Position; i--) p_Indices[i] = p_Indices[i-1];
	p_Indices[Position] = Index;
}
void dsuIndexList::Remove(int Position){
	if((Position < 0) || (Position >= p_IndCount)) DSTHROW(dueOutOfBoundary);
	for(int i=Position; i<p_IndCount-1; i++) p_Indices[i] = p_Indices[i+1];
	Resize(p_IndCount - 1);
}
void dsuIndexList::RemoveAll(){
	Resize(0);
}
int dsuIndexList::GetIndex(int Position) const{
	if((Position < 0) || (Position >= p_IndCount)) DSTHROW(dueOutOfBoundary);
	return p_Indices[Position];
}
void dsuIndexList::SetIndex(int Position, int Index){
	if((Position < 0) || (Position >= p_IndCount)) DSTHROW(dueOutOfBoundary);
	p_Indices[Position] = Index;
}
int dsuIndexList::FindIndex(int Index) const{
	for(int i=0; i<p_IndCount; i++){
		if(p_Indices[i] == Index) return i;
	}
	return -1;
}
int dsuIndexList::operator[](int Position) const{
	if((Position < 0) || (Position >= p_IndCount)) DSTHROW(dueOutOfBoundary);
	return p_Indices[Position];
}

