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
#include <string.h>
#include "../dragonscript_config.h"
#include "dspNodes.h"
#include "dspParserCheckCode.h"
#include "../exceptions.h"

// node array class
/////////////////////
dspListNodeEntry::dspListNodeEntry(dspBaseNode *Node){
	p_Node = Node; p_NextNode = NULL;
}
dspListNodeEntry::~dspListNodeEntry(){
	if(p_Node) delete p_Node;
}

dspListNodes::dspListNodes(){
	p_TopNode = p_TailNode = NULL;
	p_NodeCount = 0;
}
dspListNodes::~dspListNodes(){
	Clear();
}
void dspListNodes::Clear(){
	dspListNodeEntry *vCurNode=p_TopNode, *vNextNode;
	while(vCurNode){
		vNextNode = vCurNode->GetNextNode();
		delete vCurNode; vCurNode = vNextNode;
	}
	p_TopNode = p_TailNode = NULL;
	p_NodeCount = 0;
}
void dspListNodes::AddNode(dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	dspListNodeEntry *vNewNode = new dspListNodeEntry(Node);
	if(!vNewNode) DSTHROW(dueOutOfMemory);
	if(p_TailNode){
		p_TailNode->SetNextNode(vNewNode); p_TailNode = vNewNode; p_NodeCount++;
	}else{
		p_TopNode = p_TailNode = vNewNode; p_NodeCount = 1;
	}
}
dspBaseNode *dspListNodes::PopNode(){
	if(!p_TopNode) return NULL;
	dspListNodeEntry *vNodeEntry = p_TopNode;
	dspBaseNode *vNode = vNodeEntry->GetNode();
	p_TopNode->p_Node = NULL;
	p_TopNode = p_TopNode->GetNextNode();
	if(!p_TopNode) p_TailNode = NULL;
	p_NodeCount--;
	delete vNodeEntry;
	return vNode;
}

dspListNodeIterator::dspListNodeIterator(dspListNodes *List){
	if(!List) DSTHROW(dueInvalidParam);
	p_List = List; p_CurNode = p_List->p_TopNode;
}
dspBaseNode **dspListNodeIterator::Rewind(){
	p_CurNode = p_List->p_TopNode;
	return p_CurNode ? p_CurNode->GetNodePtr() : NULL;
}
dspBaseNode **dspListNodeIterator::GetNext(){
	if(!p_CurNode) DSTHROW(dueInvalidParam);
	dspListNodeEntry *vEntry = p_CurNode;
	p_CurNode = p_CurNode->GetNextNode();
	return vEntry->GetNodePtr();
}


// basic node class
/////////////////////
// constructor, destructor
dspBaseNode::dspBaseNode(int Type, dsScriptSource *Source, int LineNum, int CharNum){
	if(!Source || LineNum < 1 || CharNum < 1) DSTHROW(dueInvalidParam);
	p_NodeType = Type; p_LineNum = LineNum; p_CharNum = CharNum;
	p_Source = Source;
}
dspBaseNode::dspBaseNode(int Type, dspBaseNode *Node){
	if(!Node) DSTHROW(dueInvalidParam);
	p_NodeType = Type; p_Source = Node->p_Source;
	p_LineNum = Node->p_LineNum; p_CharNum = Node->p_CharNum;
}
dspBaseNode::~dspBaseNode(){ }
// default function for the overloadables
const char *dspBaseNode::GetTerminalString(){
	return "";
}
bool dspBaseNode::CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode){
	printf("[OOPS!] unhandled checkcode (nodetype: %i)\n", p_NodeType);
	CheckCode->SetTypeNothing();
	return false;
}
void dspBaseNode::CompileCode(dspParserCompileCode *CompileCode){
	printf("[OOPS!] unhandled compilenode (nodetype: %i)\n", p_NodeType);
	DSTHROW(dueInvalidParam);
}
// protected function
void dspBaseNode::p_PrePrintDebug(int Level, const char *Prefix){
	for(int i=0; i<Level;i++) printf("  ");
	printf("%s",Prefix);
}
void dspBaseNode::p_PostPrintDebug(){
	printf(" [%i/%i]\n", p_LineNum, p_CharNum);
}


// error indication node
//////////////////////////
dspNodeError::dspNodeError(dspBaseNode *RefNode) : dspBaseNode(ntError,RefNode) { }
#ifndef __NODBGPRINTF__
void dspNodeError::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Error Node");
	p_PostPrintDebug();
}
#endif

// native class error node
////////////////////////////
dspNodeNatClassErr::dspNodeNatClassErr() : dspBaseNode(ntError, &p_dummySource, 1, 1){ }
#ifndef __NODBGPRINTF__
void dspNodeNatClassErr::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Native Class Code Error Node");
	p_PostPrintDebug();
}
#endif

// ident node
///////////////
dspNodeIdent::dspNodeIdent(dsScriptSource *Source, int LineNum, int CharNum, const char *Name) : dspBaseNode(ntIdent,Source,LineNum,CharNum) {
	if(!Name || Name[0]=='\0') DSTHROW(dueInvalidParam);
	const int size = ( int )strlen( Name );
	if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( p_Name, size + 1, Name, size );
	#else
		strncpy(p_Name, Name, size + 1);
	#endif
	p_Name[ size ] = 0;
}
dspNodeIdent::~dspNodeIdent(){ delete [] p_Name; }
const char *dspNodeIdent::GetTerminalString(){ return p_Name; }
#ifndef __NODBGPRINTF__
void dspNodeIdent::DebugPrint(int Level, const char *Prefix){
	p_PrePrintDebug(Level, Prefix);
	printf("- Identifier = %s", p_Name);
	p_PostPrintDebug();
}
#endif

