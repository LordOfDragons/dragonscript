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
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "dragonscript_config.h"
#include "dsEngine.h"
#include "dsPackage.h"
#include "dsRunTime.h"
#include "dsMemoryManager.h"
#include "dsGarbageCollector.h"
#include "dsDefaultEngineManager.h"
#include "utils/dsuStack.h"
#include "utils/dsuArray.h"
#include "utils/dsuString.h"
#include "parser/dspParser.h"
#include "objects/dsClass.h"
#include "objects/dsFunction.h"
#include "objects/dsVariable.h"
#include "objects/dsByteCode.h"
#include "objects/dsSignature.h"
#include "objects/dsClassParserInfo.h"
#include "packages/dsEnginePackages.h"
#include "packages/dsBaseEnginePackage.h"
#include "packages/default/package_default.h"
#include "paksources/dsEnginePackageSource.h"
#include "exceptions.h"

#ifdef OS_W32
	#include <windows.h>
	#undef GetObject
#endif


/* script corresponding to exception classes
class Exception{
	private var String pReason;
	public constructor create() pReason = "";
	public constructor create(string Reason) pReason = Reason;
	public const function String GetReason() return pReason;
}
class EInvalidParam extends Exception{
	public constructor create() init create("Invalid Parameter specified");
}
class EInvalidAction extends Exception{
	public constructor create() init create("Invalid action");
}
class EOutOfMemory extends Exception{
	public constructor create() init create("Out of Memory");
}
class EOutOfBounds extends Exception{
	public constructor create() init create("Index out of boundaries");
}
class ENullPointer extends Exception{
	public constructor create() init create("Invalid operation on a null object");
}
class EDivisionByZero extends Exception{
	public constructor create() init create("Division by zero");
}
class EInvalidCast extends Exception{
	public constructor create() init create("Invalid object cast");
}
*/

// definitions
#define SE_MSGBUFSIZE			250

#define SE_BASICCLASSNUM		15	/* number of basic classes */
#define SE_EXCEPCLASSNUM		8	/* number of exception classes */
#define SE_INTCLASSNUM			(SE_BASICCLASSNUM + SE_EXCEPCLASSNUM)
#define SE_CHECKOOM(expr)		if(!(expr)) DSTHROW(dueOutOfMemory)

#define SE_ESTR_INVPARAM		"Invalid parameter specified"
#define SE_ESTR_INVACTION		"Invalid action"
#define SE_ESTR_OUTOFMEM		"Out of memory"
#define SE_ESTR_OUTOFBOUNDS		"Index out of boundaries"
#define SE_ESTR_NULLPTR			"Invalid operation on a null object"
#define SE_ESTR_DIVBYZERO		"Division by zero"
#define SE_ESTR_INVCAST			"Invalid object cast"

#if defined OS_UNIX || defined OS_BEOS || defined OS_MACOS
#	define PATH_SEPARATOR	'/'
#elif defined OS_W32
#	define PATH_SEPARATOR	'\\'
#endif


// enumerations
enum eInternalClasses{
	icObject, // object class [primitive internal]
	icVoid, // void class [primitive internal]
	icByte, // byte class [primitive internal]
	icBool, // bool class [primitive internal]
	icInt, // int class [primitive internal]
	icFloat, // float class [primitive internal]
	icNull, // null class [primitive internal]
	icString, // string class [basic internal][native]
	icArray, // array class [basic internal][native]
	icDictionary, // dictionary class [basic internal][native]
	icSet, // set class [basic internal][native]
	icEnumeration, // enumeration class [basic internal][native]
	icWeakRef, // weak reference class [basic internal][native]
	icObjectRef, // object reference class [basic internal][native]
	icBlock, // block class [basic internal][native]
	icTimeDate, // block class [basic internal][native]
	icObjByte, // object byte class [basic internal]
	icObjBool, // object bool class [basic internal]
	icObjInt, // object int class [basic internal]
	icObjFloat, // object float class [basic internal]
	icException, // exception class
	icEInvParam, // invalid parameter exception
	icEInvAction, // invalid action exception
	icEOutOfMemory, // out of memory exception
	icEOutOfBounds, // out of bounds exception
	icENullPointer, // null pointer exception
	icEDivisionByZero, // division by zero exception
	icEInvalidCast, // invalid cast exception
};

// class dsEngine
////////////////////////
// constructor, destructor
dsEngine::dsEngine(){
	p_Classes = NULL;
	p_Packages = NULL;
	p_ConstStrTable = NULL;
	p_LockCount = 0;
	pTextBuffer = NULL;
	pTextBufferLen = 0;
	p_EngineManager = NULL;
	p_EngPkg = NULL;
	p_MainRT = NULL;
	pMemMgr = NULL;
	pGC = NULL;
	pClsVoid = NULL;
	pClsByte = NULL;
	pClsBool = NULL;
	pClsInt = NULL;
	pClsFloat = NULL;
	pClsObj = NULL;
	pClsNull = NULL;
	pClsStr = NULL;
	pClsArr = NULL;
	pClsDict = NULL;
	pClsSet = NULL;
	pClsBlock = NULL;
	pClsExcep = NULL;
	pClsTimeDate = NULL;
	pClsEnumeration = NULL;
	try{
		if(!(p_Classes = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_Packages = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_ConstStrTable = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_PakSources = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_SharePath = new dsuArray)) DSTHROW(dueOutOfMemory);
		p_BuildSharePath();
		AddDefaultPackageSources();
		if(!(p_EngineManager = new dsDefaultEngineManager)) DSTHROW(dueOutOfMemory);
		p_CreateEngineClass();
		if(!(p_EngPkg = new dsEnginePackages(this))) DSTHROW(dueOutOfMemory);
		if(!(pMemMgr = new dsMemoryManager(this))) DSTHROW(dueOutOfMemory);
		if(!(pGC = new dsGarbageCollector(pMemMgr))) DSTHROW(dueOutOfMemory);
		if(!(p_MainRT = new dsRunTime(this))) DSTHROW(dueOutOfMemory);
		srand((unsigned int)time(NULL));
		
	}catch( ... ){
		if(p_MainRT) delete p_MainRT;
		if(pGC) delete pGC;
		if(pMemMgr) delete pMemMgr;
		if(p_EngPkg) delete p_EngPkg;
		if(p_EngineManager) delete p_EngineManager;
		if( pTextBuffer ){
			free( pTextBuffer );
		}
		if(p_SharePath) delete p_SharePath;
		if(p_PakSources) delete p_PakSources;
		if(p_ConstStrTable) delete p_ConstStrTable;
		if(p_Classes) delete p_Classes;
		if(p_Packages) delete p_Packages;
		throw;
	}
}
dsEngine::~dsEngine(){
	Clear();
	
	// if libraries are unloaded the native classes become invalid. if native package
	// libraries would be unloaded before the classes segfaults would happen
	delete p_EngPkg;
	
	// free main runtime
	delete p_MainRT;
	delete pGC;
	delete pMemMgr;
	// remove engine internal hard coded classes
	// dsuString MUST be deleted as last class because it's the only class
	// able of having constant objects in function statements.
	for(int i=SE_INTCLASSNUM-1; i>=0; i--) p_Classes->Remove(i);
	// engine manager
	if(p_EngineManager) delete p_EngineManager;
	// message output
	if( pTextBuffer ){
		free( pTextBuffer );
	}
	// array
	if(p_SharePath) delete p_SharePath;
	if(p_PakSources) delete p_PakSources;
	if(p_ConstStrTable) delete p_ConstStrTable;
	if(p_Classes) delete p_Classes;
	if(p_Packages) delete p_Packages;
}
// engine manager
void dsEngine::SetEngineManager(dsBaseEngineManager *Manager){
	if(!Manager) DSTHROW(dueInvalidParam);
	if(p_EngineManager) delete p_EngineManager;
	p_EngineManager = Manager;
}
// package sources
int dsEngine::GetPackageSourceCount() const{
	return p_PakSources->Length();
}
void dsEngine::ClearPackageSources(){
	p_PakSources->RemoveAll();
}
void dsEngine::AddDefaultPackageSources(){
	dsPackageSource *pakSrc = NULL;
	try{
		// add engine package source
		pakSrc = new dsEnginePackageSource(this);
		if(!pakSrc) DSTHROW(dueOutOfMemory);
		AddPackageSource(pakSrc);
		pakSrc = NULL;
	}catch( ... ){
		if(pakSrc) delete pakSrc;
		throw;
	}
}
void dsEngine::AddPackageSource(dsPackageSource *source){
	if(!source) DSTHROW(dueInvalidParam);
	if(p_PakSources->ExistsObject(source)) DSTHROW(dueInvalidParam);
	p_PakSources->Add(source);
}
dsPackageSource *dsEngine::GetPackageSource(int index) const{
	return (dsPackageSource*)p_PakSources->GetObject(index);
}
dsPackageSource *dsEngine::FindPackageSource(const char *pakname) const{
	dsPackageSource *pkgSrc;
	// we search the package sources in the inverse order so user
	// specified packages sources come first
	for(int i=p_PakSources->Length()-1; i>=0; i--){
		pkgSrc = (dsPackageSource*)p_PakSources->GetObject(i);
		if(pkgSrc->CanHandle(pakname)) return pkgSrc;
	}
	// nothing can handle this pakname
	return NULL;
}
void dsEngine::RemovePackageSource(dsPackageSource *source){
	if(!source) DSTHROW(dueInvalidParam);
	int index = p_PakSources->FindObject(source);
	if(index == -1) DSTHROW(dueInvalidParam);
	p_PakSources->Remove(index);
}

// class management
int dsEngine::GetClassCount() const{
	return p_Classes->Length();
}
dsClass *dsEngine::GetClass(int Index) const{
	return (dsClass*)p_Classes->GetObject(Index);
}
dsClass *dsEngine::GetClass(const char *Name) const{
	if(!Name) DSTHROW(dueInvalidParam);
	const char *checkName;
	dsClass *vClass;
	// determine class parts
	const char *nextClass = (const char *)strchr(Name, '.');
	int nameLen = nextClass ? (int)(nextClass - Name) : ( int )strlen(Name);
	// find class
	for(int i=0; i<p_Classes->Length(); i++){
		vClass = (dsClass*)p_Classes->GetObject(i);
		checkName = vClass->GetName();
		if(strncmp(checkName, Name, nameLen)==0 && checkName[nameLen]=='\0'){
			// if there is a subpart descent into class
			if(nextClass){
				vClass = vClass->GetInnerClass(nextClass+1);
			}
			return vClass;
		}
	}
	return NULL;
}

// package management
int dsEngine::GetPackageCount() const{
	return p_Packages->Length();
}
dsPackage *dsEngine::GetPackage(int Index) const{
	return (dsPackage*)p_Packages->GetObject(Index);
}
dsPackage *dsEngine::GetPackage(const char *Name) const{
	if(!Name) DSTHROW(dueInvalidParam);
	dsPackage *vPackage;
	for(int i=0; i<p_Packages->Length(); i++){
		vPackage = (dsPackage*)p_Packages->GetObject(i);
		if(strcmp((vPackage)->GetName(), Name) == 0) return vPackage;
	}
	return NULL;
}
bool dsEngine::ExistsPackage(const char *Name) const{
	if(!Name) DSTHROW(dueInvalidParam);
	return GetPackage(Name) != NULL;
}
bool dsEngine::AddPackage(dsPackage *Package){
	dspParser vParser(this);
	dsClass *vClass;
	int i;
	// check the package for correctness
	if(!Package) DSTHROW(dueInvalidParam);
	if(strlen(Package->GetName()) > 0){
		if(ExistsPackage(Package->GetName())) DSTHROW(dueInvalidParam);
	}
	if(Package->GetScriptCount()==0 && Package->GetHostClassCount()==0) DSTHROW(dueInvalidParam);
	// lock engine
	p_LockCount++;
	try{
		// parse scripts
		if(!vParser.ParsePackage(Package)){
			PrintMessageFormat( "[DS] Compilation failed: Errors(%i), Warnings(%i).", vParser.GetErrorCount(), vParser.GetWarningCount() );
			DSTHROW(dseInvalidSyntax);
//		}else{
//			PrintMessage( "[DS] Compilation succeeded: Errors(%i), Warnings(%i).\n", vParser.GetErrorCount(), vParser.GetWarningCount() );
		}
		// init static values of class
		for(i=0; i<Package->GetClassCount(); i++){
			Package->GetClass(i)->InitStatics(p_MainRT);
		}
		// clean up
		p_LockCount--;
		p_Packages->Add(Package);
	}catch( const duException &e ){
		e.PrintError();
		for(i=0; i<Package->GetClassCount(); i++){
			Package->GetClass(i)->FreeStatics(p_MainRT);
		}
		for(i=Package->GetClassCount()-1; i>=0; i--){
			vClass = Package->GetClass(i);
			if(vClass->GetParent()){
				vClass->GetParent()->p_RemoveInnerClass(vClass);
			}else{
				p_RemoveClass(vClass);
			}
		}
		p_LockCount--;
		if(e.IsNamed("InvalidSyntax")) return false; else throw;
		
	}catch( ... ){
		for(i=0; i<Package->GetClassCount(); i++){
			Package->GetClass(i)->FreeStatics(p_MainRT);
		}
		for(i=Package->GetClassCount()-1; i>=0; i--){
			vClass = Package->GetClass(i);
			if(vClass->GetParent()){
				vClass->GetParent()->p_RemoveInnerClass(vClass);
			}else{
				p_RemoveClass(vClass);
			}
		}
		p_LockCount--;
		throw;
	}
	return true;
}
bool dsEngine::LoadPackage(const char *Name){
	// check if the package is already loaded
	if(ExistsPackage(Name)) return true;
	// find package source able to handle the name
	dsPackageSource *pkgSrc = FindPackageSource(Name);
	if(!pkgSrc) return false;
	// load the package
	dsPackage *thePkg = NULL;
	try{
		thePkg = pkgSrc->LoadPackage(Name);
		if(!thePkg) DSTHROW(dueOutOfMemory);
		// add package no matter if it is empty or not
		return AddPackage(thePkg);
	}catch( ... ){
		if(thePkg) delete thePkg;
		return false;
	}
}
// engine management
void dsEngine::Clear(){
	if( p_LockCount > 0 ){
		DSTHROW( dseEngineLocked );
	}
	
	int i;
	
	// remove all packages
	for( i=p_Packages->Length()-1; i>=0; i-- ){
		p_Packages->Remove( i );
	}
	
	// remove all objects
	p_MainRT->ClearStack();
	for( i=p_Classes->Length()-1; i>=0; i-- ){
		( ( dsClass* )p_Classes->GetObject( i ) )->FreeStatics( p_MainRT );
	}
	p_MainRT->ClearStack();
	
	p_MainRT->CleanUp();
	
	// run garbage collector
	pGC->RunGarbageCollector();
	
	p_MainRT->CleanUp(); // this is the final call. after this the runtime does not clean up anything anymore
	
	// deletion of classes must go from last create to first created.
	// don't remove here the engine internal hard coded classes
	for( i=p_Classes->Length()-1; i>=SE_INTCLASSNUM; i-- ){
		p_Classes->Remove( i );
	}
	
	//p_Classes->Resize(SE_INTCLASSNUM); // should do the trick too... to check
}
// debugging
void dsEngine::PrintClasses( bool WithInternals ){
	PrintMessageFormat( "[DS] List of loaded classes (%s internal classes)", WithInternals ? "with" : "without" );
	int vStart = WithInternals ? 0 : SE_INTCLASSNUM;
	for(int i=vStart; i<p_Classes->Length(); i++){
		((dsClass*)p_Classes->GetObject(i))->PrintClass(true);
	}
}

void dsEngine::PrintMessage( const char *message ){
	if( ! message ){
		DSTHROW( dueInvalidParam );
	}
	
	p_EngineManager->OutputMessage( message );
}

void dsEngine::PrintMessageFormat( const char *format, ... ){
	if( ! format ) DSTHROW( dueInvalidParam );
	
	int requiredLength;
	va_list list;
	
	va_start( list, format );
	requiredLength = vsnprintf( NULL, 0, format, list );
	if( requiredLength < 0 ){
		DSTHROW( dueInvalidParam ); // broken vsnprintf implementation
	}
	va_end( list );
	
	pPreareTextBuffer( requiredLength );
	
	va_start( list, format );
	if( vsnprintf( pTextBuffer, requiredLength + 1, format, list ) != requiredLength ){
		DSTHROW( dueInvalidParam ); // broken vsnprintf implementation
	}
	va_end( list );
	
	p_EngineManager->OutputMessage( pTextBuffer );
}



// constant string table
int dsEngine::AddConstString(const char *String){
	dsuString *vResStr;
	for(int i=0; i<p_ConstStrTable->Length(); i++){
		vResStr = (dsuString*)p_ConstStrTable->GetObject(i);
		if(strcmp(vResStr->Pointer(), String) == 0) return i;
	}
	if(!(vResStr = new dsuString(String))) DSTHROW(dueOutOfMemory);
	p_ConstStrTable->Add(vResStr);
	return p_ConstStrTable->Length() - 1;
}
const char *dsEngine::GetConstString(int Index){
	dsuString *vResStr = (dsuString*)p_ConstStrTable->GetObject(Index);
	return vResStr->Pointer();
}
// shared path list
int dsEngine::GetSharedPathCount() const{
	return p_SharePath->Length();
}
const char *dsEngine::GetSharedPath(int Index) const{
	return ((dsuString*)p_SharePath->GetObject(Index))->Pointer();
}

// private functions
// create classes
void dsEngine::p_CreateEngineClass(){
	dsClass *cls = NULL;
	int i;
	// create classes
	try{
		// object
		cls = new dsClassObject;
		p_Classes->Add( cls );
		pClsObj = cls;
		cls = NULL;
		
		// void
		cls = new dsClassVoid;
		p_Classes->Add( cls );
		pClsVoid = cls;
		cls = NULL;
		
		// byte
		cls = new dsClassByte;
		p_Classes->Add( cls );
		pClsByte = cls;
		cls = NULL;
		
		// bool
		cls = new dsClassBool;
		p_Classes->Add( cls );
		pClsBool = cls;
		cls = NULL;
		
		// int
		cls = new dsClassInt;
		p_Classes->Add( cls );
		pClsInt = cls;
		cls = NULL;
		
		// float
		cls = new dsClassFloat;
		p_Classes->Add( cls );
		pClsFloat = cls;
		cls = NULL;
		
		// null
		cls = new dsClassNull;
		p_Classes->Add( cls );
		pClsNull = cls;
		cls = NULL;
		
		// string
		cls = new dsClassString;
		p_Classes->Add( cls );
		pClsStr = cls;
		cls = NULL;
		
		// array
		cls = new dsClassArray;
		p_Classes->Add( cls );
		pClsArr = cls;
		cls = NULL;
		
		// dictionary
		cls = new dsClassDictionary;
		p_Classes->Add( cls );
		pClsDict = cls;
		cls = NULL;
		
		// set
		cls = new dsClassSet;
		p_Classes->Add( cls );
		pClsSet = cls;
		cls = NULL;
		
		// enumeration
		cls = new dsClassEnumeration;
		p_Classes->Add( cls );
		pClsEnumeration = cls;
		cls = NULL;
		
		// weak reference
		cls = new dsClassWeakRef;
		p_Classes->Add( cls );
		cls = NULL;
		
		// object reference
		cls = new dsClassObjectRef;
		p_Classes->Add( cls );
		cls = NULL;
		
		// block
		cls = new dsClassBlock;
		p_Classes->Add( cls );
		pClsBlock = cls;
		cls = NULL;
		
		// time date
		cls = new dsClassTimeDate;
		p_Classes->Add( cls );
		pClsTimeDate = cls;
		cls = NULL;
		
		// object byte
		cls = new dsClassObjectByte;
		p_Classes->Add( cls );
		cls = NULL;
		
		// object boolean
		cls = new dsClassObjectBoolean;
		p_Classes->Add( cls );
		cls = NULL;
		
		// object integer
		cls = new dsClassObjectInteger;
		p_Classes->Add( cls );
		cls = NULL;
		
		// object float
		cls = new dsClassObjectFloat;
		p_Classes->Add( cls );
		cls = NULL;
		
		// exception
		cls = new dsClassException;
		p_Classes->Add( cls );
		pClsExcep = cls;
		cls = NULL;
		
	}catch( ... ){
		if(cls) delete cls;
		throw;
	}
	// set base classes
	for(i=icVoid; i<=icException; i++){
		((dsClass*)p_Classes->GetObject(i))->SetBaseClass( pClsObj );
	}
	// create members
	for(i=icObject; i<=icException; i++){
		((dsClass*)p_Classes->GetObject(i))->CreateClassMembers(this);
	}
	// update sizes and function listings
	for(i=icObject; i<=icException; i++){
		((dsClass*)p_Classes->GetObject(i))->CalcMemberOffsets();
		((dsClass*)p_Classes->GetObject(i))->UpdateFuncList();
	}
	// add exception classes
	pCreateExceptionClasses();
}

void dsEngine::pCreateExceptionClasses(){
	pCreateEngExceptionClass( "EInvalidParam", SE_ESTR_INVPARAM );
	pCreateEngExceptionClass( "EInvalidAction", SE_ESTR_INVACTION );
	pCreateEngExceptionClass( "EOutOfMemory", SE_ESTR_OUTOFMEM );
	pCreateEngExceptionClass( "EOutOfBounds", SE_ESTR_OUTOFBOUNDS );
	pCreateEngExceptionClass( "ENullPointer", SE_ESTR_NULLPTR );
	pCreateEngExceptionClass( "EDivisionByZero", SE_ESTR_DIVBYZERO );
	pCreateEngExceptionClass( "EInvalidCast", SE_ESTR_INVCAST );
}

void dsEngine::pCreateEngExceptionClass( const char *name, const char *desc ){
	// create exception class
	dsClass * const classEngExep = new dsClass( name, DSCT_CLASS, DSTM_PUBLIC );
	if( ! classEngExep ){
		DSTHROW( dueOutOfMemory );
	}
	
	classEngExep->SetBaseClass( pClsExcep );
	classEngExep->CalcMemberOffsets();
	p_Classes->Add( classEngExep );
	
	
	// Constructor
	dsFunction * const func = new dsFunction( classEngExep, DSFUNC_CONSTRUCTOR,
		DSFT_CONSTRUCTOR, DSTM_PUBLIC, pClsVoid );
	
	classEngExep->AddFunction( func );
	dsByteCode * const byteCode = func->GetByteCode();
	
	// init Create(<string>);
	dsByteCode::sCodeCString codeCStr;
	codeCStr.code = dsByteCode::ebcCStr;
	codeCStr.index = AddConstString( desc );
	byteCode->AddCode( codeCStr );
	
	dsByteCode::sCode code;
	code.code = dsByteCode::ebcPush;
	byteCode->AddCode( code );
	
	code.code = dsByteCode::ebcSuper;
	byteCode->AddCode( code );
	
	dsByteCode::sCodeDirectCall codeDCall;
	codeDCall.code = dsByteCode::ebcDCall;
	codeDCall.function = pClsExcep->GetFunction( 1 );
	byteCode->AddCode( codeDCall );
	
	// return
	code.code = dsByteCode::ebcRet;
	byteCode->AddCode( code );
	
	
	// Constructor(String)
	dsFunction * const func2 = new dsFunction( classEngExep, DSFUNC_CONSTRUCTOR,
		DSFT_CONSTRUCTOR, DSTM_PUBLIC, pClsVoid );
	func2->GetSignature()->AddParameter( pClsStr );
	
	classEngExep->AddFunction( func2 );
	dsByteCode * const byteCode2 = func2->GetByteCode();
	
	// init Create(<string>);
	dsByteCode::sCodeParam codeParam;
	codeParam.code = dsByteCode::ebcParam;
	codeParam.index = 0;
	byteCode2->AddCode( codeParam );
	
	code.code = dsByteCode::ebcPush;
	byteCode2->AddCode( code );
	
	code.code = dsByteCode::ebcSuper;
	byteCode2->AddCode( code );
	
	codeDCall.code = dsByteCode::ebcDCall;
	codeDCall.function = pClsExcep->GetFunction( 1 );
	byteCode2->AddCode( codeDCall );
	
	// return
	code.code = dsByteCode::ebcRet;
	byteCode2->AddCode( code );
	
	
	// update function listing
	classEngExep->UpdateFuncList();
}

void dsEngine::p_AddClass(dsClass *Class){
	if(!Class) DSTHROW(dueInvalidParam);
	if(GetClass(Class->GetName())){
		DSTHROW(dueInvalidParam);
	}
	if(!Class->GetBaseClass()) Class->SetBaseClass((dsClass*)p_Classes->GetObject(icObject));
	p_Classes->Add(Class);
}
void dsEngine::p_RemoveClass(dsClass *Class){
	for(int i=p_Classes->Length()-1; i>=SE_INTCLASSNUM; i--){
		if((dsClass*)p_Classes->GetObject(i) == Class){
			p_Classes->Remove(i);
			return;
		}
	}
	DSTHROW(dueInvalidParam);
}

void dsEngine::p_BuildSharePath(){
	// add shared path list
	#ifdef OS_W32
	const int separator = ';';
	#else
	const int separator = ':';
	#endif
	
	char *envPath = nullptr;

	#ifdef OS_W32
		envPath = nullptr;
		TCHAR bufEnvPath[ 256 ];
		char bufEnvPathChar[ 256 ];
		if( GetEnvironmentVariable( L"DS_PAKAGE_PATH", &bufEnvPath[ 0 ], sizeof( bufEnvPath ) ) ){
			size_t size = 0;
			if( ! wcstombs_s( &size, bufEnvPathChar, 256, bufEnvPath, wcslen( bufEnvPath ) + 1 ) ){
				envPath = bufEnvPathChar;
			}
		}
	#else
		envPath = ( char* )getenv( "DS_PAKAGE_PATH" );
	#endif

	if( envPath ){
		char *buffer = NULL;
		
		try{
			while( true ){
				char *findSep = strchr( envPath, separator );
				if( ! findSep ){
					findSep = envPath + strlen( envPath );
				}
				
				const int len = ( int )( findSep - envPath );
				if( len > 0 ){
					buffer = new char[ len + 1 ];
					#ifdef OS_W32_VS
						strncpy_s( buffer, len + 1, envPath, len );
					#else
						strncpy( buffer, envPath, len + 1 );
					#endif
					buffer[ len ] = '\0';
					p_AddSharePath( buffer );
					delete [] buffer;
					buffer = NULL;
				}
				
				envPath = findSep;
				if( findSep[ 0 ] == '\0' ){
					break;
				}
				envPath = findSep + 1;
			}
			
		}catch( ... ){
			if( buffer ){
				delete [] buffer;
			}
			throw;
		}
		
	}else{
		p_AddSharePath( DS_PAK_PATH );
	}
	
	// add windows based share path list
	//p_AddSharePath("C:\\Programme\\DragonScript\\Packages\\");
	
	// debug: print out all share path
	/*
	PrintMessage( "[DEBUG] Share Path List:" );
	for(int d=0; d<p_SharePath->Length(); d++){
		PrintMessage( "  - %s\n", ((dsuString*)p_SharePath->GetObject(d))->Pointer() );
	}
	*/
}
void dsEngine::p_AddSharePath(const char *path){
	dsuString *newStr = NULL;
	char *newPath = NULL;
	int len;
	try{
		len = ( int )strlen(path);
		newPath = new char[len+2];
		if(!newPath) DSTHROW(dueOutOfMemory);
		#ifdef OS_W32_VS
			strncpy_s( newPath, len + 1, path, len );
		#else
			strncpy(newPath, path, len + 1);
		#endif
		newPath[ len ] = 0;
		if(newPath[len-1] != PATH_SEPARATOR){
			newPath[len] = PATH_SEPARATOR;
			newPath[len+1] = '\0';
		}
		newStr = new dsuString(newPath);
		if(!newStr) DSTHROW(dueOutOfMemory);
		delete [] newPath;
		newPath = NULL;
		p_SharePath->Add(newStr);
	}catch( ... ){
		if(newPath) delete [] newPath;
		if(newStr) delete newStr;
		throw;
	}
}

void dsEngine::pPreareTextBuffer( int requiredLength ){
	if( requiredLength > pTextBufferLen ){
		pTextBuffer = ( char* )realloc( pTextBuffer, requiredLength + 1 );
		if( ! pTextBuffer ){
			DSTHROW( dueOutOfMemory );
		}
		
		pTextBufferLen = requiredLength;
	}
}
