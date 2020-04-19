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
#include "dspNodes.h"
#include "dspTypeModifiers.h"
#include "../exceptions.h"

// definitions
#define SE_MSGBUFSIZE			250

// local definitions
static int p_Flags[TM_COUNT] = {
	DSTM_PUBLIC, DSTM_PROTECTED, DSTM_PRIVATE,
	DSTM_NATIVE, DSTM_ABSTRACT, DSTM_FIXED, DSTM_STATIC
};

// dspTypeModifiers
/////////////////////
// constructor, destructor
dspTypeModifiers::dspTypeModifiers(){
	if(!(p_MsgBuf = new char[SE_MSGBUFSIZE+1])) DSTHROW(dueOutOfMemory);
	for(int i=0; i<TM_COUNT; i++) p_TypeNodes[i] = NULL;
}
dspTypeModifiers::~dspTypeModifiers(){
	delete [] p_MsgBuf;
	for(int i=0; i<TM_COUNT; i++){
    	if(p_TypeNodes[i]){
        	delete p_TypeNodes[i];
        	p_TypeNodes[i] = NULL;
    	}
	}
}
// information
bool dspTypeModifiers::IsTypeModSet(int TypeMod) const{
	for(int i=0; i<TM_COUNT; i++){
		if(p_Flags[i] == TypeMod) return p_TypeNodes[i] != NULL;
	}
	DSTHROW(dueInvalidParam);
}
dspBaseNode *dspTypeModifiers::GetTypeNode(int TypeMod) const{
	for(int i=0; i<TM_COUNT; i++){
		if(p_Flags[i] == TypeMod) return p_TypeNodes[i];
	}
	DSTHROW(dueInvalidParam);
}
int dspTypeModifiers::GetTypeModValue() const{
	int vTypeMod = 0;
	for(int i=0; i<TM_COUNT; i++){
		if(p_TypeNodes[i]) vTypeMod |= p_Flags[i];
	}
	return vTypeMod;
}
// management
void dspTypeModifiers::SetTypeMod(int TypeMod, dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	for(int i=0; i<TM_COUNT; i++){
		if(p_Flags[i] == TypeMod){
			if(p_TypeNodes[i]) DSTHROW(dueInvalidParam);
			p_TypeNodes[i] = Node; return;
		}
	}
	DSTHROW(dueInvalidParam);
}
void dspTypeModifiers::UnsetTypeMod(int TypeMod){
	for(int i=0; i<TM_COUNT; i++){
		if(p_Flags[i] == TypeMod){
			if(p_TypeNodes[i]){
				delete p_TypeNodes[i];
				p_TypeNodes[i] = NULL;
			} return;
		}
	}
	DSTHROW(dueInvalidParam);
}

