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
#ifndef _DSSIGNATURE_H_
#define _DSSIGNATURE_H_

// includes

// prefefinitions
class dsClass;

// class dsSignature
class DS_DLL_EXPORT dsSignature{
private:
	dsClass **p_Parameters;
	int p_ParamCount;
public:
	// constructor, destructor
	dsSignature();
	~dsSignature();
	// management
	void Clear();
	void AddParameter(dsClass *Type);
	inline int GetCount() const{ return p_ParamCount; }
	dsClass *GetParameter(int Index) const;
	bool IsEqualTo(dsSignature *Signature) const;
	void CopyTo(dsSignature *Signature);
};

// end of include only once
#endif

