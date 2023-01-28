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
#include "dragonscript_config.h"
#include "dsFile.h"
#include "exceptions.h"

// class dsFile
/////////////////
// constructor, destructor
dsFile::dsFile(const char *Filename){
	if(!Filename) DSTHROW(dueInvalidParam);
	// store filename
	int size = ( int )strlen( Filename );
	if(!(p_Filename = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Filename, size + 1, Filename, size );
	#else
		strncpy(p_Filename, Filename, size + 1);
	#endif
	p_Filename[ size ] = 0;
	// extract file title from filename
	const char *vFindSlash = strrchr(Filename, '\\');
	vFindSlash = vFindSlash ? (vFindSlash + 1) : (char*)Filename;
	size = ( int )strlen( vFindSlash );
	if(!(p_Filetitle = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Filetitle, size + 1, vFindSlash, size );
	#else
		strncpy(p_Filetitle, vFindSlash, size + 1);
	#endif
	p_Filetitle[ size ] = 0;
	p_File = NULL;
}
dsFile::~dsFile(){
	Close();
	delete [] p_Filetitle;
	delete [] p_Filename;
}
// implementation of dsScriptSource
const char *dsFile::GetName(){
	return (const char *)p_Filetitle;
}
void dsFile::Open(){
	Close();
	// open file
	#ifdef OS_W32_VS
		if( fopen_s( &p_File, p_Filename, "r") ){
			DSTHROW( dueOpenFile );
		}
	#else
		p_File = fopen(p_Filename, "r");
		if(!p_File) DSTHROW(dueOpenFile);
	#endif
}
int dsFile::ReadData(char *Buffer, int Size){
	int vBytes = ( int )fread(Buffer, 1, Size, p_File);
	if(vBytes==0 && ferror(p_File)) DSTHROW(dueReadFile);
	return vBytes;
}
void dsFile::Close(){
	if(p_File){
		fclose(p_File);
		p_File = NULL;
	}
}

