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
#ifndef _DS_EXCEPTIONS_H_
#define _DS_EXCEPTIONS_H_

// includes
#include "exceptions/duException.h"

// dragon script specific exceptions
class dseInvalidClass : public duException{
public: inline dseInvalidClass(const char *Info, const char *File, int Line) :
	duException("InvalidClass","Class not found",Info,File,Line){}
};

class dseInvalidType : public duException{
public: inline dseInvalidType(const char *Info, const char *File, int Line) :
	duException("InvalidType","Invalid Object Type for this operation",Info,File,Line){}
};
class dseInvalidPrimitiveType : public duException{
public: inline dseInvalidPrimitiveType(const char *Info, const char *File, int Line) :
	duException("InvalidPrimitiveType","Invalid Primitive Type for this operation",Info,File,Line){}
};
class dseInvalidOnInterface : public duException{
public: inline dseInvalidOnInterface(const char *Info, const char *File, int Line) :
	duException("InvalidOnInterface","Operation not allowed on an interface",Info,File,Line){}
};
class dseInvalidCast : public duException{
public: inline dseInvalidCast(const char *Info, const char *File, int Line) :
	duException("InvalidCast","Object is not castable to the specified type",Info,File,Line){}
};
class dseInvalidSignature : public duException{
public: inline dseInvalidSignature(const char *Info, const char *File, int Line) :
	duException("InvalidSignature","No matching function signature found",Info,File,Line){}
};
class dseInvalidOperator : public duException{
public: inline dseInvalidOperator(const char *Info, const char *File, int Line) :
	duException("InvalidOperator","Invalid Operator specified",Info,File,Line){}
};
class dseClassNotNative : public duException{
public: inline dseClassNotNative(const char *Info, const char *File, int Line) :
	duException("ClassNotNative","This Operation is invalid on a non-native Class",Info,File,Line){}
};
class dseFunctionNotNative : public duException{
public: inline dseFunctionNotNative(const char *Info, const char *File, int Line) :
	duException("FunctionNotNative","This Operation is invalid on a non-native Function",Info,File,Line){}
};
class dseInvalidTypeModifier : public duException{
public: inline dseInvalidTypeModifier(const char *Info, const char *File, int Line) :
	duException("InvalidTypeModifier","Invalid Type Modifier specified",Info,File,Line){}
};
class dseNextObjectExisting : public duException{
public: inline dseNextObjectExisting(const char *Info, const char *File, int Line) :
	duException("NextObjectExisting","A next object already exists",Info,File,Line){}
};
class dseInvalidStatement : public duException{
public: inline dseInvalidStatement(const char *Info, const char *File, int Line) :
	duException("InvalidStatement","Invalid Statement",Info,File,Line){}
};
class dseScriptedException : public duException{
public: inline dseScriptedException(const char *Info, const char *File, int Line) :
	duException("ScriptedException","A scripted exception has occured",Info,File,Line){}
};
class dseEngineLocked : public duException{
public: inline dseEngineLocked(const char *Info, const char *File, int Line) :
	duException("EngineLocked","Engine is locked, action not applicable",Info,File,Line){}
};

class dseNoScriptLoaded : public duException{
public: inline dseNoScriptLoaded(const char *Info, const char *File, int Line) :
	duException("NoScriptLoaded","No Script loaded for preprocessing",Info,File,Line){}
};
class dseInvalidSyntax : public duException{
public: inline dseInvalidSyntax(const char *Info, const char *File, int Line) :
	duException("InvalidSyntax","Invalid Syntax in Script",Info,File,Line){}
};
class dseInvalidCode : public duException{
public: inline dseInvalidCode(const char *Info, const char *File, int Line) :
	duException("InvalidCode","Invalid Code in Script",Info,File,Line){}
};

// directory exceptions
class dseDirectoryNotFound : public duException{
public: inline dseDirectoryNotFound(const char *Info, const char *File, int Line) :
	duException("DirectoryNotFound","Directory does not exist",Info,File,Line){}
};
class dseDirectoryRead : public duException{
public: inline dseDirectoryRead(const char *Info, const char *File, int Line) :
	duException("DirectoryReadFailed","Directory read error",Info,File,Line){}
};

// end of include only once
#endif
