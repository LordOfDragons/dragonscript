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
#include <stdlib.h>

#include "dragonscript_config.h"
#include "dsScriptSource.h"
#include "dsDefaultEngineManager.h"
#include "dsFile.h"
#include "dsPackage.h"
#include "exceptions.h"

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

#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
#	define PATH_SEPARATOR	'/'
#elif defined OS_W32
#	define PATH_SEPARATOR	'\\'
#endif


// class dsDefaultEngineManager
/////////////////////////////////

void dsDefaultEngineManager::OutputMessage(const char *Message){
	printf("%s\n", Message);
}

void dsDefaultEngineManager::OutputWarning(const char *Message, int WarnID,
dsScriptSource *Script, int Line, int Position){
	printf("[WARN#%i] %s(%i/%i): %s\n", WarnID, Script->GetName(), Line, Position, Message);
}

void dsDefaultEngineManager::OutputWarningMore(const char *Message){
	printf("   %s\n", Message);
}

void dsDefaultEngineManager::OutputError(const char *Message, int ErrorID,
dsScriptSource *Script, int Line, int Position){
	printf("[ERR#%i] %s(%i/%i): %s\n", ErrorID, Script->GetName(), Line, Position, Message);
}

void dsDefaultEngineManager::OutputErrorMore(const char *Message){
	printf("   %s\n", Message);
}

bool dsDefaultEngineManager::ContinueParsing(){
	return true;
}

void dsDefaultEngineManager::AddPackageScriptFiles(dsPackage &package, const char *baseDirectory){
#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
	dsScriptSource *scriptSource=NULL;
	const char *pattern="ds";
	char *newPath=NULL;
	DIR *dir=NULL;
	dirent *dirEntry;
	struct stat dirStat;
	int len;
	try{
		// open directory
		dir = opendir(baseDirectory);
		if(!dir){
			return;
		}
		
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
			newPath = new char[strlen(baseDirectory)+strlen(dirEntry->d_name)+2];
			if(!newPath) DSTHROW(dueOutOfMemory);
			sprintf(newPath, "%s%s", baseDirectory, dirEntry->d_name);
			len = strlen(newPath);
			// check if the file or directory exists
			if(stat(newPath, &dirStat) == 0){
				// check if the file is a directory
				if( S_ISDIR(dirStat.st_mode) ){
					// append slash as this is a directory
					newPath[len++] = PATH_SEPARATOR;
					newPath[len] = '\0';
					// descent into directory recursively
					AddPackageScriptFiles(package, newPath);
				// check if this is a file
				}else if( S_ISREG(dirStat.st_mode) ){
					// check if this a valid script file '*.ds'
					if(MatchesExtension(dirEntry->d_name, pattern)){
						// add script to package
						scriptSource = new dsFile(newPath);
						if(!scriptSource) DSTHROW(dueOutOfMemory);
						package.AddScript(scriptSource); scriptSource = NULL;
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
	
#elif defined OS_W32
	dsScriptSource *scriptSource=NULL;
	const char *searchPattern="*.*";
	const char *matchPattern="ds";
	char *newPath=NULL, *buffer=NULL;
	wchar_t *widePath = NULL;
	char *utf8 = NULL;
	HANDLE searchHandle=INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAW dirEntry;
	try{
		// start search in directory
		const int size = ( int )strlen(baseDirectory) + ( int )strlen(searchPattern) + 1;
		newPath = new char[size];
		if(!newPath) DSTHROW(dueOutOfMemory);
		snprintf(newPath, size, "%s%s", baseDirectory, searchPattern);
		
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
		
		searchHandle = FindFirstFileW( widePath, &dirEntry );
		
		delete [] widePath;
		widePath = NULL;
		
		delete [] newPath; newPath = NULL;
		if(searchHandle == INVALID_HANDLE_VALUE){
			return;
		}
		// do the search
		while(true){
			const int utf8Len = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS,
				dirEntry.cFileName, -1, NULL, 0, NULL, NULL );
			if( utf8Len == 0 ){
				DSTHROW( dueInvalidParam );
			}
			
			utf8 = new char[ utf8Len ];
			if( ! WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS,
			dirEntry.cFileName, -1, utf8, utf8Len, NULL, NULL ) ){
				DSTHROW( dueInvalidParam );
			}
			
			// skip '.' and '..'
			if( strcmp( utf8, "." ) != 0  && strcmp( utf8, ".." ) != 0 ){
				// check if the file is a directory
				if(dirEntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
					// build pathname
					const int size = ( int )strlen(baseDirectory) + ( int )strlen(utf8) + 2;
					newPath = new char[size];
					if(!newPath) DSTHROW(dueOutOfMemory);
					snprintf(newPath, size, "%s%s%c", baseDirectory, utf8, PATH_SEPARATOR);
					// descent into directory recursively
					AddPackageScriptFiles(package, newPath);
					// clean up
					delete [] newPath; newPath = NULL;
				// otherwise it is a common file of the correct extension
				}else{
					// build pathname
					const int size = ( int )strlen(baseDirectory) + ( int )strlen(utf8) + 1;
					newPath = new char[size];
					if(!newPath) DSTHROW(dueOutOfMemory);
					snprintf(newPath, size, "%s%s", baseDirectory, utf8);
					// check if this a valid script file '*.ds'
					if(MatchesExtension(utf8, matchPattern)){
						// add script to package
						scriptSource = new dsFile(newPath);
						if(!scriptSource) DSTHROW(dueOutOfMemory);
						package.AddScript(scriptSource); scriptSource = NULL;
					}
					// clean up
					delete [] newPath; newPath = NULL;
				}
			}
			
			delete [] utf8;
			utf8 = NULL;
			
			// read next entry
			if(!FindNextFileW(searchHandle, &dirEntry)){
			    if(GetLastError() == ERROR_NO_MORE_FILES) break;
			    DSTHROW(dseDirectoryRead);
			}
		}
		// clean up
		FindClose(searchHandle); searchHandle=INVALID_HANDLE_VALUE;
	}catch( ... ){
		if(scriptSource) delete scriptSource;
		if( widePath ){
			delete [] widePath;
		}
		if( utf8 ){
			delete [] utf8;
		}
		if(newPath) delete [] newPath;
		if(buffer) delete [] buffer;
		if(searchHandle==INVALID_HANDLE_VALUE) FindClose(searchHandle);
		throw;
	}
	#endif
}

bool dsDefaultEngineManager::MatchesExtension(const char *path, const char *extension) const{
	const char *pathExt = (const char *)strrchr(path, '.');
	if(!pathExt) return false;
	return strcmp(pathExt+1, extension) == 0;
}
