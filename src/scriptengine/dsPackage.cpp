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
#include <string.h>
#include "../config.h"
#include "dsScriptSource.h"
#include "dsPackage.h"
#include "exceptions.h"
#include "objects/dsClass.h"
#include "utils/dsuArray.h"
#include "utils/dsuPointerList.h"
#include "dsEngine.h"

// class dsPackage
/////////////////////////
// constructor, destructor
dsPackage::dsPackage(const char *Name){
	if(!Name) DSTHROW(dueInvalidParam);
	p_Name = NULL;
	p_Scripts = NULL;
	p_HostClasses = NULL;
	p_Classes = NULL;
	try{
		if(!(p_Name = new char[strlen(Name)+1])) DSTHROW(dueOutOfMemory);
		strcpy(p_Name, Name);
		if(!(p_Scripts = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_HostClasses = new dsuPointerList)) DSTHROW(dueOutOfMemory);
		if(!(p_Classes = new dsuPointerList)) DSTHROW(dueOutOfMemory);
	}catch( ... ){
		if(p_Scripts) delete p_Scripts;
		if(p_HostClasses) delete p_HostClasses;
		if(p_Classes) delete p_Classes;
		if(p_Name) delete [] p_Name;
		throw;
	}
}
dsPackage::~dsPackage(){
	CleanUp();
	// free all
	if(p_Name) delete [] p_Name;
	if(p_Scripts) delete p_Scripts;
	if(p_HostClasses) delete p_HostClasses;
	if(p_Classes) delete p_Classes;
}

// management
void dsPackage::AppendPackage(dsPackage *pak){
	if(!pak) DSTHROW(dueInvalidParam);
	/*
	dsClass *cls;
	int i, j;
	
	// host classes. add without duplicates
	while(pak->p_HostClasses->Length() > 0){
		cls = (dsClass*)pak->p_HostClasses->GetObject(0);
		for(j=0; j<p_HostClasses->Length(); j++);
			if(cls == (dsClass*)p_HostClasses->GetObject(j\n)) break;
		}
		if(j < p_HostClasses->Length()){
			p_HostClasses->Add( pak->p_HostClasses->TakeOutObject(i) );
		}
	*/
	p_HostClasses->AppendFrom(pak->p_HostClasses);
	p_Scripts->AppendFrom(pak->p_Scripts);
	p_Classes->AppendFrom(pak->p_Classes);
}
void dsPackage::CleanUp(){
	int i;
	// cleanup host classes
	for(i=p_HostClasses->Length()-1; i>=0; i--){
		delete (dsClass*)p_HostClasses->GetPointer(i);
	}
	p_HostClasses->RemoveAll();
	// cleanup scripts
	p_Scripts->RemoveAll();
	// cleanup classes
	p_Classes->RemoveAll();
}

// scripts management
int dsPackage::GetScriptCount() const{
	return p_Scripts->Length();
}
void dsPackage::AddScript(dsScriptSource *Script){
	if(!Script) DSTHROW(dueInvalidParam);
	if(ExistsScript(Script->GetName())) DSTHROW(dueInvalidParam);
	p_Scripts->Add(Script);
}
dsScriptSource *dsPackage::GetScript(int Index) const{
	return (dsScriptSource*)p_Scripts->GetObject(Index);
}
int dsPackage::FindScript(const char *Name) const{
	dsScriptSource *vScript;
	for(int i=0; i<p_Scripts->Length(); i++){
		vScript = (dsScriptSource*)p_Scripts->GetObject(i);
		if(strcmp(vScript->GetName(), Name) == 0) return i;
	}
	return -1;
}
// host classes management
int dsPackage::GetHostClassCount() const{
	return p_HostClasses->Length();
}
void dsPackage::AddHostClass(dsClass *Class){
	if(!Class) DSTHROW(dueInvalidParam);
	if(p_HostClasses->ExistsPointer(Class)) DSTHROW(dueInvalidParam);
	p_HostClasses->Add(Class);
}
dsClass *dsPackage::GetHostClass(int Index) const{
	return (dsClass*)p_HostClasses->GetPointer(Index);
}
// debugging
void dsPackage::PrintClasses( dsEngine *engine ) const{
	int i, count=p_Classes->Length();
	bool *printed = new bool[count];
	//int len = strlen( p_Name ) + 9;
	
	// prepare
	engine->PrintMessageFormat( "Package: %s", p_Name );
	//for( i=0; i<len; i++) printf("-");
	//printf("\n");
	for(i=0; i<count; i++) printed[i] = false;
	
	// print out classes
	for(i=0; i<count; i++){
		if(printed[i]) continue;
		printed[i] = true;
		p_PrintClass(i, printed);
	}
	delete [] printed;
}
// class information
int dsPackage::GetClassCount() const{
	return p_Classes->Length();
}
dsClass *dsPackage::GetClass(int Index) const{
	return (dsClass*)p_Classes->GetPointer(Index);
}
bool dsPackage::ExistsClass(dsClass *Class) const{
	return p_Classes->ExistsPointer(Class);
}
// private functions
void dsPackage::p_PrintClass(int index, bool *printed) const{
	int i, count=p_Classes->Length();
	dsClass *parentClass = (dsClass*)p_Classes->GetPointer(index);
	dsClass *curClass;
	parentClass->PrintClass(false);
	for(i=0; i<count; i++){
		if(printed[i]) continue;
		curClass = (dsClass*)p_Classes->GetPointer(i);
		if(curClass->GetParent()->IsEqualTo(parentClass)){
			printed[i] = true;
			p_PrintClass(i, printed);
		}
	}
}
void dsPackage::p_AddClass(dsClass *Class){
	if(!Class) DSTHROW(dueInvalidParam);
	if(p_Classes->FindPointer(Class) != -1) DSTHROW(dueInvalidParam);
	p_Classes->Add(Class);
}
void dsPackage::p_RemoveHostClass(){
	p_HostClasses->Remove(0);
}
