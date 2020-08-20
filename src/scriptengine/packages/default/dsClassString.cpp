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



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "../../../config.h"

#include "dsClassString.h"
#include "dsClassArray.h"
#include "dsClassObject.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// native data structure
struct sStrNatData{
	char *str;
};

// Native Functions
/////////////////////

// public func new()
dsClassString::nfCreate::nfCreate( const sInitData &init ) : dsFunction( init.clsStr, "new",
DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassString::nfCreate::RunFunction( dsRunTime *rt, dsValue *myself ){
	sStrNatData &nd = *( ( sStrNatData* )p_GetNativeData( myself ) );
	
	nd.str = NULL;
	
	char * const nstr = new char[ 1 ];
	if( ! nstr ){
		DSTHROW( dueOutOfMemory );
	}
	nstr[ 0 ] = '\0';
	nd.str = nstr;
}

// public func new( byte character, int count )
dsClassString::nfCreateFilled::nfCreateFilled( const sInitData &init ) : dsFunction( init.clsStr,
"new", DSFT_CONSTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsByte ); // character
	p_AddParameter( init.clsInt ); // count
}
void dsClassString::nfCreateFilled::RunFunction( dsRunTime *rt, dsValue *myself ){
	sStrNatData &nd = *( ( sStrNatData* )p_GetNativeData( myself ) );
	
	const byte character = rt->GetValue( 0 )->GetByte();
	const int count = rt->GetValue( 1 )->GetInt();
	
	if( count < 0 ){
		DSTHROW( dueInvalidParam );
	}
	
	char * const nstr = new char[ count + 1 ];
	if( ! nstr ){
		DSTHROW( dueOutOfMemory );
	}
	memset( nstr, character, count );
	nstr[ count ] = '\0';
	nd.str = nstr;
}

// func destructor()
dsClassString::nfDestructor::nfDestructor( const sInitData &init ) : dsFunction( init.clsStr,
"destructor", DSFT_DESTRUCTOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassString::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
	sStrNatData &nd = *( ( sStrNatData* )p_GetNativeData( myself ) );
	
	if( nd.str ){
		delete [] nd.str;
		nd.str = NULL;
	}
}



// public func bool empty()
dsClassString::nfEmpty::nfEmpty( const sInitData &init ) : dsFunction( init.clsStr, "empty",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
}
void dsClassString::nfEmpty::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	
	rt->PushBool( str[ 0 ] == '\0' );
}

// public func int getLength()
dsClassString::nfGetLength::nfGetLength( const sInitData &init ) : dsFunction( init.clsStr, "getLength",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassString::nfGetLength::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	
	rt->PushInt( strlen( str ) );
}

// public func byte getAt( int index )
dsClassString::nfGetAt::nfGetAt( const sInitData &init ) : dsFunction( init.clsStr,
"getAt", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsByte ){
	p_AddParameter( init.clsInt ); // index
}
void dsClassString::nfGetAt::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	int index = rt->GetValue( 0 )->GetInt();
	const int len = strlen( str );
	
	if( index < 0 ){
		index += len;
	}
	if( ( index < 0 ) || ( index >= len ) ){
		DSTHROW( dueOutOfBoundary );
	}
	
	rt->PushByte( str[ index ] );
}

// get substring
static const char *substring( const char * const str, int start, int end ){
	const int len = strlen( str );
	
	char *newstr = NULL;
	
	if( start < 0 ){
		start = len + start;
	}
	if( end < 0 ){
		end = len + end;
	}
	
	if( start < 0 ){
		start = 0;
	}
	if( end > len ){
		end = len;
	}
	
	if( start < end ){
		const int count = end - start;
		
		newstr = new char[ count + 1 ];
		strncpy( newstr, str + start, count );
		newstr[ count ] = '\0';
	}
	
	if( ! newstr ){
		newstr = new char[ 1 ];
		newstr[ 0 ] = '\0';
	}
	
	return newstr;
}

// public func String substring( int start )
dsClassString::nfSubString::nfSubString( const sInitData &init ) : dsFunction( init.clsStr,
"substring", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsInt ); // start
}
void dsClassString::nfSubString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int start = rt->GetValue( 0 )->GetInt();
	const char * const newstr = substring( str, start, strlen( str ) );
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	
	delete [] newstr;
}

// public func String substring( int start, int end )
dsClassString::nfSubString2::nfSubString2( const sInitData &init ) : dsFunction( init.clsStr,
"substring", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsInt ); // start
	p_AddParameter( init.clsInt ); // end
}
void dsClassString::nfSubString2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int start = rt->GetValue( 0 )->GetInt();
	const int end = rt->GetValue( 1 )->GetInt();
	const char * const newstr = substring( str, start, end );
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	
	delete [] newstr;
}



// find character in string
static int findInString( const char * const str, int character, int start, int end ){
	const int len = strlen( str );
	int i;
	
	if( start < 0 ){
		start = len + start;
	}
	if( end < 0 ){
		end = len + end;
	}
	
	if( start < 0 ){
		start = 0;
	}
	if( end > len ){
		end = len;
	}
	
	for( i=start; i<end; i++ ){
		if( str[ i ] == character ){
			return i;
		}
	}
	
	return -1;
}

// public func int find( byte character )
dsClassString::nfFind::nfFind( const sInitData &init ) : dsFunction( init.clsStr,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsByte ); // character
}
void dsClassString::nfFind::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	rt->PushInt( findInString( str, character, 0, strlen( str ) ) );
}

// public func int find( byte character, int start )
dsClassString::nfFind2::nfFind2( const sInitData &init ) : dsFunction( init.clsStr,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsByte ); // character
	p_AddParameter( init.clsInt ); // start
}
void dsClassString::nfFind2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	const int start = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findInString( str, character, start, strlen( str ) ) );
}

// public func int find( byte character, int start, int end )
dsClassString::nfFind3::nfFind3( const sInitData &init ) : dsFunction( init.clsStr,
"find", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsByte ); // character
	p_AddParameter( init.clsInt ); // start
	p_AddParameter( init.clsInt ); // end
}
void dsClassString::nfFind3::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	const int start = rt->GetValue( 1 )->GetInt();
	const int end = rt->GetValue( 2 )->GetInt();
	rt->PushInt( findInString( str, character, start, end ) );
}

// fins character using a list of characters
static int findAnyInString( const char * const str, const char * const characters, int start, int end ){
	const int ccount = strlen( characters );
	int i, found, foundBest = -1;
	
	for( i=0; i<ccount; i++ ){
		found = findInString( str, characters[ i ], start, end );
		
		if( found != -1 && ( foundBest == -1 || found < foundBest ) ){
			foundBest = found;
		}
	}
	
	return foundBest;
}

// public func int findAny( String characters )
dsClassString::nfFindAny::nfFindAny( const sInitData &init ) : dsFunction( init.clsStr,
"findAny", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // characters
}
void dsClassString::nfFindAny::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const characters = rt->GetValue( 0 )->GetString();
	rt->PushInt( findAnyInString( str, characters, 0, strlen( str ) ) );
}

// public func int findAny( String characters, int start )
dsClassString::nfFindAny2::nfFindAny2( const sInitData &init ) : dsFunction( init.clsStr,
"findAny", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // characters
	p_AddParameter( init.clsInt ); // start
}
void dsClassString::nfFindAny2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const characters = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findAnyInString( str, characters, start, strlen( str ) ) );
}

// public func int findAny( String characters, int start, int end )
dsClassString::nfFindAny3::nfFindAny3( const sInitData &init ) : dsFunction( init.clsStr,
"findAny", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // characters
	p_AddParameter( init.clsInt ); // start
	p_AddParameter( init.clsInt ); // end
}
void dsClassString::nfFindAny3::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const characters = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	const int end = rt->GetValue( 2 )->GetInt();
	rt->PushInt( findAnyInString( str, characters, start, end ) );
}

// find character in string
static int findStringInString( const char * const str, const char * const findstr, int start, int end ){
	const int slen = strlen( findstr );
	const int len = strlen( str ) - slen + 1;
	int i;
	
	if( len < 0 ){
		return -1;
	}
	
	if( start < 0 ){
		start = len + start;
	}
	if( end < 0 ){
		end = len + end;
	}
	
	if( start < 0 ){
		start = 0;
	}
	if( end > len ){
		end = len;
	}
	
	for( i=start; i<end; i++ ){
		if( strncmp( str + i, findstr, slen ) == 0 ){
			return i;
		}
	}
	
	return -1;
}

// public func int findString( String string )
dsClassString::nfFindString::nfFindString( const sInitData &init ) : dsFunction( init.clsStr,
"findString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // string
}
void dsClassString::nfFindString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const findstr = rt->GetValue( 0 )->GetString();
	rt->PushInt( findStringInString( str, findstr, 0, strlen( str ) ) );
}

// public func int findString( String string, int start )
dsClassString::nfFindString2::nfFindString2( const sInitData &init ) : dsFunction( init.clsStr,
"findString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // string
	p_AddParameter( init.clsInt ); // start
}
void dsClassString::nfFindString2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const findstr = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findStringInString( str, findstr, start, strlen( str ) ) );
}

// public func int findString( String string, int start, int end )
dsClassString::nfFindString3::nfFindString3( const sInitData &init ) : dsFunction( init.clsStr,
"findString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // string
	p_AddParameter( init.clsInt ); // start
	p_AddParameter( init.clsInt ); // end
}
void dsClassString::nfFindString3::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const findstr = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	const int end = rt->GetValue( 2 )->GetInt();
	rt->PushInt( findStringInString( str, findstr, start, end ) );
}

// find character in string in reverse order
static int findInStringReverse( const char * const str, int character, int start, int end ){
	const int len = strlen( str );
	int i;
	
	if( start < 0 ){
		start = len + start;
	}
	if( end < 0 ){
		end = len + end;
	}
	
	if( start < 0 ){
		start = 0;
	}
	if( end > len ){
		end = len;
	}
	
	for( i=end-1; i>=start; i-- ){
		if( str[ i ] == character ){
			return i;
		}
	}
	
	return -1;
}

// public func int findReverse( byte character )
dsClassString::nfFindReverse::nfFindReverse( const sInitData &init ) : dsFunction( init.clsStr,
"findReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsByte ); // character
}
void dsClassString::nfFindReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	rt->PushInt( findInStringReverse( str, character, 0, strlen( str ) ) );
}

// public func int findReverse( byte character, int start )
dsClassString::nfFindReverse2::nfFindReverse2( const sInitData &init ) : dsFunction( init.clsStr,
"findReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsByte ); // character
	p_AddParameter( init.clsInt ); // start
}
void dsClassString::nfFindReverse2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	const int start = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findInStringReverse( str, character, start, strlen( str ) ) );
}

// public func int findReverse( byte character, int start, int end )
dsClassString::nfFindReverse3::nfFindReverse3( const sInitData &init ) : dsFunction( init.clsStr,
"findReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsByte ); // character
	p_AddParameter( init.clsInt ); // start
	p_AddParameter( init.clsInt ); // end
}
void dsClassString::nfFindReverse3::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	const int start = rt->GetValue( 1 )->GetInt();
	const int end = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findInStringReverse( str, character, start, end ) );
}

// find any character in string in reverse
static int findAnyInStringReverse( const char * const str, const char * const characters, int start, int end ){
	const int ccount = strlen( characters );
	int i, found, foundBest = -1;
	
	for( i=0; i<ccount; i++ ){
		found = findInStringReverse( str, characters[ i ], start, end );
		
		if( found != -1 && found > foundBest ){
			foundBest = found;
		}
	}
	
	return foundBest;
}

// public func int findAnyReverse( String characters )
dsClassString::nfFindAnyReverse::nfFindAnyReverse( const sInitData &init ) : dsFunction( init.clsStr,
"findAnyReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // characters
}
void dsClassString::nfFindAnyReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const characters = rt->GetValue( 0 )->GetString();
	rt->PushInt( findAnyInStringReverse( str, characters, 0, strlen( str ) ) );
}

// public func int findAnyReverse( String characters, int start )
dsClassString::nfFindAnyReverse2::nfFindAnyReverse2( const sInitData &init ) : dsFunction( init.clsStr,
"findAnyReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // characters
	p_AddParameter( init.clsInt ); // start
}
void dsClassString::nfFindAnyReverse2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const characters = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findAnyInStringReverse( str, characters, start, strlen( str ) ) );
}

// public func int findAnyReverse( String characters, int start, int end )
dsClassString::nfFindAnyReverse3::nfFindAnyReverse3( const sInitData &init ) : dsFunction( init.clsStr,
"findAnyReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // characters
	p_AddParameter( init.clsInt ); // start
	p_AddParameter( init.clsInt ); // end
}
void dsClassString::nfFindAnyReverse3::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const characters = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	const int end = rt->GetValue( 2 )->GetInt();
	rt->PushInt( findAnyInStringReverse( str, characters, start, end ) );
}

// find character in string in reverse order
static int findStringInStringReverse( const char * const str, const char * const findstr, int start, int end ){
	const int slen = strlen( findstr );
	const int len = strlen( str ) - slen + 1;
	int i;
	
	if( len < 0 ){
		return -1;
	}
	
	if( start < 0 ){
		start = len + start;
	}
	if( end < 0 ){
		end = len + end;
	}
	
	if( start < 0 ){
		start = 0;
	}
	if( end > len ){
		end = len;
	}
	
	for( i=end-1; i>=start; i-- ){
		if( strncmp( str + i, findstr, slen ) == 0 ){
			return i;
		}
	}
	
	return -1;
}

// public func int findStringReverse( String string )
dsClassString::nfFindStringReverse::nfFindStringReverse( const sInitData &init ) : dsFunction( init.clsStr,
"findStringReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // string
}
void dsClassString::nfFindStringReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const findstr = rt->GetValue( 0 )->GetString();
	rt->PushInt( findStringInStringReverse( str, findstr, 0, strlen( str ) ) );
}

// public func int findStringReverse( String string, int start )
dsClassString::nfFindStringReverse2::nfFindStringReverse2( const sInitData &init ) : dsFunction( init.clsStr,
"findStringReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // string
	p_AddParameter( init.clsInt ); // start
}
void dsClassString::nfFindStringReverse2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const findstr = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findStringInStringReverse( str, findstr, start, strlen( str ) ) );
}

// public func int findStringReverse( String string, int start, int end )
dsClassString::nfFindStringReverse3::nfFindStringReverse3( const sInitData &init ) : dsFunction( init.clsStr,
"findStringReverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // string
	p_AddParameter( init.clsInt ); // start
	p_AddParameter( init.clsInt ); // end
}
void dsClassString::nfFindStringReverse3::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const findstr = rt->GetValue( 0 )->GetString();
	const int start = rt->GetValue( 1 )->GetInt();
	const int end = rt->GetValue( 1 )->GetInt();
	rt->PushInt( findStringInStringReverse( str, findstr, start, end ) );
}



// public func String reverse()
dsClassString::nfReverse::nfReverse( const sInitData &init ) : dsFunction( init.clsStr,
"reverse", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassString::nfReverse::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int len = strlen( str );
	
	char * const newstr = new char[ len + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	
	int i;
	for( i=0; i<len; i++ ){
		newstr[ i ] = str[ len - 1 - i ];
	}
	newstr[ len ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	
	delete [] newstr;
}

// public func Array split( byte character )
dsClassString::nfSplit::nfSplit( const sInitData &init ) : dsFunction( init.clsStr,
"split", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsByte ); // character
}
void dsClassString::nfSplit::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassString * const clsString = ( dsClassString* )GetOwnerClass();
	dsClassArray * const clsArray = ( dsClassArray* )rt->GetEngine()->GetClassArray();
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	const int len = strlen( str );
	const char *splitstr = NULL;
	dsValue *valsplitstr = NULL;
	dsValue *vallist = NULL;
	int i, start = -1;
	
	try{
		vallist = clsArray->CreateArray( rt );
		valsplitstr = rt->CreateValue( clsString );
		
		for( i=0; i<len; i++ ){
			if( start == -1 ){
				if( str[ i ] != character ){
					start = i;
				}
				
			}else{
				if( str[ i ] == character ){
					splitstr = substring( str, start, i );
					rt->SetString( valsplitstr, splitstr );
					delete [] splitstr;
					splitstr = NULL;
					
					clsArray->AddObject( rt, vallist->GetRealObject(), valsplitstr );
					
					start = -1;
				}
			}
		}
		
		if( start != -1 ){
			splitstr = substring( str, start, len );
			rt->SetString( valsplitstr, splitstr );
			delete [] splitstr;
			splitstr = NULL;
			
			clsArray->AddObject( rt, vallist->GetRealObject(), valsplitstr );
		}
		
		// push the result and clean up
		rt->FreeValue( valsplitstr );
		valsplitstr = NULL;
		
		rt->PushValue( vallist );
		rt->FreeValue( vallist );
		
	}catch( ... ){
		if( splitstr ){
			delete [] splitstr;
		}
		if( valsplitstr ){
			rt->FreeValue( valsplitstr );
		}
		if( vallist ){
			rt->FreeValue( vallist );
		}
		throw;
	}
}

// public func Array split( String characters )
dsClassString::nfSplit2::nfSplit2( const sInitData &init ) : dsFunction( init.clsStr,
"split", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsArr ){
	p_AddParameter( init.clsStr ); // characters
}
void dsClassString::nfSplit2::RunFunction( dsRunTime *rt, dsValue *myself ){
	dsClassString * const clsString = ( dsClassString* )GetOwnerClass();
	dsClassArray * const clsArray = ( dsClassArray* )rt->GetEngine()->GetClassArray();
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const characters = rt->GetValue( 0 )->GetString();
	const int clen = strlen( characters );
	const int len = strlen( str );
	const char *splitstr = NULL;
	dsValue *valsplitstr = NULL;
	dsValue *vallist = NULL;
	int i, j, start = -1;
	
	try{
		vallist = clsArray->CreateArray( rt );
		valsplitstr = rt->CreateValue( clsString );
		
		for( i=0; i<len; i++ ){
			if( start == -1 ){
				for( j=0; j<clen; j++ ){
					if( str[ i ] == characters[ j ] ){
						break;
					}
				}
				if( j == clen ){
					start = i;
				}
				
			}else{
				for( j=0; j<clen; j++ ){
					if( str[ i ] == characters[ j ] ){
						splitstr = substring( str, start, i );
						rt->SetString( valsplitstr, splitstr );
						delete [] splitstr;
						splitstr = NULL;
						
						clsArray->AddObject( rt, vallist->GetRealObject(), valsplitstr );
						
						start = -1;
						break;
					}
				}
			}
		}
		
		if( start != -1 ){
			splitstr = substring( str, start, len );
			rt->SetString( valsplitstr, splitstr );
			delete [] splitstr;
			splitstr = NULL;
			
			clsArray->AddObject( rt, vallist->GetRealObject(), valsplitstr );
		}
		
		// push the result and clean up
		rt->FreeValue( valsplitstr );
		valsplitstr = NULL;
		
		rt->PushValue( vallist );
		rt->FreeValue( vallist );
		
	}catch( ... ){
		if( splitstr ){
			delete [] splitstr;
		}
		if( valsplitstr ){
			rt->FreeValue( valsplitstr );
		}
		if( vallist ){
			rt->FreeValue( vallist );
		}
		throw;
	}
}

// public func String replace( byte replace, byte with )
dsClassString::nfReplace::nfReplace( const sInitData &init ) : dsFunction( init.clsStr,
"replace", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsByte ); // replace
	p_AddParameter( init.clsByte ); // with
}
void dsClassString::nfReplace::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte replace = rt->GetValue( 0 )->GetByte();
	const byte with = rt->GetValue( 1 )->GetByte();
	const int len = strlen( str );
	
	char * const newstr = new char[ len + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	
	int i;
	for( i=0; i<len; i++ ){
		if( str[ i ] == replace ){
			newstr[ i ] = with;
			
		}else{
			newstr[ i ] = str[ i ];
		}
	}
	newstr[ len ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	
	delete [] newstr;
}

// public func String replace2( String replace, byte with )
dsClassString::nfReplace2::nfReplace2( const sInitData &init ) : dsFunction( init.clsStr,
"replace", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsStr ); // replace
	p_AddParameter( init.clsByte ); // with
}
void dsClassString::nfReplace2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const replace = rt->GetValue( 0 )->GetString();
	const byte with = rt->GetValue( 1 )->GetByte();
	const int rlen = strlen( replace );
	const int len = strlen( str );
	
	char * const newstr = new char[ len + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	
	int i, j;
	for( i=0; i<len; i++ ){
		for( j=0; j<rlen; j++ ){
			if( str[ i ] == replace[ j ] ){
				break;
			}
		}
		
		if( j == rlen ){
			newstr[ i ] = str[ i ];
			
		}else{
			newstr[ i ] = with;
		}
	}
	newstr[ len ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	
	delete [] newstr;
}

// public func String replaceString( String replace, String with )
dsClassString::nfReplaceString::nfReplaceString( const sInitData &init ) : dsFunction( init.clsStr,
"replaceString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsStr ); // replace
	p_AddParameter( init.clsStr ); // with
}
void dsClassString::nfReplaceString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const char * const replace = rt->GetValue( 0 )->GetString();
	const char * const with = rt->GetValue( 1 )->GetString();
	const int reallen = strlen( str );
	const int rlen = strlen( replace );
	const int len = reallen - rlen + 1;
	
	if( rlen == 0 || len <= 0 ){
		rt->PushValue( myself );
		return;
	}
	
	const int wlen = strlen( with );
	const int difflen = wlen - rlen;
	int newlen = reallen;
	int i, npos;
	
	for( i=0; i<len; i++ ){
		if( strncmp( str + i, replace, rlen ) == 0 ){
			i += rlen - 1;
			newlen += difflen;
		}
	}
	
	char * const newstr = new char[ newlen + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	
	for( npos=0, i=0; i<len; i++ ){
		if( strncmp( str + i, replace, rlen ) == 0 ){
			if( wlen > 0 ){
				strncpy( newstr + npos, with, wlen );
				npos += wlen;
			}
			i += rlen - 1;
			
		}else{
			newstr[ npos++ ] = str[ i ];
		}
	}
	for( ; i<reallen; i++ ){
		newstr[ npos++ ] = str[ i ];
	}
	
	newstr[ newlen ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	
	delete [] newstr;
}



// public func String trimLeft()
dsClassString::nfTrimLeft::nfTrimLeft( const sInitData &init ) : dsFunction( init.clsStr,
"trimLeft", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassString::nfTrimLeft::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int len = strlen( str );
	int i;
	
	for( i=0; i<len; i++ ){
		const char character = str[ i ];
		if( ! ( ( ( character >= 0x9 ) && ( character <= 0xD ) ) || ( character == 0x20 ) ) ){
			break;
		}
	}
	
	char * const newstr = new char[ len - i + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	strncpy( newstr, str + i, len - i );
	newstr[ len - i ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String trimRight()
dsClassString::nfTrimRight::nfTrimRight( const sInitData &init ) : dsFunction( init.clsStr,
"trimRight", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassString::nfTrimRight::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int len = strlen( str );
	int i;
	
	for( i=len-1; i>=0; i-- ){
		const char character = str[ i ];
		if( ! ( ( ( character >= 0x9 ) && ( character <= 0xD ) ) || ( character == 0x20 ) ) ){
			break;
		}
	}
	
	char * const newstr = new char[ i + 2 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	strncpy( newstr, str, i + 1 );
	newstr[ i + 1 ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String trimBoth()
dsClassString::nfTrimBoth::nfTrimBoth( const sInitData &init ) : dsFunction( init.clsStr,
"trimBoth", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassString::nfTrimBoth::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int len = strlen( str );
	int i, j;
	
	for( i=0; i<len; i++ ){
		const char character = str[ i ];
		if( ! ( ( ( character >= 0x9 ) && ( character <= 0xD ) ) || ( character == 0x20 ) ) ){
			break;
		}
	}
	for( j=len-1; j>=0; j-- ){
		const char character = str[ j ];
		if( ! ( ( ( character >= 0x9 ) && ( character <= 0xD ) ) || ( character == 0x20 ) ) ){
			break;
		}
	}
	
	char * const newstr = new char[ j - i + 2 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	strncpy( newstr, str + i, j - i + 1 );
	newstr[ j - i + 1 ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String toLower()
dsClassString::nfToLower::nfToLower( const sInitData &init ) : dsFunction( init.clsStr,
"toLower", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassString::nfToLower::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int len = strlen( str );
	int i;
	
	char * const newstr = new char[ len + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	for( i=0; i<len; i++ ){
		newstr[ i ] = ( char )tolower( str[ i ] );
	}
	
	newstr[ len ] = '\0';
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String toUpper()
dsClassString::nfToUpper::nfToUpper( const sInitData &init ) : dsFunction( init.clsStr,
"toUpper", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassString::nfToUpper::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int len = strlen( str );
	int i;
	
	char * const newstr = new char[ len + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	for( i=0; i<len; i++ ){
		newstr[ i ] = ( char )toupper( str[ i ] );
	}
	
	newstr[ len ] = '\0';
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func int toInt()
dsClassString::nfToInt::nfToInt( const sInitData &init ) : dsFunction( init.clsStr, "toInt",
DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}
void dsClassString::nfToInt::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	rt->PushInt( ( int )strtol( str, NULL, 10 ) );
}

// public func float toFloat()
dsClassString::nfToFloat::nfToFloat( const sInitData &init ) : dsFunction( init.clsStr,
"toFloat", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsFlt ){
}
void dsClassString::nfToFloat::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	rt->PushFloat( strtof( str, NULL ) );
}



// public func int compare( Object other )
dsClassString::nfCompare::nfCompare( const sInitData &init ) : dsFunction( init.clsStr,
"compare", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsObj ); // other
}
void dsClassString::nfCompare::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsClass * const clsStr = ( dsClassString* )GetOwnerClass();
	dsValue * const valother = rt->GetValue( 0 );
	
	if( ! valother->GetRealObject() ){
		DSTHROW( dueNullPointer );
	}
	
	if( valother->GetType()->GetPrimitiveType() == DSPT_OBJECT && valother->GetRealObject()->GetType() == clsStr ){
		const char * const otherstr = valother->GetString();
		rt->PushInt( strcmp( str, otherstr ) );
		
	}else{
		DSTHROW( dueInvalidParam );
	}
}

// public func int compareNoCase( String other )
dsClassString::nfCompareNoCase::nfCompareNoCase( const sInitData &init ) : dsFunction(init.clsStr,
"compareNoCase", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
	p_AddParameter( init.clsStr ); // other
}
void dsClassString::nfCompareNoCase::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsValue * const valother = rt->GetValue( 0 );
	
	if( ! valother->GetRealObject() ){
		DSTHROW( dueNullPointer );
	}
	
	char * const otherstr = ( ( sStrNatData* )p_GetNativeData( valother ) )->str;
	const int len = strlen( str );
	int i;
	
	for( i=0; i<=len; i++ ){
		const char character = ( char )tolower( str[ i ] );
		const char newcharacter = ( char )tolower( otherstr[ i ] );
		if( character != newcharacter ){
			rt->PushInt( character < newcharacter ? -1 : 1 );
			return;
		}
	}
	
	rt->PushInt( 0 );
}

// public func bool startsWith( String string )
dsClassString::nfStartsWith::nfStartsWith( const sInitData &init ) :
dsFunction( init.clsStr, "startsWith", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsStr ); // string
}
void dsClassString::nfStartsWith::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsValue * const valother = rt->GetValue( 0 );
	if( ! valother->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "string" );
	}
	const char * const otherstr = ( ( sStrNatData* )p_GetNativeData( valother ) )->str;
	
	const size_t lenOther = strlen( otherstr );
	rt->PushBool( lenOther <= strlen( str ) && strncmp( str, otherstr, lenOther ) == 0 );
}

// public func bool startsWithNoCase( String string )
dsClassString::nfStartsWithNoCase::nfStartsWithNoCase( const sInitData &init ) :
dsFunction( init.clsStr, "startsWithNoCase", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsStr ); // string
}
void dsClassString::nfStartsWithNoCase::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsValue * const valother = rt->GetValue( 0 );
	if( ! valother->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "string" );
	}
	const char * const otherstr = ( ( sStrNatData* )p_GetNativeData( valother ) )->str;
	
	const size_t lenOther = strlen( otherstr );
	if( lenOther > strlen( str ) ){
		rt->PushBool( false );
		return;
	}
	
	size_t i;
	for( i=0; i<lenOther; i++ ){
		if( tolower( str[ i ] ) != tolower( otherstr[ i ] ) ){
			rt->PushBool( false );
			return;
		}
	}
	rt->PushBool( true );
}

// public func bool endsWith( String string )
dsClassString::nfEndsWith::nfEndsWith( const sInitData &init ) :
dsFunction( init.clsStr, "endsWith", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsStr ); // string
}
void dsClassString::nfEndsWith::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsValue * const valother = rt->GetValue( 0 );
	if( ! valother->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "string" );
	}
	const char * const otherstr = ( ( sStrNatData* )p_GetNativeData( valother ) )->str;
	
	const size_t lenOther = strlen( otherstr );
	const size_t lenStr = strlen( str );
	rt->PushBool( lenOther <= lenStr && strcmp( str + ( lenStr - lenOther ), otherstr ) == 0 );
}

// public func bool endsWithNoCase( String string )
dsClassString::nfEndsWithNoCase::nfEndsWithNoCase( const sInitData &init ) :
dsFunction( init.clsStr, "endsWithNoCase", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsStr ); // string
}
void dsClassString::nfEndsWithNoCase::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char *str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsValue * const valother = rt->GetValue( 0 );
	if( ! valother->GetRealObject() ){
		DSTHROW_INFO( dueNullPointer, "string" );
	}
	const char * const otherstr = ( ( sStrNatData* )p_GetNativeData( valother ) )->str;
	
	const size_t lenOther = strlen( otherstr );
	const size_t lenStr = strlen( str );
	if( lenOther > lenStr ){
		rt->PushBool( false );
		return;
	}
	
	size_t i;
	str += lenStr - lenOther;
	for( i=0; i<lenOther; i++ ){
		if( tolower( str[ i ] ) != tolower( otherstr[ i ] ) ){
			rt->PushBool( false );
			return;
		}
	}
	rt->PushBool( true );
}



// public func int hashCode()
dsClassString::nfHashCode::nfHashCode( const sInitData &init ) :
dsFunction(init.clsStr, "hashCode", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsInt ){
}

void dsClassString::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char *str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	
	// SDBM hash algorithm
	unsigned long hash = 0;
	int c;
	
	c = *str++;
	while( c ){
		hash = c + ( hash << 6 ) + ( hash << 16 ) - hash;
		c = *str++;
	}
	
	rt->PushInt( hash );
}

// public func bool equals(Object obj)
dsClassString::nfEquals::nfEquals( const sInitData &init ) :
dsFunction(init.clsStr, "equals", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsBool ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassString::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsClass * const clsStr = ( dsClassString* )GetOwnerClass();
	dsValue * const obj = rt->GetValue( 0 );
	
	if( obj->GetType()->GetPrimitiveType() != DSPT_OBJECT || ! obj->GetRealObject()
	|| obj->GetRealObject()->GetType() != clsStr ){
		rt->PushBool( false );
		
	}else{
		const char * const otherStr = ( ( sStrNatData* )p_GetNativeData( obj ) )->str;
		rt->PushBool( strcmp( str, otherStr ) == 0 );
	}
}

// public func String toString()
dsClassString::nfToString::nfToString( const sInitData &init ) :
dsFunction( init.clsStr, "toString", DSFT_FUNCTION, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
}
void dsClassString::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushValue( myself );
}



// public func String +( String str )
dsClassString::nfOpAdd::nfOpAdd( const sInitData &init ) : dsFunction(init.clsStr, "+",
DSFT_OPERATOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsStr ); // str
}
void dsClassString::nfOpAdd::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsValue * const obj = rt->GetValue( 0 );
	const char *otherStr = "(null)";
	char *newStr = NULL;
	
	if( obj->GetRealObject() ){
		otherStr = ( ( sStrNatData* )p_GetNativeData( obj ) )->str;
	}
	
	newStr = new char[ strlen( str )+strlen( otherStr ) + 1 ];
	if( ! newStr ){
		DSTHROW( dueOutOfMemory );
	}
	
	strcpy( newStr, str );
	strcat( newStr, otherStr );
	
	try{
		rt->PushString( newStr );
		
	}catch( ... ){
		delete [] newStr;
		throw;
	}
	delete [] newStr;
}

// public func String +( byte Byte )
dsClassString::nfOpAddByte::nfOpAddByte( const sInitData &init ) : dsFunction( init.clsStr,
"+", DSFT_OPERATOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsByte ); // Byte
}
void dsClassString::nfOpAddByte::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const byte character = rt->GetValue( 0 )->GetByte();
	const int len = strlen( str );
	
	char * const newstr = new char[ len + 2 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	strncpy( newstr, str, len );
	newstr[ len ] = character;
	newstr[ len + 1 ] = '\0';
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String +( bool value )
dsClassString::nfOpAddBool::nfOpAddBool( const sInitData &init ) : dsFunction( init.clsStr,
"+", DSFT_OPERATOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsBool ); // value
}
void dsClassString::nfOpAddBool::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const bool value = rt->GetValue( 0 )->GetBool();
	
	char * const newstr = new char[ strlen( str ) + 6 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	strcpy( newstr, str );
	strcat( newstr, value ? "true" : "false" );
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String +( int value )
dsClassString::nfOpAddInt::nfOpAddInt( const sInitData &init ) : dsFunction( init.clsStr,
"+", DSFT_OPERATOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsInt ); // value
}
void dsClassString::nfOpAddInt::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const int value = rt->GetValue( 0 )->GetInt();
	
	char * const newstr = new char[ strlen( str ) + 12 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	sprintf( newstr, "%s%i", str, value );
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String +( float value )
dsClassString::nfOpAddFloat::nfOpAddFloat( const sInitData &init ) : dsFunction( init.clsStr,
"+", DSFT_OPERATOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsFlt ); // value
}
void dsClassString::nfOpAddFloat::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	const float value = rt->GetValue( 0 )->GetFloat();
	
	char * const newstr = new char[ strlen( str ) + 20 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	sprintf( newstr, "%s%g", str, value );
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}

// public func String +( Object obj )
dsClassString::nfOpAddObject::nfOpAddObject( const sInitData &init ) : dsFunction( init.clsStr, "+",
DSFT_OPERATOR, DSTM_PUBLIC | DSTM_NATIVE, init.clsStr ){
	p_AddParameter( init.clsObj ); // obj
}
void dsClassString::nfOpAddObject::RunFunction( dsRunTime *rt, dsValue *myself ){
	const char * const str = ( ( sStrNatData* )p_GetNativeData( myself ) )->str;
	dsValue * const object = rt->GetValue( 0 );
	
	// get string by calling toString on the object if the object is not null
	const char *otherstr = "(null)";
	if( object->GetRealType()->GetPrimitiveType() != DSPT_OBJECT || object->GetRealObject() ){
		const int funcIndexToString = ( ( dsClassObject* )rt->GetEngine()->GetClassObject() )->GetFuncIndexToString();
		rt->RunFunctionFast( object, funcIndexToString );
		otherstr = rt->GetReturnString();
	}
	
	// create new string adding both
	char * const newstr = new char[ strlen( str ) + strlen( otherstr ) + 1 ];
	if( ! newstr ){
		DSTHROW( dueOutOfMemory );
	}
	strcpy( newstr, str );
	strcat( newstr, otherstr );
	
	try{
		rt->PushString( newstr );
		
	}catch( ... ){
		delete [] newstr;
		throw;
	}
	delete [] newstr;
}



// Class dsClassString
////////////////////////

// Constructor, destructor
////////////////////////////

dsClassString::dsClassString() : dsClass( "String", DSCT_CLASS,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_FIXED ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( sStrNatData ) );
}

dsClassString::~dsClassString(){
}



// Management
///////////////

void dsClassString::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	// store classes
	init.clsStr = this;
	init.clsObj = engine->GetClassObject();
	init.clsVoid = engine->GetClassVoid();
	init.clsByte = engine->GetClassByte();
	init.clsBool = engine->GetClassBool();
	init.clsInt = engine->GetClassInt();
	init.clsFlt = engine->GetClassFloat();
	init.clsArr = engine->GetClassArray();
	
	// functions
	AddFunction( new nfCreate( init ) );
	AddFunction( new nfCreateFilled( init ) );
	AddFunction( new nfDestructor( init ) );
	
	AddFunction( new nfEmpty( init ) );
	AddFunction( new nfGetLength( init ) );
	AddFunction( new nfGetAt( init ) );
	AddFunction( new nfSubString( init ) );
	AddFunction( new nfSubString2( init ) );
	
	AddFunction( new nfFind( init ) );
	AddFunction( new nfFind2( init ) );
	AddFunction( new nfFind3( init ) );
	AddFunction( new nfFindAny( init ) );
	AddFunction( new nfFindAny2( init ) );
	AddFunction( new nfFindAny3( init ) );
	AddFunction( new nfFindString( init ) );
	AddFunction( new nfFindString2( init ) );
	AddFunction( new nfFindString3( init ) );
	AddFunction( new nfFindReverse( init ) );
	AddFunction( new nfFindReverse2( init ) );
	AddFunction( new nfFindReverse3( init ) );
	AddFunction( new nfFindAnyReverse( init ) );
	AddFunction( new nfFindAnyReverse2( init ) );
	AddFunction( new nfFindAnyReverse3( init ) );
	AddFunction( new nfFindStringReverse( init ) );
	AddFunction( new nfFindStringReverse2( init ) );
	AddFunction( new nfFindStringReverse3( init ) );
	
	AddFunction( new nfReverse( init ) );
	AddFunction( new nfSplit( init ) );
	AddFunction( new nfSplit2( init ) );
	AddFunction( new nfReplace( init ) );
	AddFunction( new nfReplace2( init ) );
	AddFunction( new nfReplaceString( init ) );
	
	AddFunction( new nfTrimLeft( init ) );
	AddFunction( new nfTrimRight( init ) );
	AddFunction( new nfTrimBoth( init ) );
	AddFunction( new nfToLower( init ) );
	AddFunction( new nfToUpper( init ) );
	AddFunction( new nfToInt( init ) );
	AddFunction( new nfToFloat( init ) );
	
	AddFunction( new nfCompare( init ) );
	AddFunction( new nfCompareNoCase( init ) );
	AddFunction( new nfStartsWith( init ) );
	AddFunction( new nfStartsWithNoCase( init ) );
	AddFunction( new nfEndsWith( init ) );
	AddFunction( new nfEndsWithNoCase( init ) );
	
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfToString( init ) );
	
	AddFunction( new nfOpAdd( init ) );
	AddFunction( new nfOpAddByte( init ) );
	AddFunction( new nfOpAddBool( init ) );
	AddFunction( new nfOpAddInt( init ) );
	AddFunction( new nfOpAddFloat( init ) );
	AddFunction( new nfOpAddObject( init ) );
}



// Management
///////////////

const char *dsClassString::GetRealObjectString( dsRealObject *myself ) const{
	if( ! myself ){
		DSTHROW( dueInvalidParam );
	}
	return ( const char* )( ( sStrNatData* )p_GetNativeData( myself->GetBuffer() ) )->str;
}

void dsClassString::SetRealObjectString( dsRealObject* object, const char* string ){
	// Object is considered NOT initialized thus there is no valid string pointer
	// storen in the native dataset of Object.
	if( ! object || ! string ){
		DSTHROW( dueInvalidParam );
	}
	
	sStrNatData &nd = *( ( sStrNatData* )p_GetNativeData( object->GetBuffer() ) );
	
	nd.str = new char[ strlen( string ) + 1 ];
	if( ! nd.str ){
		DSTHROW( dueOutOfMemory );
	}
	strcpy( nd.str, string );
}
