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
#ifndef _DSFILE_H_
#define _DSFILE_H_

// includes
#include <stdio.h>
#include "dsScriptSource.h"

// class dsFile
class DS_DLL_EXPORT dsFile : public dsScriptSource{
private:
	char *p_Filename;
	char *p_Filetitle;
	FILE *p_File;
public:
	// constructor, destructor
	dsFile(const char *Filename);
	~dsFile();
	// implementation of dsScriptSource
	const char *GetName();
	void Open();
	int ReadData(char *Buffer, int Size);
	void Close();
};

// end of include only once
#endif

