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

#ifndef _DSPOPTIMIZEFUNCTION_H_
#define _DSPOPTIMIZEFUNCTION_H_

#include "../objects/optimized/dsFunctionOptimized.h"

class dspNodeClassFunction;
class dspParserCheck;
class dspParserCheckFunction;
class dspNodeMembVar;


/**
 * \brief Optimize functions.
 */
class dspOptimizeFunction{
private:
	const dspParserCheckFunction &pCheckFunction;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create optimize function. */
	dspOptimizeFunction( const dspParserCheckFunction &checkFunction );
	
	/** \brief Clean up optimize function. */
	~dspOptimizeFunction();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Optimize function if possible. */
	void Optimize( const dspParserCheck &parserCheck );
	
	
	
private:
	void pOptimizeVoid( const dspBaseNode &nodeSta );
	void pOptimizeReturn( const dspBaseNode &nodeRetVal );
};

#endif
