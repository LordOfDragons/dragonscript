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
#include "../dragonscript_config.h"
#include "dsClass.h"
#include "dsValue.h"
#include "dsRealObject.h"
#include "../exceptions.h"
#include "../packages/default/dsClassString.h"
#include "../packages/default/dsClassArray.h"

// class dsValue
//////////////////
// constructor

dsValue::dsValue() : p_Type(nullptr){
}

dsValue::dsValue(dsClass *Type){
	if(!Type) DSTHROW(dueInvalidParam);
	p_Type = Type;
	p_Data.SetRealObject(NULL);
}
dsValue::dsValue(dsClass *Type, dsRealObject *obj){
	if(!Type) DSTHROW(dueInvalidParam);
	p_Type = Type;
	p_Data.SetRealObject(obj);
}
dsValue::dsValue(dsValue *value){
	if(!value) DSTHROW(dueInvalidParam);
	p_Type = value->p_Type;
	p_Data = value->p_Data;
}
// management
void dsValue::Empty(){
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
// special data retrieval
const char *dsValue::GetString() const{
//	if(strcmp(p_Type->GetName(), "String") != 0) DSTHROW(dseInvalidType);
	dsRealObject *vObj = p_Data.GetRealObject();
	if(!vObj) DSTHROW(dueNullPointer);
	return ((dsClassString*)p_Type)->GetRealObjectString(vObj);
}
dsClass *dsValue::GetRealType() const{
	if(p_Type->GetPrimitiveType()==DSPT_OBJECT && p_Data.GetRealObject()){
		return p_Data.GetRealObject()->GetType();
	}
	return p_Type;
}
