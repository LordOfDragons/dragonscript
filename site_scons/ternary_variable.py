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

import string

__all__ = ['TernaryVariable',]

# Support for variable accepting values of type yes, no and auto
class _TernaryVariable():
	Yes = 'yes'
	No = 'no'
	Auto = 'auto'
	
	_yesValues = ('y', 'yes', 'true', 't', '1', 'on', 'all')
	_noValues = ('n', 'no', 'false', 'f', '0', 'off', 'none')
	_autoValues = ('auto')
	_allValues = (Yes, No, Auto)
	
	def _mapValue(self, text):
		textLower = text.lower()
		if textLower in self._yesValues:
			return self.Yes
		if textLower in self._noValues:
			return self.No
		if textLower in self._autoValues:
			return self.Auto
		raise ValueError('Invalid value for ternary option: {}'.format(text))
	
	def _validate(self, key, value, env):
		if not env[key] in self._allValues:
			raise SCons.Errors.UserError('Invalid value for ternary option {}: {}'.format(key, env[key]))
	
	def __call__(self, key, help, default=None):
		if default is None:
			default = self.Auto
		return (key, '{} (yes|no|auto)'.format(help), default, self._validate, self._mapValue )

TernaryVariable = _TernaryVariable()
