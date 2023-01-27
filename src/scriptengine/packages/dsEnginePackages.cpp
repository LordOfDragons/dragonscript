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
#include <string.h>
#include "../dragonscript_config.h"
#include "dsEnginePackages.h"
#include "dsBaseEnginePackage.h"
#include "dsNativePackage.h"
//#include "math/dsEnginePackageMath.h"
#include "../dsEngine.h"
#include "../exceptions.h"

// class dsEnginePackage
//////////////////////////
// constructor, destructor
dsEnginePackages::dsEnginePackages(dsEngine *Engine){
	if(!Engine) DSTHROW(dueInvalidParam);
	p_Engine = Engine;
	p_Packages = NULL;
	p_PkgCount = 0;
	p_PkgSize = 0;
	p_NatPaks = NULL;
	p_NatPakCount = 0;
	p_NatPakSize = 0;
//	try{
//		p_AddPackage(new dsEnginePackageMath(p_Engine));
//	}catch( ... ){
//		p_Clear(); throw;
//	}
}
dsEnginePackages::~dsEnginePackages(){
	p_Clear();
}

// package management
dsBaseEnginePackage *dsEnginePackages::GetPackage(int Index) const{
	if(Index<0 || Index>=p_PkgCount) DSTHROW(dueOutOfBoundary);
	return p_Packages[Index];
}
dsBaseEnginePackage *dsEnginePackages::GetPackage(const char *Name) const{
	for(int i=0; i<p_PkgCount; i++){
		if(strcmp(p_Packages[i]->GetName(), Name) == 0) return p_Packages[i];
	}
	return NULL;
}

// native package management
dsNativePackage *dsEnginePackages::GetNativePackage(int Index) const{
	if(Index<0 || Index>=p_NatPakCount) DSTHROW(dueOutOfBoundary);
	return p_NatPaks[Index];
}
dsNativePackage *dsEnginePackages::GetNativePackage(const char *filename) const{
	for(int i=0; i<p_NatPakCount; i++){
		if(strcmp(filename, p_NatPaks[i]->GetFilename()) == 0) return p_NatPaks[i];
	}
	return NULL;
}
void dsEnginePackages::AddNatPackage(dsNativePackage *Package){
	if(!Package) DSTHROW(dueInvalidParam);
	if(GetNativePackage(Package->GetFilename())) DSTHROW(dueInvalidParam);
	// enlarge array if needed
	if(p_NatPakCount == p_NatPakSize){
		dsNativePackage **vNewArray = new dsNativePackage*[p_NatPakSize+1];
		if(!vNewArray) DSTHROW(dueOutOfMemory);
		if(p_NatPaks){
			for(int i=0; i<p_NatPakCount; i++) vNewArray[i] = p_NatPaks[i];
			delete [] p_NatPaks;
		}
		p_NatPaks = vNewArray;
		p_NatPakSize++;
	}
	// add Scene
	p_NatPaks[p_NatPakCount] = Package;
	p_NatPakCount++;
}

// private functions
void dsEnginePackages::p_Clear(){
	if(p_NatPaks){
		for(int i=0; i<p_NatPakCount; i++) delete p_NatPaks[i];
		delete [] p_NatPaks;
	}
	if(p_Packages){
		for(int i=0; i<p_PkgCount; i++) delete p_Packages[i];
		delete [] p_Packages;
	}
}
void dsEnginePackages::p_AddPackage(dsBaseEnginePackage *Package){
	// enlarge array if needed
	if(p_PkgCount == p_PkgSize){
		dsBaseEnginePackage **vNewArray = new dsBaseEnginePackage*[p_PkgSize+1];
		if(!vNewArray) DSTHROW(dueOutOfMemory);
		if(p_Packages){
			for(int i=0; i<p_PkgCount; i++) vNewArray[i] = p_Packages[i];
			delete [] p_Packages;
		}
		p_Packages = vNewArray;
		p_PkgSize++;
	}
	// add Scene
	p_Packages[p_PkgCount] = Package;
	p_PkgCount++;
}
