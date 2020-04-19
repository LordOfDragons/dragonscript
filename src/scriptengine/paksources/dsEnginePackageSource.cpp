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
#include "../../config.h"
#include "dsEnginePackageSource.h"
#include "../dsEngine.h"
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
	dsPackage *pkg=NULL;
	const char *sharedPath;
	char *pakPath=NULL;
	// try to load package from engine if existing
	engPkg = engPkgList->GetPackage(name);
	if(engPkg){
		return engPkg->CreatePackage();
	}
	// if this is not an internal package it has to be a
	// script package. try to load it
	sharedPath = p_FindPackage(name);
	if(sharedPath){
		pkg = new dsPackage(name);
		if(!pkg) DSTHROW(dueOutOfMemory);
		try{
			// create package
			pakPath = new char[strlen(sharedPath)+strlen(name)+2];
			if(!pakPath) DSTHROW(dueOutOfMemory);
			// add scripts
			sprintf(pakPath, "%s%s%c", sharedPath, name, PATH_SEPARATOR);
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
	const char *sharedPath;
	char *buffer=NULL;
	bool found=false;
	DWORD attribs;
	// look in all shared path to find a directory with the given name
	for(int i=0; i<pEngine->GetSharedPathCount(); i++){
		sharedPath = pEngine->GetSharedPath(i);
		try{
			buffer = new char[strlen(sharedPath)+strlen(name)+1];
			if(!buffer) DSTHROW(dueOutOfMemory);
			sprintf(buffer, "%s%s", sharedPath, name);
			attribs = GetFileAttributes(buffer);
			found = (attribs != INVALID_FILE_ATTRIBUTES) && (attribs & FILE_ATTRIBUTE_DIRECTORY);
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
#endif

#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
void dsEnginePackageSource::p_AddScripts(dsPackage *pak, const char *path){
	dsScriptSource *scriptSource=NULL;
	const char *pattern="ds";
	char *newPath=NULL;
	DIR *dir=NULL;
	dirent *dirEntry;
	struct stat dirStat;
	int len;
	try{
		// open directory
		dir = opendir(path);
		if(!dir) DSTHROW(dseDirectoryNotFound);
		// do the search
		while(true){
			// read next entry
			errno = 0;
			if(!(dirEntry = readdir(dir))){
				if(errno == 0) break;
				DSTHROW(dseDirectoryRead);
			}
			// skip '.' and '..'
			if(strcmp(dirEntry->d_name, ".") == 0) continue;
			if(strcmp(dirEntry->d_name, "..") == 0) continue;
			// create path
			newPath = new char[strlen(path)+strlen(dirEntry->d_name)+2];
			if(!newPath) DSTHROW(dueOutOfMemory);
			sprintf(newPath, "%s%s", path, dirEntry->d_name);
			len = strlen(newPath);
			// check if the file or directory exists
			if(stat(newPath, &dirStat) == 0){
				// check if the file is a directory
				if( S_ISDIR(dirStat.st_mode) ){
					// append slash as this is a directory
					newPath[len++] = PATH_SEPARATOR;
					newPath[len] = '\0';
					// descent into directory recursively
					p_AddScripts(pak, newPath);
				// check if this is a file
				}else if( S_ISREG(dirStat.st_mode) ){
					// check if this a valid script file '*.ds'
					if(p_MatchesExt(dirEntry->d_name, pattern)){
						// add script to package
						scriptSource = new dsFile(newPath);
						if(!scriptSource) DSTHROW(dueOutOfMemory);
						pak->AddScript(scriptSource); scriptSource = NULL;
					}
				}
			}
			// clean up
			delete [] newPath; newPath = NULL;
		}
		// clean up
		closedir(dir); dir=NULL;
	}catch( ... ){
		if(scriptSource) delete scriptSource;
		if(newPath) delete [] newPath;
		if(dir) closedir(dir);
		throw;
	}
}
#elif defined OS_W32
void dsEnginePackageSource::p_AddScripts(dsPackage *pak, const char *path){
	dsScriptSource *scriptSource=NULL;
	const char *searchPattern="*.*";
	const char *matchPattern="ds";
	char *newPath=NULL, *buffer=NULL;
	HANDLE searchHandle=INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA dirEntry;
	try{
		// start search in directory
		newPath = new char[strlen(path)+strlen(searchPattern)+1];
		if(!newPath) DSTHROW(dueOutOfMemory);
		sprintf(newPath, "%s%s", path, searchPattern);
		searchHandle = FindFirstFile(newPath, &dirEntry);
		delete [] newPath; newPath = NULL;
		if(searchHandle == INVALID_HANDLE_VALUE){
		    if(GetLastError() != ERROR_NO_MORE_FILES) DSTHROW(dseDirectoryRead);
		}
		// do the search
		while(true){
			// skip '.' and '..'
			if( (strcmp(dirEntry.cFileName, ".") != 0)
			&& (strcmp(dirEntry.cFileName, "..") != 0) ){
				// check if the file is a directory
				if(dirEntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
					// build pathname
					newPath = new char[strlen(path)+strlen(dirEntry.cFileName)+2];
					if(!newPath) DSTHROW(dueOutOfMemory);
					sprintf(newPath, "%s%s%c", path, dirEntry.cFileName, PATH_SEPARATOR);
					// descent into directory recursively
					p_AddScripts(pak, newPath);
					// clean up
					delete [] newPath; newPath = NULL;
				// otherwise it is a common file of the correct extension
				}else{
					// build pathname
					newPath = new char[strlen(path)+strlen(dirEntry.cFileName)+1];
					if(!newPath) DSTHROW(dueOutOfMemory);
					sprintf(newPath, "%s%s", path, dirEntry.cFileName);
					// check if this a valid script file '*.ds'
					if(p_MatchesExt(dirEntry.cFileName, matchPattern)){
						// add script to package
						scriptSource = new dsFile(newPath);
						if(!scriptSource) DSTHROW(dueOutOfMemory);
						pak->AddScript(scriptSource); scriptSource = NULL;
					}
					// clean up
					delete [] newPath; newPath = NULL;
				}
			}
			// read next entry
			if(!FindNextFile(searchHandle, &dirEntry)){
			    if(GetLastError() == ERROR_NO_MORE_FILES) break;
			    DSTHROW(dseDirectoryRead);
			}
		}
		// clean up
		FindClose(searchHandle); searchHandle=INVALID_HANDLE_VALUE;
	}catch( ... ){
		if(scriptSource) delete scriptSource;
		if(newPath) delete [] newPath;
		if(buffer) delete [] buffer;
		if(searchHandle==INVALID_HANDLE_VALUE) FindClose(searchHandle);
		throw;
	}
}
#endif

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
	dsEnginePackages *engPkg = pEngine->GetEnginePackages();
	dsNativePackage *natPak;
	const char *libName = pak->GetName();
	char *newPath=NULL;
	DWORD attribs;
	try{
		// build name of library. it is fixed so we know what to look for.
		newPath = new char[strlen(path)+strlen(libName)+5];
		if(!newPath) DSTHROW(dueOutOfMemory);
		sprintf(newPath, "%s%s.dll", path, libName);
		// check if file exists
		attribs = GetFileAttributes(newPath);
		if( (attribs!=INVALID_FILE_ATTRIBUTES) && !(attribs&FILE_ATTRIBUTE_DIRECTORY) ){
			// try to load file
			natPak = engPkg->GetNativePackage(newPath);
			if(!natPak){
				natPak = new dsNativePackage(newPath);
				if(!natPak) DSTHROW(dueOutOfMemory);
			}
			// add native classes to package
			natPak->LoadIntoPackage(pEngine, pak);
		}
		// clean up
		delete [] newPath; newPath = NULL;
	}catch( ... ){
		if(newPath) delete [] newPath;
		throw;
	}
}
#endif

bool dsEnginePackageSource::p_MatchesExt(const char *path, const char *ext){
	const char *pathExt = (const char *)strrchr(path, '.');
	if(!pathExt) return false;
	return strcmp(pathExt+1, ext) == 0;
}

