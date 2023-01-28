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
#ifndef _DSGARBAGECOLLECTOR_H_
#define _DSGARBAGECOLLECTOR_H_

#include "dragonscript_export.h"

// predefinitions
class dsMemoryManager;
class dsClass;
class dsValue;
class dsRealObject;
class dsValue;



// class dsGarbageCollector
class DS_DLL_EXPORT dsGarbageCollector{
private:
	dsMemoryManager *pMemMgr;
	dsClass *pClsObj;
	// garbage collection
	dsRealObject *pHeadObj, *pTailObj;
public:
	// constructor, destructor
	dsGarbageCollector(dsMemoryManager *memMgr);
	~dsGarbageCollector();
	// management
	inline dsMemoryManager *GetMemoryManager() const{ return pMemMgr; }
	// garbage collection
	dsRealObject *CreateObject(dsClass *type);
	void FreeObject(dsRealObject *obj);
	void RunGarbageCollector();
private:
};

// end of include only once
#endif

