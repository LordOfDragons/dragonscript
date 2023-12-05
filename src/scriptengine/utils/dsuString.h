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
#ifndef _CSTRING_H_
#define _CSTRING_H_

// include needed headers
#include <stdlib.h>
#include <string.h>
#include "dsuArrayable.h"

// class dsuString
class DS_DLL_EXPORT dsuString : public dsuArrayable{
protected:
	char *p_String;
public:
	// constructors, destructors
	dsuString();
	dsuString(const char *str);
	dsuString(const dsuString &String);
	~dsuString();
	// management
	inline int Length() const{ return ( int )strlen(p_String); }
	inline char *Pointer() const{ return p_String; }
	void Empty();
	inline bool IsEmpty() const{ return p_String[0] == 0; }
	char GetChar(int Position) const;
	void SetChar(int Position, char Char);
	void Resize(int Size);
	// modifying
	void MakeLower();
	void MakeUpper();
	void TrimLeft();
	void TrimRight();
	void Trim();
	void Reverse();
	void ReplaceChar(char OldChar, char NewChar);
	void Insert(int Index, char Char);
	void Insert(int Index, const char *String);
	void Delete(int Start, int aLength = -1);
	// conversion
	inline int ToInt() const{ return ( int )strtol( p_String, NULL, 10 ); }
	inline double ToDouble() const{ return ( float )strtof( p_String, NULL ); }
	void SetInt(int Value);
	void SetDouble(double Value);
	// quoting
	bool IsQuoted() const;
	void Quote();
	void Unquote();
	// operators
	const dsuString &operator=(const char *String);
	const dsuString &operator=(const dsuString &String);
	const dsuString &operator+=(const char *String);
	// assignement operators
	char operator[](int Position) const;
	char &operator[](int Position);
	// type cast operators
	inline operator char*() const{ return p_String; }
	// from arrayable
	//bool IsEqual(dsuArrayable *Object);
};

// end of include only once
#endif

