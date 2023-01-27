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



// Project: DragonScript - Interpreter
// Version: 1.0
// File: main file, entry point, launch of interpreter

// includes
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../scriptengine/libdscript.h"
#include "../scriptengine/dsFile.h"
#include "dsiEngineManager.h"
#include "dsiClassSystem.h"
#include "dsiClassApplication.h"
#include "paksources/dsPakSrcLocaleFile.h"

#ifdef DSI_MEASURE_RUN_TIME
#include <sys/time.h>
#endif

// definitions
////////////////
#define DSI_PKG_APP		"DSI-Scripts"
#define DSI_PKG_HOST	"DSI-HostClasses"

// local variables
////////////////////
char *p_scriptFile = NULL;
char **p_scriptArgs = NULL;
int p_countScriptArgs = 0;
bool p_debugMode = false;
bool p_dispHelp = false;
bool p_argEndMode = false;
dsEngine *p_scriptEngine = NULL;
dsPackage *p_scriptPackage = NULL;
dsClass *p_clsApp = NULL;
int p_retCode = 0;

// function defintions
////////////////////////
bool parseCommandLine(int argc, char **argv);
bool initScriptEngine();
bool compileScript();
bool runScript();
void cleanUp();
dsClass *findAppClass(dsClass *aClass);

// main
/////////
int main(int argc, char *argv[]){
	// parse command line
	if(!parseCommandLine(argc, argv)){
		cleanUp();
		return -2;
	}
	// check if there is an input file specified
	if(!p_scriptFile){
		printf("-- Missing Script File!\n\n");
		p_dispHelp = true;
	}
	// display help if needed
	if(p_dispHelp){
		printf(
			"*** Dragon-Script Interpreter ***\n"
			"*********************************\n"
			"* Version 1.0\n"
			"*\n"
			"\n"
			"Syntax:\n"
			"  dsi [options] [--] scriptfile.ds [ arguments ... ]\n"
			"\n"
			"Options:\n"
			"  -h | --help      display this help screen\n"
			"  --               specifies the end of option arguments. any upcoming argument is\n"
			"                   unmodified to the script\n"
			"\n"
			"  --debug-mode     displays heavy debugging informations\n"
			"\n"
			);
		return -1;
	}
	// init script engine
	if(!initScriptEngine()){
		cleanUp();
		return -3;
	}
	// compile scripts
	if(!compileScript()){
		cleanUp();
		return -4;
	}
	// show engine informations
	if(p_debugMode){
		//p_scriptEngine->PrintClasses(false);
		for(int i=0; i<p_scriptEngine->GetPackageCount(); i++){
			p_scriptEngine->GetPackage(i)->PrintClasses( p_scriptEngine );
		}
//		p_scriptPackage->PrintClasses();
	}
	// run scripts
	if(!runScript()){
		cleanUp();
		return -5;
	}
	// clean up
	cleanUp();
	return p_retCode;
}


// parseCommandLine
/////////////////////
bool parseCommandLine(int argc, char **argv){
	char **newArray = NULL;
	char *curArg;
	int i, j;
	// parse command line
	for(i=1; i<argc; i++){
		curArg = argv[i];
		// check if argument is an option (and argEndMode is not set)
		if(!p_argEndMode && curArg[0]=='-'){
			// end of arguments mode. if this is found any arguments
			// following are considered arguments for the script
			if(strcmp(curArg, "--") == 0){
				p_argEndMode = true;
			// display help
			}else if(strcmp(curArg, "-h")==0 || strcmp(curArg, "--help")==0){
				p_dispHelp = true;
			// use debug mode
			}else if(strcmp(curArg, "--debug-mode") == 0){
				p_debugMode = true;
			// unknown argument
			}else{
				printf("[DSI] Unknown option '%s' found.\n\n", curArg);
				return false;
			}
		// argument is a script file (or argEndMode is set)
		}else{
			// check if there is already a script file specified
			if(p_scriptFile){
				// the argument is considered an argument for the script file
				if(!(newArray = new char*[p_countScriptArgs+1])){
					printf("[ERR] OutOfMemory!\n");
					return false;
				}
				if(p_scriptArgs){
					for(j=0; j<p_countScriptArgs; j++) newArray[j] = p_scriptArgs[j];
					delete [] p_scriptArgs;
				}
				p_scriptArgs = newArray;
				// add new argument
				p_scriptArgs[p_countScriptArgs++] = curArg;
			}else{
				p_scriptFile = curArg;
			}
		}
	}
	// finished parsing
	return true;
}

// initScriptEngine
/////////////////////
bool initScriptEngine(){
	dsPackage *vPackage = NULL;
	dsPackageSource *pkgSrc = NULL;
	dsiEngineManager *vEngMgr = NULL;
	dsiClassSystem *vClsSystem = NULL;
	dsiClassApplication *vClsApp = NULL;
	try{
		// prepare engine
		if(!(p_scriptEngine = new dsEngine)) DSTHROW(dueOutOfMemory);
		if(!(vEngMgr = new dsiEngineManager)) DSTHROW(dueOutOfMemory);
		p_scriptEngine->SetEngineManager(vEngMgr); vEngMgr = NULL;
		// add package sources
		pkgSrc = new dsPakSrcLocaleFile;
		if(!pkgSrc) DSTHROW(dueOutOfMemory);
		p_scriptEngine->AddPackageSource(pkgSrc); pkgSrc = NULL;
		// add host classes
		if(!(vPackage = new dsPackage(DSI_PKG_HOST))) DSTHROW(dueOutOfMemory);
		
		if(!(vClsSystem = new dsiClassSystem)) DSTHROW(dueOutOfMemory);
		vPackage->AddHostClass(vClsSystem);
		vClsSystem = NULL;

		if(!(vClsApp = new dsiClassApplication(p_countScriptArgs, p_scriptArgs))) DSTHROW(dueOutOfMemory);
		vPackage->AddHostClass(vClsApp);
		p_clsApp = vClsApp;
		vClsApp = NULL;
		
		if(!p_scriptEngine->AddPackage(vPackage)) DSTHROW(dseInvalidSyntax);
	}catch( const duException &e ){
		if(vPackage) delete vPackage;
		if(pkgSrc) delete pkgSrc;
		if(vClsSystem) delete vClsSystem;
		if(vClsApp) delete vClsApp;
		e.PrintError();
		return false;
	}catch( ... ){
		if(vPackage) delete vPackage;
		if(pkgSrc) delete pkgSrc;
		if(vClsSystem) delete vClsSystem;
		if(vClsApp) delete vClsApp;
		return false;
	}
	// finished
	return true;
}

// compileScript
///////////////////
bool compileScript(){
	dsPackage *vPackage = NULL;
	dsFile *vScriptFile = NULL;
	try{
		// create package
		if(!(vPackage = new dsPackage(DSI_PKG_APP))) DSTHROW(dueOutOfMemory);
		// create file from script
		if(!(vScriptFile = new dsFile(p_scriptFile))) DSTHROW(dueOutOfMemory);
		// add file to package
		vPackage->AddScript(vScriptFile);
		vScriptFile = NULL;
		// add package to engine. this compiles the package
		if(!p_scriptEngine->AddPackage(vPackage)) DSTHROW(dseInvalidSyntax);
		// store away package for extracting informations later on
		p_scriptPackage = vPackage;
	}catch( const duException &e ){
		if(vScriptFile) delete vScriptFile;
		if(vPackage) delete vPackage;
		e.PrintError();
		return false;
	}catch( ... ){
		if(vScriptFile) delete vScriptFile;
		if(vPackage) delete vPackage;
		return false;
	}
	// finished
	return true;
}

// runScript
///////////////
bool runScript(){
	dsRunTime *vRT = p_scriptEngine->GetMainRunTime();
	dsClass *vClsEntry = NULL;
	dsValue *vAppObj = NULL;
	dsFunction *vDefConstr = NULL;
	try{
		// create signature of function Main
		// void Main(void);
		// find entry class
		for(int i=0; i<p_scriptPackage->GetClassCount(); i++){
			dsClass *clsFound = findAppClass(p_scriptPackage->GetClass(i));
			if(clsFound){
				if(vClsEntry){
					DSTHROW_INFO(dseInvalidSyntax, "Multiple classes found extending 'Application'!");
				}
				vClsEntry = clsFound;
			}
		}
		if(!vClsEntry){
			DSTHROW_INFO(dseInvalidSyntax, "No Class found extending 'Application'!");
		}
		// create values
		vAppObj = vRT->CreateValue(vClsEntry);
		// check if there is a valid default constructor in the class
		vDefConstr = vClsEntry->GetFirstConstructor();
		if(!vDefConstr || vDefConstr->GetSignature()->GetCount() > 0){
			DSTHROW_INFO(dseInvalidSyntax, "No Constructor without Arguments found in application class!");
		}
		// create object and start execution
#ifdef DSI_MEASURE_RUN_TIME
		timeval mrtBegin;
		gettimeofday( &mrtBegin, NULL );
#endif
		try{
			vRT->CreateObject(vAppObj, vClsEntry, 0);
			vRT->RunFunction(vAppObj, "run", 0 );
		}catch( ... ){
			vRT->PrintExceptionTrace();
		}
#ifdef DSI_MEASURE_RUN_TIME
		timeval mrtEnd;
		gettimeofday( &mrtEnd, NULL );
		timeval mrtDifference;
		timersub( &mrtEnd, &mrtBegin, &mrtDifference );
		printf( "DSI: Script Run-Time %ld.%03lds\n", mrtDifference.tv_sec, mrtDifference.tv_usec / 1000 );
#endif
		p_retCode = vRT->GetReturnInt();
		// free object
		vRT->FreeValue(vAppObj);
	}catch( const duException &e ){
		if(vAppObj) vRT->FreeValue(vAppObj);
		e.PrintError();
		return false;
	}catch( ... ){
		if(vAppObj) vRT->FreeValue(vAppObj);
		return false;
	}
	// finished
	return true;
}

// cleanUp
////////////
void cleanUp(){
	if(p_scriptEngine) delete p_scriptEngine;
	if(p_scriptArgs) delete [] p_scriptArgs;
}

// findAppClass
/////////////////
dsClass *findAppClass(dsClass *aClass){
	dsClass *baseClass = aClass->GetBaseClass();
	if(baseClass && baseClass->IsEqualTo(p_clsApp)) return aClass;
/*
	if(aClass->GetClassType() & DSCT_NAMESPACE){
		dsClass *vClsEntry = NULL;
		for(int i=0; i<aClass->GetInnerClassCount(); i++){
			dsClass *clsFound = findAppClass(aClass->GetInnerClass(i));
			if(clsFound){
				if(vClsEntry){
					DSTHROW_INFO(dseInvalidSyntax, "Multiple classes found extending 'Application'!");
				}
				vClsEntry = clsFound;
			}
		}
		return vClsEntry;
	}
*/
	return NULL;
}
