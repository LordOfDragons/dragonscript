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

from . import android
from . import mingw
from . import gcc

from SCons.Variables import Variables, EnumVariable
from SCons.Script import ARGUMENTS

def generate(env):
	params = Variables(env['PARAMETER_SOURCE'], ARGUMENTS)
	params.Add(EnumVariable('crosscompile', 'Enable cross compile', 'no', ['no', 'android', 'mingw', 'gcc']))
	params.Update(env)
	
	configGroup = None
	try:
		configReport = env.configReport
		configGroup = 'Cross-Compile'
		configReport.add('Enable cross compile', crosscompile, configGroup)
	except:
		pass
	
	env.SetDefault(CROSSCOMPILE_CFLAGS = '')
	env.SetDefault(CROSSCOMPILE_CPPFLAGS = '')
	env.SetDefault(CROSSCOMPILE_CXXFLAGS = '')
	env.SetDefault(CROSSCOMPILE_LINKFLAGS = '')
	env.SetDefault(CROSSCOMPILE_LIBS = '')
	env.SetDefault(CROSSCOMPILE_PROGRAM_LIBS = '')
	
	if env['crosscompile'] == 'android':
		android.generate(env, configGroup)
	elif env['crosscompile'] == 'mingw':
		mingw.generate(env, configGroup)
	elif env['crosscompile'] == 'gcc':
		gcc.generate(env, configGroup)

def exists(env):
	if env['crosscompile'] == 'android':
		return android.exists(env)
	elif env['crosscompile'] == 'mingw':
		return mingw.exists(env)
	elif env['crosscompile'] == 'gcc':
		return gcc.exists(env)
	return True
