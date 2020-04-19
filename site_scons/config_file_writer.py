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
import numbers

__all__ = ['writeConfigFile',]

## Write autotools type config file for C++ projects.
#  \param filename Filename to write config to
#  \param entries Dictionary of entries mapping constant name to define values.
#                 Define values can be boolean, numeric or string values.
def writeConfigFile(filename, entries):
	text = []
	
	text.append('/*')
	text.append('Configuration File. Do not edit since')
	text.append('this is a generated file and all changes')
	text.append('will be lost the next time scons runs.')
	text.append('*/')
	text.append('')
	
	for key in sorted(entries.keys()):
		if entries[key]:
			text.append('/* enable {} */'.format(key))
			text.append('#ifndef {}'.format(key))
			if entries[key] is True:
				text.append('#define {} 1'.format(key))
			elif isinstance(entries[key], numbers.Number):
				text.append('#define {} {}'.format(entries[key]))
			else:
				text.append('#define {} "{}"'.format(entries[key].replace('"', '\\"')))
			text.append('#endif')
		else:
			text.append('/* disable {} */'.format(key))
			text.append('#ifdef {}'.format(key))
			text.append('#undef {}'.format(key))
			text.append('#endif')
		text.append('')
	
	text.append('/* End of configuration file. */')
	
	with open(filename, 'w') as f:
		f.write(str(os.linesep).join(text))
