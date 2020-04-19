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
#ifndef _DSEXCEPTIONTRACEPOINT_H_
#define _DSEXCEPTIONTRACEPOINT_H_

// predefinitions
class dsClass;
class dsFunction;



// includes



// class dsExceptionTracePoint
class dsExceptionTracePoint{
private:
	dsClass *pClass;
	dsFunction *pFunction;
	int pLine;
public:
	// constructor, destructor
	dsExceptionTracePoint( dsClass *callClass, dsFunction *function, int line );
	~dsExceptionTracePoint();
	// management
	inline dsClass *GetClass() const{ return pClass; }
	inline dsFunction *GetFunction() const{ return pFunction; }
	inline int GetLine() const{ return pLine; }
	void Set( dsClass *callClass, dsFunction *function, int line );
};

// end of include only once
#endif

