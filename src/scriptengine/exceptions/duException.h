/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2017, Roland Plüss (roland@rptd.ch)
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

#ifndef _DUEXCEPTION_H_
#define _DUEXCEPTION_H_

#include "../utils/dsuString.h"

class dsRealObject;
class dsValue;
class dsClass;
class dsRunTime;


/**
 * \brief Exception class.
 */
class duException{
private:
	const char *p_Name; // name of exception
	const char *p_Desc; // description of exception
	dsuString p_Info; // additional info about the exception
	const char *p_File; // name of source file where the exception occured
	int p_Line; // number of the line where the exception occured
	
public:
	/** \name Constructors, Destructors */
	/*@{*/
	/**
	 * \brief Create a new exception object.
	 * \param[in] name Unique name to identify the exception. If \em NULL set to a default string.
	 * \param[in] description Description of the reason for the exception. If \em NULL set to a default string.
	 * \param[in] info Addition information. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	duException(const char *name, const char *description, const char *info, const char *file, int line);
	
	/**
	 * \brief Clean up exception object.
	 */
	virtual ~duException();
	/*@}*/
	
	/** \name Accessors */
	/*@{*/
	/** \brief Unique name to identify the exception. */
	inline const char *GetName() const{ return p_Name; }
	
	/** \brief Description of the reason for the exception. */
	inline const char *GetDescription() const{ return p_Desc; }
	
	/** \brief Additional information. */
	inline const dsuString &GetInfo() const{ return p_Info; }
	
	/** \brief Name of the source file where the exception occured. */
	inline const char *GetFile() const{ return p_File; }
	
	/** \brief Line number in file where the exception occured. */
	inline int GetLine() const{ return p_Line; }
	/*@}*/
	
	/** \name Tests */
	/*@{*/
	/** \brief Test if the exception has the given name. */
	bool IsNamed(const char *Name) const;
	/*@}*/
	
	/** \name Display Functions */
	/*@{*/
	/**
	 * \brief Display a formated output of the stored informations.
	 * \details Print each line obtained by FormatOutput to stdout.
	 */
	void PrintError() const;
};



/**
 * \brief Generic exception.
 */
class dueGeneric : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueGeneric( const char *info, const char *file, int line );
};

/**
 * \brief Invalid parameter exception.
 */
class dueInvalidParam : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueInvalidParam( const char *info, const char *file, int line );
};

/**
 * \brief Out of memory exception.
 */
class dueOutOfMemory : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueOutOfMemory( const char *info, const char *file, int line );
};

/**
 * \brief Out of boundary exception.
 */
class dueOutOfBoundary : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueOutOfBoundary( const char *info, const char *file, int line );
};

/**
 * \brief Stack empty exception.
 */
class dueStackEmpty : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueStackEmpty( const char *info, const char *file, int line );
};

/**
 * \brief Stack overflow exception.
 */
class dueStackOverflow : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueStackOverflow( const char *info, const char *file, int line );
};

/**
 * \brief Divison by zero exception.
 */
class dueDivisionByZero : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueDivisionByZero( const char *info, const char *file, int line );
};

/**
 * \brief Null pointer exception.
 */
class dueNullPointer : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueNullPointer( const char *info, const char *file, int line );
};

/**
 * \brief Invalid action exception.
 */
class dueInvalidAction : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueInvalidAction( const char *info, const char *file, int line );
};



/**
 * \brief File not found exception.
 */
class dueFileNotFound : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueFileNotFound( const char *info, const char *file, int line );
};

/**
 * \brief File exists exception.
 */
class dueFileExists : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueFileExists( const char *info, const char *file, int line );
};

/**
 * \brief Open file exception.
 */
class dueOpenFile : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueOpenFile( const char *info, const char *file, int line );
};

/**
 * \brief Read file exception.
 */
class dueReadFile : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueReadFile( const char *info, const char *file, int line );
};

/**
 * \brief Write file exception.
 */
class dueWriteFile : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueWriteFile( const char *info, const char *file, int line );
};



/**
 * \brief Test case failed exception.
 */
class dueTestFailed : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueTestFailed( const char *info, const char *file, int line );
};

/**
 * \brief Test assertion failed exception.
 */
class dueAssertion : public duException{
public:
	/**
	 * \brief Create a new exception object.
	 * \param[in] info Additional informations helpfull to track the reason for the exception. If \em NULL set to a default string.
	 * \param[in] file Name of the source file where the exception occured. If \em NULL set to a default string.
	 * \param[in] line Line number in file where the exception occured. If \em NULL set to a default string.
	 */
	dueAssertion( const char *info, const char *file, int line );
};

extern dsuString ErrorBuildInfo( const char *format, ... );
extern dsuString ErrorObjectType( const dsValue *value );
extern dsuString ErrorCastInfo( const dsValue *from, const dsClass *to );
extern dsuString ErrorCastInfo( const dsClass *from, const dsClass *to );
extern dsuString ErrorValueInfo( const dsRunTime &rt, const dsValue *value );



/**
 * \brief Throw an exception of the given type.
 * \details The type specified has to be an exception class subclassing deException. The file
 *          and line informations are obtained from the location the macro is expanded.
 * \param cls Class name of the exception to throw
 */
#define DSTHROW(cls) throw cls("", __FILE__, __LINE__)

/**
 * \brief Throw an exception of the given type with additional informations.
 * 
 * The type specified has to be an exception class subclassing deException. The file and line
 * informations are obtained from the location the macro is expanded. In additiona the
 * information field of the exception is assigned the constant string 'info'.
 * \param cls Class name of the exception to throw
 * \param info Constant string to assign to the exception object as
 *             additional informations
 */
#define DSTHROW_INFO(cls,info) throw cls(info, __FILE__, __LINE__)

/**
 * \brief Throw an exception of the given type with additional informations.
 * 
 * The type specified has to be an exception class subclassing deException. The file and line
 * informations are obtained from the location the macro is expanded. In additiona the
 * information field of the exception is assigned the constant string 'info'.
 * \param cls Class name of the exception to throw
 * \param info Constant string to assign to the exception object as
 *             additional informations
 */
#define DSTHROW_INFO_FMT(cls,format,...) throw cls(ErrorBuildInfo(format, __VA_ARGS__), __FILE__, __LINE__)

#endif
