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

#include <time.h>
#include <stdio.h>
#include "../../../config.h"
#include "dsClassTimeDate.h"
#include "../../dsEngine.h"
#include "../../dsRunTime.h"
#include "../../exceptions.h"
#include "../../objects/dsValue.h"
#include "../../objects/dsSignature.h"
#include "../../objects/dsRealObject.h"
#include "../../objects/dsClassParserInfo.h"

/*
class TimeDate:

constructors, destructors:
	// Create time-date object with current time and date.
	public func new()
	
	// Create time-date object.
	public func new( int year, int month, int day )
	
	// creates time-date object.
	public func new( int yeat, int month, int day, int hours, int minutes, int seconds )
	
	// Destructor
	public func destructor()
	
accessors:
	// Year.
	public func int getYear()
	
	// Month starting with 0 for January.
	public func int getMonth()
	
	// Day of month starting with 1 for the first day.
	public func int getDay()
	
	// Day of week starting with 0 for Sunday.
	public func int getDayOfWeek()
	
	// Day of year starting with 0 for the first day.
	public func int getDayOfYear()
	
	// Hours
	public func int getHours()
	
	// Minutes
	public func int getMinutes()
	
	// Seconds
	public func int getSeconds()
	
	// Format according to POSIX strftime implementation
	public func String format( String format )
	
	// Add time
	public func TimeDate add( int days, int hours, int minutes, int seconds )
	
	// Seconds since time
	public func int secondsSince( TimeDate timeDate )
	
misc stuff:
	// compares if the obj is equal to another object
	public func bool equals( Object other )
	
	// calculates hashCode of this object
	public func int hashCode()
	
	// returns the a string representation of the object
	public func String toString()
*/


#ifdef OS_W32
struct tm *localtime_r( const time_t *timep, struct tm *result ){
	return result = localtime( timep );
}
#endif


// Native functions
/////////////////////

// constructors, destructors
//////////////////////////////

// public func new()
dsClassTimeDate::nfNew::nfNew( const sInitData &init ) :
dsFunction( init.clsTimeDate, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassTimeDate::nfNew::RunFunction( dsRunTime *rt, dsValue *myself ){
	time_t timeVal = time( NULL );
	struct tm convTime;
	if( ! localtime_r( &timeVal, &convTime ) ){
		DSTHROW( dueInvalidParam );
	}
	
	dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	nd.year = convTime.tm_year + 1900;
	nd.month = convTime.tm_mon;
	nd.day = convTime.tm_mday;
	nd.dayOfWeek = convTime.tm_wday;
	nd.dayOfYear = convTime.tm_yday;
	nd.hour = convTime.tm_hour;
	nd.minute = convTime.tm_min;
	nd.second = convTime.tm_sec;
}

// public func new( int year, int month, int day )
dsClassTimeDate::nfNew2::nfNew2( const sInitData &init ) :
dsFunction( init.clsTimeDate, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInteger ); // year
	p_AddParameter( init.clsInteger ); // month
	p_AddParameter( init.clsInteger ); // day
}
void dsClassTimeDate::nfNew2::RunFunction( dsRunTime *rt, dsValue *myself ){
	const int year = rt->GetValue( 0 )->GetInt();
	const int month = rt->GetValue( 1 )->GetInt();
	const int day = rt->GetValue( 2 )->GetInt();
	if( year < 0 ){
		DSTHROW_INFO( dueInvalidParam, "year less than 0" );
	}
	if( month < 0 ){
		DSTHROW_INFO( dueInvalidParam, "month less than 0" );
	}
	if( month > 11 ){
		DSTHROW_INFO( dueInvalidParam, "month larger than 11" );
	}
	if( day < 1 ){
		DSTHROW_INFO( dueInvalidParam, "day less than 1" );
	}
	if( day > 31 ){
		DSTHROW_INFO( dueInvalidParam, "day larger than 31" );
	}
	
	struct tm convTime;
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = year - 1900;
	convTime.tm_mon = month;
	convTime.tm_mday = day;
	const time_t timeVal = mktime( &convTime );
	if( timeVal == ( time_t )-1 ){
		DSTHROW( dueInvalidParam );
	}
	
	memset( &convTime, 0, sizeof( convTime ) );
	if( ! localtime_r( &timeVal, &convTime ) ){
		DSTHROW( dueInvalidParam );
	}
	
	dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	nd.year = convTime.tm_year + 1900;
	nd.month = convTime.tm_mon;
	nd.day = convTime.tm_mday;
	nd.dayOfWeek = ( convTime.tm_wday - 1 + 7 ) % 7; // 0=sundary but we want 0=monday
	nd.dayOfYear = convTime.tm_yday;
	nd.hour = convTime.tm_hour;
	nd.minute = convTime.tm_min;
	nd.second = convTime.tm_sec;
}

// public func new( int yeat, int month, int day, int hours, int minutes, int seconds )
dsClassTimeDate::nfNew3::nfNew3( const sInitData &init ) :
dsFunction( init.clsTimeDate, DSFUNC_CONSTRUCTOR, DSFT_CONSTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
	p_AddParameter( init.clsInteger ); // year
	p_AddParameter( init.clsInteger ); // month
	p_AddParameter( init.clsInteger ); // day
	p_AddParameter( init.clsInteger ); // hour
	p_AddParameter( init.clsInteger ); // minute
	p_AddParameter( init.clsInteger ); // second
}
void dsClassTimeDate::nfNew3::RunFunction( dsRunTime *rt, dsValue *myself ){
	const int year = rt->GetValue( 0 )->GetInt();
	const int month = rt->GetValue( 1 )->GetInt();
	const int day = rt->GetValue( 2 )->GetInt();
	const int hour = rt->GetValue( 3 )->GetInt();
	const int minute = rt->GetValue( 4 )->GetInt();
	const int second = rt->GetValue( 5 )->GetInt();
	if( year < 0 ){
		DSTHROW_INFO( dueInvalidParam, "year less than 0" );
	}
	if( month < 0 ){
		DSTHROW_INFO( dueInvalidParam, "month less than 0" );
	}
	if( month > 11 ){
		DSTHROW_INFO( dueInvalidParam, "month larger than 11" );
	}
	if( day < 1 ){
		DSTHROW_INFO( dueInvalidParam, "day less than 1" );
	}
	if( day > 31 ){
		DSTHROW_INFO( dueInvalidParam, "day larger than 31" );
	}
	if( hour < 0 ){
		DSTHROW_INFO( dueInvalidParam, "hour less than 0" );
	}
	if( hour > 23 ){
		DSTHROW_INFO( dueInvalidParam, "hour larger than 23" );
	}
	if( minute < 0 ){
		DSTHROW_INFO( dueInvalidParam, "minute less than 0" );
	}
	if( minute > 59 ){
		DSTHROW_INFO( dueInvalidParam, "minute larger than 59" );
	}
	if( second < 0 ){
		DSTHROW_INFO( dueInvalidParam, "second less than 0" );
	}
	if( second > 60 ){
		DSTHROW_INFO( dueInvalidParam, "second larger than 60" );
	}
	
	struct tm convTime;
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = year - 1900;
	convTime.tm_mon = month;
	convTime.tm_mday = day;
	convTime.tm_hour = hour;
	convTime.tm_min = minute;
	convTime.tm_sec = second;
	const time_t timeVal = mktime( &convTime );
	if( timeVal == ( time_t )-1 ){
		DSTHROW( dueInvalidParam );
	}
	
	memset( &convTime, 0, sizeof( convTime ) );
	if( ! localtime_r( &timeVal, &convTime ) ){
		DSTHROW( dueInvalidParam );
	}
	
	dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	nd.year = convTime.tm_year + 1900;
	nd.month = convTime.tm_mon;
	nd.day = convTime.tm_mday;
	nd.dayOfWeek = convTime.tm_wday;
	nd.dayOfYear = convTime.tm_yday;
	nd.hour = convTime.tm_hour;
	nd.minute = convTime.tm_min;
	nd.second = convTime.tm_sec;
}

// public func destructor
dsClassTimeDate::nfDestructor::nfDestructor( const sInitData &init ) :
dsFunction( init.clsTimeDate, DSFUNC_DESTRUCTOR, DSFT_DESTRUCTOR,
DSTM_PUBLIC | DSTM_NATIVE, init.clsVoid ){
}
void dsClassTimeDate::nfDestructor::RunFunction( dsRunTime *rt, dsValue *myself ){
}



// public func int getYear()
dsClassTimeDate::nfGetYear::nfGetYear( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getYear", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetYear::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.year );
}

// public func int getMonth()
dsClassTimeDate::nfGetMonth::nfGetMonth( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getMonth", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetMonth::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.month );
}

// public func int getDay()
dsClassTimeDate::nfGetDay::nfGetDay( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getDay", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetDay::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.day );
}

// public func int getDayOfWeek()
dsClassTimeDate::nfGetDayOfWeek::nfGetDayOfWeek( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getDayOfWeek", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetDayOfWeek::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.dayOfWeek );
}

// public func int getDayOfYear()
dsClassTimeDate::nfGetDayOfYear::nfGetDayOfYear( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getDayOfYear", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetDayOfYear::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.dayOfYear );
}

// public func int getHour()
dsClassTimeDate::nfGetHour::nfGetHour( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getHour", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetHour::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.hour );
}

// public func int getMinute()
dsClassTimeDate::nfGetMinute::nfGetMinute( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getMinute", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetMinute::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.minute );
}

// public func int getSecond()
dsClassTimeDate::nfGetSecond::nfGetSecond( const sInitData &init ) :
dsFunction( init.clsTimeDate, "getSecond", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfGetSecond::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	rt->PushInt( nd.second );
}

// public func String format( String format )
dsClassTimeDate::nfFormat::nfFormat( const sInitData &init ) :
dsFunction( init.clsTimeDate, "format", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
	p_AddParameter( init.clsString ); // format
}
void dsClassTimeDate::nfFormat::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	const char * const format = rt->GetValue( 0 )->GetString();
	
	struct tm convTime;
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = nd.year - 1900;
	convTime.tm_mon = nd.month;
	convTime.tm_mday = nd.day;
	convTime.tm_hour = nd.hour;
	convTime.tm_min = nd.minute;
	convTime.tm_sec = nd.second;
	
	char buffer[ 1024 ];
	buffer[ strftime( buffer, 1024, format, &convTime ) ] = 0;
	rt->PushString( buffer );
}

// public func TimeDate add( int days, int hours, int minutes, int seconds )
dsClassTimeDate::nfAdd::nfAdd( const sInitData &init ) :
dsFunction( init.clsTimeDate, "add", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsTimeDate ){
	p_AddParameter( init.clsInteger ); // days
	p_AddParameter( init.clsInteger ); // hours
	p_AddParameter( init.clsInteger ); // minutes
	p_AddParameter( init.clsInteger ); // seconds
}
void dsClassTimeDate::nfAdd::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	const int days = rt->GetValue( 0 )->GetInt();
	const int hours = rt->GetValue( 1 )->GetInt();
	const int minutes = rt->GetValue( 2 )->GetInt();
	const int seconds = rt->GetValue( 3 )->GetInt();
	
	struct tm convTime;
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = nd.year - 1900;
	convTime.tm_mon = nd.month;
	convTime.tm_mday = nd.day;
	convTime.tm_hour = nd.hour;
	convTime.tm_min = nd.minute;
	convTime.tm_sec = nd.second;
	time_t timeVal = mktime( &convTime );
	if( timeVal == ( time_t )-1 ){
		DSTHROW( dueInvalidParam );
	}
	
	timeVal += days * 86400 + hours * 3600 + minutes * 60 + seconds;
	
	if( ! localtime_r( &timeVal, &convTime ) ){
		DSTHROW( dueInvalidParam );
	}
	
	dsClassTimeDate::sTimeDate timeDate;
	timeDate.year = convTime.tm_year + 1900;
	timeDate.month = convTime.tm_mon;
	timeDate.day = convTime.tm_mday;
	timeDate.dayOfWeek = convTime.tm_wday;
	timeDate.dayOfYear = convTime.tm_yday;
	timeDate.hour = convTime.tm_hour;
	timeDate.minute = convTime.tm_min;
	timeDate.second = convTime.tm_sec;
	
	( ( dsClassTimeDate* )GetOwnerClass() )->PushTimeDate( rt, timeDate );
}

// public func int secondsSince( TimeDate timeDate )
dsClassTimeDate::nfSecondsSince::nfSecondsSince( const sInitData &init ) :
dsFunction( init.clsTimeDate, "secondsSince", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
	p_AddParameter( init.clsTimeDate ); // timeDate
}
void dsClassTimeDate::nfSecondsSince::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate &clsTimeDate = *( ( dsClassTimeDate* )GetOwnerClass() );
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	const dsClassTimeDate::sTimeDate other( clsTimeDate.GetTimeDate( rt->GetValue( 0 )->GetRealObject() ) );
	
	struct tm convTime;
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = nd.year - 1900;
	convTime.tm_mon = nd.month;
	convTime.tm_mday = nd.day;
	convTime.tm_hour = nd.hour;
	convTime.tm_min = nd.minute;
	convTime.tm_sec = nd.second;
	const time_t timeVal1 = mktime( &convTime );
	if( timeVal1 == ( time_t )-1 ){
		DSTHROW( dueInvalidParam );
	}
	
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = other.year - 1900;
	convTime.tm_mon = other.month;
	convTime.tm_mday = other.day;
	convTime.tm_hour = other.hour;
	convTime.tm_min = other.minute;
	convTime.tm_sec = other.second;
	const time_t timeVal2 = mktime( &convTime );
	if( timeVal2 == ( time_t )-1 ){
		DSTHROW( dueInvalidParam );
	}
	
	rt->PushInt( ( int )( timeVal1 - timeVal2 ) );
}



// public func int hashCode()
dsClassTimeDate::nfHashCode::nfHashCode( const sInitData &init ) :
dsFunction(init.clsTimeDate, "hashCode", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsInteger ){
}
void dsClassTimeDate::nfHashCode::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	
	struct tm convTime;
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = nd.year - 1900;
	convTime.tm_mon = nd.month;
	convTime.tm_mday = nd.day;
	convTime.tm_hour = nd.hour;
	convTime.tm_min = nd.minute;
	convTime.tm_sec = nd.second;
	const time_t timeVal = mktime( &convTime );
	if( timeVal == ( time_t )-1 ){
		DSTHROW( dueInvalidParam );
	}
	
	rt->PushInt( ( int )timeVal );
}

// public func bool equals( Object obj )
dsClassTimeDate::nfEquals::nfEquals( const sInitData &init ) :
dsFunction( init.clsTimeDate, "equals", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsBoolean ){
	p_AddParameter( init.clsObject ); // obj
}
void dsClassTimeDate::nfEquals::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	dsClass * const clsTimeDate = ( dsClassTimeDate* )GetOwnerClass();
	dsValue * const obj = rt->GetValue( 0 );
	
	if( p_IsObjOfType( obj, clsTimeDate ) ){
		const dsClassTimeDate::sTimeDate &nd2 = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( obj ) );
		rt->PushBool( nd.year == nd2.year
			&& nd.month == nd2.month
			&& nd.day == nd2.day
			&& nd.hour == nd2.hour
			&& nd.minute == nd2.minute
			&& nd.second == nd2.second );
		
	}else{
		rt->PushBool( false );
	}
}

// public func String toString()
dsClassTimeDate::nfToString::nfToString( const sInitData &init ) :
dsFunction( init.clsTimeDate, "toString", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE, init.clsString ){
}
void dsClassTimeDate::nfToString::RunFunction( dsRunTime *rt, dsValue *myself ){
	const dsClassTimeDate::sTimeDate &nd = *( ( dsClassTimeDate::sTimeDate* )p_GetNativeData( myself ) );
	
	struct tm convTime;
	memset( &convTime, 0, sizeof( convTime ) );
	convTime.tm_year = nd.year - 1900;
	convTime.tm_mon = nd.month;
	convTime.tm_mday = nd.day;
	convTime.tm_hour = nd.hour;
	convTime.tm_min = nd.minute;
	convTime.tm_sec = nd.second;
	
	char buffer[ 200 ];
	buffer[ strftime( buffer, 200, "%Y-%m-%d %H:%M:%S", &convTime ) ] = 0;
	rt->PushString( buffer );
}



// Class dsClassTimeDate
//////////////////////////

// Constructor, destructor
////////////////////////////

dsClassTimeDate::dsClassTimeDate() :
dsClass( "TimeDate", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE ){
	GetParserInfo()->SetBase( "Object" );
	p_SetNativeDataSize( sizeof( dsClassTimeDate::sTimeDate ) );
}

dsClassTimeDate::~dsClassTimeDate(){
}



// Management
///////////////

void dsClassTimeDate::CreateClassMembers( dsEngine *engine ){
	sInitData init;
	
	init.clsTimeDate = this;
	init.clsVoid = engine->GetClassVoid();
	init.clsInteger = engine->GetClassInt();
	init.clsBoolean = engine->GetClassBool();
	init.clsObject = engine->GetClassObject();
	init.clsString = engine->GetClassString();
	
	AddFunction( new nfNew( init ) );
	AddFunction( new nfNew2( init ) );
	AddFunction( new nfNew3( init ) );
	AddFunction( new nfDestructor( init ) );
	
	AddFunction( new nfGetYear( init ) );
	AddFunction( new nfGetMonth( init ) );
	AddFunction( new nfGetDay( init ) );
	AddFunction( new nfGetDayOfWeek( init ) );
	AddFunction( new nfGetDayOfYear( init ) );
	AddFunction( new nfGetHour( init ) );
	AddFunction( new nfGetMinute( init ) );
	AddFunction( new nfGetSecond( init ) );
	AddFunction( new nfFormat( init ) );
	AddFunction( new nfAdd( init ) );
	AddFunction( new nfSecondsSince( init ) );
	
	AddFunction( new nfHashCode( init ) );
	AddFunction( new nfEquals( init ) );
	AddFunction( new nfToString( init ) );
}

dsClassTimeDate::sTimeDate dsClassTimeDate::GetTimeDate( dsRealObject *myself ) const{
	if( ! myself ){
		DSTHROW( dueInvalidParam );
	}
	return *( ( sTimeDate* )p_GetNativeData( myself->GetBuffer() ) );
}

void dsClassTimeDate::PushTimeDate( dsRunTime *rt, const sTimeDate &timeDate ){
	if( ! rt ){
		DSTHROW( dueInvalidParam );
	}
	
	rt->CreateObjectNakedOnStack( this );
	*( ( sTimeDate* )p_GetNativeData( rt->GetValue( 0 )->GetRealObject()->GetBuffer() ) ) = timeDate;
}
