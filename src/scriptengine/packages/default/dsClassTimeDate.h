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

#ifndef _DSCLASSTIMEDATE_H_
#define _DSCLASSTIMEDATE_H_

#include "../../objects/dsClass.h"
#include "../../objects/dsFunction.h"

class dsEngine;
class dsValue;



/**
 * \brief Time/Date class.
 */
class DS_DLL_EXPORT dsClassTimeDate : public dsClass {
public:
	struct sTimeDate{
		int year;
		int month;
		int day;
		int dayOfWeek;
		int dayOfYear;
		int hour;
		int minute;
		int second;
	};
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create native class. */
	dsClassTimeDate();
	
	/** \brief Clean up native class. */
	virtual ~dsClassTimeDate();
	/*@}*/
	
	
	
public:
	/** \name Management */
	/*@{*/
	/** \brief Create class members. */
	void CreateClassMembers( dsEngine *engine );
	
	/** \brief Get time from object. */
	sTimeDate GetTimeDate( dsRealObject *myself ) const;
	
	/** \brief Push time. */
	void PushTimeDate( dsRunTime *rt, const sTimeDate &timeDate );
	/*@}*/
	
	
	
private:
	struct sInitData{
		dsClass *clsTimeDate;
		dsClass *clsVoid;
		dsClass *clsInteger;
		dsClass *clsBoolean;
		dsClass *clsObject;
		dsClass *clsString;
	};
	
#define DEF_NATFUNC(name) \
	class name : public dsFunction{ \
	public: \
		name(const sInitData &init); \
		void RunFunction( dsRunTime *rt, dsValue *myself ); \
	}
	
	DEF_NATFUNC( nfNew );
	DEF_NATFUNC( nfNew2 );
	DEF_NATFUNC( nfNew3 );
	DEF_NATFUNC( nfDestructor );
	
	DEF_NATFUNC( nfGetYear );
	DEF_NATFUNC( nfGetMonth );
	DEF_NATFUNC( nfGetDay );
	DEF_NATFUNC( nfGetDayOfWeek );
	DEF_NATFUNC( nfGetDayOfYear );
	DEF_NATFUNC( nfGetHour );
	DEF_NATFUNC( nfGetMinute );
	DEF_NATFUNC( nfGetSecond );
	DEF_NATFUNC( nfFormat );
	DEF_NATFUNC( nfAdd );
	DEF_NATFUNC( nfSecondsSince );
	
	DEF_NATFUNC( nfHashCode );
	DEF_NATFUNC( nfEquals );
	DEF_NATFUNC( nfToString );
#undef DEF_NATFUNC
};

#endif
