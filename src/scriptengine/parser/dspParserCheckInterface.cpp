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
#include "dspParserCheckInterface.h"
#include "dspParserCheckType.h"
#include "dspNodes.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"


// dspParserCheckInterface
////////////////////////////
// constructor, destructor
dspParserCheckInterface::dspParserCheckInterface(dspBaseNode *node, dspParserCheckType *type){
	if(!node || !type) DSTHROW(dueInvalidParam);
	p_node = node;
	p_class = NULL;
	p_type = type;
}
dspParserCheckInterface::~dspParserCheckInterface(){
	if( p_type ){
		delete p_type;
	}
}
// management
void dspParserCheckInterface::SetClass(dsClass *cls){
	p_class = cls;
}
