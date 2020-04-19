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
#ifndef _DSCLASSPARSERINFO_H_
#define _DSCLASSPARSERINFO_H_

// class dsClassParserInfo
class dsClassParserInfo{
private:
	char *p_parent; // full classname of parent namespace/class
	char *p_base; // full classname of base class/interface
	char **p_ifaces; // full classnames of all interfaces
	int p_ifaceCount;
	char **p_reqs; // requires packages
	int p_reqCount;
public:
	// constructor, destructor
	dsClassParserInfo();
	~dsClassParserInfo();
	// management
	inline const char *GetParent() const{ return (const char *)p_parent; }
	inline const char *GetBase() const{ return (const char *)p_base; }
	inline int GetInterfaceCount() const{ return p_ifaceCount; }
	const char *GetInterface(int index) const;
	inline int GetRequiresCount() const{ return p_reqCount; }
	const char *GetRequires(int index) const;
	/* set the full classname of the parent class. "" means top-level.
	 */
	void SetParent(const char *className);
	/* set the full classname of the base class. "" means no base class.
	 */
	void SetBase(const char *className);
	/* add interface by full classname.
	 */
	void AddInterface(const char *className);
	/* add required package.
	 */
	void AddRequiredPackage(const char *packageName);
private:
	bool p_IsValid(const char *name) const;
};

// end of include only once
#endif

