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



#ifndef _DSCLASSSTRING_H_
#define _DSCLASSSTRING_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;



/**
 * \brief String native class.
 * \author Roland Plüss
 * \version 1.0
 * \date 2015
 */
class dsClassString : public dsClass {
friend class dsRunTime;
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Creates a new class. */
	dsClassString();
	/** \brief Cleans up the string. */
	virtual ~dsClassString();
	/*@}*/
	
	/** @name Management */
	/*@{*/
	/** \brief Create class members. */
	void CreateClassMembers( dsEngine *engine );
	/** \brief Retrieve character string stored in an object. */
	const char *GetRealObjectString( dsRealObject *myself ) const;
	/*@}*/
	
private:
	void SetRealObjectString( dsRealObject *object, const char *string );
	
private:
	struct sInitData{
		dsClass *clsStr, *clsObj, *clsVoid, *clsByte, *clsBool, *clsInt, *clsFlt, *clsArr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	
	DEF_NATFUNC( nfCreate );
	DEF_NATFUNC( nfCreateFilled );
	DEF_NATFUNC( nfDestructor );
	
	DEF_NATFUNC( nfEmpty );
	DEF_NATFUNC( nfGetLength );
	DEF_NATFUNC( nfGetAt );
	DEF_NATFUNC( nfSubString );
	DEF_NATFUNC( nfSubString2 );
	DEF_NATFUNC( nfFormat );
	
	DEF_NATFUNC( nfFind );
	DEF_NATFUNC( nfFind2 );
	DEF_NATFUNC( nfFind3 );
	DEF_NATFUNC( nfFindAny );
	DEF_NATFUNC( nfFindAny2 );
	DEF_NATFUNC( nfFindAny3 );
	DEF_NATFUNC( nfFindString );
	DEF_NATFUNC( nfFindString2 );
	DEF_NATFUNC( nfFindString3 );
	DEF_NATFUNC( nfFindReverse );
	DEF_NATFUNC( nfFindReverse2 );
	DEF_NATFUNC( nfFindReverse3 );
	DEF_NATFUNC( nfFindAnyReverse );
	DEF_NATFUNC( nfFindAnyReverse2 );
	DEF_NATFUNC( nfFindAnyReverse3 );
	DEF_NATFUNC( nfFindStringReverse );
	DEF_NATFUNC( nfFindStringReverse2 );
	DEF_NATFUNC( nfFindStringReverse3 );
	
	DEF_NATFUNC( nfReverse );
	DEF_NATFUNC( nfSplit );
	DEF_NATFUNC( nfSplit2 );
	DEF_NATFUNC( nfReplace );
	DEF_NATFUNC( nfReplace2 );
	DEF_NATFUNC( nfReplaceString );
	
	DEF_NATFUNC( nfTrimLeft );
	DEF_NATFUNC( nfTrimRight );
	DEF_NATFUNC( nfTrimBoth );
	DEF_NATFUNC( nfToLower );
	DEF_NATFUNC( nfToUpper );
	DEF_NATFUNC( nfToInt );
	DEF_NATFUNC( nfToFloat );
	
	DEF_NATFUNC( nfCompare );
	DEF_NATFUNC( nfCompareNoCase );
	DEF_NATFUNC( nfStartsWith );
	DEF_NATFUNC( nfStartsWithNoCase );
	DEF_NATFUNC( nfEndsWith );
	DEF_NATFUNC( nfEndsWithNoCase );
	
	DEF_NATFUNC( nfHashCode );
	DEF_NATFUNC( nfEquals );
	DEF_NATFUNC( nfToString );
	
	DEF_NATFUNC( nfOpAdd );
	DEF_NATFUNC( nfOpAddByte );
	DEF_NATFUNC( nfOpAddBool );
	DEF_NATFUNC( nfOpAddInt );
	DEF_NATFUNC( nfOpAddFloat );
	DEF_NATFUNC( nfOpAddObject );
#undef DEF_NATFUNC
};

#endif

