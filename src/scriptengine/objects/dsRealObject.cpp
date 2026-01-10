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
#include <stdio.h>
#include <stdlib.h>
#include "../dragonscript_config.h"
#include "dsClass.h"
#include "dsRealObject.h"
#include "../exceptions.h"

// class dsRealObject
////////////////////////
// private functions for dsRunTime
/*
dsRealObject::dsRealObject(dsClass *Type){
	int vSize = Type->SizeOf();
	p_Type = Type;
	p_RefCount = 0;
	p_WeakRefCount = 0;
	p_Data = NULL;
	p_Prev = NULL;
	p_Next = NULL;
	if(vSize > 0){
		if(!(p_Data = new char[vSize])) DSTHROW(dueOutOfMemory);
		Type->InitVariables(this);
	}
}
*/
dsRealObject::dsRealObject(char *orgPtr) :
p_Type(nullptr),
p_RefCount(0),
p_WeakRefCount(0),
p_Prev(nullptr),
p_Next(nullptr),
pOrgPtr(orgPtr){
}
dsRealObject::~dsRealObject(){
	Clear();
}
// management
void dsRealObject::Clear(){
//	if(p_Data){
//		delete [] p_Data;
//		p_Data = NULL;
//	}
}
