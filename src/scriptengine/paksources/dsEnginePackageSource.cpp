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
#include <string.h>
#include "../dragonscript_config.h"
#include "dsEnginePackageSource.h"
#include "../dsEngine.h"
#include "../dsBaseEngineManager.h"
#include "../dsFile.h"
#include "../dsPackage.h"
#include "../packages/dsEnginePackages.h"
#include "../packages/dsBaseEnginePackage.h"
#include "../packages/dsNativePackage.h"
#include "../exceptions.h"

#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
#	include <errno.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <unistd.h>
#	include <dirent.h>
#elif defined OS_W32
#	ifdef _WIN32_WINNT
#		undef _WIN32_WINNT
#	endif
#	define _WIN32_WINNT _WIN32_WINNT_WIN7
#	include <windows.h>
#else
#	error "No valid OS_* symbol defined!"
#endif


// definitions
#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
#	define PATH_SEPARATOR	'/'
#elif defined OS_W32
#	define PATH_SEPARATOR	'\\'
#endif



// class dsEnginePackageSource
////////////////////////////////
// constructor, destructor
dsEnginePackageSource::dsEnginePackageSource(dsEngine *engine){
	if(!engine) DSTHROW(dueInvalidParam);
	pEngine = engine;
}
dsEnginePackageSource::~dsEnginePackageSource(){ }
// determine if the given name matches an engine
// package, either native or scripted
bool dsEnginePackageSource::CanHandle(const char *name){
	dsEnginePackages *engPkg = pEngine->GetEnginePackages();
	// check engine packages for match
	if(engPkg->GetPackage(name)) return true;
	// otherwise check the share directories to see if one matches.
	if(p_FindPackage(name)) return true;
	// nothing found
	return false;
}
// loads the given engine package
dsPackage *dsEnginePackageSource::LoadPackage(const char *name){
	dsEnginePackages *engPkgList = pEngine->GetEnginePackages();
	dsBaseEnginePackage *engPkg;
	dsPackage *pkg = nullptr;
	const char *sharedPath;
	char *pakPath = nullptr;
	
	// try to load package from engine if existing
	engPkg = engPkgList->GetPackage(name);
	if(engPkg){
		pkg = engPkg->CreatePackage();
		sharedPath = p_FindPackage(name);
		if(sharedPath){
			try{
				const int size = (int)strlen(sharedPath) + (int)strlen(name) + 2;
				pakPath = new char[size];
				snprintf(pakPath, size, "%s%s%c", sharedPath, name, PATH_SEPARATOR);
				p_AddScripts(pkg, pakPath);
				delete [] pakPath;
				pakPath = nullptr;
				
			}catch( ... ){
				if(pakPath){
					delete [] pakPath;
				}
				delete pkg;
				throw;
			}
		}
		return pkg;
	}
	
	// if this is not an internal package it has to be a
	// script package. try to load it
	sharedPath = p_FindPackage(name);
	if(sharedPath){
		pkg = new dsPackage(name);
		if(!pkg) DSTHROW(dueOutOfMemory);
		try{
			// create package
			const int size = ( int )strlen(sharedPath) + ( int )strlen(name) + 2;
			pakPath = new char[size];
			if(!pakPath) DSTHROW(dueOutOfMemory);
			// add scripts
			snprintf(pakPath, size, "%s%s%c", sharedPath, name, PATH_SEPARATOR);
			p_AddScripts(pkg, pakPath);
			// look for a native class library and add it
			p_AddNativeLib(pkg, pakPath);
			// clean up
			delete [] pakPath; pakPath=NULL;
		}catch( ... ){
			if(pakPath) delete [] pakPath;
			delete pkg;
			throw;
		}
		return pkg;
	}
	// this should never happen. something went wrong so we
	// bail out with an exception
	DSTHROW(dueInvalidAction);
}

// private functions
#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
const char *dsEnginePackageSource::p_FindPackage(const char *name){
	const char *sharedPath;
	struct stat dirStat;
	char *buffer=NULL;
	bool found=false;
	// look in all shared path to find a directory with the given name
	for(int i=0; i<pEngine->GetSharedPathCount(); i++){
		sharedPath = pEngine->GetSharedPath(i);
		try{
			buffer = new char[strlen(sharedPath)+strlen(name)+1];
			if(!buffer) DSTHROW(dueOutOfMemory);
			strcpy(buffer, sharedPath);
			strcat(buffer, name);
			found = (stat(buffer, &dirStat) == 0) && S_ISDIR(dirStat.st_mode);
			delete [] buffer; buffer = NULL;
		}catch( ... ){
			if(buffer) delete [] buffer;
			throw;
		}
		if(found) return sharedPath;
	}
	// nothing found
	return NULL;
}
#elif defined OS_W32
const char *dsEnginePackageSource::p_FindPackage(const char *name){
	const int count = pEngine->GetSharedPathCount();
	bool found = false;
	int i;
	
	// look in all shared path to find a directory with the given name
	for( i=0; i<count; i++ ){
		const char * const sharedPath = pEngine->GetSharedPath( i );
		wchar_t *widePath = NULL;
		char *buffer = NULL;
		
		try{
			const int size = ( int )strlen( sharedPath ) + ( int )strlen( name ) + 1;
			buffer = new char[ size ];
			snprintf( buffer, size, "%s%s", sharedPath, name );
			
			const int widePathLen = MultiByteToWideChar( CP_UTF8,
				MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, buffer, -1, NULL, 0 );
			if( widePathLen == 0 ){
				DSTHROW( dueInvalidParam );
			}
			
			widePath = new wchar_t[ widePathLen ];
			if( ! MultiByteToWideChar( CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
			buffer, -1, widePath, widePathLen ) ){
				DSTHROW( dueInvalidParam );
			}
			
			WIN32_FILE_ATTRIBUTE_DATA fa{};
			found = GetFileAttributesExW(widePath, GetFileExInfoStandard, &fa)
				&& (fa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
			
			delete [] widePath;
			widePath = NULL;
			
			delete [] buffer;
			buffer = NULL;
			
		}catch( ... ){
			if( widePath ){
				delete [] widePath;
			}
			if( buffer ){
				delete [] buffer;
			}
			throw;
		}
		
		if( found ){
			return sharedPath;
		}
	}
	
	// nothing found
	return NULL;
}
#endif

void dsEnginePackageSource::p_AddScripts(dsPackage *pak, const char *path){
	pEngine->GetEngineManager()->AddPackageScriptFiles(*pak, path);
}

#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
void dsEnginePackageSource::p_AddNativeLib(dsPackage *pak, const char *path){
	// library has to be named 'pak-name.so', for example, 'Math.so'
	dsEnginePackages *engPkg = pEngine->GetEnginePackages();
	dsNativePackage *natPak;
	const char *libName = pak->GetName();
	char *newPath=NULL;
	struct stat fileStat;
	try{
		// build name of library. it is fixed so we know what to look for.
		#ifdef OS_MACOS
		newPath = new char[strlen(path)+strlen(libName)+7];
		sprintf(newPath, "%s%s.dylib", path, libName);
		#else
		newPath = new char[strlen(path)+strlen(libName)+4];
		sprintf(newPath, "%s%s.so", path, libName);
		#endif
		// check if file exists
		if((stat(newPath, &fileStat)==0) && S_ISREG(fileStat.st_mode)){
			// try to load file
			natPak = engPkg->GetNativePackage(newPath);
			if(!natPak){
				natPak = new dsNativePackage(newPath);
				if(!natPak) DSTHROW(dueOutOfMemory);
				engPkg->AddNatPackage( natPak );
			}
			// add native classes to package if 
			natPak->LoadIntoPackage(pEngine, pak);
		}
		// clean up
		delete [] newPath; newPath = NULL;
	}catch( ... ){
		if(newPath) delete [] newPath;
		throw;
	}
}
#elif defined OS_W32
void dsEnginePackageSource::p_AddNativeLib(dsPackage *pak, const char *path){
	// library has to be named 'pak-name.dll', for example, 'Math.dll'
	dsEnginePackages &engPkg = *pEngine->GetEnginePackages();
	const char * const libName = pak->GetName();
	wchar_t *widePath = NULL;
	char *newPath = NULL;
	
	try{
		// build name of library. it is fixed so we know what to look for.
		const int size = ( int )strlen( path ) + ( int )strlen( libName ) + 5;
		newPath = new char[ size ];
		snprintf( newPath, size, "%s%s.dll", path, libName );
		
		const int widePathLen = MultiByteToWideChar( CP_UTF8,
			MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, newPath, -1, NULL, 0 );
		if( widePathLen == 0 ){
			DSTHROW( dueInvalidParam );
		}
		
		widePath = new wchar_t[ widePathLen ];
		if( ! MultiByteToWideChar( CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		newPath, -1, widePath, widePathLen ) ){
			DSTHROW( dueInvalidParam );
		}
		
		// check if file exists
		WIN32_FILE_ATTRIBUTE_DATA fa;
		if( ! GetFileAttributesExW( widePath, GetFileExInfoStandard, &fa ) ){
			DSTHROW( dueInvalidParam );
		}
		
		if( ( fa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY ){
			// try to load file
			dsNativePackage *natPak = engPkg.GetNativePackage( newPath );
			if( ! natPak ){
				natPak = new dsNativePackage(newPath);
			}
			
			// add native classes to package
			natPak->LoadIntoPackage( pEngine, pak );
		}
		
		// clean up
		delete [] widePath;
		widePath = NULL;
		
		delete [] newPath;
		newPath = NULL;
		
	}catch( ... ){
		if( widePath ){
			delete [] widePath;
		}
		if( newPath ){
			delete [] newPath;
		}
		throw;
	}
}
#endif
