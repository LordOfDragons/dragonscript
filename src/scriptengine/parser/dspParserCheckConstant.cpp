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
#include "dspParserCheck.h"
#include "dspParserCheckCode.h"
#include "dspParserCheckConstant.h"
#include "../dsEngine.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsConstant.h"

// dspParserCheckConstant
///////////////////////////
// constructor, destructor
dspParserCheckConstant::dspParserCheckConstant(dspNodeClassVariable *Node, dsClass *Type, int TypeMods){
	if(!Node || !Type) DSTHROW(dueInvalidParam);
	p_VarNode = Node;
	p_Type = Type;
	p_Constant = NULL;
	p_TypeMods = TypeMods;
	p_Locked = false;
}
dspParserCheckConstant::~dspParserCheckConstant(){
}
// management
void dspParserCheckConstant::CreateConstant(dsClass *OwnerClass, dspParserCheck *ParserCheck){
	if(!OwnerClass || !ParserCheck || p_Constant) DSTHROW(dueInvalidParam);
	dsEngine *vEngine = ParserCheck->GetEngine();
	dspBaseNode *vInitNode;
	// check if the constant is valid (thus not already visited)
	if(p_Locked){
		ParserCheck->ErrorConstLoop(p_VarNode, OwnerClass->GetName()); DSTHROW(dseInvalidSyntax);
	}
	p_Locked = true;
	
	// check init value
	dspParserCheckCode vCheckCode(ParserCheck, OwnerClass);
	
	p_VarNode->SetTypeClass( p_Type );
	if( p_VarNode->CheckCode( &vCheckCode, ( dspBaseNode** )&p_VarNode ) ){
		// create constant
		vInitNode = p_VarNode->GetInitNode();
		if(vInitNode->GetNodeType() == ntByte){
			if(!(p_Constant = new dsConstant(p_VarNode->GetName(), vEngine->GetClassByte(), ((dspNodeByte*)vInitNode)->GetValue()))) DSTHROW(dueOutOfMemory);
		}else if(vInitNode->GetNodeType() == ntBool){
			if(!(p_Constant = new dsConstant(p_VarNode->GetName(), vEngine->GetClassBool(), ((dspNodeBool*)vInitNode)->GetValue()))) DSTHROW(dueOutOfMemory);
		}else if(vInitNode->GetNodeType() == ntInt){
			if(!(p_Constant = new dsConstant(p_VarNode->GetName(), vEngine->GetClassInt(), ((dspNodeInt*)vInitNode)->GetValue()))) DSTHROW(dueOutOfMemory);
		}else if(vInitNode->GetNodeType() == ntFloat){
			if(!(p_Constant = new dsConstant(p_VarNode->GetName(), vEngine->GetClassFloat(), ((dspNodeFloat*)vInitNode)->GetValue()))) DSTHROW(dueOutOfMemory);
		}
		
		// add constant if possible
		if(p_Constant){
			if( OwnerClass->FindConstant( p_VarNode->GetName(), false ) || OwnerClass->FindVariable( p_VarNode->GetName(), false ) ){
				ParserCheck->ErrorDupVariable(p_VarNode, OwnerClass->GetName(), p_VarNode->GetName());
			}else{
				OwnerClass->AddConstant(p_Constant);
			}
		}
	}
}

