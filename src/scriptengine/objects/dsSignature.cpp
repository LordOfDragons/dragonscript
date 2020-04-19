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
#include <stdlib.h>
#include "../../config.h"
#include "dsClass.h"
#include "dsSignature.h"
#include "../exceptions.h"

// class dsSignature
///////////////////////////
// constructor, destructor
dsSignature::dsSignature(){
	p_Parameters = NULL;
	p_ParamCount = 0;
}
dsSignature::~dsSignature(){
	Clear();
}
// management
void dsSignature::Clear(){
	if(p_Parameters){
		delete [] p_Parameters;
		p_Parameters = NULL;
	}
	p_ParamCount = 0;
}
void dsSignature::AddParameter(dsClass *Type){
	if(!Type) DSTHROW(dueInvalidParam);
	dsClass **vNewParams = new dsClass*[p_ParamCount+1];
	if(!vNewParams) DSTHROW(dueOutOfMemory);
	if(p_Parameters){
		for(int i=0; i<p_ParamCount; i++) vNewParams[i] = p_Parameters[i];
		delete [] p_Parameters;
	}
	p_Parameters = vNewParams;
	p_Parameters[p_ParamCount] = Type;
	p_ParamCount++;
}
dsClass *dsSignature::GetParameter(int Index) const{
	if((Index < 0) || (Index >= p_ParamCount)) DSTHROW(dueOutOfBoundary);
	return p_Parameters[Index];
}
bool dsSignature::IsEqualTo(dsSignature *Signature) const{
	if(p_ParamCount != Signature->p_ParamCount) return false;
	for(int i=0; i<p_ParamCount; i++){
		if(!p_Parameters[i]->IsEqualTo(Signature->p_Parameters[i])) return false;
	}
	return true;
}
void dsSignature::CopyTo(dsSignature *Signature){
	if(!Signature) DSTHROW(dueInvalidParam);
	for(int i=0; i<p_ParamCount; i++){
		Signature->AddParameter( (dsClass*)p_Parameters[i] );
	}
}

