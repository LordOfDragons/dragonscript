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


#include "../../scriptengine/libdscript.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include "dsPakSrcLocaleFile.h"
#include "../../scriptengine/dsFile.h"

#ifdef OS_W32_VS
	#include <direct.h>
	#include <windows.h>
	#define DSI_PATH_SEPARATOR '\\'
#else
	#include <unistd.h>
	#define DSI_PATH_SEPARATOR '/'
#endif


// class dsPakSrcLocaleFile
////////////////////////////////
// constructor, destructor
dsPakSrcLocaleFile::dsPakSrcLocaleFile(){
	int bufLen=FILENAME_MAX, pathLen;
	pAppDir = NULL;
	try{
		// determine the current working directory
		while(true){
			pAppDir = new char[bufLen+2];
			if(!pAppDir) DSTHROW(dueOutOfMemory);
			errno = 0;

			#ifdef OS_W32_VS
				wchar_t buffer[ FILENAME_MAX ];
				if( ! _wgetcwd( buffer, FILENAME_MAX ) ){
					DSTHROW( dueInvalidAction );
				}

				size_t size = 0;
				if( wcstombs_s( &size, pAppDir, FILENAME_MAX + 1, buffer, wcslen( buffer ) + 1 ) ){
					DSTHROW( dueInvalidAction );
				}

				break;
			#else
				if(getcwd(pAppDir, bufLen)) break;
			#endif

			delete [] pAppDir;
			pAppDir = NULL;
			bufLen += 100;
		}
		pathLen = ( int )strlen(pAppDir);
		if(pAppDir[pathLen-1] != DSI_PATH_SEPARATOR){
			pAppDir[pathLen] = DSI_PATH_SEPARATOR;
			pAppDir[pathLen+1] = '\0';
		}
	}catch( ... ){
		if(pAppDir) delete [] pAppDir;
		throw;
	}
}
dsPakSrcLocaleFile::~dsPakSrcLocaleFile(){
	if(pAppDir) delete [] pAppDir;
}
// determine if the given name matches an engine
// package, either native or scripted
bool dsPakSrcLocaleFile::CanHandle(const char *name){
	char *deli;
	// check for file sources. they are recognized as having
	// a '.ds' separator at the end
	deli = (char*)strrchr(name, '.');
	if(deli && strcmp(deli, ".ds")==0) return true;
	// not interesting for us
	return false;
}
// loads the given engine package
dsPackage *dsPakSrcLocaleFile::LoadPackage(const char *name){
	dsPackage *scrPkg=NULL;
	const char *ext=strrchr(name, '.');
	dsScriptSource *sourceFile=NULL;
	// create script package
	scrPkg = new dsPackage(name);
	if(!scrPkg) DSTHROW(dueOutOfMemory);
	// check if this is a single script file
	if(strcmp(ext, ".ds") == 0){
		sourceFile = p_FindScriptFile(name);
		if(!sourceFile){
			delete scrPkg; scrPkg=NULL;
		}else{
			scrPkg->AddScript(sourceFile); sourceFile=NULL;
		}
	// everything else is not supported
	}else{
		DSTHROW_INFO(dueInvalidAction, "Invalid File Extension");
	}
	// return what we got
	return scrPkg;
}


/* try to locate script file in the current directory or the
 * shared directory
 */
dsScriptSource *dsPakSrcLocaleFile::p_FindScriptFile(const char *name){
	char *filename=NULL;
	int appDirLen=( int )strlen(pAppDir);
	int nameLen=( int )strlen(name);
	dsScriptSource *sourceFile=NULL;
	struct stat fs;
	try{
		// try to find the file in the current working directory
		filename = new char[appDirLen+nameLen+1];
		if(!filename) DSTHROW(dueOutOfMemory);

		#ifdef OS_W32_VS
			strncpy_s( filename, appDirLen + 1, pAppDir, appDirLen );
			strncpy_s( filename + appDirLen, nameLen + 1, name, nameLen );
			filename[ appDirLen + nameLen ] = 0;
		#else
			strncpy(filename, pAppDir, appDirLen + 1);
			strncpy(filename + appDirLen, name, nameLen + 1);
		#endif
		
		if(stat(filename, &fs) != 0){ // not found
			delete [] filename; filename=NULL;
		}

		// try to find the file in some other place (todo)
		// if(!filename){ ... }
		
		// verify that the file is a regular file
		if( ! filename ){
			DSTHROW( dueInvalidAction );
		}

		#ifdef OS_W32_VS
			TCHAR bufFileName[ 256 ];
			size_t size = 0;
			if( mbstowcs_s( &size, bufFileName, 256, filename, strlen( filename ) + 1 ) ){
				DSTHROW( dueInvalidAction );
			}
			const DWORD fileAttrs = GetFileAttributes( bufFileName );
			if( ( fileAttrs & ( FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY ) ) != 0 ){
				DSTHROW( dueInvalidAction );
			}
		#else
			if( !S_ISREG(fs.st_mode) ) DSTHROW_INFO(dueInvalidAction, "Not A regular file");
		#endif
		// create source
		sourceFile = new dsFile(filename);
		delete [] filename; filename=NULL;
		if(!sourceFile) DSTHROW(dueOutOfMemory);
	}catch( ... ){
		if(filename) delete [] filename;
		if(sourceFile) delete sourceFile;
		throw;
	}
	return sourceFile;
}
