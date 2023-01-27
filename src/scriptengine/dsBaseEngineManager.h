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
#ifndef _DSBASEENGINEMANAGER_H_
#define _DSBASEENGINEMANAGER_H_

#include "dragonscript_export.h"

// predefinitions
class dsPackage;
class dsScriptSource;

// class dsBaseEngineManager
// -------------------------
// Base class for the engine manager class.
class DS_DLL_EXPORT dsBaseEngineManager{
public:
	virtual ~dsBaseEngineManager();
	// message output
	virtual void OutputMessage(const char *Message) = 0;
	virtual void OutputWarning(const char *Message, int WarnID, dsScriptSource *Script, int Line, int Position) = 0;
	virtual void OutputWarningMore(const char *Message) = 0;
	virtual void OutputError(const char *Message, int ErrorID, dsScriptSource *Script, int Line, int Position) = 0;
	virtual void OutputErrorMore(const char *Message) = 0;
	// parser management
	virtual bool ContinueParsing() = 0;
};

// end of include only once
#endif

