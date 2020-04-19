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
#include "../dsPackage.h"
#include "dsNativePackage.h"
#include "../dsPackageWrapper.h"
#include "../dsEngine.h"
#include "../exceptions.h"


// definitions
typedef bool (*FUNC_CREATEPACKAGE)(dsPackageWrapper*);


// inner class cWrapper
/////////////////////////
dsNativePackage::cWrapper::cWrapper(dsPackage *pak){
	if(!pak) DSTHROW(dueInvalidParam);
	pPackage = pak;
}
dsNativePackage::cWrapper::~cWrapper(){ }
void dsNativePackage::cWrapper::AddNativeClass(dsClass *cls){
	if(!cls) DSTHROW(dueInvalidParam);
	pPackage->AddHostClass(cls);
}

// class dsNativePackage
//////////////////////////
// constructor, destructor
dsNativePackage::dsNativePackage(const char *filename){
	if(!filename) DSTHROW(dueInvalidParam);
	pFilename = NULL;
#ifdef OS_BEOS
	pHandle = 0;
#else
	pHandle = NULL;
#endif
	// init
	try{
		// store filename
		pFilename = new char[strlen(filename)+1];
		if(!pFilename) DSTHROW(dueOutOfMemory);
		strcpy(pFilename, filename);
		// try loading module located at pFilename
#if defined HAVE_DLFCN_H
		pHandle = dlopen(pFilename, RTLD_NOW);
		if( ! pHandle ) DSTHROW_INFO( dueInvalidAction, dlerror() );
#elif defined OS_BEOS
		pHandle = load_add_on( pFilename );
		if( ! pHandle ) DSTHROW_INFO( dueInvalidAction, "load_add_on" );
#elif defined OS_W32
		pHandle = LoadLibrary(pFilename);
		if(!pHandle){
			int err = GetLastError();
			LPVOID lpMsgBuf;
			FormatMessage( 
			    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			    FORMAT_MESSAGE_FROM_SYSTEM | 
			    FORMAT_MESSAGE_IGNORE_INSERTS,
			    NULL,
			    err,
			    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			    (LPTSTR) &lpMsgBuf,
			    0,
			    NULL 
			);
			// Display the string.
			printf( "[error loadlib] %i: %s\n", err, (LPCSTR)lpMsgBuf);
			// Free the buffer.
			LocalFree( lpMsgBuf );

			DSTHROW_INFO(dueInvalidAction, "LoadLibrary");
		}
#endif
		// look for the CreatePackage function
#if defined HAVE_DLFCN_H
		if(!dlsym(pHandle, "CreatePackage")) DSTHROW_INFO(dueInvalidAction, "dlsym");
#elif defined OS_BEOS
		void *location = NULL;
		if( get_image_symbol( pHandle, "CreatePackage", B_SYMBOL_TYPE_TEXT, &location ) != B_OK ) DSTHROW_INFO( dueInvalidAction, "get_image_symbol" );
#elif defined OS_W32
		if(!GetProcAddress(pHandle, "CreatePackage")) DSTHROW_INFO(dueInvalidAction, "GetProcAddress");
#endif
	}catch( ... ){
#if defined HAVE_DLFCN_H
		if(pHandle) dlclose(pHandle);
#elif defined OS_BEOS
		if( pHandle ) unload_add_on( pHandle );
#elif defined OS_W32
		if(pHandle) FreeLibrary(pHandle);
#endif
		if(pFilename) delete [] pFilename;
		throw;
	}
}
dsNativePackage::~dsNativePackage(){
	// close module if a handler exist
	if( pHandle ){
#if defined HAVE_DLFCN_H
		dlclose(pHandle);
		pHandle = NULL;
#elif defined OS_BEOS
		unload_add_on( pHandle );
		pHandle = 0;
#elif defined OS_W32
		FreeLibrary(pHandle);
		pHandle = NULL;
#endif
	}
	// free other memory
	if(pFilename){
		delete [] pFilename;
		pFilename = NULL;
	}
}

// modules management
void dsNativePackage::LoadIntoPackage(dsEngine *engine, dsPackage *package){
	if(!engine || !package) DSTHROW(dueInvalidParam);
	FUNC_CREATEPACKAGE pFuncCreatePackage;
	cWrapper wrapper(package);
	// look for the CreatePackage function
#if defined HAVE_DLFCN_H
	pFuncCreatePackage = (FUNC_CREATEPACKAGE)dlsym(pHandle, "CreatePackage");
#elif defined OS_BEOS
	if( get_image_symbol( pHandle, "CreatePackage", B_SYMBOL_TYPE_TEXT, ( void** )&pFuncCreatePackage ) != B_OK ){
		DSTHROW_INFO( dueInvalidAction, "get_image_symbol" );
	}
#elif defined OS_W32
	pFuncCreatePackage = (FUNC_CREATEPACKAGE)GetProcAddress(pHandle, "CreatePackage");
#endif
	if(!pFuncCreatePackage) DSTHROW_INFO(dueInvalidAction, "dlsym");
	// create package
	if(!pFuncCreatePackage(&wrapper)) DSTHROW_INFO(dueInvalidAction, "CreatePackage()");
}
