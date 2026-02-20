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
#ifndef _DSMEMORYMANAGER_H_
#define _DSMEMORYMANAGER_H_

#include "dragonscript_export.h"

// predefinitions
class dsEngine;
class dsClass;
class dsValue;
class dsRealObject;
class dsValue;



//definitions
#define DSMM_DEFSIZE		4096	// default size of value strip array


// class dsMemoryManager
class DS_DLL_EXPORT dsMemoryManager{
friend class dsGarbageCollector;
private:
	dsEngine *pEngine;
	dsClass *pClsObj;
public:
	// constructor, destructor
	dsMemoryManager(dsEngine *Engine);
	~dsMemoryManager();
	// management
	inline dsEngine *GetEngine() const{ return pEngine; }
	// value strip memory management
	dsValue *AllocateValueStrip(int count);
	void FreeValueStrip(dsValue *valueStrip);
	// single value memory management
	dsValue *AllocateValue();
	void FreeValue(dsValue *value);
private:
	// object memory management
	dsRealObject *AllocateObject(int size);
	void FreeObject(dsRealObject *obj);
};

// end of include only once
#endif

