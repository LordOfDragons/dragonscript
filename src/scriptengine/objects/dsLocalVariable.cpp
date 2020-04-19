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
#include "../../config.h"
#include "dsLocalVariable.h"
#include "../exceptions.h"

// class dsLocalVariable
//////////////////////////
// constructor, destructor
dsLocalVariable::dsLocalVariable(dsClass *OwnerClass, const char *Name, dsClass *Type){
	if(/*!OwnerClass || */!Name || !Type) DSTHROW(dueInvalidParam);
	if(!(p_Name = new char[strlen(Name)+1])) DSTHROW(dueOutOfMemory);
	strcpy(p_Name, Name);
	p_Type = Type;
}
dsLocalVariable::~dsLocalVariable(){
	delete [] p_Name;
}
