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



// includes
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "dsClassMath.h"

// native functions
/////////////////////
// public static const function int abs(int n)
dsClassMath::nfabs::nfabs(const sInitData &init) : dsFunction(init.clsMath, "abs", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsInt){
	p_AddParameter(init.clsInt); // n
}
void dsClassMath::nfabs::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushInt( abs( rt->GetValue(0)->GetInt() ) );
}

// public static const function float acos(float n)
dsClassMath::nfacos::nfacos(const sInitData &init) : dsFunction(init.clsMath, "acos", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // n
}
void dsClassMath::nfacos::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)acos( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float asin(float n)
dsClassMath::nfasin::nfasin(const sInitData &init) : dsFunction(init.clsMath, "asin", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // n
}
void dsClassMath::nfasin::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)asin( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float atan(float n)
dsClassMath::nfatan::nfatan(const sInitData &init) : dsFunction(init.clsMath, "atan", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // n
}
void dsClassMath::nfatan::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)atan( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float atan2(float y, float x)
dsClassMath::nfatan2::nfatan2(const sInitData &init) : dsFunction(init.clsMath, "atan2", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // y
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nfatan2::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)atan2( rt->GetValue(0)->GetFloat(), rt->GetValue(1)->GetFloat() ) );
}

// public static const function float ceil(float x)
dsClassMath::nfceil::nfceil(const sInitData &init) : dsFunction(init.clsMath, "ceil", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nfceil::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)ceil( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float cos(float x)
dsClassMath::nfcos::nfcos(const sInitData &init) : dsFunction(init.clsMath, "cos", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}

void dsClassMath::nfcos::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)cos( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float cosh(float x)
dsClassMath::nfcosh::nfcosh(const sInitData &init) : dsFunction(init.clsMath, "cosh", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nfcosh::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)cosh( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float exp(float x)
dsClassMath::nfexp::nfexp(const sInitData &init) : dsFunction(init.clsMath, "exp", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}

void dsClassMath::nfexp::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)exp( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float fabs(float x)
dsClassMath::nffabs::nffabs(const sInitData &init) : dsFunction(init.clsMath, "fabs", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nffabs::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)fabs( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float floor(float x)
dsClassMath::nffloor::nffloor(const sInitData &init) : dsFunction(init.clsMath, "floor", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nffloor::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)floor( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float fmod(float x, float y)
dsClassMath::nffmod::nffmod(const sInitData &init) : dsFunction(init.clsMath, "fmod", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
	p_AddParameter(init.clsFlt); // y
}
void dsClassMath::nffmod::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)fmod( rt->GetValue(0)->GetFloat(), rt->GetValue(1)->GetFloat() ) );
}

// public static const function float log(float x)
dsClassMath::nflog::nflog(const sInitData &init) : dsFunction(init.clsMath, "log", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nflog::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)log( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float log10(float x)
dsClassMath::nflog10::nflog10(const sInitData &init) : dsFunction(init.clsMath, "log10", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nflog10::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)log10( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float pow(float x, float y)
dsClassMath::nfpow::nfpow(const sInitData &init) : dsFunction(init.clsMath, "pow", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
	p_AddParameter(init.clsFlt); // y
}
void dsClassMath::nfpow::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)pow( rt->GetValue(0)->GetFloat(), rt->GetValue(1)->GetFloat() ) );
}

// public static const function float sin(float x)
dsClassMath::nfsin::nfsin(const sInitData &init) : dsFunction(init.clsMath, "sin", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nfsin::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)sin( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float sinh(float x)
dsClassMath::nfsinh::nfsinh(const sInitData &init) : dsFunction(init.clsMath, "sinh", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nfsinh::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)sinh( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float sqrt(float x)
dsClassMath::nfsqrt::nfsqrt(const sInitData &init) : dsFunction(init.clsMath, "sqrt", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nfsqrt::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)sqrt( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float tan(float x)
dsClassMath::nftan::nftan(const sInitData &init) : dsFunction(init.clsMath, "tan", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nftan::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)tan( rt->GetValue(0)->GetFloat() ) );
}

// public static const function float tanh(float x)
dsClassMath::nftanh::nftanh(const sInitData &init) : dsFunction(init.clsMath, "tanh", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt){
	p_AddParameter(init.clsFlt); // x
}
void dsClassMath::nftanh::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushFloat( (float)tanh( rt->GetValue(0)->GetFloat() ) );
}

// public func int min( int a, int b )
dsClassMath::nfMinI::nfMinI( const sInitData &init ) : dsFunction( init.clsMath, "min", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsInt ){
	p_AddParameter( init.clsInt ); // a
	p_AddParameter( init.clsInt ); // b
}
void dsClassMath::nfMinI::RunFunction( dsRunTime *rt, dsValue *myself ){
	int a = rt->GetValue( 0 )->GetInt();
	int b = rt->GetValue( 1 )->GetInt();
	
	if( a < b ){
		rt->PushInt( a );
		
	}else{
		rt->PushInt( b );
	}
}

// public func float min( float a, float b )
dsClassMath::nfMinF::nfMinF( const sInitData &init ) : dsFunction( init.clsMath, "min", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt ){
	p_AddParameter( init.clsFlt ); // a
	p_AddParameter( init.clsFlt ); // b
}
void dsClassMath::nfMinF::RunFunction( dsRunTime *rt, dsValue *myself ){
	float a = rt->GetValue( 0 )->GetFloat();
	float b = rt->GetValue( 1 )->GetFloat();
	
	if( a < b ){
		rt->PushFloat( a );
		
	}else{
		rt->PushFloat( b );
	}
}

// public func int max( int a, int b )
dsClassMath::nfMaxI::nfMaxI( const sInitData &init ) : dsFunction( init.clsMath, "max", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsInt ){
	p_AddParameter( init.clsInt ); // a
	p_AddParameter( init.clsInt ); // b
}
void dsClassMath::nfMaxI::RunFunction( dsRunTime *rt, dsValue *myself ){
	int a = rt->GetValue( 0 )->GetInt();
	int b = rt->GetValue( 1 )->GetInt();
	
	if( a > b ){
		rt->PushInt( a );
		
	}else{
		rt->PushInt( b );
	}
}

// public func float max( float a, float b )
dsClassMath::nfMaxF::nfMaxF( const sInitData &init ) : dsFunction( init.clsMath, "max", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt ){
	p_AddParameter( init.clsFlt ); // a
	p_AddParameter( init.clsFlt ); // b
}
void dsClassMath::nfMaxF::RunFunction( dsRunTime *rt, dsValue *myself ){
	float a = rt->GetValue( 0 )->GetFloat();
	float b = rt->GetValue( 1 )->GetFloat();
	
	if( a > b ){
		rt->PushFloat( a );
		
	}else{
		rt->PushFloat( b );
	}
}

// public func clamp( int value, int lower, int upper )
dsClassMath::nfClampI::nfClampI( const sInitData &init ) : dsFunction( init.clsMath, "clamp", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsInt ){
	p_AddParameter( init.clsInt ); // value
	p_AddParameter( init.clsInt ); // lower
	p_AddParameter( init.clsInt ); // upper
}
void dsClassMath::nfClampI::RunFunction( dsRunTime *rt, dsValue *myself ){
	const int value = rt->GetValue( 0 )->GetInt();
	const int lower = rt->GetValue( 1 )->GetInt();
	const int upper = rt->GetValue( 2 )->GetInt();
	
	if( upper > lower ){
		if( value > upper ){
			rt->PushInt( upper );
			
		}else if( value < lower ){
			rt->PushInt( lower );
			
		}else{
			rt->PushInt( value );
		}
		
	}else{
		rt->PushInt( lower );
	}
}

// public func clamp( float value, float lower, float upper )
dsClassMath::nfClampF::nfClampF( const sInitData &init ) : dsFunction( init.clsMath, "clamp", DSFT_FUNCTION,
DSTM_PUBLIC | DSTM_NATIVE | DSTM_STATIC, init.clsFlt ){
	p_AddParameter( init.clsFlt ); // value
	p_AddParameter( init.clsFlt ); // lower
	p_AddParameter( init.clsFlt ); // upper
}
void dsClassMath::nfClampF::RunFunction( dsRunTime *rt, dsValue *myself ){
	const float value = rt->GetValue( 0 )->GetFloat();
	const float lower = rt->GetValue( 1 )->GetFloat();
	const float upper = rt->GetValue( 2 )->GetFloat();
	
	if( upper > lower ){
		if( value > upper ){
			rt->PushFloat( upper );
			
		}else if( value < lower ){
			rt->PushFloat( lower );
			
		}else{
			rt->PushFloat( value );
		}
		
	}else{
		rt->PushFloat( lower );
	}
}



// class dsClassMath
///////////////////////
// constructor, destructor
dsClassMath::dsClassMath() : dsClass("Math", DSCT_CLASS, DSTM_PUBLIC | DSTM_NATIVE) {
	GetParserInfo()->SetBase("Object");
	p_SetNativeDataSize(0);
}
dsClassMath::~dsClassMath(){ }

// management
void dsClassMath::CreateClassMembers(dsEngine *engine){
	sInitData init;
	
	// store classes
	init.clsMath = this;
	init.clsInt = engine->GetClassInt();
	init.clsFlt = engine->GetClassFloat();
	
	// enumeration eWindowStates:
	AddConstant( new dsConstant("M_E", init.clsFlt, (float)2.718282) );
	AddConstant( new dsConstant("M_LOG2E", init.clsFlt, (float)1.442695) );
	AddConstant( new dsConstant("M_LOG10E", init.clsFlt, (float)0.434294) );
	AddConstant( new dsConstant("M_LN2", init.clsFlt, (float)0.693147) );
	AddConstant( new dsConstant("M_LN10", init.clsFlt, (float)2.302585) );
	AddConstant( new dsConstant("M_PI", init.clsFlt, (float)3.141593) );
	AddConstant( new dsConstant("M_PI_2", init.clsFlt, (float)1.570796) );
	AddConstant( new dsConstant("M_PI_4", init.clsFlt, (float)0.785398) );
	AddConstant( new dsConstant("M_1_PI", init.clsFlt, (float)0.318310) );
	AddConstant( new dsConstant("M_2_PI", init.clsFlt, (float)0.636620) );
	AddConstant( new dsConstant("M_2_SQRTPI", init.clsFlt, (float)1.128379) );
	AddConstant( new dsConstant("M_SQRT2", init.clsFlt, (float)1.414214) );
	AddConstant( new dsConstant("M_SQRT1_2", init.clsFlt, (float)0.707107) );
	
	// math
	AddFunction(new nfabs(init));
	AddFunction(new nfacos(init));
	AddFunction(new nfasin(init));
	AddFunction(new nfatan(init));
	AddFunction(new nfatan2(init));
	AddFunction(new nfceil(init));
	AddFunction(new nfcos(init));
	AddFunction(new nfcosh(init));
	AddFunction(new nfexp(init));
	AddFunction(new nffabs(init));
	AddFunction(new nffloor(init));
	AddFunction(new nffmod(init));
	AddFunction(new nflog(init));
	AddFunction(new nflog10(init));
	AddFunction(new nfpow(init));
	AddFunction(new nfsin(init));
	AddFunction(new nfsinh(init));
	AddFunction(new nfsqrt(init));
	AddFunction(new nftan(init));
	AddFunction(new nftanh(init));
	AddFunction( new nfMinI( init ) );
	AddFunction( new nfMinF( init ) );
	AddFunction( new nfMaxI( init ) );
	AddFunction( new nfMaxF( init ) );
	AddFunction( new nfClampI( init ) );
	AddFunction( new nfClampF( init ) );
}
