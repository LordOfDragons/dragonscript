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
import fnmatch

__all__ = ['globFiles',]

## Glob files in directory relative to env and adds all matching files.
#
#  The added files have directory prefixed.
#
#  \param search Directory under SConscript directory to search
#  \param pattern Pattern to match files against
#  \param result List to add found files
#  \param recursive Search in sub-directories
def globFiles(env, search, pattern, result, recursive=True):
	oldcwd = os.getcwd()
	os.chdir(env.Dir('.').srcnode().abspath)
	
	for root, dirs, files in os.walk(search):
		if recursive:
			dirs[:] = [x for x in dirs if x[0] != '.']
		else:
			del dirs[:]
		
		for f in fnmatch.filter(files, pattern):
			result.append(os.path.join(root, f))
	
	os.chdir( oldcwd )
