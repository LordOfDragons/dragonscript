# -*- coding: utf-8 -*-
#
# DragonScript
#
# Copyright (C) 2017, Plüss Roland ( roland@rptd.ch )
#
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either 
# version 2 of the License, or (at your option) any later 
# version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

import os
import platform

from SCons.Variables import Variables, BoolVariable
from SCons.Script import ARGUMENTS

def generate(env):
	params = Variables(env['PARAMETER_SOURCE'], ARGUMENTS)
	params.Add(BoolVariable('with_verbose', 'Enable verbose build commands', False))
	params.Update(env)
	
	try:
		configReport = env.configReport
		configReport.add('Enable verbose compile output', with_verbose)
	except:
		pass
	
	if not env['with_verbose']:
		env['CCCOMSTR'] = 'Compiling $SOURCES'
		env['CXXCOMSTR'] = 'Compiling $SOURCES'
		env['LINKCOMSTR'] = 'Linking $TARGET'
		env['SHCCCOMSTR'] = 'Compiling $SOURCES'
		env['SHCXXCOMSTR'] = 'Compiling $SOURCES'
		env['SHLINKCOMSTR'] = 'Linking $TARGET'
		env['LDMODULECOMSTR'] = 'Linking $TARGET'
		env['RANLIBCOMSTR'] = 'Indexing $TARGET'
		env['ARCOMSTR'] = 'Linking $TARGET'
		env['TARCOMSTR'] = 'Archiving $TARGET'
		env['ZIPCOMSTR'] = 'Zipping $Target'

def exists(env):
	return True
