# -*- coding: utf-8 -*-
#
# DragonScript
#
# Copyright (C) 2023, Plüss Roland ( roland@rptd.ch )
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

import string

__all__ = ['StringVariable',]

# Support for variable accepting values of string type
class _StringVariable():
	def __call__(self, key, help, default=None):
		if default is None:
			default = ''
		return (key, '{} (string)'.format(help), default)

StringVariable = _StringVariable()
