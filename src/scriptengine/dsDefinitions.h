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
#ifndef _DSDEFINITIONS_H_
#define _DSDEFINITIONS_H_

// typedefs
typedef unsigned char byte;

// type modifiers
#define DSTM_PUBLIC			0x1
#define DSTM_PROTECTED		0x2
#define DSTM_PRIVATE		0x4

#define DSTM_NATIVE			0x8
#define DSTM_ABSTRACT		0x10
#define DSTM_FIXED			0x20
#define DSTM_STATIC			0x40

#define DSTM_ALLTYPES		0x7F

// class types
#define DSCT_CLASS			0
#define DSCT_INTERFACE		1
#define DSCT_NAMESPACE		2

// function types
#define DSFT_FUNCTION		0
#define DSFT_CONSTRUCTOR	1
#define DSFT_DESTRUCTOR		2
#define DSFT_OPERATOR		3

// primitive types
#define DSPT_VOID			0
#define DSPT_BYTE			1
#define DSPT_BOOL			2
#define DSPT_INT			3
#define DSPT_FLOAT			4
#define DSPT_NULL			5
#define DSPT_OBJECT			6

// destructor function name
#define DSFUNC_CONSTRUCTOR	"new"
#define DSFUNC_DESTRUCTOR	"destructor"

// inlines
inline bool dsiIsValidClassType(int ct){ return ct>=DSCT_CLASS && ct<=DSCT_NAMESPACE; }
inline bool dsiIsValidFuncType(int ft){ return ft>=DSFT_FUNCTION && ft<=DSFT_OPERATOR; }
inline bool dsiIsValidPrimType(int pt){ return pt>=DSPT_VOID && pt<=DSPT_OBJECT; }

// alignement
#define DS_ALIGNMENT sizeof( void* )

// end of include only once
#endif

