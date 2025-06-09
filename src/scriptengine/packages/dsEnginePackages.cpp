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

#include <stdio.h>
#include <string.h>

#include "../dragonscript_config.h"

#include "dsEnginePackages.h"
#include "dsBaseEnginePackage.h"
#include "dsNativePackage.h"
#include "../dsEngine.h"
#include "../exceptions.h"


#ifdef WITH_INTERNAL_PACKAGES
extern dsBaseEnginePackage *dsMathRegisterInternalModule();
extern dsBaseEnginePackage *dsIntrospectionRegisterInternalModule();
#endif


// Class dsEnginePackage
//////////////////////////

// Constructor, destructor
////////////////////////////

dsEnginePackages::dsEnginePackages(dsEngine *engine) :
pEngine(engine),
pPackages(nullptr),
pPkgCount(0),
pPkgSize(0),
pNatPaks(nullptr),
pNatPakCount(0),
pNatPakSize(0)
{
	if(!engine){
		DSTHROW(dueInvalidParam);
	}
	
#ifdef WITH_INTERNAL_PACKAGES
	try{
		pAddPackage(dsMathRegisterInternalModule());
		pAddPackage(dsIntrospectionRegisterInternalModule());
		
	}catch( ... ){
		pClear();
		throw;
	}
#endif
}

dsEnginePackages::~dsEnginePackages(){
	pClear();
}


// Package management
///////////////////////

dsBaseEnginePackage *dsEnginePackages::GetPackage(int index) const{
	if(index<0 || index>=pPkgCount){
		DSTHROW(dueOutOfBoundary);
	}
	return pPackages[index];
}

dsBaseEnginePackage *dsEnginePackages::GetPackage(const char *Name) const{
	int i;
	for(i=0; i<pPkgCount; i++){
		if(strcmp(pPackages[i]->GetName(), Name) == 0){
			return pPackages[i];
		}
	}
	return nullptr;
}


// Native package management
//////////////////////////////

dsNativePackage *dsEnginePackages::GetNativePackage(int index) const{
	if(index<0 || index>=pNatPakCount){
		DSTHROW(dueOutOfBoundary);
	}
	return pNatPaks[index];
}

dsNativePackage *dsEnginePackages::GetNativePackage(const char *filename) const{
	int i;
	for(i=0; i<pNatPakCount; i++){
		if(strcmp(filename, pNatPaks[i]->GetFilename()) == 0){
			return pNatPaks[i];
		}
	}
	return nullptr;
}

void dsEnginePackages::AddNatPackage(dsNativePackage *package){
	if(!package){
		DSTHROW(dueInvalidParam);
	}
	if(GetNativePackage(package->GetFilename())){
		DSTHROW(dueInvalidParam);
	}
	
	if(pNatPakCount == pNatPakSize){
		dsNativePackage **newArray = new dsNativePackage*[pNatPakSize+1];
		if(pNatPaks){
			int i;
			for(i=0; i<pNatPakCount; i++){
				newArray[i] = pNatPaks[i];
			}
			delete [] pNatPaks;
		}
		pNatPaks = newArray;
		pNatPakSize++;
	}
	
	pNatPaks[pNatPakCount] = package;
	pNatPakCount++;
}


// Private functions
//////////////////////

void dsEnginePackages::pClear(){
	int i;
	if(pNatPaks){
		for(i=0; i<pNatPakCount; i++){
			delete pNatPaks[i];
		}
		delete [] pNatPaks;
	}
	if(pPackages){
		for(i=0; i<pPkgCount; i++){
			delete pPackages[i];
		}
		delete [] pPackages;
	}
}

void dsEnginePackages::pAddPackage(dsBaseEnginePackage *package){
	if(pPkgCount == pPkgSize){
		dsBaseEnginePackage **newArray = new dsBaseEnginePackage*[pPkgSize+1];
		if(pPackages){
			int i;
			for(i=0; i<pPkgCount; i++){
				newArray[i] = pPackages[i];
			}
			delete [] pPackages;
		}
		pPackages = newArray;
		pPkgSize++;
	}
	
	pPackages[pPkgCount] = package;
	pPkgCount++;
}
