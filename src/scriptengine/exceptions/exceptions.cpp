/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2017, Roland Plüss (roland@rptd.ch)
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
#include <string.h>

#include "duException.h"



// Subclass Constructors
//////////////////////////

dueGeneric::dueGeneric( const char *info, const char *file, int line ) :
duException( "Generic", "An Exception has occured", info, file, line ){
}

dueInvalidParam::dueInvalidParam( const char *info, const char *file, int line ) :
duException( "InvalidParam", "Invalid Parameter specified", info, file, line ){
}

dueOutOfMemory::dueOutOfMemory( const char *info, const char *file, int line ) :
duException( "OutOfMemory", "There is not enough Memory left", info, file, line ){
}

dueOutOfBoundary::dueOutOfBoundary( const char *info, const char *file, int line ) :
duException( "OutOfBoundary", "Index is outside allowed boundaries", info, file, line ){
}

dueStackEmpty::dueStackEmpty( const char *info, const char *file, int line ) :
duException( "StackEmpty", "Stack is empty", info, file, line ){
}

dueStackOverflow::dueStackOverflow( const char *info, const char *file, int line ) :
duException( "StackOverflow", "Stack Overflow", info, file, line ){
}

dueDivisionByZero::dueDivisionByZero( const char *info, const char *file, int line ) :
duException( "DivisionByZero", "Division By Zero", info, file, line ){
}

dueNullPointer::dueNullPointer( const char *info, const char *file, int line ) :
duException( "NullPointer", "Null Pointer", info, file, line ){
}

dueInvalidAction::dueInvalidAction( const char *info, const char *file, int line ) :
duException( "InvalidAction", "Invalid Action (internal error)", info, file, line ){
}



dueFileNotFound::dueFileNotFound( const char *info, const char *file, int line ) :
duException( "FileNotFound", "File does not exist", info, file, line ){
}

dueFileExists::dueFileExists( const char *info, const char *file, int line ) :
duException( "FileExists", "File does exist already", info, file, line ){
}

dueOpenFile::dueOpenFile( const char *info, const char *file, int line ) :
duException( "OpenFileFailed", "Open File failed", info, file, line ){
}

dueReadFile::dueReadFile( const char *info, const char *file, int line ) :
duException( "ReadFileFailed", "Can not read from file", info, file, line ){
}

dueWriteFile::dueWriteFile( const char *info, const char *file, int line ) :
duException( "WriteFileFailed", "Can not write to file", info, file, line ){
}



dueTestFailed::dueTestFailed( const char *info, const char *file, int line ) :
duException( "TestCaseFailed", "Test Case failed", info, file, line ){
}

dueAssertion::dueAssertion( const char *info, const char *file, int line ) :
duException( "AssertionException", "Assertion exception has occured", info, file, line ){
}
