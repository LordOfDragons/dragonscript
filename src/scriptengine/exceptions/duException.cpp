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
#include <stdarg.h>

#include "duException.h"
#include "../dsEngine.h"
#include "../dsRunTime.h"
#include "../objects/dsRealObject.h"
#include "../objects/dsValue.h"
#include "../objects/dsClass.h"
#include "../packages/default/dsClassEnumeration.h"



// Class duException
/////////////////////

// Constructors, Destructors
//////////////////////////////

duException::duException(const char *Name, const char *Description, const char *Info, const char *File, int Line){
	p_Name = Name;
	p_Desc = Description;
	p_File = File;
	p_Line = Line;
	if(!p_Name) p_Name = "";
	if(!p_Desc) p_Desc = "";
	if(!p_File) p_File = "";
	if(p_Line < 0) p_Line = 0;
	
	if( Info && Info[ 0 ] ){
		p_Info = Info;
	}
}

duException::~duException(){
}



// Tests
//////////

bool duException::IsNamed(const char *Name) const{
	return strcmp(p_Name, Name) == 0;
}

DS_FUNC_EXPORT_IMPL dsuString ErrorBuildInfo( const char *format, ... ){
	va_list args;
	va_start( args, format );
	
	va_list copyargs; // required since vsnprintf modifies vargs
	va_copy( copyargs, args );
	const int length = vsnprintf( NULL, 0, format, copyargs );
	va_end( copyargs );
	
	dsuString info;
	info.Resize( length );
	vsnprintf( info.Pointer(), length + 1, format, args );
	va_end( args );
	
	return info;
}

DS_FUNC_EXPORT_IMPL dsuString ErrorObjectType( const dsValue *value ){
	if( ! value ){
		return dsuString( "null" );
	}
	
	dsuString name;
	if( value->GetType()->GetPrimitiveType() == DSPT_OBJECT ){
		value->GetRealObject()->GetType()->GetFullName( &name );
		
	}else{
		value->GetType()->GetFullName( &name );
	}
	return name;
}

DS_FUNC_EXPORT_IMPL dsuString ErrorCastInfo( const dsValue *from, const dsClass *to ){
	if( ! from ){
		return dsuString();
	}
	
	if( from->GetType()->GetPrimitiveType() != DSPT_OBJECT ){
		return ErrorCastInfo( from->GetType(), to );
	}
	
	dsuString nameFrom, nameTo;
	if( from->GetRealObject() ){
		from->GetRealObject()->GetType()->GetFullName( &nameFrom );
		
	}else{
		nameFrom = "null";
	}
	to->GetFullName( &nameTo );
	return ErrorBuildInfo( "%s to %s", nameFrom.Pointer(), nameTo.Pointer() );
}

DS_FUNC_EXPORT_IMPL dsuString ErrorCastInfo( const dsClass *from, const dsClass *to ){
	if( ! from || ! to ){
		return dsuString();
	}
	
	dsuString nameFrom, nameTo;
	from->GetFullName( &nameFrom );
	to->GetFullName( &nameTo );
	return ErrorBuildInfo( "%s to %s", nameFrom.Pointer(), nameTo.Pointer() );
}

DS_FUNC_EXPORT_IMPL dsuString ErrorValueInfo( const dsRunTime &rt, const dsValue *value ){
	dsuString info;
	if( value ){
		switch( value->GetType()->GetPrimitiveType() ){
		case DSPT_BOOL:
			info = value->GetBool() ? "bool(true)" : "bool(false)";
			break;
			
		case DSPT_BYTE:
			info.Resize( 10 );
			snprintf( info.Pointer(), 10, "byte(%d)", value->GetByte() );
			break;
			
		case DSPT_INT:
			info.Resize( 50 );
			snprintf( info.Pointer(), 25, "int(%d)", value->GetInt() );
			break;
			
		case DSPT_FLOAT:
			info.Resize( 30 );
			snprintf( info.Pointer(), 30, "float(%g)", value->GetFloat() );
			break;
			
		case DSPT_OBJECT:
			if( value->GetRealObject() ){
				dsClass * const type = value->GetRealObject()->GetType();
				if( type == rt.GetEngine()->GetClassString() ){
					info = "String '";
					info += value->GetString();
					info += "'";
					
				}else if( type->CastableTo( rt.GetEngine()->GetClassEnumeration() ) ){
					type->GetFullName( &info );
					info += ".";
					info += ( ( dsClassEnumeration* )rt.GetEngine()->GetClassEnumeration() )
						->GetConstantName( *value->GetRealObject() );
					
				}else{
					type->GetFullName( &info );
				}
				
			}else{
				info = "null";
			}
			break;
			
		default:
			value->GetType()->GetFullName( &info );
		}
		
	}else{
		info = "null";
	}
	return info;
}



// Display functions
//////////////////////

void duException::PrintError() const{
	printf("[EXCEPTION OCCURED]\n");
	printf("  Exception:   %s\n", p_Name);
	printf("  Description: %s\n", p_Desc);
	printf("  Infos:       %s\n", p_Info.Pointer());
	printf("  Source File: %s\n", p_File);
	printf("  Source Line: %i\n", p_Line);
}
