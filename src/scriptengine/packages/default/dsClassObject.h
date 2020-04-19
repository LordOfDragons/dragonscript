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

#ifndef _DSCLASSOBJECT_H_
#define _DSCLASSOBJECT_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;

/**
 * \brief Object class.
 * \author Roland Plüss
 * \version 1.0
 * \date 2015
 */
class dsClassObject : public dsClass {
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Creates a new class. */
	dsClassObject();
	/** \brief Cleans up the string. */
	virtual ~dsClassObject();
	/*@}*/
	
	/** @name Management */
	/*@{*/
	/** \brief Create class members. */
	void CreateClassMembers( dsEngine *engine );
	
	/** \brief Function index of toString(). */
	inline int GetFuncIndexToString() const{ return pFuncIndexToString; }
	
	/** \brief Function index of hashCode(). */
	inline int GetFuncIndexHashCode() const{ return pFuncIndexHashCode; }
	
	/** \brief Function index of equals(). */
	inline int GetFuncIndexEquals() const{ return pFuncIndexEquals; }
	
	/** \brief Function index of compare(). */
	inline int GetFuncIndexCompare() const{ return pFuncIndexCompare; }
	/*@}*/

private:
	int pFuncIndexToString;
	int pFuncIndexHashCode;
	int pFuncIndexEquals;
	int pFuncIndexCompare;
	
	struct sInitData{
		dsClass *clsObj, *clsVoid, *clsBool, *clsInt, *clsStr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	
	DEF_NATFUNC( nfNew );
	DEF_NATFUNC( nfClassName );
	
	DEF_NATFUNC( nfToString );
	DEF_NATFUNC( nfHashCode );
	DEF_NATFUNC( nfEquals );
	DEF_NATFUNC( nfCompare );
#undef DEF_NATFUNC
};

#endif

