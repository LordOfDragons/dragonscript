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
#include "dspNodes.h"
#include "dspParserCheckScript.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"

// dspParserCheckScript
////////////////////////
// constructor, destructor
dspParserCheckScript::dspParserCheckScript(dspNodeScript *Script){
	if(!Script) DSTHROW(dueInvalidParam);
	p_Script = Script;
	p_Pins = NULL;
	p_PinCount = 0;
}
dspParserCheckScript::~dspParserCheckScript(){
	if(p_Pins) delete [] p_Pins;
}
// management
// pins
void dspParserCheckScript::AddPin(dsClass *Namespace){
	if(!Namespace) DSTHROW(dueInvalidParam);
	dsClass **vNewArray = new dsClass*[p_PinCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_Pins){
		for(int i=0; i<p_PinCount; i++) vNewArray[i] = p_Pins[i];
		delete [] p_Pins;
	}
	vNewArray[p_PinCount] = Namespace;
	p_Pins = vNewArray;
	p_PinCount++;
}
dsClass *dspParserCheckScript::GetPin(int index) const{
	if(index<0 || index>=p_PinCount) DSTHROW(dueOutOfBoundary);
	return p_Pins[index];
}
