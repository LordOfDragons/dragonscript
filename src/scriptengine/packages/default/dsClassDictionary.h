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

#ifndef _DSCLASSDICTIONARY_H_
#define _DSCLASSDICTIONARY_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;


/**
 * \brief Dictionary native class implementing an associative array.
 * \author Roland Plüss
 * \version 1.0
 * \date 2015
 */
class dsClassDictionary : public dsClass {
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Creates a new class. */
	dsClassDictionary();
	/** \brief Cleans up the string. */
	virtual ~dsClassDictionary();
	/*@}*/
	
	/** @name Management */
	/*@{*/
	/** \brief Create class members. */
	void CreateClassMembers( dsEngine *engine );
	
	/** \brief Creates a new dictionary object. For direct access by native classes. */
	dsValue *CreateDictionary( dsRunTime *rt );
	/** \brief Retrieves the value for a key. If found writes the value ot value if not NULL. Returns true if found. */
	bool GetValue( dsRunTime *rt, dsRealObject *myself, dsValue *key, dsValue **value ) const;
	/** \brief Adds an object to the dictionary. For direct access by native classes. */
	void SetValue( dsRunTime *rt, dsRealObject *myself, dsValue *key, dsValue *value ) const;
	/** \brief Removes an object from the dictionary. For direct access by native classes. Returns true if removed. */
	bool RemoveKey( dsRunTime *rt, dsRealObject *myself, dsValue *key ) const;
	/** \brief Remove all keys. */
	void RemoveAllKeys( dsRunTime *rt, dsRealObject *myself ) const;
	
	/** \brief Check dictionary load and re-size if required. */
	void CheckLoad( dsRunTime *rt, dsRealObject *myself ) const;
	/*@}*/
	
private:
	struct sInitData{
		dsClass *clsDict, *clsVoid, *clsInt, *clsObj, *clsBool, *clsBlock, *clsStr, *clsArr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	
	DEF_NATFUNC( nfCreate );
	DEF_NATFUNC( nfCopy );
	DEF_NATFUNC( nfDestructor );
	
	DEF_NATFUNC( nfGetCount );
	DEF_NATFUNC( nfHas );
	DEF_NATFUNC( nfGetAt );
	DEF_NATFUNC( nfGetAt2 );
	DEF_NATFUNC( nfSetAt );
	DEF_NATFUNC( nfSetAll );
	DEF_NATFUNC( nfRemove );
	DEF_NATFUNC( nfRemoveIfExisting );
	DEF_NATFUNC( nfRemoveAll );
	
	DEF_NATFUNC( nfGetKeys );
	DEF_NATFUNC( nfGetValues );
	
	DEF_NATFUNC( nfForEach );
	DEF_NATFUNC( nfForEachKey );
	DEF_NATFUNC( nfForEachValue );
	DEF_NATFUNC( nfFind );
	DEF_NATFUNC( nfFindKey );
	DEF_NATFUNC( nfMap );
	DEF_NATFUNC( nfCollect );
	DEF_NATFUNC( nfCount );
	DEF_NATFUNC( nfRemoveIf );
	DEF_NATFUNC( nfInject );
	DEF_NATFUNC( nfInjectKey );
	DEF_NATFUNC( nfInjectValue );
	
	DEF_NATFUNC( nfOpAdd );
	
	DEF_NATFUNC( nfEquals );
	DEF_NATFUNC( nfToString );
#undef DEF_NATFUNC
};

#endif

