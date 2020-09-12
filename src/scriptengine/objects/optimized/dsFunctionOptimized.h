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

#ifndef _DSFUNCTIONOPTIMIZED_H_
#define _DSFUNCTIONOPTIMIZED_H_

class dsRunTime;
class dsValue;


/**
 * \brief Base class for optimized functions.
 */
class dsFunctionOptimized{
public:
	/** \brief Optimization type. */
	enum eType{
		etEmpty,
		etGetClassVar,
		etGetClassVarStatic,
		etGetClassConst,
		etGetParameter,
		etGetContextVar
	};
	
	
	
private:
	eType pType;
	
	
	
protected:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create function. */
	dsFunctionOptimized( eType type );
	
public:
	/** \brief Clean up optimized function. */
	virtual ~dsFunctionOptimized();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Optimization type. */
	inline eType GetType() const{ return pType; }
	
	/** \brief Run function. */
	virtual void RunFunction( dsRunTime *rt, dsValue *myself ) = 0;
};

#endif

