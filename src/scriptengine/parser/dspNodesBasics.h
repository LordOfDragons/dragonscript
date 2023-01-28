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
#ifndef _DSPNODESBASICS_H_
#define _DSPNODESBASICS_H_

// node array class
class DS_DLL_EXPORT dspListNodeEntry{
friend class dspListNodes;
private:
	dspBaseNode *p_Node;
	dspListNodeEntry *p_NextNode;
public:
	dspListNodeEntry(dspBaseNode *Node);
	~dspListNodeEntry();
	inline dspBaseNode *GetNode() const{ return p_Node; }
	inline dspBaseNode **GetNodePtr(){ return &p_Node; }
	inline dspListNodeEntry *GetNextNode() const{ return p_NextNode; }
	inline void SetNextNode(dspListNodeEntry *next){ p_NextNode = next; }
};
class DS_DLL_EXPORT dspListNodes{
friend class dspListNodeIterator;
private:
	dspListNodeEntry *p_TopNode, *p_TailNode;
	int p_NodeCount;
public:
	dspListNodes();
	~dspListNodes();
	inline int GetNodeCount() const{ return p_NodeCount; }
	void Clear();
	void AddNode(dspBaseNode *Node);
	dspBaseNode *PopNode();
};
class DS_DLL_EXPORT dspListNodeIterator{
private:
	dspListNodes *p_List;
	dspListNodeEntry *p_CurNode;
public:
	dspListNodeIterator(dspListNodes *List);
	inline dspListNodes *GetList() const{ return p_List; }
	inline int GetCount() const{ return p_List->p_NodeCount; }
	inline bool HasNext() const{ return p_CurNode != NULL; }
	dspBaseNode **Rewind();
	dspBaseNode **GetNext();
};


// basic node class
class DS_DLL_EXPORT dspBaseNode : public dsuArrayable{
private:
	int p_NodeType, p_LineNum, p_CharNum;
	dsScriptSource *p_Source;
public:
	// constructor, destructor
	dspBaseNode(int Type, dsScriptSource *Source, int LineNum, int CharNum);
	dspBaseNode(int Type, dspBaseNode *Node);
	~dspBaseNode();
	// information
	inline dsScriptSource *GetSource() const{ return p_Source; }
	inline int GetLineNum() const{ return p_LineNum; }
	inline int GetCharNum() const{ return p_CharNum; }
	inline int GetNodeType() const{ return p_NodeType; }
	// overloadables
	virtual const char *GetTerminalString();
	virtual bool CheckCode(dspParserCheckCode *CheckCode, dspBaseNode **ppThisNode);
	virtual void CompileCode(dspParserCompileCode *CompileCode);
#ifndef __NODBGPRINTF__
	virtual void DebugPrint(int Level, const char *Prefix) = 0;
#endif
protected:
	void p_PrePrintDebug(int Level, const char *Prefix);
	void p_PostPrintDebug();
};

// error indication node
class DS_DLL_EXPORT dspNodeError : public dspBaseNode{
public:
	dspNodeError(dspBaseNode *RefNode);
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// native class error node
class DS_DLL_EXPORT dspNodeNatClassErr : public dspBaseNode{
private:
	class DummySource : public dsScriptSource{
	public:
		DummySource(){}
		const char *GetName(){ return "< Native Class Code >"; }
		void Open(){}
		int ReadData(char *Buffer, int Size){ return 0; }
		void Close(){}
	} p_dummySource;
public:
	dspNodeNatClassErr();
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
};

// ident node
class DS_DLL_EXPORT dspNodeIdent : public dspBaseNode{
private:
	char *p_Name;
public:
	dspNodeIdent(dsScriptSource *Source, int LineNum, int CharNum, const char *Name);
	~dspNodeIdent();
	const char *GetTerminalString();
#ifndef __NODBGPRINTF__
	void DebugPrint(int Level, const char *Prefix);
#endif
	inline const char *GetName() const{ return (const char *)p_Name; }
};

// end of include only once
#endif

