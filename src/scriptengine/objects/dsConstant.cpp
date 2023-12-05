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
#include "../dragonscript_config.h"
#include "dsClass.h"
#include "dsConstant.h"
#include "../exceptions.h"

// class dsConstant
/////////////////////
// constructor, destructor
dsConstant::dsConstant(const char *Name, dsClass *Type){
	if(!Name || !Type) DSTHROW(dseInvalidType);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size + 1);
	#endif
	p_Name[ size ] = 0;
	p_Type = Type;
	p_Empty();
}
dsConstant::dsConstant(const char *Name, dsClass *Type, byte InitValue){
	if(!Name || !Type) DSTHROW(dueInvalidParam);
	if(Type->GetPrimitiveType() != DSPT_BYTE) DSTHROW(dseInvalidType);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size + 1);
	#endif
	p_Name[ size ] = 0;
	p_Type = Type;
	p_Data.SetByte(InitValue);
}
dsConstant::dsConstant(const char *Name, dsClass *Type, bool InitValue){
	if(!Name || !Type) DSTHROW(dueInvalidParam);
	if(Type->GetPrimitiveType() != DSPT_BOOL) DSTHROW(dseInvalidType);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size + 1);
	#endif
	p_Name[ size ] = 0;
	p_Type = Type;
	p_Data.SetBool(InitValue);
}
dsConstant::dsConstant(const char *Name, dsClass *Type, int InitValue){
	if(!Name || !Type) DSTHROW(dueInvalidParam);
	if(Type->GetPrimitiveType() != DSPT_INT) DSTHROW(dseInvalidType);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size + 1);
	#endif
	p_Name[ size ] = 0;
	p_Type = Type;
	p_Data.SetInt(InitValue);
}
dsConstant::dsConstant(const char *Name, dsClass *Type, float InitValue){
	if(!Name || !Type) DSTHROW(dueInvalidParam);
	if(Type->GetPrimitiveType() != DSPT_FLOAT) DSTHROW(dseInvalidType);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size + 1);
	#endif
	p_Name[ size ] = 0;
	p_Type = Type;
	p_Data.SetFloat(InitValue);
}
dsConstant::~dsConstant(){
	delete [] p_Name;
}
// private functions
void dsConstant::p_Empty(){
	switch(p_Type->GetPrimitiveType()){
	case DSPT_BYTE:
		p_Data.SetByte(0); break;
	case DSPT_BOOL:
		p_Data.SetBool(false); break;
	case DSPT_INT:
		p_Data.SetInt(0); break;
	case DSPT_FLOAT:
		p_Data.SetFloat(0); break;
	default:
		p_Data.SetRealObject(NULL); break;
	}
}

