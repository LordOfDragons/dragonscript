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
#ifndef _DSPPARSERCHECKTYPE_H_
#define _DSPPARSERCHECKTYPE_H_

// includes

// predefinitions
class dsClass;
class dsEngine;
class dspBaseNode;

// class dspParserCheckType
class dspParserCheckType{
private:
	char **p_Names;
	int p_NameCount;
public:
	// constructor, destructor
	dspParserCheckType(const char *Name, dspParserCheckType *Type = NULL);
	~dspParserCheckType();
	// management
	inline int GetNameCount() const{ return p_NameCount; }
	const char *GetName(int Index) const;
	const char *GetLastName() const;
	void AddName(const char *Name);
	void AddNameFront(const char *Name);
	dsClass *GetClass(dsEngine *Engine, dsClass *BaseClass) const;
	bool Exists(dsEngine *Engine) const;
	dspParserCheckType *Copy();
	// information
	bool MatchesType(dspParserCheckType *Type) const;
	// helper functions
	static dspParserCheckType *GetTypeFromNode(dspBaseNode *Node);
	static dspParserCheckType *GetTypeFromFullName(const char *name);
	// debugging
#ifndef __NODBGPRINTF__
	void DebugPrint() const;
#endif
};

// end of include only once
#endif

