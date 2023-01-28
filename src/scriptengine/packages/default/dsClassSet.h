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

#ifndef _DSSET_H_
#define _DSSET_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;


/**
 * \brief Set script class.
 */
class DS_DLL_EXPORT dsClassSet : public dsClass {
private:
	dsClass *p_ClsObj;
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create script class. */
	dsClassSet();
	
	/** \brief Clean up script class. */
	virtual ~dsClassSet();
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Create class members. */
	void CreateClassMembers(dsEngine *engine);
	
	/** \brief Object class. */
	inline dsClass *GetClassObject() const{ return p_ClsObj; }
	
	/**
	 * \brief Create set.
	 * 
	 * For direct access by native classes.
	 */
	dsValue *CreateSet( dsRunTime *rt );
	
	/**
	 * \brief Create new set object with arguments from stack as initial content.
	 * 
	 * For direct access by native classes.
	 * 
	 * \param[in] argumentCount Count of arguments to consume.
	 */
	dsValue *CreateSet( dsRunTime *rt, int argumentCount );
	
	/**
	 * \brief Add object to set.
	 * 
	 * For direct access by native classes.
	 */
	void AddObject( dsRunTime *rt, dsRealObject *myself, dsValue *value ) const;
	
	/**
	 * \brief Remove object from set.
	 * 
	 * For direct access by native classes.
	 */
	void RemoveObject( dsRunTime *rt, dsRealObject *myself, int index ) const;
	
	/**
	 * \brief Count of items.
	 * 
	 * For direct access by native classes.
	 */
	int GetObjectCount( dsRunTime *rt, dsRealObject *myself ) const;
	
	/**
	 * \brief Get object at index.
	 * 
	 * For direct access by native classes.
	 */
	dsValue *GetObjectAt( dsRunTime *rt, dsRealObject *myself, int index ) const;
	
	/**
	 * \brief Object is present.
	 * 
	 * For direct access by native classes.
	 */
	bool HasObject( dsRunTime *rt, dsRealObject *myself, dsValue *value ) const;
	
	
	
private:
	struct sInitData{
		dsClass *clsSet;
		dsClass *clsVoid;
		dsClass *clsInteger;
		dsClass *clsObject;
		dsClass *clsBool;
		dsClass *clsBlock;
		dsClass *clsString;
		dsClass *clsArray;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC( nfNew );
	DEF_NATFUNC( nfCopy );
	DEF_NATFUNC( nfDestructor );
	DEF_NATFUNC( nfNewWith1 );
	DEF_NATFUNC( nfNewWith2 );
	DEF_NATFUNC( nfNewWith3 );
	DEF_NATFUNC( nfNewWith4 );
	DEF_NATFUNC( nfNewWith5 );
	DEF_NATFUNC( nfNewWith6 );
	DEF_NATFUNC( nfNewWith7 );
	DEF_NATFUNC( nfNewWith8 );
	DEF_NATFUNC( nfNewWith9 );
	DEF_NATFUNC( nfNewWith10 );
	DEF_NATFUNC( nfNewFromArray );
	
	DEF_NATFUNC( nfGetCount );
	DEF_NATFUNC( nfAdd );
	DEF_NATFUNC( nfAddAll );
	DEF_NATFUNC( nfHas );
	DEF_NATFUNC( nfRemove );
	DEF_NATFUNC( nfRemoveAll );
	DEF_NATFUNC( nfRemoveAll2 );
	
	DEF_NATFUNC( nfForEach );
	DEF_NATFUNC( nfForEachCastable );
	DEF_NATFUNC( nfForEachWhile );
	DEF_NATFUNC( nfForEachWhileCastable );
	DEF_NATFUNC( nfMap );
	DEF_NATFUNC( nfCollect );
	DEF_NATFUNC( nfCollectCastable );
	DEF_NATFUNC( nfFold );
	DEF_NATFUNC( nfFind );
	DEF_NATFUNC( nfFindCastable );
	DEF_NATFUNC( nfInject );
	DEF_NATFUNC( nfCount );
	DEF_NATFUNC( nfCountCastable );
	DEF_NATFUNC( nfRemoveIf );
	DEF_NATFUNC( nfRemoveIfCastable );
	
	DEF_NATFUNC( nfRandom );
	DEF_NATFUNC( nfToArray );
	
	DEF_NATFUNC( nfEquals );
	DEF_NATFUNC( nfToString );
	
	DEF_NATFUNC( nfOperatorPlus );
	DEF_NATFUNC( nfOperatorMinus );
	DEF_NATFUNC( nfOperatorPlusAssign );
	DEF_NATFUNC( nfOperatorMinusAssign );
#undef DEF_NATFUNC
};

#endif

