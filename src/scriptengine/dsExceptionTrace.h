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
#ifndef _DSEXCEPTIONTRACE_H_
#define _DSEXCEPTIONTRACE_H_

// predefinitions
class dsExceptionTracePoint;
class dsClass;
class dsFunction;



// includes



// class dsExceptionTrace
class dsExceptionTrace{
private:
	char *pReason;
	dsExceptionTracePoint **pPoints;
	int pPointCount, pPointSize;
public:
	// constructor, destructor
	dsExceptionTrace();
	dsExceptionTrace( const char *reason );
	~dsExceptionTrace();
	// management
	inline const char *GetReason() const{ return ( const char * )pReason; }
	void SetReason( const char *reason );
	// trace point management
	inline int GetPointCount() const{ return pPointCount; }
	dsExceptionTracePoint *GetPointAt( int index ) const;
	void AddPoint( dsExceptionTracePoint *point );
	void AddPoint( dsClass *callClass, dsFunction *function, int line );
	void RemoveAllPoints();
private:
};

// end of include only once
#endif

