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
#include <stdlib.h>
#include <string.h>
#include "../../config.h"
#include "dsClass.h"
#include "dsFunction.h"
#include "dsFuncList.h"
#include "dsSignature.h"
#include "../utils/dsuString.h"
#include "../exceptions.h"

// dsFuncList
///////////////
// constructor, destructor
dsFuncList::dsFuncList(){
	p_Funcs = NULL;
	p_FuncCount = 0;
}
dsFuncList::~dsFuncList(){
	Clear();
}
// management
void dsFuncList::AddFunction(dsFunction *Function){
	if(!Function) DSTHROW(dueInvalidParam);
	dsFunction *addFunc = NULL;
	int i, fType;
	// replace all functions in the list matching this function with
	// the new function. only non-abstract functions can do this.
	if( ( Function->GetTypeModifiers() & DSTM_ABSTRACT ) != DSTM_ABSTRACT ){
		for( i=0; i<p_FuncCount; i++ ){
			if( ! Function->IsEqualTo( p_Funcs[ i ] ) ){
				continue;
			}
			
			fType = p_Funcs[i]->GetFuncType();
			if( fType != DSFT_FUNCTION && fType != DSFT_OPERATOR ){
				continue;
			}
			
			if( ( p_Funcs[ i ]->GetTypeModifiers() & DSTM_PRIVATE ) == DSTM_PRIVATE ){
				// private functions can not be replaced (bug: private function shadowing)
				continue;
			}
			
			// function is replaced
			p_Funcs[i] = Function;
		}
	}
	
	// enlarge list
	dsFunction **vNewArray = new dsFunction*[p_FuncCount+1];
	if(!vNewArray) DSTHROW(dueOutOfMemory);
	if(p_Funcs){
		for(i=0; i<p_FuncCount; i++) vNewArray[i] = p_Funcs[i];
		delete [] p_Funcs;
	}
	p_Funcs = vNewArray;
	
	// add function to list
	if(Function->GetTypeModifiers() & DSTM_ABSTRACT){
		// if this function is abstract we try to find the latest
		// function in the list that matches and is not abstract.
		for(i=0; i<p_FuncCount; i++){
			if(Function->IsEqualTo(p_Funcs[i])){
				fType = p_Funcs[i]->GetFuncType();
				if(fType==DSFT_FUNCTION || fType==DSFT_OPERATOR){
					if( !(p_Funcs[i]->GetTypeModifiers() & DSTM_ABSTRACT) ){
						addFunc = p_Funcs[i];
					}
				}
			}
		}
	}
	if(addFunc){
		vNewArray[p_FuncCount] = addFunc;
	}else{
		vNewArray[p_FuncCount] = Function;
	}
	p_FuncCount++;
}
dsFunction *dsFuncList::GetFunction(int Index) const{
	if(Index<0 || Index>=p_FuncCount) DSTHROW(dueOutOfBoundary);
	return p_Funcs[Index];
}
int dsFuncList::GetIndexOf(dsFunction *func) const{
	for(int i=0; i<p_FuncCount; i++){
		if(p_Funcs[i] == func) return i;
	}
	return -1;
}
void dsFuncList::Clear(){
	if(p_Funcs){
		delete [] p_Funcs;
		p_Funcs = NULL;
		p_FuncCount = 0;
	}
}
// filtering
/*
void dsFuncList::FilterByClass(dsuIndexList *Result, dsClass *Class){
	if(!Result || !Class) DSTHROW(dueInvalidParam);
	Result->RemoveAll();
	for(int i=0; i<p_FuncCount; i++){
		if(p_Funcs[i]->GetOwnerClass() == Class) Result->AddIndex(i);
	}
}
void dsFuncList::FilterByName(dsuIndexList *Result, const char *Name){
	if(!Result || !Class) DSTHROW(dueInvalidParam);
	Result->RemoveAll();
	for(int i=0; i<p_FuncCount; i++){
		if(strcmp(p_Funcs[i]->GetName(), Name) == 0) Result->AddIndex(i);
	}
}
*/
// searching
dsFunction *dsFuncList::FindBestFunction(dsClass *Class, const char *Name, dsSignature *Signature, bool nullCheck) const{
	dsFunction *vCurFunc, *vBestFunc;
	dsSignature *vCurSig;
	dsClass *vCurParam, *vCheckParam;
	int vCurMatch, vBestMatch, vFoundCount;
	int i, j, vCurParamCount, vCheckParamCount;
	// loop through all functions
	vBestFunc = NULL;
	vBestMatch = 0;
	vFoundCount = 0;
	vCheckParamCount = Signature->GetCount();
	for(i=p_FuncCount-1; i>=0; i--){
		// get function and check if the name matches
		// if the function is the same as the current one skip it
		vCurFunc = p_Funcs[i];
		if(vBestFunc == vCurFunc) continue;
		if(strcmp(vCurFunc->GetName(), Name) != 0) continue;
		if(Class && !Class->IsEqualTo(vCurFunc->GetOwnerClass())) continue;
		// get signature and check if the func-sig has at the same number of args as the check-sig
		vCurSig = vCurFunc->GetSignature();
		vCurParamCount = vCurSig->GetCount();
		vCurMatch = 0;
		if(vCurParamCount != vCheckParamCount) continue;
		// check if the args do match
		for(j=0; j<vCurParamCount; j++){
			vCurParam = vCurSig->GetParameter(j);
			if(j < vCheckParamCount){
				vCheckParam = Signature->GetParameter(j);
				if(vCheckParam->IsEqualTo(vCurParam)){
					vCurMatch++;
				}else if(vCheckParam->GetPrimitiveType() != DSPT_NULL){
					if(!vCheckParam->CastableTo(vCurParam)){
						vCurMatch = -1; break;
					}
				}
			}else{
				vCurMatch = -1; break; // here would be allowed default params. comes soon
			}
		}
		if(vCurMatch == -1) continue;
		// determine if this function is better
		if(vCurMatch == vCheckParamCount) return vCurFunc;
		if(vBestFunc){
			if(vCurMatch > vBestMatch){
				vBestFunc = vCurFunc;
				vBestMatch = vCurMatch;
				vFoundCount = 1;
			}else if(vCurMatch == vBestMatch){
				vFoundCount++;
			}
		}else{
			vBestFunc = vCurFunc;
			vFoundCount = 1;
			vBestMatch = vCurMatch;
		}
	}
	// return result
	return (vBestFunc && vFoundCount==1) ? vBestFunc : NULL;
}

// debuggin
void dsFuncList::DebugPrint() const{
	dsFunction *func;
	dsClass *owner;
	dsuString fullName;
	for(int i=0; i<p_FuncCount; i++){
		func = p_Funcs[i];
		owner = func->GetOwnerClass();
		fullName.Empty();
		owner->GetFullName(&fullName);
		printf("[funclist] %s.%s\n", fullName.Pointer(), func->GetName());
	}
}
