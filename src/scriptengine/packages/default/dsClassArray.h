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
#ifndef _DSARRAY_H_
#define _DSARRAY_H_

// includes
#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

// predefinitions
class dsEngine;
class dsValue;

// class dsClassArray
class dsClassArray : public dsClass {
private:
	dsClass *p_ClsObj;
public:
	// constructor, destructor
	dsClassArray();
	~dsClassArray();
	// management
	void CreateClassMembers(dsEngine *engine);
	// internal functions
	inline dsClass *GetClassObject() const{ return p_ClsObj; }
	
	/**
	 * \brief Create new array object.
	 * 
	 * For direct access by native classes.
	 */
	dsValue *CreateArray( dsRunTime *rt );
	
	/**
	 * \brief Create new array object with arguments from stack as initial content.
	 * 
	 * For direct access by native classes.
	 * 
	 * \param[in] argumentCount Count of arguments to consume.
	 */
	dsValue *CreateArray( dsRunTime *rt, int argumentCount );
	
	/**
	 * \brief Adds an object to the array.
	 * 
	 * For direct access by native classes.
	 */
	void AddObject( dsRunTime *rt, dsRealObject *myself, dsValue *value );
	
	/**
	 * \brief Removes an object from the array.
	 * 
	 * For direct access by native classes.
	 */
	void RemoveObject( dsRunTime *rt, dsRealObject *myself, int index );
	
	/**
	 * \brief Count of items.
	 * 
	 * For direct access by native classes.
	 */
	int GetObjectCount( dsRunTime *rt, dsRealObject *myself );
	
	/**
	 * \brief Get object at index.
	 * 
	 * For direct access by native classes.
	 */
	dsValue *GetObjectAt( dsRunTime *rt, dsRealObject *myself, int index );
	
	/**
	 * \brief Set object at index.
	 * 
	 * For direct access by native classes.
	 */
	void SetObjectAt( dsRunTime *rt, dsRealObject *myself, int index, dsRealObject *object );
	
private:
	struct sInitData{
		dsClass *clsArr, *clsVoid, *clsInt, *clsObj, *clsBool, *clsBlock, *clsStr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC( nfCreate );
	DEF_NATFUNC( nfCopy );
	DEF_NATFUNC( nfCreateSize );
	DEF_NATFUNC( nfCreateCount );
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
	
	DEF_NATFUNC(nfLength);
	DEF_NATFUNC(nfSize);
	DEF_NATFUNC(nfSetSize);
	DEF_NATFUNC(nfResize);
	DEF_NATFUNC( nfAdd );
	DEF_NATFUNC( nfAddAll );
	DEF_NATFUNC( nfInsert );
	DEF_NATFUNC( nfInsertAll );
	DEF_NATFUNC( nfIndexOf );
	DEF_NATFUNC( nfHas );
	DEF_NATFUNC(nfRemove);
	DEF_NATFUNC(nfRemove2);
	DEF_NATFUNC( nfRemoveFrom );
	DEF_NATFUNC(nfRemoveAll);
	DEF_NATFUNC(nfSetObject);
	DEF_NATFUNC(nfMove);
	DEF_NATFUNC( nfGetObject );
	
	DEF_NATFUNC( nfForEach );
	DEF_NATFUNC( nfForEach2 );
	DEF_NATFUNC( nfForEach3 );
	DEF_NATFUNC( nfForEachReverse );
	DEF_NATFUNC( nfForEachWhile );
	DEF_NATFUNC( nfForEachWhile2 );
	DEF_NATFUNC( nfForEachWhile3 );
	DEF_NATFUNC( nfForEachWhileReverse );
	DEF_NATFUNC( nfMap );
	DEF_NATFUNC( nfMap2 );
	DEF_NATFUNC( nfMap3 );
	DEF_NATFUNC( nfMapReverse );
	DEF_NATFUNC( nfCollect );
	DEF_NATFUNC( nfCollect2 );
	DEF_NATFUNC( nfCollect3 );
	DEF_NATFUNC( nfCollectReverse );
	DEF_NATFUNC( nfFold );
	DEF_NATFUNC( nfFold2 );
	DEF_NATFUNC( nfFold3 );
	DEF_NATFUNC( nfFoldReverse );
	DEF_NATFUNC( nfCount );
	DEF_NATFUNC( nfCount2 );
	DEF_NATFUNC( nfCount3 );
	DEF_NATFUNC( nfCountReverse );
	DEF_NATFUNC( nfFind );
	DEF_NATFUNC( nfFind2 );
	DEF_NATFUNC( nfFind3 );
	DEF_NATFUNC( nfFindReverse );
	DEF_NATFUNC( nfInject );
	DEF_NATFUNC( nfInject2 );
	DEF_NATFUNC( nfInject3 );
	DEF_NATFUNC( nfInjectReverse );
	DEF_NATFUNC( nfRemoveIf );
	DEF_NATFUNC( nfSlice );
	DEF_NATFUNC( nfSlice2 );
	DEF_NATFUNC( nfSlice3 );
	
	DEF_NATFUNC( nfSort );
	DEF_NATFUNC( nfSort2 );
	DEF_NATFUNC( nfSorted );
	DEF_NATFUNC( nfSorted2 );
	
	DEF_NATFUNC( nfRandom );
	
	DEF_NATFUNC( nfOpAdd );
	DEF_NATFUNC( nfOpAddAssign );
	
	DEF_NATFUNC( nfEquals );
	DEF_NATFUNC( nfToString );
#undef DEF_NATFUNC
};

// end of include only once
#endif

