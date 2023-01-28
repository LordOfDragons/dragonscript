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
#include <string.h>
#include <stdlib.h>
#include "dragonscript_config.h"
#include "dsEngine.h"
#include "dsExceptionTrace.h"
#include "dsExceptionTracePoint.h"
#include "exceptions.h"



// class dsExceptionTrace
///////////////////////////

// constructor, destructor
dsExceptionTrace::dsExceptionTrace(){
	pReason = NULL;
	pPoints = NULL;
	pPointCount = 0;
	pPointSize = 0;
	SetReason( "" );
}
dsExceptionTrace::dsExceptionTrace( const char *reason ){
	pReason = NULL;
	pPoints = NULL;
	pPointCount = 0;
	pPointSize = 0;
	SetReason( reason );
}
dsExceptionTrace::~dsExceptionTrace(){
	RemoveAllPoints();
	if( pPoints ) delete [] pPoints;
	if( pReason ) delete [] pReason;
}

// management
void dsExceptionTrace::SetReason( const char *reason ){
	if( ! reason ) DSTHROW( dueInvalidParam );
	const int size = ( int )strlen( reason );
	char *newStr = new char[ size + 1 ];
	if( ! newStr ) DSTHROW( dueOutOfMemory );
	#ifdef OS_W32_VS
		strncpy_s( newStr, size + 1, reason, size );
	#else
		strncpy( newStr, reason, size );
	#endif
	newStr[ size ] = 0;
	if( pReason ) delete [] pReason;
	pReason = newStr;
}

// trace point management
dsExceptionTracePoint *dsExceptionTrace::GetPointAt( int index ) const{
	if( index < 0 || index >= pPointCount ) DSTHROW( dueInvalidParam );
	return pPoints[ index ];
}
void dsExceptionTrace::AddPoint( dsExceptionTracePoint *point ){
	if( ! point ){
		DSTHROW( dueInvalidParam );
	}
	
	if( pPointCount == 50 ){ // emergency break
		delete point;
		return;
	}
	
	if( pPointCount == pPointSize ){
		int i, newSize = pPointSize * 3 / 2 + 1;
		dsExceptionTracePoint **newArray = new dsExceptionTracePoint*[ newSize ];
		if( ! newArray ) DSTHROW( dueOutOfMemory );
		if( pPoints ){
			for( i=0; i<pPointCount; i++ ) newArray[ i ] = pPoints[ i ];
			delete [] pPoints;
		}
		pPoints = newArray;
		pPointSize = newSize;
	}
	pPoints[ pPointCount ] = point;
	pPointCount++;
}
void dsExceptionTrace::AddPoint( dsClass *callClass, dsFunction *function, int line ){
	if( ! callClass || ! function || line < 0 ) DSTHROW( dueInvalidParam );
	dsExceptionTracePoint *point = NULL;
	try{
		point = new dsExceptionTracePoint( callClass, function, line );
		if( ! point ) DSTHROW( dueOutOfMemory );
		AddPoint( point );
	}catch( ... ){
		if( point ) delete point;
		throw;
	}
}
void dsExceptionTrace::RemoveAllPoints(){
	int i;
	for( i=0; i<pPointCount; i++ ){
		if( pPoints[ i ] ) delete pPoints[ i ];
	}
	pPointCount = 0;
}
