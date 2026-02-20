# -*- coding: utf-8 -*-
#
# DragonScript
#
# Copyright (C) 2025, Plüss Roland ( roland@rptd.ch )
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

def generate(env, configGroup):
	try:
		env.Tool('emscripten', toolpath=[os.environ['EMSCRIPTEN_TOOL_PATH']])
	except Exception as e:
		print('EMScripten missing. Build using "emscons scons ..."')
		raise e
	
	env.Append(CPPFLAGS = ['-DWEB_WASM'])
	env.Append(CPPFLAGS = ['-O3'])
	
	# disable some nag warnings which are plain out stupid
	#env.Append(CPPFLAGS = ['-Wno-nontrivial-memcall'])
	
	# env.Append(LINKFLAGS = ['-s', 'SIDE_MODULE=1'])

def exists(env):
	return True
