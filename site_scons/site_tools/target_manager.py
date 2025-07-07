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
from SCons.Util import NodeList

def generate(env):
	## Generic target.
	class _Target(object):
		def __init__(self, description, target=None):
			self.description = description
			if target:
				if isinstance(target, (list, tuple)):
					self.target = target
				else:
					self.target = [target]
			else:
				self.target = []
		
		## Create alias for all targets stored so far
		def alias(self, env, name):
			self.target = [env.Alias(name, self.target)]
	
	## Library target with support to be used by other targets to compile against in-source
	#
	#  To use a target of this type in building use the applyBuild(env) method. This method
	#  updates various construction parameters like CPPPATH and LIBPATH to add the appropriate
	#  path to build against the build library.
	#  
	#  Supports in addition to the build target install and archive target to be defined.
	#  Archive target is used to build a release archive or self-installer. It is a dictionary
	#  with target as value and the name inside the archive as key.
	#  
	#  By default install and archive are empty.
	class _TargetLibrary(_Target):
		def __init__(self, description, target=None, **args):
			super(_TargetLibrary, self).__init__(description)
			if target:
				if isinstance(target, (list, tuple)):
					self.build = target
				else:
					self.build = [target]
			else:
				self.build = []
			self.install = []
			self.archive = {}
			self.params = {}
			self.specials = {}
			self.addParameters(**args)
		
		## Install files with first n destination directories cut.
		#
		#  For example using cutDirCount = 1 the file 'src/dir/file.h' is installed under path as 'dir/file.h'.
		#
		#  \param path Path to install files to
		#  \param files Files to install
		#  \param cutDirCount Number of parent directories to cut inside path
		def installCutDirs(self, env, path, files, cutDirCount):
			self.install.extend([env.InstallAs(os.path.join(path, *f.split(os.sep)[cutDirCount:]), f) for f in files])
		
		## Archive files with first n destination directories cut.
		#
		#  For example using cutDirCount = 1 the file 'src/dir/file.h' is archived under path as 'dir/file.h'.
		#
		#  \param path Path to archive files in
		#  \param files Files to archive
		#  \param cutDirCount Number of parent directories to cut inside path
		def archiveCutDirs(self, env, path, files, cutDirCount):
			self.archive.update({os.path.normpath(os.path.join(path, *f.split(os.sep)[cutDirCount:])): env.File(f).srcnode() for f in files})
		
		## Archive library.
		#
		#  \param path Path to archive library in
		#  \param library Library to archive
		def archiveLibrary(self, env, path, library, static=False):
			for l in library:
				self.archive[os.path.normpath(os.path.join(path, l.name))] = l
				
				if static or 'SHLIBVERSION' not in env:
					continue
				
				version = env['SHLIBVERSION'].split('.')
				lname = l.name.split('.')
				
				if len(version) > 1:
					symlname = '.'.join(lname[0:1-len(version)])
					self.archive[os.path.normpath(os.path.join(path, symlname))] = env.File(symlname)
				
				if len(version) > 0:
					symlname = '.'.join(lname[0:-len(version)])
					self.archive[os.path.normpath(os.path.join(path, symlname))] = env.File(symlname)
		
		## Create build alias for all build targets stored so far
		def aliasBuild(self, env, name):
			self.build = [env.Alias(name, self.build)]
		
		## Create install alias for all install targets stored so far
		def aliasInstall(self, env, name):
			self.install = [env.Alias(name, self.install)]
		
		## Add parameters required for this target to be used for in-source building by C++ based targets.
		#
		#  Adds CPPPATH, CPPFLAGS and LINKPATH parameters
		#  
		#  \param env Environment to use to find directories in.
		#  \param libs Libraries including build ones other targets required to link against.
		#  \param includeDir Directory containing headers required by other targets to build.
		#  \param libDir Directory containing build library.
		def addParametersBuildCPP(self, env, libs, includeDir, libDir='.'):
			if isinstance(includeDir, (list, tuple)):
				self.addParameters(CPPPATH = [env.Dir(x) for x in includeDir])
			else:
				self.addParameters(CPPPATH = [env.Dir(includeDir)])
			if isinstance(libDir, (list, tuple)):
				self.addParameters(LIBPATH = [env.Dir(x) for x in libDir])
			else:
				self.addParameters(LIBPATH = [env.Dir(libDir)])
			if isinstance(libs, (list, tuple)):
				self.addParameters(LIBS = libs)
			else:
				self.addParameters(LIBS = [libs])
		
		## Add parameters to allow building using this library in-source.
		#  
		#  Works similar to Environment.Append
		def addParameters(self, **args):
			for key in args.keys():
				if not key in self.params:
					self.params[key] = []
				if isinstance(args[key], (list, tuple)):
					self.params[key].extend(args[key])
				else:
					self.params[key].append(args[key])
		
		## Apply parameters to allow building using this library in-source.
		#
		#  Updates various construction parameters like CPPPATH and LIBPATH to add the appropriate
		#  path to build against the build library.
		def applyBuild(self, env):
			if self.params:
				env.Append(**self.params)
	
	## Program target
	#
	#  Supports in addition to the build target install and archive target to be defined.
	#  Archive target is used to build a release archive or self-installer. It is a dictionary
	#  with target as value and the name inside the archive as key.
	#  
	#  By default install and archive are empty.
	class _TargetProgram(_Target):
		def __init__(self, description, target=None):
			super(_TargetProgram, self).__init__(description)
			if isinstance(target, (list, tuple)):
				self.build = target
			else:
				self.build = [target]
			self.install = []
			self.archive = {}
		
		## Install files with first n destination directories cut.
		#
		#  For example using cutDirCount = 1 the file 'src/dir/file.res' is installed under path as 'dir/file.res'.
		#
		#  \param path Path to install files to
		#  \param files Files to install
		#  \param cutDirCount Number of parent directories to cut inside path
		def installCutDirs(self, env, path, files, cutDirCount):
			self.install.extend([env.InstallAs(os.path.join(path, *f.split(os.sep)[cutDirCount:]), f) for f in files])
		
		## Archive files with first n destination directories cut.
		#
		#  For example using cutDirCount = 1 the file 'src/dir/file.res' is archived under path as 'dir/file.res'.
		#
		#  \param path Path to archive files in
		#  \param files Files to archive
		#  \param cutDirCount Number of parent directories to cut inside path
		def archiveCutDirs(self, env, path, files, cutDirCount):
			self.archive.update({os.path.normpath(os.path.join(path, *f.split(os.sep)[cutDirCount:])): env.File(f).srcnode() for f in files})
		
		## Archive program.
		#
		#  \param path Path to archive library in
		#  \param program Program to archive
		def archiveProgram(self, env, path, program):
			self.archive.update({os.path.normpath(os.path.join(path, p.name)): p for p in program})
		
		## Create build alias for all build targets stored so far
		def aliasBuild(self, env, name):
			self.build = [env.Alias(name, self.build)]
		
		## Create install alias for all install targets stored so far
		def aliasInstall(self, env, name):
			self.install = [env.Alias(name, self.install)]
	
	class TargetManager:
		Target = _Target
		TargetLibrary = _TargetLibrary
		TargetProgram = _TargetProgram
		
		def __init__(self, env):
			self.targets = {}
			self.env = env
		
		def add(self, description, target):
			self.targets[description] = target
		
		def createReport(self):
			maxLenName = 1
			maxLenTarget = 1
			
			keys = sorted(self.targets.keys())
			
			for key in keys:
				lenTarget = len(key)
				lenName = len(self.targets[key].description)
				if lenTarget > maxLenTarget:
					maxLenTarget = lenTarget
				if lenName > maxLenName:
					maxLenName = lenName
			
			helpText = []
			helpText.append('scons <target> { <target> ... }:')
			helpText.append('    Build the target(s)')
			helpText.append('scons <target>_install { <target>_install ... }:')
			helpText.append('    Build and install target(s)')
			helpText.append('scons <target> { <target> ... } -c:')
			helpText.append('    Clear target(s) build')
			helpText.append('')
			
			helpText.append('{} | Description'.format('Target'.ljust(maxLenTarget)))
			helpText.append('{}-+-{}'.format(''.ljust(maxLenTarget, '-'), ''.ljust(maxLenName, '-')))
			for key in keys:
				helpText.append('{} | {}'.format(key.ljust(maxLenTarget), self.targets[key].description))
			helpText.append('')
			
			return '\n'.join(helpText)
		
		def updateHelp(self):
			self.env.Help(self.createReport())
		
		def __repr__(self):
			return self.createReport()
	
	env.targetManager = TargetManager(env)

def exists(env):
	return True
