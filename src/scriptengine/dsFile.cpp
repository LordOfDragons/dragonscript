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
#include "../config.h"
#include "dsFile.h"
#include "exceptions.h"

// class dsFile
/////////////////
// constructor, destructor
dsFile::dsFile(const char *Filename){
	if(!Filename) DSTHROW(dueInvalidParam);
	// store filename
	if(!(p_Filename = new char[strlen(Filename)+1])) DSTHROW(dueOutOfMemory);
	strcpy(p_Filename, Filename);
	// extract file title from filename
	const char *vFindSlash = strrchr(Filename, '\\');
	vFindSlash = vFindSlash ? (vFindSlash + 1) : (char*)Filename;
	if(!(p_Filetitle = new char[strlen(vFindSlash)+1])) DSTHROW(dueOutOfMemory);
	strcpy(p_Filetitle, vFindSlash);
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
	p_File = fopen(p_Filename, "r");
	if(!p_File) DSTHROW(dueOpenFile);
}
int dsFile::ReadData(char *Buffer, int Size){
	int vBytes = fread(Buffer, 1, Size, p_File);
	if(vBytes==0 && ferror(p_File)) DSTHROW(dueReadFile);
	return vBytes;
}
void dsFile::Close(){
	if(p_File){
		fclose(p_File);
		p_File = NULL;
	}
}

