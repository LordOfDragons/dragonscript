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

from SCons.Variables import Variables, PathVariable, EnumVariable
from SCons.Script import ARGUMENTS

def generate(env, configGroup):
	# add parameters to configurate toolchain
	params = Variables(env['PARAMETER_SOURCE'], ARGUMENTS)
	params.Add(('mingw_compiler', 'MinGW compiler (\'auto\' to auto detect)', 'auto'))
	params.Update(env)
	
	try:
		configReport = env.configReport
		configReport.add('MinGW compiler prefix', 'mingw_compiler', configGroup)
	except:
		pass
	
	# get configuration parameters
	compiler = env.subst(env['mingw_compiler'])
	if compiler == 'auto':
		autodetect = []
		autodetect.append('x86_64-w64-mingw64-')
		autodetect.append('x86_64-w64-mingw32-')
		autodetect.append('x86_64-w32-mingw64-')
		autodetect.append('x86_64-w32-mingw32-')
		
		envAutoDetect = env.Clone()
		conf = envAutoDetect.Configure()
		for x in autodetect:
			envAutoDetect['CXX'] = '{}gcc'.format(x)
			if conf.CheckCXX():
				compiler = x
				break
		conf.Finish()
	
	if not compiler:
		raise 'No working MinGW Cross Compiler found'
	
	env.Tool('mingw')
	
	env['CC'] = compiler + 'gcc'
	env['CXX'] = compiler + 'g++'
	env['LD'] = compiler + 'ld'
	env['STRIP'] = compiler + 'strip'
	env['OBJCOPY'] = compiler + 'objcopy'
	env['AS'] = compiler + 'as'
	env['AR'] = compiler + 'ar'
	env['RANLIB'] = compiler + 'ranlib'
	
	# these changes are required since the mingw tool in scons is not aware of cross compiling and
	# uses linux conventions where it should use windows ones
	env.Append(SHCCFLAGS = ['-fno-PIC']) # remove -fPIC if included. just to silence misleading warnings
	env['SHLIBPREFIX'] = ''
	env['SHLIBSUFFIX'] = '.dll'
	env['LIBPREFIX'] = ''
	env['LIBPREFIXES'] = ''
	env['LIBSUFFIX'] = '.lib'
	env['LIBSUFFIXES'] = ['.lib', '.a']
	env['PROGPREFIX'] = ''
	env['PROGSUFFIX'] = '.exe'

def exists(env):
	return True
