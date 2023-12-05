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
#ifndef _DSPTYPEMODIFIERS_H_
#define _DSPTYPEMODIFIERS_H_

#include "../dragonscript_export.h"

// definitions
#define TM_COUNT	7

// predefinitions
class dsScriptSource;
class dsEngine;
class dspNodeKeyword;

// class dspTypeModifiers
class DS_DLL_EXPORT dspTypeModifiers{
private:
	dspBaseNode *p_TypeNodes[TM_COUNT];
	char *p_MsgBuf;
public:
	// constructor, destructor
	dspTypeModifiers();
	~dspTypeModifiers();
	// information
	bool IsTypeModSet(int TypeMod) const;
	dspBaseNode *GetTypeNode(int TypeMod) const;
	int GetTypeModValue() const;
	// management
	void SetTypeMod(int TypeMod, dspBaseNode *Node);
	void UnsetTypeMod(int TypeMod);
};

// end of include only once
#endif

