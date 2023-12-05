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

#ifndef _LIBDSCRIPT_H_
#define _LIBDSCRIPT_H_

#include "dragonscript_config.h"

#include "exceptions.h"
#include "dsEngine.h"
#include "dsScriptSource.h"
#include "dsPackage.h"
#include "dsPackageSource.h"
#include "dsPackageWrapper.h"
#include "dsRunTime.h"
#include "dsBaseEngineManager.h"
#include "dsExceptionTrace.h"
#include "dsExceptionTracePoint.h"
#include "objects/dsClass.h"
#include "objects/dsValue.h"
#include "objects/dsFunction.h"
#include "objects/dsFuncList.h"
#include "objects/dsVariable.h"
#include "objects/dsConstant.h"
#include "objects/dsSignature.h"
#include "objects/dsClassParserInfo.h"
#include "objects/dsRealObject.h"
#include "packages/dsBaseEnginePackage.h"
#include "packages/default/package_default.h"
#include "utils/dsuArrayable.h"
#include "utils/dsuArray.h"
#include "utils/dsuIndexList.h"
#include "utils/dsuIndexStack.h"
#include "utils/dsuPointerList.h"
#include "utils/dsuPointerStack.h"
#include "utils/dsuStack.h"
#include "utils/dsuString.h"

#endif
