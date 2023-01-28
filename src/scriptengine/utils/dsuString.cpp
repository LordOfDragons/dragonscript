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
#include <stdio.h>
#include <ctype.h>
#include "../dragonscript_config.h"
#include "dsuString.h"
#include "../exceptions.h"

// class dsuString
//////////////////
// constructors, destructors
dsuString::dsuString(){
	p_String = new char[1];
	p_String[0] = '\0';
}
dsuString::dsuString(const char *String){
	int vStrLen = ( int )strlen(String);
	if(vStrLen > 0){
		p_String = new char[vStrLen+1];
		if(!p_String) DSTHROW(dueOutOfMemory);
		#ifdef OS_W32_VS
			strncpy_s( p_String, vStrLen + 1, String, vStrLen );
		#else
			strncpy(p_String, String, vStrLen);
		#endif
		p_String[vStrLen] = 0;
	}else{
		p_String = new char[1];
		p_String[0] = '\0';
	}
}
dsuString::dsuString(const dsuString &String){
	int vStrLen = ( int )strlen(String.p_String);
	if(vStrLen > 0){
		p_String = new char[vStrLen+1];
		if(!p_String) DSTHROW(dueOutOfMemory);
		#ifdef OS_W32_VS
			strncpy_s( p_String, vStrLen + 1, String.p_String, vStrLen );
		#else
			strncpy(p_String, String.p_String, vStrLen);
		#endif
		p_String[vStrLen] = 0;
	}else{
		p_String = new char[1];
		p_String[0] = '\0';
	}
}
dsuString::~dsuString(){
	if(p_String) delete [] p_String;
	p_String = NULL;
}
// management
void dsuString::Empty(){
	if(p_String){
		delete [] p_String;
		p_String = new char[1];
		p_String[0] = '\0';
	}
}
char dsuString::GetChar(int Position) const{
	if( (Position < 0) || (Position >= Length()) ) DSTHROW(dueOutOfBoundary);
	return p_String[Position];
}
void dsuString::SetChar(int Position, char Char){
	if( (Position < 0) || (Position >= Length()) ) DSTHROW(dueOutOfBoundary);
	p_String[Position] = Char;
}
void dsuString::Resize(int Size){
	if(Size < 0) DSTHROW(dueInvalidParam);
	char *vNewString = new char[Size+1];
	if(!vNewString) DSTHROW(dueOutOfMemory);
	memset(vNewString, ' ', Size);
	vNewString[Size] = '\0';
	delete [] p_String;
	p_String = vNewString;
}
// modifying
void dsuString::MakeLower(){
	int i, len=Length();
	for(i=0; i<len; i++){
		p_String[i] = (char)tolower(p_String[i]);
	}
}
void dsuString::MakeUpper(){
	int i, len=Length();
	for(i=0; i<len; i++){
		p_String[i] = (char)toupper(p_String[i]);
	}
}
void dsuString::TrimLeft(){
	int i, len=Length();
	for(i=0; i<len; i++){
		if(!isspace(p_String[i])){
			Delete(0, i);
			return;
		}
	}
}
void dsuString::TrimRight(){
	for(int i=Length()-1; i>=0; i--){
		if(!isspace(p_String[i])){
			Delete(i+1);
			return;
		}
	}
}
void dsuString::Trim(){
	TrimLeft();
	TrimRight();
}
void dsuString::Reverse(){
	int i;
	int len = Length();
	int flen = len / 2;
	char vFlip;
	for(i=0; i<flen; i++){
		vFlip = p_String[i];
		p_String[i] = p_String[len-1-i];
		p_String[len-1-i] = vFlip;
	}
}
void dsuString::ReplaceChar(char OldChar, char NewChar){
	int i, len=Length();
	for(i=0; i<len; i++){
		if(p_String[i] == OldChar) p_String[i] = NewChar;
	}
}
void dsuString::Insert(int Index, char Char){
	if( (Index < 0) || (Index > Length()) ) DSTHROW(dueOutOfBoundary);
	const int size = Length();
	char *vString = new char[size + 2];
	if(!vString) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( vString, Index + 1, p_String, Index );
	#else
		strncpy(vString, p_String, Index);
	#endif
	vString[Index] = Char;
	#ifdef OS_W32_VS
		strncpy_s( vString+Index+1, size - Index + 1, p_String+Index, size - Index );
	#else
		strncpy(vString+Index+1, p_String+Index, size - Index);
	#endif
	vString[size+1] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::Insert(int Index, const char *String){
	if( (Index < 0) || (Index > Length()) ) DSTHROW(dueOutOfBoundary);
	const int size1 = Length();
	const int size2 = ( int )strlen( String );
	char *vString = new char[size1 + size2 + 1];
	if(!vString) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( vString, Index + 1, p_String, Index );
		strncpy_s( vString + Index, size2 + 1, String, size2 );
		strncpy_s( vString + Index + size2, size1 - Index + 1, p_String + Index, size1 - Index );
	#else
		strncpy(vString, p_String, Index);
		strncpy(vString+Index, String, size2);
		strncpy(vString+Index+size2, p_String+Index, size1-Index);
	#endif
	vString[ size1 + size2 ] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::Delete(int Start, int aLength){
	if(aLength == 0) return;
	if( (Start < 0) || (Start >= Length()) ) DSTHROW(dueOutOfBoundary);
	if(aLength == -1) aLength = Length() - Start;
	const int size1 = Length();
	const int size2 = size1 - ( Start + aLength );
	char *vString = new char[size1 - aLength + 1];
	if(!vString) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( vString, Start + 1, p_String, Start );
		strncpy_s( vString + Start, size2 + 1, p_String + Start + aLength, size2 );
	#else
		strncpy(vString, p_String, Start);
		strncpy(vString+Start, p_String+Start+aLength, size2);
	#endif
	vString[size1 - aLength] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
// conversion
void dsuString::SetInt(int Value){
	char vNum[12];
	snprintf((char*)(&vNum), sizeof( vNum ), "%i", Value);
	const int size = ( int )strlen( vNum );
	char *vString = new char[size+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( vString, size + 1, vNum, size );
	#else
		strncpy(vString, vNum, size);
	#endif
	vString[size] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::SetDouble(double Value){
	char vNum[20];
	snprintf((char*)(&vNum), sizeof( vNum ), "%f", Value);
	const int size = ( int )strlen( vNum );
	char *vString = new char[size+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( vString, size + 1, vNum, size );
	#else
		strncpy(vString, vNum, size);
	#endif
	vString[size] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
// quoting
bool dsuString::IsQuoted() const{
	if(Length() >= 2){
		return (p_String[0] == '"') && (p_String[Length()-1] == '"');
	}else{
		return false;
	}
}
void dsuString::Quote(){
	char *vString = new char[Length()+3];
	if(!vString) DSTHROW(dueOutOfMemory);
	const int size = Length();
	vString[0] = '"';
	#ifdef OS_W32_VS
		strncpy_s( vString + 1, size + 1, p_String, size );
	#else
		strncpy(vString+1, p_String, size);
	#endif
	vString[size+1] = '"';
	vString[size+2] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::Unquote(){
	if(IsQuoted()){
		char *vString = new char[Length()-1];
		if(!vString) DSTHROW(dueOutOfMemory);
		const int size = Length() - 2;
		#ifdef OS_W32_VS
			strncpy_s( vString, size + 1, p_String+1, size );
		#else
			strncpy(vString, p_String+1, size);
		#endif
		vString[size] = 0;
		if(p_String) delete [] p_String;
		p_String = vString;
	}
}
// operators
const dsuString &dsuString::operator=(const char *String){
	const int size = ( int )strlen( String );
	char *vString = new char[size+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( vString, size + 1, String, size );
	#else
		strncpy(vString, String, size);
	#endif
	vString[size] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
	return *this;
}
const dsuString &dsuString::operator=(const dsuString &String){
	const int size = String.Length();
	char *vString = new char[size+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	#ifdef OS_W32_VS
		strncpy_s( vString, size + 1, String.p_String, size );
	#else
		strncpy(vString, String.p_String, size);
	#endif
	vString[size] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
	return *this;
}
// operator functions
const dsuString &dsuString::operator+=(const char *String){
	if(strlen(String) > 0){
		const int size1 = Length();
		const int size2 = ( int )strlen( String );
		int vStrLen = size1 + size2;
		char *vString = new char[vStrLen+1];
		if(!vString) DSTHROW(dueOutOfMemory);
		if(size1 > 0){
			#ifdef OS_W32_VS
				strncpy_s( vString, size1 + 1, p_String, size1 );
			#else
				strncpy(vString, p_String, size1);
			#endif
		}
		if(size2 > 0){
			#ifdef OS_W32_VS
				strncpy_s( vString + size1, size2 + 1, String, size2 );
			#else
				strncpy(vString+size1, String, size2);
			#endif
		}
		vString[vStrLen] = 0;
		if(p_String) delete [] p_String;
		p_String = vString;
	}
	return *this;
}
// assignement operators
char dsuString::operator[](int Position) const{
	if( (Position < 0) || (Position >= Length()) ) DSTHROW(dueOutOfBoundary);
	return p_String[Position];
}
char &dsuString::operator[](int Position){
	if( (Position < 0) || (Position >= Length()) ) DSTHROW(dueOutOfBoundary);
	return p_String[Position];
}
// from arrayable
/*
bool dsuString::IsEqual(dsuArrayable *Object){
	return (strcmp(p_String, ((dsuString*)Object)->p_String) == 0);
}
*/

