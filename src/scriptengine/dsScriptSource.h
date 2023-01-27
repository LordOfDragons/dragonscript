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
#ifndef _DSSCRIPTSOURCE_H_
#define _DSSCRIPTSOURCE_H_

// includes
#include "utils/dsuArrayable.h"

// class dsScriptSource
class DS_DLL_EXPORT dsScriptSource : public dsuArrayable{
public:
	// get name of script (usually a relative filename)
	virtual const char *GetName() = 0;
	// open script for reading
	virtual void Open() = 0;
	// read data of up to size length into buffer and return size of data read
	virtual int ReadData(char *Buffer, int Size) = 0;
	// close script and do cleanup
	virtual void Close() = 0;
};

// end of include only once
#endif

