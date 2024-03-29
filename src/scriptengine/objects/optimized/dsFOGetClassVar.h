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

#ifndef _DSFOGETCLASSVAR_H_
#define _DSFOGETCLASSVAR_H_

#include "dsFunctionOptimized.h"

class dsVariable;


/**
 * \brief Function optimizing getter returning class variable.
 */
class DS_DLL_EXPORT dsFOGetClassVar : public dsFunctionOptimized{
private:
	dsVariable &pVariable;
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create function. */
	dsFOGetClassVar( dsVariable &variable );
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Variable. */
	inline dsVariable &GetVariable() const{ return pVariable; }
	
	/** \brief Run function. */
	virtual void RunFunction( dsRunTime *rt, dsValue *myself );
};

#endif
