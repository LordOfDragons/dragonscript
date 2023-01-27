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

import tarfile
import zipfile
import os
import re

from SCons.Builder import Builder
from SCons.Action import Action

def generate(env):
	reWindowsDrive = re.compile('^([A-Z]:[/\\\\])|([A-Z][A-Z0-9]*//)', re.I)
	
	def normalizePath(path):
		path = os.path.normpath(path)
		
		path = os.path.splitdrive(path)[1] # return (drive, tail)
		
		# for cross compiling splitdrive wont work since python is compiled for
		# the host system and os.path methods work with host system parameters.
		# use a simple regular expression check to see if we need to cut the
		# start of the string
		match = reWindowsDrive.match(path)
		if match:
			path = path[match.end():]
		
		return path
	
	def buildArchiveTarBz2(env, target, source):
		with tarfile.open( target[0].abspath, 'w:bz2', dereference=True) as arcfile:
			for path, node in env['ARCHIVE_FILES'].items():
				info = arcfile.gettarinfo(node.abspath, normalizePath(path))
				#info.mode = ...
				info.uid = 0
				info.gid = 0
				info.uname = 'root'
				info.gname = 'root'
				with open(node.abspath, 'rb') as nf:
					arcfile.addfile(info, nf)
	
	def buildArchiveZip(env, target, source):
		with zipfile.ZipFile(target[0].abspath, 'w', zipfile.ZIP_DEFLATED) as arcfile:
			for path, node in env['ARCHIVE_FILES'].items():
				arcfile.write(node.abspath, normalizePath(path))
	
	env.Append(BUILDERS={'ArchiveTarBz2': Builder(action=Action(
		buildArchiveTarBz2, '$ARCHIVETARBZ2COMSTR'), suffix='.tar.bz2', src_suffix='')})
	env['ARCHIVETARBZ2COM'] = 'Archiving "$TARGET"'
	env['ARCHIVETARBZ2COMSTR'] = env['ARCHIVETARBZ2COM']
	
	env.Append(BUILDERS={'ArchiveZip': Builder(action=Action(
		buildArchiveZip, '$ARCHIVEZIPCOMSTR'), suffix='.zip', src_suffix='')})
	env['ARCHIVEZIPCOM'] = 'Archiving "$TARGET"'
	env['ARCHIVEZIPCOMSTR'] = env['ARCHIVEZIPCOM']
	
	try:
		class _TargetArchive(env.targetManager.Target):
			formatTarBz2 = 'tarbz2'
			formatZip = 'zip'
			
			def __init__(self, description, target=None, **args):
				super(_TargetArchive, self).__init__(description, target)
			
			def archiveFiles(self, env, target, files, format=formatTarBz2):
				if format == self.formatTarBz2:
					self.target.extend(env.ArchiveTarBz2(target, files.values(), ARCHIVE_FILES=files))
				elif format == self.formatZip:
					self.target.extend(env.ArchiveZip(target, files.values(), ARCHIVE_FILES=files))
				else:
					raise 'Invalid format {}'.format(format)
		env.targetManager.TargetArchive = _TargetArchive
	except:
		pass

def exists(env):
	return True
