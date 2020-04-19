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
#ifndef _DSPPARSERCHECKSCRIPT_H_
#define _DSPPARSERCHECKSCRIPT_H_

// includes

// predefinitions
class dsClass;
class dsEngine;
class dspNodeScript;

// class dspParserCheckScript
class dspParserCheckScript{
private:
	dspNodeScript *p_Script;
	dsClass **p_Pins;
	int p_PinCount;
public:
	// constructor, destructor
	dspParserCheckScript(dspNodeScript *Script);
	~dspParserCheckScript();
	// management
	inline dspNodeScript *GetScript() const{ return p_Script; }
	// pins
	inline int GetPinCount() const{ return p_PinCount; }
	void AddPin(dsClass *Namespace);
	dsClass *GetPin(int index) const;
private:
};

// end of include only once
#endif

