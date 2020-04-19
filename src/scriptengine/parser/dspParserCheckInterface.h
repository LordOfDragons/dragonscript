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
#ifndef _DSPPARSERCHECKINTERFACE_H_
#define _DSPPARSERCHECKINTERFACE_H_

// includes

// predefinitions
class dsClass;
class dspBaseNode;
class dspParserCheckType;


// class dspParserCheckInterface
class dspParserCheckInterface{
private:
	dspBaseNode *p_node;
	dsClass *p_class;
	dspParserCheckType *p_type;
public:
	// constructor, destructor
	dspParserCheckInterface(dspBaseNode *node, dspParserCheckType *type);
	~dspParserCheckInterface();
	// management
	inline dspBaseNode *GetNode() const{ return p_node; }
	inline dsClass *GetClass() const{ return p_class; }
	inline dspParserCheckType *GetType() const{ return p_type; }
	void SetClass(dsClass *cls);
};

// end of include only once
#endif

