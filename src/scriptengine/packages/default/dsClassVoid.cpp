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
#include "../../../config.h"
#include "dsClassVoid.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

// native functions
/////////////////////


// class dsClassVoid
///////////////////////
// constructor, destructor
dsClassVoid::dsClassVoid() : dsClass("void", DSCT_CLASS,
DSTM_PUBLIC | DSTM_FIXED | DSTM_NATIVE, DSPT_VOID) {
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(0);
}
dsClassVoid::~dsClassVoid(){ }
// native functions and operators

// management
void dsClassVoid::CreateClassMembers(dsEngine *engine){
}
