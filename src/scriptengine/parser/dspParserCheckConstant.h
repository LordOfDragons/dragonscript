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
#ifndef _DSPPARSERCHECKCONSTANT_H_
#define _DSPPARSERCHECKCONSTANT_H_

// includes

// predefinitions
class dsClass;
class dsEngine;
class dsConstant;
class dspNodeClassVariable;

// class dspParserCheckConstant
class DS_DLL_EXPORT dspParserCheckConstant{
private:
	dspNodeClassVariable *p_VarNode;
	dsClass *p_Type;
	dsConstant *p_Constant;
	int p_TypeMods;
	bool p_Locked;
public:
	// constructor, destructor
	dspParserCheckConstant(dspNodeClassVariable *Node, dsClass *Type, int TypeMods);
	~dspParserCheckConstant();
	// management
	inline dspNodeClassVariable *GetVarNode() const{ return p_VarNode; }
	inline dsClass *GetType() const{ return p_Type; }
	inline dsConstant *GetConstant() const{ return p_Constant; }
	inline int GetTypeMods() const{ return p_TypeMods; }
	void CreateConstant(dsClass *OwnerClass, dspParserCheck *ParserCheck);
};

// end of include only once
#endif

