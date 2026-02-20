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

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include "dragonscript_config.h"
#include "dsEngine.h"
#include "dsMemoryManager.h"
#include "objects/dsValue.h"
#include "objects/dsRealObject.h"
#include "exceptions.h"



// class dsMemoryManager
//////////////////////////

// constructor, destructor
dsMemoryManager::dsMemoryManager(dsEngine *engine){
	if(!engine) DSTHROW(dueInvalidParam);
	pEngine = engine;
	pClsObj = engine->GetClassObject();
}
dsMemoryManager::~dsMemoryManager(){
}

// management

// value strip memory management
dsValue *dsMemoryManager::AllocateValueStrip(int count){
	if(count < 1) DSTHROW(dueInvalidParam);
	// dummy. just a simple new for the time beeing
	dsValue *newStrip = new dsValue[count];
	// finished
	return newStrip;
}
void dsMemoryManager::FreeValueStrip(dsValue *valueStrip){
	if(!valueStrip) DSTHROW(dueInvalidParam);
	// dummy delete.
	delete [] valueStrip;
}

// single value memory management
dsValue *dsMemoryManager::AllocateValue(){
	// dummy. just a simple new for the time beeing
	dsValue * const newValue = new dsValue;
	// finished
	return newValue;
}
void dsMemoryManager::FreeValue(dsValue *value){
	if(!value) DSTHROW(dueInvalidParam);
	// dummy delete.
	delete value;
}

// object memory management
dsRealObject *dsMemoryManager::AllocateObject(int size){
	if(size < 1) DSTHROW(dueInvalidParam);
	// dummy. just a simple malloc for the time beeing
	char * const orgPtr = new char[size];
	if(!orgPtr) DSTHROW(dueOutOfMemory);
	
	dsRealObject * const newObj = dsAllocPlacementNew<dsRealObject>(orgPtr, orgPtr);
	
	// finished
	return newObj;
}
void dsMemoryManager::FreeObject(dsRealObject *obj){
	if(!obj) DSTHROW(dueInvalidParam);
	// dummy free.
	char * const orgPtr = obj->pOrgPtr;
	obj->~dsRealObject();
	delete [] orgPtr;
}

// private functions
