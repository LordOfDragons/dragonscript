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
	ndkroot = ''
	if 'NDK_ROOT' in os.environ:
		ndkroot = os.path.expanduser(os.environ['NDK_ROOT'])
	
	# add parameters to configurate toolchain
	params = Variables(env['PARAMETER_SOURCE'], ARGUMENTS)
	params.Add(PathVariable('android_ndkroot',
		'Path to Android NDK toolchain (NDK_ROOT env-param by default)',
		ndkroot, PathVariable.PathAccept ) )
	params.Add(EnumVariable('android_arch', 'Android architecture to build for',
		'armv7a', ['armv7','armv7a','aarch64','x86','x86_64'] ) )
	params.Add(('android_apilevel', 'Android API level', '18'))
	params.Add(('android_gccversion', 'Android NDK GCC version', '4.9'))
	params.Update(env)
	
	try:
		configReport = env.configReport
		configReport.add('Path to Android NDK toolchain', 'android_ndkroot', configGroup)
		configReport.add('Android architecture to build for', 'android_arch', configGroup)
		configReport.add('Android API level', 'android_apilevel', configGroup)
		configReport.add('Android NDK GCC version', 'android_gccversion', configGroup)
	except:
		pass
	
	# get configuration parameters
	ndkroot = env.subst(env['android_ndkroot'])
	arch = env['android_arch']
	apilevel = env['android_apilevel']
	gccVersion = env['android_gccversion']
	
	# set construction variables
	stlsupport = 'gnu-libstdc++'
	
	if env['android_arch'] == 'armv7':
		abi = 'androideabi'
		fullarch = 'armeabi-v7'
		hardfp = False
		
	elif env['android_arch'] == 'armv7a':
		abi = 'androideabi'
		fullarch = 'armeabi-v7a'
		hardfp = True
		
	elif env['android_arch'] == 'aarch64':
		abi = 'android'
		fullarch = 'arm64-v8a'
		hardfp = True
		
	elif env['android_arch'] == 'x86':
		abi = 'android'
		fullarch = 'x86'
		hardfp = False
		
	elif env['android_arch'] == 'x86_64':
		abi = 'android'
		fullarch = 'x86_64'
		hardfp = False
		
	else:
		raise Exception('Invalid architecture {}'.format(env['android_arch']))
	
	compilerPrefix = '{}-{}-{}{}-'.format(arch, platform.system().lower(), abi, apilevel)
	compilerPrefix2 = '{}-{}-{}-'.format(arch, platform.system().lower(), abi)
	
	pathToolchain = os.path.join(ndkroot, 'toolchains', 'llvm', 'prebuilt',
		'-'.join([platform.system().lower(), platform.machine()]))
	env['ANDROID_TOOLCHAIN'] = pathToolchain
	
	pathBin = os.path.join(pathToolchain, 'bin')
	env['ANDROID_BIN'] = pathBin
	
	compiler = os.path.join(pathBin, compilerPrefix)
	compiler2 = os.path.join(pathBin, compilerPrefix2)
	env['ANDROID_COMPILER'] = compiler
	
	env['ANDROID_FULLARCH'] = fullarch
	env['ANDROID_HARDFP'] = hardfp
	
	env['CC'] = compiler + 'clang'
	env['CXX'] = compiler + 'clang++'
	env['LD'] = compiler2 + 'ld'
	env['STRIP'] = compiler2 + 'strip'
	env['OBJCOPY'] = compiler2 + 'objcopy'
	env['AS'] = compiler2 + 'as'
	env['AR'] = compiler2 + 'ar'
	env['RANLIB'] = compiler2 + 'ranlib'
	env['NASM'] = compiler2 + 'yasm'
	
	# newer NDK changed compiler file names. how lovely U_U
	if env.Detect(os.path.join(pathBin, 'llvm-ar')):
		pathCompiler2 = os.path.join(pathBin, 'llvm')
		env['LD'] = '{}-link'.format(pathCompiler2)
		env['STRIP'] = '{}-strip'.format(pathCompiler2)
		env['OBJCOPY'] = '{}-objcopy'.format(pathCompiler2)
		env['AS'] = '{}-as'.format(pathCompiler2)
		env['AR'] = '{}-ar'.format(pathCompiler2)
		env['RANLIB'] = '{}-ranlib'.format(pathCompiler2)
	
	# libraries to link against required for libraries and binaries to load on android.
	# additional libraries can be required. android is in general quite picky about
	# these libraries and the loading order
	env.Append(CROSSCOMPILE_IBS = ['m', 'z', 'log', 'c', 'android'])
	
	# libs.append( 'gnustl_static' ) # stl support using static gnustl
	env.Append(CROSSCOMPILE_CPPFLAGS = ['-O3']) # default is -O2 so try to squeeze out a bit more speed if possible
	
	# hardware floating point support
	if hardfp:
		env.Append(CROSSCOMPILE_CPPFLAGS = ['-D_NDK_MATH_NO_SOFTFP=1'])
	
	# apply cross compile flags to build environment
	env.Append(LIBS = env['CROSSCOMPILE_LIBS'])
	env.Append(CFLAGS = env['CROSSCOMPILE_CFLAGS'])
	env.Append(CPPFLAGS = env['CROSSCOMPILE_CPPFLAGS'])
	env.Append(CXXFLAGS = env['CROSSCOMPILE_CXXFLAGS'])
	env.Append(LINKFLAGS = env['CROSSCOMPILE_LINKFLAGS'])

def exists(env):
	return os.path.exists(env.subst(env['CC']))
