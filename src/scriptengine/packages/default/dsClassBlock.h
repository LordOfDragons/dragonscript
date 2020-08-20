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
#ifndef _DSCLASSBLOCK_H_
#define _DSCLASSBLOCK_H_

// includes
#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

// predefinitions
class dsEngine;
class dsValue;
class dsRealObject;

// class dsClassBlock
class dsClassBlock : public dsClass {
private:
public:
	// constructor, destructor
	dsClassBlock();
	~dsClassBlock();
	// management
	void CreateClassMembers(dsEngine *engine);
	
	/** Retrieves the owner of the block. */
	dsRealObject *GetOwner( dsRealObject *myself ) const;
	/** Retrieves the number of context variables. */
	int GetContextVariableCount( dsRealObject *myself ) const;
	/** Retrieves a context variable. */
	dsValue *GetContextVariableAt( dsRealObject *myself, int index ) const;
	
	/** \brief Get function signature of block. */
	const dsSignature &GetSignature( dsRealObject *myself );
	
	// internal functions
	void CreateBlock( dsRunTime *rt, dsRealObject *owner, dsFunction *func, dsValue *value, int contextVariableCount );
	void CastArguments(dsRunTime *rt, dsSignature *funcSig, int argCount);
	void RunIt( dsRunTime *rt, dsValue *myself );
	
	/** \brief Function index for run(). */
	inline int GetFuncIndexRun0() const{ return pFuncIndexRun0; }
	
	/** \brief Function index for run(Object). */
	inline int GetFuncIndexRun1() const{ return pFuncIndexRun1; }
	
	/** \brief Function index for run(Object,Object). */
	inline int GetFuncIndexRun2() const{ return pFuncIndexRun2; }
	
	/** \brief Function index for run(Object,Object,Object). */
	inline int GetFuncIndexRun3() const{ return pFuncIndexRun3; }
	
	/** \brief Function index for run(Object,Object,Object,Object). */
	inline int GetFuncIndexRun4() const{ return pFuncIndexRun4; }
	
	/** \brief Function index for run(Object,Object,Object,Object,Object). */
	inline int GetFuncIndexRun5() const{ return pFuncIndexRun5; }
	
	/** \brief Function index for run(Array). */
	inline int GetFuncIndexRunWith() const{ return pFuncIndexRunWith; }
	
	
	
private:
	int pFuncIndexRun0;
	int pFuncIndexRun1;
	int pFuncIndexRun2;
	int pFuncIndexRun3;
	int pFuncIndexRun4;
	int pFuncIndexRun5;
	int pFuncIndexRunWith;
	
	struct sInitData{
		dsClass *clsBlock, *clsVoid, *clsInt, *clsBool, *clsObj, *clsArr;
	};
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	DEF_NATFUNC(nfDestructor);
	DEF_NATFUNC(nfRun0);
	DEF_NATFUNC(nfRun1);
	DEF_NATFUNC(nfRun2);
	DEF_NATFUNC(nfRun3);
	DEF_NATFUNC(nfRun4);
	DEF_NATFUNC(nfRun5);
	DEF_NATFUNC(nfRunWith);
	DEF_NATFUNC(nfHashCode);
	DEF_NATFUNC(nfEquals);
//	DEF_NATFUNC(nfToString);
#undef DEF_NATFUNC
};

// end of include only once
#endif

