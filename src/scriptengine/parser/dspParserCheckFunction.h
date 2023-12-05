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
#ifndef _DSPPARSERCHECKFUNCTION_H_
#define _DSPPARSERCHECKFUNCTION_H_

// includes

// predefinitions
class dsClass;
class dsEngine;
class dsFunction;
class dsSignature;
class dspBaseNode;
class dspNodeScript;
class dspParserCheck;
class dspNodeClassFunction;

// class dspParserCheckFunction
class DS_DLL_EXPORT dspParserCheckFunction{
private:
	dspNodeScript *p_Script;
	dspNodeClassFunction *p_FuncNode;
	dsFunction *p_Function;
	dsSignature *p_Signature;
	char **p_ArgNames;
	int p_ArgNameCount;
public:
	// constructor, destructor
	dspParserCheckFunction(dspNodeScript *Script, dspNodeClassFunction *Node);
	dspParserCheckFunction(dspNodeScript *Script, dsFunction *Function);
	~dspParserCheckFunction();
	// management
	inline dspNodeScript *GetScript() const{ return p_Script; }
	inline dspNodeClassFunction *GetFuncNode() const{ return p_FuncNode; }
	inline dsFunction *GetFunction() const{ return p_Function; }
	inline dsSignature *GetSignature() const{ return p_Signature; }
	void CreateFunction(dsClass *OwnerClass, dspParserCheck *ParserCheck);
	void SetFunction(dsFunction *func);
	// arguments
	inline int GetArgumentCount() const{ return p_ArgNameCount; }
	const char *GetArgumentAt( int index ) const;
	int FindArgument(const char *Name) const;
	void AddArgName(const char *Name);
};

// end of include only once
#endif

