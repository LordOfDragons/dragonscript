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



// include only once
#ifndef _DSPNODES_H_
#define _DSPNODES_H_

// includes
#include "../dsDefinitions.h"
#include "../dsScriptSource.h"
#include "../utils/dsuArrayable.h"

// predefinitions
class dsClass;
class dsScriptSource;
class dsEngine;
class dsVariable;
class dsFunction;
class dsConstant;
class dsLocalVariable;
class dspParserCheckCode;
class dspParserCompileCode;

// class list
// ----------
// basic nodes
class dspListNodes;
class dspBaseNode;
class dspNodeError;
class dspNodeIdent;
// constant nodes
class dspNodeByte;
class dspNodeBool;
class dspNodeInt;
class dspNodeFloat;
class dspNodeString;
// operator nodes
class dspNodeOperator;
class dspNodeUnaryOperation;
class dspNodePostIncrement;
class dspNodePostDecrement;
class dspNodeBinaryOperation;
class dspNodeLogicalAnd;
class dspNodeLogicalOr;
class dspNodeCastTo;
class dspNodeCastableTo;
class dspNodeIsTypeOf;
// keyword nodes
class dspNodeKeyword;
class dspNodeNull;
class dspNodeThis;
class dspNodeSuper;
// statement nodes
class dspNodeMembVar;
class dspNodeMembFunc;
class dspNodeEmptySta;
class dspNodeIfElse;
class dspNodeCase;
class dspNodeSelect;
class dspNodeWhile;
class dspNodeDoWhile;
class dspNodeForUp;
class dspNodeForDown;
class dspNodeBreak;
class dspNodeContinue;
class dspNodeBlock;
class dspNodeVarDefList;
class dspNodeVarDef;
class dspNodeReturn;
class dspNodeThrow;
class dspNodeCatch;
class dspNodeTry;
class dspNodeStaList;
// script nodes
class dspNodeFuncArg;
class dspNodeInitConstrCall;
class dspNodeClassFunction;
class dspNodeClassVariableList;
class dspNodeClassVariable;
class dspNodeEnumEntry;
class dspNodeEnumeration;
class dspNodeClass;
class dspNodePin;
class dspNodeNamespace;
class dspNodeOption;
class dspNodeRequires;
class dspNodeIncludes;
class dspNodeScript;
// special nodes
class dspNodeAutoCast;
class dspNodeSetStaVar;
class dspNodeEnumValue;

// node types
enum eNodeTypes{
	// basic nodes
	ntError,
	ntIdent,
	// constant nodes
	ntByte,
	ntBool,
	ntInt,
	ntFloat,
	ntString,
	// operator nodes
	ntOperator,
	ntUnaryOp,
	ntPostInc,
	ntPostDec,
	ntBinaryOp,
	ntInlineIf,
	ntLogicalAnd,
	ntLogicalOr,
	ntCastTo,
	ntCastableTo,
	ntIsTypeOf,
	// keyword nodes
	ntKeyword,
	ntNull,
	ntThis,
	ntSuper,
	// statement nodes
	ntMembVar,
	ntMembFunc,
	ntEmptySta,
	ntIfElse,
	ntCase,
	ntSelect,
	ntWhile,
	ntDoWhile,
	ntForUp,
	ntForDown,
	ntBreak,
	ntContinue,
	ntBlock,
	ntVarDefList,
	ntVarDef,
	ntReturn,
	ntThrow,
	ntCatch,
	ntTry,
	ntStaList,
	// script nodes
	ntFuncArg,
	ntInitConstrCall,
	ntClassFunc,
	ntClassVarList,
	ntClassVar,
	ntEnumeration,
	ntClass,
	ntPin,
	ntNamespace,
	ntOption,
	ntRequires,
	ntIncludes,
	ntScript,
	// special nodes
	ntAutoCast,
	ntSetStaVar,
	ntEnumValue,
};

// basic nodes
#include "dspNodesBasics.h"

// constant nodes
#include "dspNodesConstants.h"

// keyword nodes
#include "dspNodesKeywords.h"

// operation nodes
#include "dspNodesOperations.h"

// statement nodes
#include "dspNodesStatements.h"

// script nodes
#include "dspNodesScript.h"

// special nodes
#include "dspNodesSpecial.h"

// end of include only once
#endif

