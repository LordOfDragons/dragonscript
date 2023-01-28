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
#include "../dragonscript_config.h"
#include "dsClass.h"
#include "dsImplClass.h"
#include "dsFunction.h"
#include "dsVariable.h"
#include "dsConstant.h"
#include "dsFuncList.h"
#include "dsSignature.h"
#include "dsRealObject.h"
#include "dsValue.h"
#include "dsClassParserInfo.h"
#include "../dsEngine.h"
#include "../dsRunTime.h"
#include "../exceptions.h"
#include "../utils/dsuArray.h"
#include "../utils/dsuString.h"

// class dsClass
///////////////////////
// constructor, destructor
dsClass::dsClass(const char *Name, int ClassType, int TypeModifiers, int PrimitiveType) {
	if(!Name) DSTHROW(dueInvalidParam);
	if(!dsiIsValidClassType(ClassType)) DSTHROW(dueInvalidParam);
	if(!dsiIsValidPrimType(PrimitiveType)) DSTHROW(dseInvalidPrimitiveType);
	// check for correct type modifiers
	if(TypeModifiers & DSTM_FIXED){
		if(TypeModifiers & DSTM_ABSTRACT) DSTHROW(dseInvalidTypeModifier);
	}
	if(TypeModifiers & DSTM_STATIC) DSTHROW(dseInvalidTypeModifier);
	if(TypeModifiers & DSTM_PUBLIC){
		if(TypeModifiers & (DSTM_PROTECTED | DSTM_PRIVATE)) DSTHROW(dseInvalidTypeModifier);
	}else if(TypeModifiers & DSTM_PROTECTED){
		if(TypeModifiers & DSTM_PRIVATE) DSTHROW(dseInvalidTypeModifier);
	}else if(!(TypeModifiers & DSTM_PRIVATE)){
		DSTHROW(dseInvalidTypeModifier);
	}
	if(ClassType != DSCT_CLASS){
		if(TypeModifiers & (DSTM_PROTECTED | DSTM_PRIVATE)) DSTHROW(dseInvalidTypeModifier);
	}
	// create rest
	p_ClassType = ClassType;
	p_TypeModifiers = TypeModifiers & DSTM_ALLTYPES;
	p_PrimitiveType = PrimitiveType;
	p_Parent = NULL;
	p_BaseClass = NULL;
	p_BaseSize = -1;
	p_NativeSize = 0;
	p_Size = -1;
	p_Name = NULL;
	p_Implements = NULL;
	p_InnerClasses = NULL;
	p_Variables = NULL;
	p_Constants = NULL;
	p_LocFuncs = NULL;
	p_BlockFuncs = NULL;
	p_FuncList = NULL;
	p_InitStaFunc = NULL;
	p_FirstConstrFunc = NULL;
	p_DestrFunc = NULL;
	p_parserInfo = NULL;
	try{
		const int size = ( int )strlen( Name );
		if(!(p_Name = new char[size+1])) DSTHROW(dueOutOfMemory);
		#ifdef OS_W32_VS
			strncpy_s( p_Name, size + 1, Name, size );
		#else
			strncpy(p_Name, Name, size + 1);
		#endif
		p_Name[ size ] = 0;
		if(!(p_Implements = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_InnerClasses = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_Variables = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_Constants = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_LocFuncs = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_BlockFuncs = new dsuArray)) DSTHROW(dueOutOfMemory);
		if(!(p_parserInfo = new dsClassParserInfo)) DSTHROW(dueOutOfMemory);
		UpdateFuncList();
	}catch( ... ){
		if(p_Name) delete [] p_Name;
		if(p_parserInfo) delete p_parserInfo;
		if(p_Implements) delete p_Implements;
		if(p_InnerClasses) delete p_InnerClasses;
		if(p_Variables) delete p_Variables;
		if(p_Constants) delete p_Constants;
		if(p_LocFuncs) delete p_LocFuncs;
		if(p_BlockFuncs) delete p_LocFuncs;
		throw;
	}
}
dsClass::~dsClass(){
	delete p_FuncList;
	if(p_InitStaFunc) delete p_InitStaFunc;
	// clear arrays
	if(p_Constants) p_Constants->RemoveAll();
	if(p_Variables) p_Variables->RemoveAll();
	if(p_LocFuncs) p_LocFuncs->RemoveAll();
	if(p_BlockFuncs) p_BlockFuncs->RemoveAll();
	if(p_InnerClasses) p_InnerClasses->RemoveAll();
	if(p_Implements) p_Implements->RemoveAll();
	// free memory
	if(p_Name) delete [] p_Name;
	if(p_parserInfo) delete p_parserInfo;
	if(p_Implements) delete p_Implements;
	if(p_InnerClasses) delete p_InnerClasses;
	if(p_Variables) delete p_Variables;
	if(p_Constants) delete p_Constants;
	if(p_LocFuncs) delete p_LocFuncs;
	if(p_BlockFuncs) delete p_BlockFuncs;
}
// object management
void dsClass::Destructor(dsRunTime *RT, dsValue *This){
	if(!RT || !This) DSTHROW(dueInvalidParam);
	dsFunction *vFunction;
	// call destructor of this class
	if( ( vFunction = GetDestructor() ) ){
		vFunction->RunFunction( RT, This );
	}
	// call destructor of base class
	if(p_BaseClass){
		p_BaseClass->Destructor(RT, This);
	}
}
void dsClass::InitVariables(dsRealObject *This){
	if(!This) DSTHROW(dueInvalidParam);
	dsVariable *vVar;
	// init variables of base class
	if(p_BaseClass) p_BaseClass->InitVariables(This);
	// init variables of this class
	for(int i=0; i<p_Variables->Length(); i++){
		vVar = (dsVariable*)p_Variables->GetObject(i);
		if(!(vVar->GetTypeModifiers() & DSTM_STATIC)){
			vVar->InitVariable( This->GetBuffer() );
		}
	}
}
void dsClass::FreeVariables(dsRunTime *RT, dsValue *This){
	if(!RT || !This) DSTHROW(dueInvalidParam);
	if(This->GetType()->GetPrimitiveType() != DSPT_OBJECT) DSTHROW(dueInvalidParam);
	dsVariable *vVar;
	void *buffer = This->GetRealObject()->GetBuffer();
	// free class variables
	for(int i=p_Variables->Length()-1; i>=0; i--){
		vVar = (dsVariable*)p_Variables->GetObject(i);
		if(!(vVar->GetTypeModifiers() & DSTM_STATIC)){
			vVar->FreeVariable(RT, buffer);
		}
	}
	// free variables of base class
	if(p_BaseClass) p_BaseClass->FreeVariables(RT, This);
}
// implements
void dsClass::AddImplement(dsClass *Class){
	dsImplClass *newClass = NULL;
	if(p_ClassType!=DSCT_CLASS && p_ClassType!=DSCT_INTERFACE) DSTHROW(dueInvalidAction);
	if(!Class) DSTHROW(dueInvalidParam);
	if((Class->p_TypeModifiers & DSTM_ABSTRACT) != DSTM_ABSTRACT ){ printf("[OOPS] %s\n", p_Name ); DSTHROW(dueInvalidParam); }
	for(int i=0; i<p_Implements->Length(); i++){
		if( ((dsImplClass*)p_Implements->GetObject(i))->GetClass() == Class ) DSTHROW(dueInvalidParam);
	}
	try{
		if(!(newClass = new dsImplClass(Class))) DSTHROW(dueOutOfMemory);
		p_Implements->Add(newClass);
	}catch( ... ){
		if(newClass) delete newClass;
	}
	UpdateFuncList();
}
int dsClass::GetImplementsCount() const{
	return p_Implements->Length();
}
dsClass *dsClass::GetImplement(int Index) const{
	return ((dsImplClass*)p_Implements->GetObject(Index))->GetClass();
}
int dsClass::GetImplFuncBase(int Index) const{
	return ((dsImplClass*)p_Implements->GetObject(Index))->GetFuncBase();
}
int dsClass::FindImplement(const char *Name, bool All) const{
	if(!Name) DSTHROW(dueInvalidParam);
	for(int i=0; i<p_Implements->Length(); i++){
		if(strcmp(((dsImplClass*)p_Implements->GetObject(i))->GetClass()->GetName(), Name) == 0) return i;
	}
	if(All && p_BaseClass) return p_BaseClass->FindImplement(Name);
	return -1;
}
int dsClass::GetImplFuncBase(dsClass *Class) const{
	if(!Class) DSTHROW(dueInvalidParam);
	if(IsEqualTo(Class)) return 0;
	dsImplClass *vImpl;
	int funcBase;
	for(int i=0; i<p_Implements->Length(); i++){
		vImpl = (dsImplClass*)p_Implements->GetObject(i);
		funcBase = vImpl->GetClass()->GetImplFuncBase(Class);
		if(funcBase != -1) return vImpl->GetFuncBase() + funcBase;
	}
	if(p_BaseClass) return p_BaseClass->GetImplFuncBase(Class);
	return -1;
}
bool dsClass::ExistsImplement(dsClass *Class) const{
	if(!Class) DSTHROW(dueInvalidParam);
	for(int i=0; i<p_Implements->Length(); i++){
		if(Class->IsEqualTo( ((dsImplClass*)p_Implements->GetObject(i))->GetClass() )) return true;
	}
	return false;
}
// inner class management
int dsClass::GetInnerClassCount() const{
	return p_InnerClasses->Length();
}
dsClass *dsClass::GetInnerClass(int Index) const{
	return (dsClass*)p_InnerClasses->GetObject(Index);
}
void dsClass::AddInnerClass(dsClass *Class){
	if(!Class) DSTHROW(dueInvalidParam);
//printf("[addinnerclass] %s,%s\n", p_Name, Class->GetName());
	if(GetInnerClass(Class->GetName())){
		printf("[oops] %s\n", Class->GetName());
		DSTHROW(dueInvalidParam);
	}
	Class->p_Parent = this;
//	if(!Class->GetBaseClass()) Class->SetBaseClass(p_Engine->GetClassObject());
	p_InnerClasses->Add(Class);
	// notfiy about adding class? no (atm)
}
dsClass *dsClass::GetInnerClass(const char *Name) const{
	if(!Name) DSTHROW(dueInvalidParam);
	dsClass *vClass;
	const char *checkName;
	// determine name parts
	const char *nextClass = (const char *)strchr(Name, '.');
	int nameLen = nextClass ? (int)(nextClass - Name) : ( int )strlen(Name);
	// find inner class
	for(int i=0; i<p_InnerClasses->Length(); i++){
		vClass = (dsClass*)p_InnerClasses->GetObject(i);
		checkName = vClass->GetName();
		if(strncmp(checkName, Name, nameLen)==0 && checkName[nameLen]=='\0'){
			// if there is a subpart descent into this class
			if(nextClass){
				vClass = vClass->GetInnerClass(nextClass+1);
			}
			return vClass;
		}
	}
	return NULL;
}
// variables
int dsClass::GetVariableCount() const{
	return p_Variables->Length();
}
dsVariable *dsClass::GetVariable(int Index) const{
	return (dsVariable*)p_Variables->GetObject(Index);
}
void dsClass::AddVariable(dsVariable *Variable){
	if(p_ClassType != DSCT_CLASS) DSTHROW(dueInvalidAction);
	if(!Variable) DSTHROW(dueInvalidParam);
	// check if the variable already exists in the list
	if(p_Variables->ExistsObject(Variable)) DSTHROW(dueInvalidParam);
	// check if a variable/constant with the same name exists already
	if( FindVariable( Variable->GetName(), false ) ) DSTHROW( dueInvalidParam );
	if( FindConstant( Variable->GetName(), false ) ) DSTHROW( dueInvalidParam );
	// add to list
	p_Variables->Add(Variable);
	// if the variable is static/const also makie sure the init-func exists
//	if(Variable->GetTypeModifiers() & (DSTM_STATIC | DSTM_FIXED)){
//		if(!p_InitStaFunc){
//			if(!(p_InitStaFunc = new dsFunction(this, "initsta", DSFT_FUNCTION, DSTM_PUBLIC, p_Engine->GetClassVoid()))) DSTHROW(dueOutOfMemory);
//		}
//	}
}
dsVariable *dsClass::FindVariable(const char *Name, bool All){
	if(!Name) DSTHROW(dueInvalidParam);
	dsVariable *vVariable;
	// try to find variable in this class
	for(int i=0; i<p_Variables->Length(); i++){
		vVariable = (dsVariable*)p_Variables->GetObject(i);
		if(strcmp(vVariable->GetName(), Name) == 0) return vVariable;
	}
	// search in the base class if needed
	if( All && p_BaseClass ){
		vVariable = p_BaseClass->FindVariable( Name, true );
		if(vVariable) return vVariable;
	}
	// no function found
	return NULL;
}
// Constants
int dsClass::GetConstantCount() const{
	return p_Constants->Length();
}
dsConstant *dsClass::GetConstant(int Index) const{
	return (dsConstant*)p_Constants->GetObject(Index);
}
void dsClass::AddConstant(dsConstant *Constant){
	if(p_ClassType!=DSCT_CLASS && p_ClassType!=DSCT_INTERFACE) DSTHROW(dueInvalidAction);
	if(!Constant) DSTHROW(dueInvalidParam);
	// check if the constant already exists in the list
	if(p_Constants->ExistsObject(Constant)) DSTHROW(dueInvalidParam);
	// check if a constant/variable with the same name exists already
	if( FindConstant( Constant->GetName(), false ) ) DSTHROW( dueInvalidParam );
	if( FindVariable( Constant->GetName(), false ) ) DSTHROW( dueInvalidParam );
	// add to list
	p_Constants->Add(Constant);
}
dsConstant *dsClass::FindConstant(const char *Name, bool All){
	if(!Name) DSTHROW(dueInvalidParam);
	dsConstant *vConstant;
	// try to find Constant in this class
	for(int i=0; i<p_Constants->Length(); i++){
		vConstant = (dsConstant*)p_Constants->GetObject(i);
		if(strcmp(vConstant->GetName(), Name) == 0) return vConstant;
	}
	// search in the base class if needed
	if(All && p_BaseClass){
		vConstant = p_BaseClass->FindConstant( Name, true );
		if(vConstant) return vConstant;
	}
	// no function found
	return NULL;
}
// functions
int dsClass::GetFunctionCount() const{
	return p_LocFuncs->Length();
}
dsFunction *dsClass::GetFunction(int Index) const{
	return (dsFunction*)p_LocFuncs->GetObject(Index);
}
void dsClass::AddFunction(dsFunction *Function){
	if(!Function) DSTHROW(dueInvalidParam);
	dsFunction *subFunc;
	// check if this function can be added to this class
	if(p_ClassType == DSCT_NAMESPACE) DSTHROW(dueInvalidAction);
	if(p_ClassType == DSCT_INTERFACE){
		if(Function->GetFuncType() != DSFT_FUNCTION) DSTHROW(dueInvalidAction);
		if(!(Function->GetTypeModifiers() & DSTM_ABSTRACT)) DSTHROW(dueInvalidParam);
		if(!(Function->GetTypeModifiers() & DSTM_PUBLIC)) DSTHROW(dueInvalidParam);
	}
	// check if the function already exists in the list
	if(p_LocFuncs->ExistsObject(Function)) DSTHROW(dueInvalidParam);
	if( ( subFunc = FindFunction(Function, true) ) ){
		// if the duplicate class is in the same class => error
		if(IsEqualTo(subFunc->GetOwnerClass())){
			printf("[oops] %s.%s\n", p_Name, Function->GetName());
			DSTHROW(dueInvalidParam);
		}
		// the function is not in the same class but has the same signature.
		// make sure the return types do not differ
		if(!Function->GetType()->IsEqualTo(subFunc->GetType())){
			printf("[oops] %s.%s\n", p_Name, Function->GetName());
			DSTHROW(dueInvalidParam);
		}
	}
	// check for duplicate constructor
	if(Function->GetFuncType() == DSFT_CONSTRUCTOR){
		if(p_ClassType == DSCT_INTERFACE) DSTHROW(dueInvalidAction);
		if(p_FirstConstrFunc){
			if(Function->GetSignature()->GetCount() < p_FirstConstrFunc->GetSignature()->GetCount()){
				p_FirstConstrFunc = Function;
			}
		}else{
			p_FirstConstrFunc = Function;
		}
	}
	// check for duplicate destructor
	if(Function->GetFuncType() == DSFT_DESTRUCTOR){
		if(p_DestrFunc) DSTHROW(dueInvalidParam);
		if(Function->GetSignature()->GetCount() > 0) DSTHROW(dueInvalidParam);
		p_DestrFunc = Function;
	}
	// add to list
	p_LocFuncs->Add(Function);
	UpdateFuncList();
}
dsFunction *dsClass::FindFunction(const char *Name, dsSignature *Signature, bool All){
	if(!Name || !Signature) DSTHROW(dueInvalidParam);
	dsFunction *vFunction;
	int i;
	// try to find function in this class
	for(i=0; i<p_LocFuncs->Length(); i++){
		vFunction = (dsFunction*)p_LocFuncs->GetObject(i);
		if(strcmp(vFunction->GetName(), Name) == 0){
			if(vFunction->GetSignature()->IsEqualTo(Signature)) return vFunction;
		}
	}
	// search in base or interface classes
	if(All){
		// search in the base class if one exists
		if(p_BaseClass){
			vFunction = p_BaseClass->FindFunction(Name, Signature);
			if(vFunction) return vFunction;
		}
		// search in all interfaces
		for(i=0; i<p_Implements->Length(); i++){
			vFunction = ((dsImplClass*)p_Implements->GetObject(i))->GetClass()->FindFunction(Name, Signature);
			if(vFunction) return vFunction;
		}
	}
	// no function found
	return NULL;
}
dsFunction *dsClass::FindFunction(dsFunction *Function, bool All){
	if(!Function) DSTHROW(dueInvalidParam);
	dsFunction *vFunction;
	int i;
	// try to find function in this class
	for(i=0; i<p_LocFuncs->Length(); i++){
		vFunction = (dsFunction*)p_LocFuncs->GetObject(i);
		if(vFunction->IsEqualTo(Function)) return vFunction;
	}
	// search in base or interface classes
	if(All){
		// search in the base class if one exists
		if(p_BaseClass){
			vFunction = p_BaseClass->FindFunction(Function);
			if(vFunction) return vFunction;
		}
		// search in all interfaces
		for(i=0; i<p_Implements->Length(); i++){
			vFunction = ((dsImplClass*)p_Implements->GetObject(i))->GetClass()->FindFunction(Function);
			if(vFunction) return vFunction;
		}
	}
	// no function found
	return NULL;
}
int dsClass::GetFunctionNameType(const char *Name){
	dsFunction *vFunction;
	for(int i=0; i<p_LocFuncs->Length(); i++){
		vFunction = (dsFunction*)p_LocFuncs->GetObject(i);
		if(strcmp(vFunction->GetName(), Name) == 0) return vFunction->GetFuncType();
	}
	return DSFT_FUNCTION;
}
bool dsClass::HasConstructors() const{
	for(int i=0; i<p_LocFuncs->Length(); i++){
		if(((dsFunction*)p_LocFuncs->GetObject(i))->GetFuncType() == DSFT_CONSTRUCTOR) return true;
	}
	return false;
}
// block functions (internal use only)
void dsClass::AddBlockFunction(dsFunction *Function){
	if(!Function) DSTHROW(dueInvalidParam);
	p_BlockFuncs->Add(Function);
}
int dsClass::GetBlockFunctionCount() const{
	return p_BlockFuncs->Length();
}
dsFunction *dsClass::GetBlockFunction(int Index) const{
	return (dsFunction*)p_BlockFuncs->GetObject(Index);
}
// base class
void dsClass::CallBaseClassConstructor(dsRunTime *RT, dsValue *This, dsFunction *Function, int ArgCount){
	if(!RT || !This || !Function) DSTHROW(dueInvalidParam);
	// run function
	Function->RunFunction( RT, This );
	// remove arguments from stack
	if(ArgCount > 0) RT->RemoveValues(ArgCount);
}
void dsClass::SetBaseClass(dsClass *BaseClass){
	if(BaseClass->GetPrimitiveType() != DSPT_OBJECT) DSTHROW(dueInvalidParam);
	p_BaseClass = BaseClass;
	UpdateFuncList();
}
// class
bool dsClass::IsEqualTo(dsClass *Class) const{
	//return strcmp(GetName(), Class->GetName()) == 0;
	return this == Class;
}

bool dsClass::CastableTo( dsClass *toType ){
	if( IsEqualTo( toType ) ){
		return true;
	}
	
	int i;
	
	switch( p_PrimitiveType ){
	case DSPT_BYTE:
	case DSPT_BOOL:
	case DSPT_INT:
	case DSPT_FLOAT:
		return ( toType->p_PrimitiveType >= DSPT_BYTE && toType->p_PrimitiveType <= DSPT_FLOAT )
			|| ( p_BaseClass && p_BaseClass->CastableTo( toType ) );
		
	case DSPT_NULL:
		return toType->p_PrimitiveType == DSPT_OBJECT;
		
	case DSPT_OBJECT:
		for( i=0; i<p_Implements->Length(); i++ ){
			if( ( ( dsImplClass* )p_Implements->GetObject( i ) )->GetClass()->CastableTo( toType ) ){
				return true;
			}
		}
		
		if( p_BaseClass ){
			return p_BaseClass->CastableTo( toType );
		}
		
		return false;
		
	default:
		DSTHROW( dseInvalidPrimitiveType );
	}
}

void dsClass::CalcMemberOffsets(){
	int vSizeVar;
	dsVariable *vVariable;
	// calculate base class member offsets
	p_BaseSize = 0;
	if(p_BaseClass){
		p_BaseClass->CalcMemberOffsets();
		p_BaseSize = p_BaseClass->SizeOf();
		if(p_BaseSize % DS_ALIGNMENT) p_BaseSize += DS_ALIGNMENT - (p_BaseSize % DS_ALIGNMENT);
	}
	// recalculate element offsets
	p_Size = p_BaseSize;
	// native class additional data
	if(p_NativeSize > 0){
		p_Size += p_NativeSize;
		if(p_Size % DS_ALIGNMENT) p_Size += DS_ALIGNMENT - (p_Size % DS_ALIGNMENT);
	}
	// variables
	for(int i=0; i<p_Variables->Length(); i++){
		vVariable = (dsVariable*)p_Variables->GetObject(i);
		if(vVariable->GetTypeModifiers() & DSTM_STATIC) continue;
		vSizeVar = vVariable->SizeOf();
		if((p_Size % DS_ALIGNMENT) + vSizeVar > DS_ALIGNMENT) p_Size += DS_ALIGNMENT - (p_Size % DS_ALIGNMENT);
		vVariable->SetOffset(p_Size);
		p_Size += vSizeVar;
	}
}
int dsClass::GetSubClassLevel(dsClass *Class) const{
	if(IsEqualTo(Class)) return 0;
	int vLevel;
	for(int i=0; i<p_Implements->Length(); i++){
		vLevel = ((dsImplClass*)p_Implements->GetObject(i))->GetClass()->GetSubClassLevel(Class);
		if(vLevel != -1) return vLevel;
	}
	if(p_BaseClass){
		vLevel = p_BaseClass->GetSubClassLevel(Class);
		if(vLevel != -1) return vLevel + 1;
	}
	return -1;
}
bool dsClass::IsSubClassOf(dsClass *Class) const{
	if(IsEqualTo(Class)) return true;
	for(int i=0; i<p_Implements->Length(); i++){
		if(((dsImplClass*)p_Implements->GetObject(i))->GetClass()->IsSubClassOf(Class)) return true;
	}
	if(p_BaseClass) return p_BaseClass->IsSubClassOf(Class);
	return false;
}
void dsClass::GetFullName(dsuString *FullName) const{
	if(!FullName) DSTHROW(dueInvalidParam);
	if(p_Parent){
		p_Parent->GetFullName(FullName);
		*FullName += ".";
	}
	*FullName += p_Name;
}
int dsClass::GetFullName(char *buffer, int len){
	// determine the length of the needed string
	int fullNameLen=0, nameLen, curLen;
	dsClass *curClass = this;
	while(curClass){
		fullNameLen += ( int )strlen(curClass->GetName()) + 1;
		curClass = curClass->GetParent();
	}
	fullNameLen--;
	// if there is a buffer
	if(buffer){
		// check if the buffer size is large enough (len is without '\0')
		if(len < fullNameLen) DSTHROW(dueInvalidParam);
		// create string containing full name
		curLen = fullNameLen;
		buffer[curLen] = '\0';
		curClass = this;
		while(curClass){
			nameLen = ( int )strlen(curClass->GetName());
			curLen -= nameLen;
			// new -Wstringop-truncation check is fail. fix is to use memcpy instead of
			// strncpy. seriously... how brain dead is this?!
			memcpy(buffer+curLen, curClass->GetName(), nameLen);
			curClass = curClass->GetParent();
			if(curClass){
				curLen--;
				buffer[curLen] = '.';
			}
		}
	}
	// return the length of the string
	return fullNameLen;
}
void dsClass::FreeStatics(dsRunTime *RT){
	if(!RT) DSTHROW(dueInvalidParam);
	dsVariable *vVar;
	int i;
	// free static variables of this class
	for(int i=p_Variables->Length()-1; i>=0; i--){
		vVar = (dsVariable*)p_Variables->GetObject(i);
		if(vVar->GetTypeModifiers() & DSTM_STATIC){
			vVar->FreeStaticValue(RT);
		}
	}
	// free static variables of inner classes
	for(i=0; i<p_InnerClasses->Length(); i++){
		((dsClass*)p_InnerClasses->GetObject(i))->FreeStatics(RT);
	}
}
void dsClass::InitStatics(dsRunTime *RT){
	if(!RT) DSTHROW(dueInvalidParam);
	// static variables are already set to empty after creation
	// now fun the init function to give them their values
	if(p_InitStaFunc){
		RT->p_ExecFunc( NULL, this, p_InitStaFunc, NULL );
	}
	// do not init static variables of inner classes
	// the engine inits each class seperatly
//	for(int i=0; i<p_InnerClasses->Length(); i++){
//		((dsClass*)p_InnerClasses->GetObject(i))->InitStatics(RT);
//	}
}
void dsClass::CreateStaFunc(dsEngine *engine){
	int i, mod;
	bool needsInit = false;
	dsClass *clsVoid = engine->GetClassVoid();
	// check for static variables
	for(i=0; i<p_Variables->Length(); i++){
		mod = ((dsVariable*)p_Variables->GetObject(i))->GetTypeModifiers();
		if(mod & (DSTM_STATIC | DSTM_FIXED)){
			needsInit = true;
			break;
		}
	}
	// create init function if needed
	if(needsInit && !p_InitStaFunc){
		p_InitStaFunc = new dsFunction(this, "initsta", DSFT_FUNCTION, DSTM_PUBLIC, clsVoid);
		if(!p_InitStaFunc) DSTHROW(dueOutOfMemory);
	}
}
void dsClass::UpdateFuncList(){
	dsFuncList *vNewFuncList = NULL;
	try{
		if(!(vNewFuncList = new dsFuncList)) DSTHROW(dueOutOfMemory);
		p_FillFuncList(vNewFuncList, 0, true);
		if(p_FuncList) delete p_FuncList;
		p_FuncList = vNewFuncList;
	}catch( ... ){
		if(vNewFuncList) delete vNewFuncList;
		throw;
	}
}



/* do nothing
 */
void dsClass::CreateClassMembers(dsEngine *engine){
}



// debugging
void dsClass::PrintClass(bool descent) const{
	// level output
	dsClass *curClass = p_Parent;
	while(curClass){
		printf("  "); curClass = curClass->p_Parent;
	}
	// child level output
	if(p_InnerClasses->Length() > 0){
		printf("+ ");
	}else{
		printf("- ");
	}
	// output class
	dsuString vFullName;
	switch(p_ClassType){
		case DSCT_CLASS: printf("class %s", p_Name); break;
		case DSCT_INTERFACE: printf("interface %s", p_Name); break;
		case DSCT_NAMESPACE: printf("namespace %s", p_Name); break;
		default: printf("<invalid class type> %s", p_Name);
	}
	if(p_BaseClass){
		p_BaseClass->GetFullName(&vFullName);
		if(strcmp(vFullName, "object") != 0){
			printf(" (%s)", vFullName.Pointer());
		}
	}
	printf("\n");
	// do the same for inner classes if descent is true
	if(descent){
		for(int i=0; i<p_InnerClasses->Length(); i++){
			((dsClass*)p_InnerClasses->GetObject(i))->PrintClass(true);
		}
	}
}
// private functions
void dsClass::p_FillFuncList(dsFuncList *FuncList, int base, bool All){
	int i;
	dsImplClass *implClass;
	dsFunction *vCurFunc;
	if(p_BaseClass){
		p_BaseClass->p_FillFuncList(FuncList, base, false);
	}
	for(i=0; i<p_Implements->Length(); i++){
		implClass = (dsImplClass*)p_Implements->GetObject(i);
		implClass->SetFuncBase( FuncList->GetCount() - base );
		implClass->GetClass()->p_FillFuncList(FuncList, FuncList->GetCount(), false);
	}
	for(i=0; i<p_LocFuncs->Length(); i++){
		vCurFunc = (dsFunction*)p_LocFuncs->GetObject(i);
		// do not add abstract functions
//		if(vCurFunc->GetTypeModifiers() & DSTM_ABSTRACT) continue;
		// add function otherwise
		FuncList->AddFunction(vCurFunc);
	}
}
void dsClass::p_SetNativeDataSize(int Size){
	if(Size < 0) DSTHROW(dueInvalidParam);
	p_NativeSize = Size;
}
void *dsClass::p_GetNativeData(void *This) const{
	return (void*)( (byte*)(This) + p_BaseSize );
}
bool dsClass::p_HasAbstractFunc(dsFunction *Function){
	dsFunction *vCheckFunc;
	for(int i=0; i<p_LocFuncs->Length(); i++){
		vCheckFunc = (dsFunction*)p_LocFuncs->GetObject(i);
		if(Function->IsEqualTo(vCheckFunc)){
			if(vCheckFunc->GetTypeModifiers() & DSTM_ABSTRACT) return true;
			break;
		}
	}
	if(p_BaseClass) return p_BaseClass->p_HasAbstractFunc(Function);
	return false;
}
void dsClass::p_SetParent(dsClass *Parent){
	p_Parent = Parent;
}
void dsClass::p_RemoveInnerClass(dsClass *Class){
	for(int i=p_InnerClasses->Length()-1; i>=0; i--){
		if((dsClass*)p_InnerClasses->GetObject(i) == Class){
			p_InnerClasses->Remove(i);
			return;
		}
	}
	DSTHROW(dueInvalidParam);
}

