/* 
 * DragonScript Script Language
 *
 * Copyright (C) 2020, Roland Plüss (roland@rptd.ch)
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

#include "dsFOGetClassVar.h"
#include "../dsClass.h"
#include "../dsRealObject.h"
#include "../dsVariable.h"
#include "../dsValue.h"
#include "../../exceptions.h"
#include "../../dsRunTime.h"


// Class dsFOGetClassVar
//////////////////////////

// Constructor, destructor
////////////////////////////

dsFOGetClassVar::dsFOGetClassVar( dsVariable &variable ) :
dsFunctionOptimized( etGetClassVar ),
pVariable( variable ){
}



// Management
///////////////

void dsFOGetClassVar::RunFunction( dsRunTime *rt, dsValue *myself ){
	rt->PushValue( pVariable.GetValue( myself->GetRealObject()->GetBuffer() ) );
}
