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

#ifndef _DSCLASSENUMERATION_H_
#define _DSCLASSENUMERATION_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;

/**
 * \brief Immutable enumeration class.
 */
class DS_DLL_EXPORT dsClassEnumeration : public dsClass {
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create script class. */
	dsClassEnumeration();
	
	/** \brief Clean up script class. */
	virtual ~dsClassEnumeration();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Create class members. */
	void CreateClassMembers( dsEngine *engine );
	
	/** \brief Get name of constant. */
	const dsuString &GetConstantName( dsRealObject &object ) const;
	
	/** \brief Get order of constant. */
	int GetConstantOrder( dsRealObject &object ) const;
	
	/** \brief Create enumeration constant and store it into value. */
	void CreateConstant( dsRunTime *rt, dsClass *enumClass, dsValue *value, const char *name, int order );
	
	/**
	 * \brief Create enumeration constants and stores them into static variables.
	 * 
	 * Convenience method to fill static variables with constants. For each static variable
	 * in the class creates a constant with the same name as the variable and the order set
	 * to the index of the variable.
	 */
	void CreateConstants( dsRunTime *rt, dsClass *enumClass );
	
	/** \brief Generate methods for subclass. */
	static void GenerateMethods( const dsEngine &engine, dsClass &subClass );
	
	/** \brief Init base class constant for generated subclass call only. */
	void SubClassInitConstant( dsClass &subClass, dsRealObject &myself, const char *name, int order ) const;
	/*@}*/
	
	
	
private:
	
	
	struct sInitData{
		dsClass *clsEnumeration;
		dsClass *clsInteger;
		dsClass *clsBoolean;
		dsClass *clsString;
		dsClass *clsObject;
		dsClass *clsVoid;
		dsClass *clsSet;
		dsClass *clsBlock;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	
	DEF_NATFUNC( nfNew );
	DEF_NATFUNC( nfDestructor );
	
	DEF_NATFUNC( nfOrder );
	DEF_NATFUNC( nfName );
	
	DEF_NATFUNC( nfSet );
	DEF_NATFUNC( nfSetWithout );
	
	DEF_NATFUNC( nfToString );
	DEF_NATFUNC( nfHashCode );
	DEF_NATFUNC( nfCompare );
	
	DEF_NATFUNC( nfAll );
	DEF_NATFUNC( nfNone );
	DEF_NATFUNC( nfForEach );
	DEF_NATFUNC( nfNamed );
	DEF_NATFUNC( nfWithOrder );
	DEF_NATFUNC( nfOpLess );
	DEF_NATFUNC( nfOpLessEqual );
	DEF_NATFUNC( nfOpGreater );
	DEF_NATFUNC( nfOpGreaterEqual );
#undef DEF_NATFUNC
};

#endif

