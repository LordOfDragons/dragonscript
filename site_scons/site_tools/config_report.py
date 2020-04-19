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

def generate(env):
	class ConfigReport:
		def __init__(self, env):
			self.entries = {'': {}}
			self.env = env
		
		def add(self, name, key, group=''):
			if not group in self.entries:
				self.entries[group] = {}
			self.entries[group][key] = name
		
		def createReport(self):
			entries = {}
			for groupKey in self.entries.keys():
				group = self.entries[groupKey]
				if not group.keys():
					continue
				groupEntries = {}
				entries[groupKey] = groupEntries
				
				for key in group.keys():
					value = env[key]
					if value is True:
						value = 'yes'
					elif value is False:
						value = 'no'
					elif value is None:
						value = ''
					else:
						value = str(value)
					groupEntries[group[key]] = value
			
			maxLenKey = 1
			maxLenValue = 1
			
			for groupKey in entries.keys():
				group = entries[groupKey]
				for key in sorted(group.keys()):
					lenKey = len(key)
					lenValue = len(group[key])
					if lenKey > maxLenKey:
						maxLenKey = lenKey
					if lenValue > maxLenValue:
						maxLenValue = lenValue
			
			text = []
			text.append('')
			text.append('Configuration Report:')
			if len(entries.keys()) > 1:
				text.append('{}---{}'.format(''.ljust(maxLenKey, '-'), ''.ljust(maxLenValue, '-')))
			else:
				text.append('{}-+-{}'.format(''.ljust(maxLenKey, '-'), ''.ljust(maxLenValue, '-')))
			
			firstGroup = True
			for groupKey in sorted(entries.keys()):
				group = entries[groupKey]
				indent = ''
				if not firstGroup:
					indent = '  '
				if groupKey:
					if not firstGroup:
						text.append('')
					text.append('{}:'.format(groupKey))
				for key in sorted(group.keys()):
					text.append('{}{} | {}'.format(indent, key.ljust(maxLenKey), group[key]))
				firstGroup = False
			text.append('')
			
			return '\n'.join(text)
		
		def save(self, filename='lastConfig.py'):
			entries = {}
			for groupKey in self.entries.keys():
				group = self.entries[groupKey]
				if not group.keys():
					continue
				groupValues = {}
				entries[groupKey] = groupValues
				
				for key in group.keys():
					value = env[key]
					if value is True:
						value = 'yes'
					elif value is False:
						value = 'no'
					elif value is None:
						value = ''
					else:
						value = str(value)
					groupValues[key] = [value, group[key]]
			
			text = []
			text.append('# Configuration')
			text.append('# =============')
			
			for groupKey in sorted(entries.keys()):
				text.append('')
				text.append('# {}:'.format(groupKey))
				text.append('# {}-'.format('-' * len(groupKey)))
				group = entries[groupKey]
				for key in sorted(group.keys()):
					text.append('# {}'.format(group[key][1]))
					text.append('{} = \'{}\''.format(key, group[key][0]))
					text.append('')
					#text.append('{} = \'{}\'  # {}'.format(key, *group[key]))
			
			with open(filename, 'w') as f:
				f.write(str(os.linesep).join(text))
		
		def __repr__(self):
			return self.createReport()
	
	env.configReport = ConfigReport(env)

def exists(env):
	return True
