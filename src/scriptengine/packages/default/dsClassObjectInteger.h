/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2018, Roland Plüss (roland@rptd.ch)
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



#ifndef _DSCLASSOBJECTINTEGER_H_
#define _DSCLASSOBJECTINTEGER_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;


/**
 * \brief Immutable integer object class.
 * 
 * Wraps a primitive int value. Allows integer values which can be null.
 */
class DS_DLL_EXPORT dsClassObjectInteger : public dsClass {
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create script class. */
	dsClassObjectInteger();
	
	/** \brief Clean up script class. */
	virtual ~dsClassObjectInteger();
	/*@}*/
	
	
	
	/** @name Management */
	/*@{*/
	/** \brief Create class members. */
	void CreateClassMembers( dsEngine *engine );
	/*@}*/
	
	
	
private:
	struct sInitData{
		dsClass *clsObjInteger;
		
		dsClass *clsInteger;
		dsClass *clsBoolean;
		dsClass *clsString;
		dsClass *clsObject;
		dsClass *clsVoid;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	
	DEF_NATFUNC( nfNew );
	
	DEF_NATFUNC( nfValue );
	
	DEF_NATFUNC( nfToString );
	DEF_NATFUNC( nfHashCode );
	DEF_NATFUNC( nfEquals );
	DEF_NATFUNC( nfCompare );
#undef DEF_NATFUNC
};

#endif

