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

#include <new>       // For std::launder and placement new
#include <cstddef>   // For std::max_align_t
#include <cstdint>   // For uintptr_t
#include <utility>   // for std::forward

#include "dragonscript_config.h"
#include "dragonscript_export.h"

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


/** Align to at least 16 bytes on Windows to prevent SIMD crashes. */
template<typename T>
consteval size_t dsAllocAlignment(){
	// std::max_align_t is often 16 on modern Windows/Linux to support x64 SIMD.
	return (alignof(T) > alignof(std::max_align_t)) ? alignof(T) : alignof(std::max_align_t);
}


/** Calculate the required data size to store data. */
template<typename T>
consteval size_t dsAllocSize(){
	return sizeof(T) + (dsAllocAlignment<T>() - 1);
}


/** Heap allocation placement new. */
template<typename T, typename... Args>
inline T* dsAllocPlacementNew(void *buffer, Args&&... args){
	const uintptr_t addr = reinterpret_cast<uintptr_t>(buffer);
	constexpr size_t align = dsAllocAlignment<T>();
	const uintptr_t alignedAddr = (addr + (align - 1)) & ~(align - 1);

	return new (reinterpret_cast<T*>(alignedAddr)) T(std::forward<Args>(args)...);
}


/** Class data aligned offset. */
template<typename T>
inline size_t dsClassDataAlignOffset(size_t offset){
	constexpr size_t minAlign = sizeof(void*);
	constexpr size_t align = (alignof(T) > minAlign) ? alignof(T) : minAlign;
	return (offset + (align - 1)) & ~(align - 1);
}


/** Aligned offset. */
template<typename T>
inline size_t dsClassDataNatDatAlignOffset(size_t offset){
	constexpr size_t align = dsAllocAlignment<T>();
	return (offset + (align - 1)) & ~(align - 1);
}


/** Byte code stride. */
template<typename T>
consteval size_t dsByteCodeStride(){
	constexpr size_t minAlign = sizeof(void*);
	constexpr size_t align = (alignof(T) > minAlign) ? alignof(T) : minAlign;
	return (sizeof(T) + align - 1) & ~(align - 1);
}


/**
 * Native class data size. Use this in native script classes when calling p_SetNativeDataSize.
 */
template<typename T>
consteval int dsNativeDataSize(){
	// return (int)(sizeof(T) + (dsAllocAlignment<T>() - 1));
	return (int)sizeof(T);
}


/**
 * Placement new of native class data. Use this in native script classes on newly created,
 * naked dsValue instances to obtain the native data.
 */
template<typename T, typename... Args>
inline T& dsNativeDataNew(void *buffer, Args&&... args){
	/*
	const uintptr_t addr = reinterpret_cast<uintptr_t>(buffer);
	constexpr size_t align = dsAllocAlignment<T>();
	const uintptr_t alignedAddr = (addr + (align - 1)) & ~(align - 1);

	return *(new (reinterpret_cast<T*>(alignedAddr)) T(std::forward<Args>(args)...));
	*/
	return *(new (reinterpret_cast<T*>(buffer)) T(std::forward<Args>(args)...));
}


/**
 * Get native class data. Use this in native script classes to access the native data
 * previously initialized with dsNativeDataNew.
 */
template<typename T>
inline T& dsNativeDataGet(void *buffer){
	/*
	const uintptr_t addr = reinterpret_cast<uintptr_t>(buffer);
	constexpr size_t align = dsAllocAlignment<T>();
	const uintptr_t aligned_addr = (addr + (align - 1)) & ~(align - 1);
	
	// std::launder is mandatory to prevent MSVC from caching old pointer values in registers
	// during optimization. prevents strange heap corruption bugs.
	return *std::launder(reinterpret_cast<T*>(aligned_addr));
	*/
	return *std::launder(reinterpret_cast<T*>(buffer));
}


#endif

