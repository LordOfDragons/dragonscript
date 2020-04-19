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
	params.Add(('gcc_compiler', 'GCC cross compiler prefix', ''))
	params.Add(('gcc_cppflags', 'GCC cross compiler cpp flags', ''))
	params.Add(('gcc_linkflags', 'GCC cross compiler link flags', ''))
	params.Add(PathVariable('gcc_sysroot', 'GCC cross compiler system root', '', PathVariable.Accept))
	params.Update(env)
	
	try:
		configReport = env.configReport
		configReport.add('GCC cross compiler prefix', 'gcc_compiler', configGroup)
		configReport.add('GCC cross compiler cpp flags', 'gcc_cppflags', configGroup)
		configReport.add('GCC cross compiler link flags', 'gcc_linkflags', configGroup)
		configReport.add('GCC cross compiler system root', 'gcc_sysroot', configGroup)
	except:
		pass
	
	# get configuration parameters
	compiler = env.subst(env['gcc_compiler'])
	sysroot = env.subst(env['gcc_sysroot'])
	cppflags = env['gcc_cppflags']
	
	env['CC'] = compiler + 'gcc'
	env['CXX'] = compiler + 'g++'
	env['LD'] = compiler + 'ld'
	env['STRIP'] = compiler + 'strip'
	env['OBJCOPY'] = compiler + 'objcopy'
	env['AS'] = compiler + 'as'
	env['AR'] = compiler + 'ar'
	env['RANLIB'] = compiler + 'ranlib'
	
	env.Append(CROSSCOMPILE_CPPFLAGS = cppflags)
	env.Append(CROSSCOMPILE_LINKFLAGS = linkflags)
	env.Append(CROSSCOMPILE_CPPFLAGS = ['--sysroot={}'.format(sysroot)])
	env.Append(CROSSCOMPILE_LINKFLAGS = ['--sysroot={}'.format(sysroot)])
	
	# apply cross compile flags to build environment
	env.Append(CFLAGS = env['CROSSCOMPILE_CFLAGS'])
	env.Append(CPPFLAGS = env['CROSSCOMPILE_CPPFLAGS'])
	env.Append(CXXFLAGS = env['CROSSCOMPILE_CXXFLAGS'])
	env.Append(LINKFLAGS = env['CROSSCOMPILE_LINKFLAGS'])

def exists(env):
	return os.path.exists(env.subst(env['CC']))
