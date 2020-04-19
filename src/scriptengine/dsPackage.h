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
#ifndef _DSPACKAGE_H_
#define _DSPACKAGE_H_

// includes
#include "utils/dsuArrayable.h"

// predefinitions
class dsClass;
class dsScriptSource;
class dsuArray;
class dsuPointerList;
class dsEngine;

// class dsPackage
class dsPackage : public dsuArrayable{
friend class dsEngine;
friend class dspParser;
friend class dspParserCheck;
private:
	char *p_Name;
	dsuArray *p_Scripts; // dsScriptSource*
	dsuPointerList *p_HostClasses; // dsClass*
	dsuPointerList *p_Classes;
public:
	// constructor, destructor
	dsPackage(const char *Name);
	virtual ~dsPackage();
	// management
	inline const char *GetName() const{ return (const char *)p_Name; }
	void AppendPackage(dsPackage *pak);
	void CleanUp();
	// scripts management
	int GetScriptCount() const;
	void AddScript(dsScriptSource *Script);
	dsScriptSource *GetScript(int Index) const;
	int FindScript(const char *Name) const;
	inline bool ExistsScript(const char *Name) const{ return FindScript(Name) != -1; }
	// host classes management
	int GetHostClassCount() const;
	void AddHostClass(dsClass *Class);
	dsClass *GetHostClass(int Index) const;
	// class information
	int GetClassCount() const;
	dsClass *GetClass(int Index) const;
	bool ExistsClass(dsClass *Class) const;
	// debugging
	void PrintClasses( dsEngine *engine ) const;
private:
	void p_PrintClass(int index, bool *printed) const;
	void p_AddClass(dsClass *Class);
	void p_RemoveHostClass();
};

// end of include only once
#endif

