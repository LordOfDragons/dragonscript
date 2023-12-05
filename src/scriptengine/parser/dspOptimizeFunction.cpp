/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2020, Roland Plüss (roland@rptd.ch)
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

#include "../dragonscript_config.h"
#include "dspNodes.h"
#include "dspOptimizeFunction.h"
#include "dspParser.h"
#include "dspParserCheck.h"
#include "dspParserCheckFunction.h"
#include "../dsEngine.h"
#include "../dsScriptSource.h"
#include "../exceptions.h"
#include "../objects/dsClass.h"
#include "../objects/dsByteCode.h"
#include "../objects/dsFunction.h"
#include "../objects/dsFuncList.h"
#include "../objects/dsSignature.h"
#include "../objects/dsVariable.h"
#include "../objects/optimized/dsFOEmpty.h"
#include "../objects/optimized/dsFOGetClassVar.h"
#include "../objects/optimized/dsFOGetClassVarStatic.h"
#include "../objects/optimized/dsFOGetParameter.h"



// #define DO_PRINT_OPTIMIZED 1

#ifdef DO_PRINT_OPTIMIZED
	#define PRINT_OPTIMIZED(t) printf("DSOptimize: %s.%s(%d): %s\n", \
		function.GetOwnerClass()->GetName(), function.GetName(), \
		function.GetSignature()->GetCount(), t);
#else
	#define PRINT_OPTIMIZED(t)
#endif



// class dspOptimizeFunction
//////////////////////////////

dspOptimizeFunction::dspOptimizeFunction( const dspParserCheckFunction &checkFunction ) :
pCheckFunction( checkFunction ){
}

dspOptimizeFunction::~dspOptimizeFunction(){
}



// Management
///////////////

void dspOptimizeFunction::Optimize( const dspParserCheck &parserCheck ){
dsFunction &function = *pCheckFunction.GetFunction();
	if( function.GetTypeModifiers() & DSTM_NATIVE
	|| function.GetFuncType() == DSFT_CONSTRUCTOR
	|| function.GetFuncType() == DSFT_DESTRUCTOR ){
		return;
	}
	
	const dspNodeClassFunction &nodeFunction = *pCheckFunction.GetFuncNode();
	if( ! nodeFunction.GetStaNode() ){
		return;
	}
	
// 			if( strcmp( function.GetOwnerClass()->GetName(), "TestClass" ) == 0 && strcmp( function.GetName(), "empty" ) == 0 ){
// 				nodeFunction.GetStaNode()->DebugPrint( 0, "AST: " );
// 				function.GetByteCode()->DebugPrint();
// 			}
	
	if( nodeFunction.GetStaNode()->GetNodeType() != eNodeTypes::ntStaList ){
		return;
	}
	dspNodeStaList &nodeStaList = *( ( dspNodeStaList* )nodeFunction.GetStaNode() );
	
	if( nodeStaList.GetNodes().GetNodeCount() == 0 ){
		function.SetOptimized( new dsFOEmpty );
		PRINT_OPTIMIZED("FOEmpty")
	}
	
	if( nodeStaList.GetNodes().GetNodeCount() != 1 ){
		return;
	}
	
	dspBaseNode * const nodeSta1 = *dspListNodeIterator( &nodeStaList.GetNodes() ).GetNext();
	
	if( function.GetType() == parserCheck.GetEngine()->GetClassVoid() ){
		// functions without return value
		pOptimizeVoid( *nodeSta1 );
		
	}else{
		// functions with return value
		if( nodeSta1->GetNodeType() != eNodeTypes::ntReturn ){
			return;
		}
		dspNodeReturn &nodeReturn = *( ( dspNodeReturn* )nodeSta1 );
		if( ! nodeReturn.GetReturnValue() ){
			return;
		}
		pOptimizeReturn( *nodeReturn.GetReturnValue() );
	}
}



// Private Functions
//////////////////////

void dspOptimizeFunction::pOptimizeVoid( const dspBaseNode &nodeRetVal ){
}

void dspOptimizeFunction::pOptimizeReturn( const dspBaseNode &nodeRetVal ){
	if( nodeRetVal.GetNodeType() != eNodeTypes::ntMembVar ){
		return;
	}
	const dspNodeMembVar &nodeMembVar = ( const dspNodeMembVar & )nodeRetVal;
	dsFunction &function = *pCheckFunction.GetFunction();
	
	if( nodeMembVar.GetParameterID() != -1 ){
		function.SetOptimized( new dsFOGetParameter( nodeMembVar.GetParameterID() ) );
		PRINT_OPTIMIZED("FOGetParameter")
		
	}else if( nodeMembVar.GetContextVariable() != -1 ){
		// TODO
		
	}else if( nodeMembVar.GetClassVariable() ){
		dsVariable &variable = *nodeMembVar.GetClassVariable();
		if( variable.GetTypeModifiers() & DSTM_STATIC ){
			pCheckFunction.GetFunction()->SetOptimized(
				new dsFOGetClassVarStatic( *nodeMembVar.GetClassVariable() ) );
			PRINT_OPTIMIZED("FOGetClassVarStatic")
			
		}else{
			if( nodeMembVar.GetRealObject()->GetNodeType() != eNodeTypes::ntThis ){
				return;
			}
			
			pCheckFunction.GetFunction()->SetOptimized(
				new dsFOGetClassVar( *nodeMembVar.GetClassVariable() ) );
			PRINT_OPTIMIZED("FOGetClassVar")
		}
	}
}
