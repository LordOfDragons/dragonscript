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
#include "dragonscript_config.h"
#include "dsEngine.h"
#include "dsGarbageCollector.h"
#include "dsMemoryManager.h"
#include "dsRunTime.h"
#include "dsBaseEngineManager.h"
#include "objects/dsClass.h"
#include "objects/dsValue.h"
#include "objects/dsByteCode.h"
#include "objects/dsFunction.h"
#include "objects/dsFuncList.h"
#include "objects/dsVariable.h"
#include "objects/dsSignature.h"
#include "objects/dsRealObject.h"
#include "utils/dsuString.h"
#include "packages/default/dsClassString.h"
#include "packages/default/dsClassBlock.h"
#include "exceptions.h"



// structures
///////////////
struct sGCType{
	dsClass *type;
	int count;
	sGCType *next;
};



// class dsGarbageCollector
/////////////////////////////

// constructor, destructor
dsGarbageCollector::dsGarbageCollector(dsMemoryManager *memMgr){
	if(!memMgr) DSTHROW(dueInvalidParam);
	pMemMgr = memMgr;
	pClsObj = memMgr->GetEngine()->GetClassObject();
	// garbage collection
	pHeadObj = NULL;
	pTailObj = NULL;
	
}
dsGarbageCollector::~dsGarbageCollector(){
	// RunGarbageCollector(); // ?? better not
}

// management

// garbage collection
dsRealObject *dsGarbageCollector::CreateObject(dsClass *type){
	if(!type) DSTHROW(dueInvalidParam);
	// create real object
	const int size = (int)dsAllocSize<dsRealObject>() + type->SizeOf();
	dsRealObject *newObj = pMemMgr->AllocateObject(size);
	newObj->p_Type = type;
	if(pTailObj){
		pTailObj->SetNext( newObj );
		newObj->SetPrevious( pTailObj );
		pTailObj = newObj;
	}else{
		pHeadObj = newObj;
		pTailObj = newObj;
	}
	// finished
//pMemMgr->GetEngine()->PrintMessage("[object created] %s", type->GetName());
	return newObj;
}
void dsGarbageCollector::FreeObject(dsRealObject *obj){
	if(!obj) DSTHROW(dueInvalidParam);
	// remove object from list
	if(obj->GetPrevious()){
		obj->GetPrevious()->SetNext( obj->GetNext() );
	}else{
		pHeadObj = obj->GetNext();
	}
	if(obj->GetNext()){
		obj->GetNext()->SetPrevious( obj->GetPrevious() );
	}else{
		pTailObj = obj->GetPrevious();
	}
	// free object
//pMemMgr->GetEngine()->PrintMessage("[object freed] %s", obj->GetType()->GetName());
	pMemMgr->FreeObject( obj );
}
void dsGarbageCollector::RunGarbageCollector(){
	dsRealObject *obj, *next;
	sGCType *typeHead=NULL, *typeTail=NULL, *typeNew=NULL, *typeCur;
	dsuString fullName;
	dsClass *type;
	// hack - get us a RT
	dsRunTime *RT = pMemMgr->GetEngine()->GetMainRunTime();
	// print out list of all leaking objects for the developer to debug the program
	try{
		obj = pTailObj;
		while(obj){
//			pMemMgr->GetEngine()->PrintMessage("[hit] %i", (int)obj->GetType());
			type = obj->GetType();
			typeCur = typeHead;
			while(typeCur && typeCur->type!=type) typeCur=typeCur->next;
			if(typeCur){
				typeCur->count++;
			}else{
				if(!(typeNew = new sGCType)) DSTHROW(dueOutOfMemory);
				typeNew->type = type;
				typeNew->count = 1;
				typeNew->next = NULL;
				if(typeTail) typeTail->next = typeNew;
				typeTail = typeNew;
				if(!typeHead) typeHead = typeNew;
			}
			obj = obj->GetPrevious();
		}
		if(typeHead){
			pMemMgr->GetEngine()->PrintMessage( "[Garbage Collector] Detected Memory Leacks!" );
			while(typeHead){
				fullName.Empty();
				typeHead->type->GetFullName(&fullName);
				pMemMgr->GetEngine()->PrintMessageFormat( "  - %s (%ix)", fullName.Pointer(), typeHead->count );
				typeCur = typeHead->next;
				delete typeHead;
				typeHead = typeCur;
			}
		}
	}catch( ... ){
		if(typeNew) delete typeNew;
		while(typeHead){
			typeCur = typeHead->next;
			delete typeHead;
			typeHead = typeCur;
		}
		throw;
	}
	// enter cleanup mode
//	p_GCCleanUpMode = true;
	// increase reference count to keep objects alive so we can process all of them
//	obj = pTailObj;
//	while(obj){
//		obj->IncRefCount(); //AddWeakRef(obj);
//		obj = obj->GetPrevious();
//	}
	// call destructor on all objects and free variables. because the ref count is
	// one higher than before no object can be freed before we are done with it.
	obj = pTailObj;
	while( obj ){
		type = obj->GetType();
		dsValue vdata( type, obj );
		obj->IncRefCount();
		
		//printf( "GC destructor %p(%i)\n", obj, obj->GetRefCount() );
		try{
			type->Destructor( RT, &vdata );
			
		}catch( ... ){
			RT->PrintExceptionTrace();
			RT->ClearExceptionTrace();
		}
		
		//printf( "GC free variables %p(%i)\n", obj, obj->GetRefCount() );
		try{
			type->FreeVariables( RT, &vdata );
			
		}catch( ... ){
			RT->PrintExceptionTrace();
			RT->ClearExceptionTrace();
		}
		
		// NOTE: if the refcount of the obj is not 1 at this place then the
		//       destructor of the object will called once again if the
		//       refcount drops to 0. this is not that bad if native classes
		//       are written a robust way but it is not desirable. there has
		//       to be a check that keeps in mind if the object has already
		//       been destructed during in a gc-cycle.
		next = obj->GetPrevious();
		obj->DecRefCount();
		
		if( obj->GetRefCount() == 0 ){
			// free object only if there is not still a weak reference around
			if( obj->GetWeakRefCount() == 0 ){
				FreeObject( obj );
				
			}else{
				obj->Clear();
			}
		}
		
		// next entry
		obj = next;
	}
	
	// now there should be no more objects around or our gc has failed.
	if( pHeadObj || pTailObj ){
		try{
			obj = pTailObj;
			
			while( obj ){
				type = obj->GetType();
				typeCur = typeHead;
				while( typeCur && typeCur->type != type ){
					typeCur = typeCur->next;
				}
				if( typeCur ){
					typeCur->count++;
					
				}else{
					if( ! ( typeNew = new sGCType ) ){
						DSTHROW( dueOutOfMemory );
					}
					typeNew->type = type;
					typeNew->count = 1;
					typeNew->next = NULL;
					if( typeTail ){
						typeTail->next = typeNew;
					}
					typeTail = typeNew;
					if( ! typeHead ){
						typeHead = typeNew;
					}
				}
				obj = obj->GetPrevious();
			}
			
			if( typeHead ){
				pMemMgr->GetEngine()->PrintMessage( "[Garbage Collector] Still objects living after GC (statics?)!" );
				while( typeHead ){
					fullName.Empty();
					typeHead->type->GetFullName( &fullName );
					pMemMgr->GetEngine()->PrintMessageFormat( "  - %s (%ix)", fullName.Pointer(), typeHead->count );
					typeCur = typeHead->next;
					delete typeHead;
					typeHead = typeCur;
				}
			}
			
		}catch( ... ){
			if( typeNew ){
				delete typeNew;
			}
			while( typeHead ){
				typeCur = typeHead->next;
				delete typeHead;
				typeHead = typeCur;
			}
			throw;
		}
	}
	
	// free all objects
	/*
	obj = pTailObj;
	while(obj){
//		pMemMgr->GetEngine()->PrintMessage("[free object on %s] %i/%i", obj->GetType()->GetName(), obj->GetRefCount(), obj->GetWeakRefCount());
		dsValue vdata(obj->GetType(), obj);
		ClearValue(&vdata);
		obj = obj->GetPrevious();
	}
	// finally free the memory of all objects
	while(pTailObj){
		obj = pTailObj;
		pTailObj = pTailObj->GetPrevious();
		delete obj;
	}
	*/
	// leave cleanup mode
//	p_GCCleanUpMode = false;
}
