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
#include "../../config.h"
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
	int vStrLen = strlen(String);
	if(vStrLen > 0){
		p_String = new char[vStrLen+1];
		if(!p_String) DSTHROW(dueOutOfMemory);
		strcpy(p_String, String);
		p_String[vStrLen] = 0;
	}else{
		p_String = new char[1];
		p_String[0] = '\0';
	}
}
dsuString::dsuString(const dsuString &String){
	int vStrLen = strlen(String.p_String);
	if(vStrLen > 0){
		p_String = new char[vStrLen+1];
		if(!p_String) DSTHROW(dueOutOfMemory);
		strcpy(p_String, String.p_String);
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
	char *vString = new char[Length()+2];
	if(!vString) DSTHROW(dueOutOfMemory);
	strncpy(vString, p_String, Index);
	vString[Index] = Char;
	strcpy(vString+Index+1, p_String+Index);
	vString[Length()+1] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::Insert(int Index, const char *String){
	if( (Index < 0) || (Index > Length()) ) DSTHROW(dueOutOfBoundary);
	char *vString = new char[Length() + strlen(String) + 1];
	if(!vString) DSTHROW(dueOutOfMemory);
	strncpy(vString, p_String, Index);
	strcpy(vString+Index, String);
	strcpy(vString+Index+strlen(String), p_String+Index);
	vString[Length() + strlen(String)] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::Delete(int Start, int aLength){
	if(aLength == 0) return;
	if( (Start < 0) || (Start >= Length()) ) DSTHROW(dueOutOfBoundary);
	if(aLength == -1) aLength = Length() - Start;
	char *vString = new char[Length() - aLength + 1];
	if(!vString) DSTHROW(dueOutOfMemory);
	strncpy(vString, p_String, Start);
	strcpy(vString+Start, p_String+Start+aLength);
	vString[Length() - aLength] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
// conversion
void dsuString::SetInt(int Value){
	char vNum[12];
	sprintf((char*)(&vNum), "%i", Value);
	char *vString = new char[strlen(vNum)+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	strcpy(vString, vNum);
	vString[strlen(vNum)] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::SetDouble(double Value){
	char vNum[20];
	sprintf((char*)(&vNum), "%f", Value);
	char *vString = new char[strlen(vNum)+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	strcpy(vString, vNum);
	vString[strlen(vNum)] = 0;
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
	vString[0] = 34;
	strcpy(vString+1, p_String);
	vString[Length()+1] = 34;
	vString[Length()+2] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
}
void dsuString::Unquote(){
	if(IsQuoted()){
		char *vString = new char[Length()-1];
		if(!vString) DSTHROW(dueOutOfMemory);
		strncpy(vString, p_String+1, Length()-2);
		vString[Length()-2] = 0;
		if(p_String) delete [] p_String;
		p_String = vString;
	}
}
// operators
const dsuString &dsuString::operator=(const char *String){
	char *vString = new char[strlen(String)+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	strcpy(vString, String);
	vString[strlen(String)] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
	return *this;
}
const dsuString &dsuString::operator=(const dsuString &String){
	char *vString = new char[String.Length()+1];
	if(!vString) DSTHROW(dueOutOfMemory);
	strcpy(vString, String.p_String);
	vString[String.Length()] = 0;
	if(p_String) delete [] p_String;
	p_String = vString;
	return *this;
}
// operator functions
const dsuString &dsuString::operator+=(const char *String){
	if(strlen(String) > 0){
		int vStrLen = Length() + strlen(String);
		char *vString = new char[vStrLen+1];
		if(!vString) DSTHROW(dueOutOfMemory);
		if(!IsEmpty()) strcpy(vString, p_String);
		if(strlen(String) > 0) strcpy(vString+Length(), String);
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

