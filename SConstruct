from ternary_variable import TernaryVariable
from string_variable import StringVariable
from glob_files import globFiles
import os
import sys
import shlex

# create parent environment and load tools
globalEnv = Environment()
globalEnv['PARAMETER_SOURCE'] = ['custom.py']
globalEnv.Tool('config_report')
globalEnv.Tool('target_manager')
globalEnv.Tool('crosscompile')
globalEnv.Tool('build_verbose')
globalEnv.Tool('archive_builder')

applyEnvVars = []
applyEnvVars.append('CFLAGS')
applyEnvVars.append('CPPFLAGS')
applyEnvVars.append('CXXFLAGS')
applyEnvVars.append('LDFLAGS')

# move to platform tool
params = Variables(globalEnv['PARAMETER_SOURCE'], ARGUMENTS)
params.Add(EnumVariable('target_platform', 'Target platform', 'auto',
	['auto', 'linux', 'windows', 'beos', 'macos', 'android']))
params.Update(globalEnv)

globalEnv.configReport.add('Target platform', 'target_platform')

targetPlatform = globalEnv['target_platform']
if targetPlatform == 'auto':
	if sys.platform == 'haiku1':
		targetPlatform = 'beos'
	elif os.name == 'win32' or sys.platform == 'win32':
		targetPlatform = 'windows'
	elif sys.platform == 'darwin':
		targetPlatform = 'macos'
	elif os.name == 'posix':
		targetPlatform = 'linux'
	else:
		raise 'Can not auto-detect platform'

globalEnv['TARGET_PLATFORM'] = targetPlatform

if targetPlatform == 'windows':
	globalEnv['WINDOWS_INSERT_DEF'] = 1 # generated .def file if dll is build
	globalEnv.Append(SHLINKFLAGS = ['-Wl,--export-all-symbols']) # windows requires this

# Haiku: The PATH found by SCons are wrong in many ways causing binaries to be not found.
#        Replace them with sane values. This is not a 'good' solution but should work.
if sys.platform == 'haiku1':
	import os
	globalEnv['ENV']['PATH'] = os.environ['PATH']

# parameters
params = Variables(globalEnv['PARAMETER_SOURCE'], ARGUMENTS)
params.Add(StringVariable('version', 'Version', '9999'))
params.Add(BoolVariable('with_debug', 'Build with debug symbols for GDB usage', False))
params.Add(BoolVariable('with_warnerrors', 'Treat warnings as errors (dev-builds)', False))
params.Add(BoolVariable('with_sanitize', 'Enable sanitizing (dev-builds)', False))
params.Add(BoolVariable('with_optimizations', 'Enable run-time optimizations', True))
params.Add(TernaryVariable('build_dsi', 'Build DragonScript Interpreter'))
params.Add(BoolVariable('dsi_measure_runtime', 'Enable DSI run-time measuring (dev-builds)', False))

params.Add(EnumVariable('archive_format', 'Archive file format', 'tarbz2', ['tarbz2', 'zip']))
params.Add(StringVariable('archive_name_library', 'Archive file name without extension for library archive', 'libdscript'))

if globalEnv['TARGET_PLATFORM'] in ['linux', 'android']:
	params.Add(PathVariable('prefix', 'System path', '/usr', PathVariable.PathAccept))
	params.Add(PathVariable('libdir', 'System libraries', '${prefix}/lib', PathVariable.PathAccept))
	params.Add(PathVariable('includedir', 'System includes', '${prefix}/include', PathVariable.PathAccept))
	params.Add(PathVariable('datadir', 'System shares', '${prefix}/share', PathVariable.PathAccept))
	params.Add(PathVariable('sysconfdir', 'System configuration', '/etc', PathVariable.PathAccept))
	params.Add(PathVariable('execdir', 'System binaries', '${prefix}/bin', PathVariable.PathAccept))
	params.Add(PathVariable('sysvardir', 'System var', '/var', PathVariable.PathAccept))
	
	applyEnvVars.append('CC')
	applyEnvVars.append('CXX')

elif globalEnv['TARGET_PLATFORM'] == 'windows':
	params.Add(PathVariable('prefix', 'System path', '/usr', PathVariable.PathAccept))
	params.Add(PathVariable('libdir', 'System libraries', '${prefix}/lib', PathVariable.PathAccept))
	params.Add(PathVariable('includedir', 'System includes', '${prefix}/include', PathVariable.PathAccept))
	params.Add(PathVariable('datadir', 'System shares', '${prefix}/share', PathVariable.PathAccept))
	params.Add(PathVariable('sysconfdir', 'System configuration', '/etc', PathVariable.PathAccept))
	params.Add(PathVariable('execdir', 'System binaries', '${prefix}/bin', PathVariable.PathAccept))
	params.Add(PathVariable('sysvardir', 'System var', '/var', PathVariable.PathAccept))

elif globalEnv['TARGET_PLATFORM'] == 'beos':
	params.Add(PathVariable('prefix', 'System path', '/boot', PathVariable.PathAccept))
	params.Add(PathVariable('libdir', 'System libraries', '${prefix}/lib', PathVariable.PathAccept))
	params.Add(PathVariable('includedir', 'System includes', '${prefix}/develop/headers', PathVariable.PathAccept))
	params.Add(PathVariable('datadir', 'System shares', '${prefix}/data', PathVariable.PathAccept))
	params.Add(PathVariable('sysconfdir', 'System configuration', '${prefix}/settings', PathVariable.PathAccept))
	params.Add(PathVariable('execdir', 'System binaries', '${prefix}/bin', PathVariable.PathAccept))
	params.Add(PathVariable('sysvardir', 'System var', '${prefix}/var', PathVariable.PathAccept))
	params.Add(PathVariable('docdir', 'System documentation', '${prefix}/documentation', PathVariable.PathAccept))

elif globalEnv['TARGET_PLATFORM'] == 'macos':
	params.Add(PathVariable('prefix', 'System path', '/usr', PathVariable.PathAccept))
	params.Add(PathVariable('libdir', 'System libraries', '${prefix}/lib', PathVariable.PathAccept))
	params.Add(PathVariable('includedir', 'System includes', '${prefix}/include', PathVariable.PathAccept))
	params.Add(PathVariable('datadir', 'System shares', '${prefix}/share', PathVariable.PathAccept))
	params.Add(PathVariable('sysconfdir', 'System configuration', '/etc', PathVariable.PathAccept))
	params.Add(PathVariable('execdir', 'System binaries', '${prefix}/bin', PathVariable.PathAccept))
	params.Add(PathVariable('sysvardir', 'System var', '/var', PathVariable.PathAccept))

params.Update(globalEnv)

# set global construction variables
for x in applyEnvVars:
	if x in os.environ:
		globalEnv.MergeFlags({x: shlex.split(os.environ[x])})

if 'CPPFLAGS' in applyEnvVars and 'CPPFLAGS' in os.environ:
	globalEnv.Append(MODULE_CPPFLAGS = shlex.split(os.environ['CPPFLAGS']))

if 'LDFLAGS' in applyEnvVars and 'LDFLAGS' in os.environ:
	globalEnv.Append(MODULE_LINKFLAGS = shlex.split(os.environ['LDFLAGS']))

if globalEnv['with_debug']:
	globalEnv.Append(CPPFLAGS = ['-g', '-fno-omit-frame-pointer'])

globalEnv.Append(CPPFLAGS = ['-Wall'])
if globalEnv['with_warnerrors']:
	globalEnv.Append(CPPFLAGS = ['-Werror'])

# determine sanitize flags to use
globalEnv.Replace(SANITIZE_FLAGS = [])

if globalEnv['with_debug'] and globalEnv['with_sanitize']:
	globalEnv.Append(SANITIZE_FLAGS = [
		'-fsanitize=address',
		'-fsanitize-address-use-after-scope',
		'-fsanitize=pointer-compare',
		'-fsanitize=pointer-subtract'])
	globalEnv.Append(SANITIZE_FLAGS = [
		'-fsanitize=leak'])
	globalEnv.Append(SANITIZE_FLAGS = [
		'-fsanitize=undefined',
		'-fsanitize=shift',
		'-fsanitize=shift-exponent',
		'-fsanitize=shift-base',
		'-fsanitize=integer-divide-by-zero',
		'-fsanitize=unreachable',
		'-fsanitize=vla-bound',
		'-fsanitize=null',
		'-fsanitize=return',
		'-fsanitize=signed-integer-overflow',
		'-fsanitize=bounds',
		'-fsanitize=bounds-strict',
		'-fsanitize=alignment',
		'-fsanitize=object-size',
		'-fsanitize=float-divide-by-zero',
		'-fsanitize=float-cast-overflow',
		'-fsanitize=nonnull-attribute',
		'-fsanitize=returns-nonnull-attribute',
		'-fsanitize=bool',
		'-fsanitize=enum',
		'-fsanitize=vptr',
		'-fsanitize=pointer-overflow',
		'-fsanitize=builtin'])
	
	if globalEnv['TARGET_PLATFORM'] in ['android']:
		globalEnv.Replace(SANITIZE_FLAGS = ['-fsanitize=address'])

# define the targets array and reports dictionary to be filled
parent_targets = {}

configGroup = 'DragonScript'
globalEnv.configReport.add('Treat warnings as errors (dev-builds)', 'with_warnerrors', configGroup)
globalEnv.configReport.add('Build with debug symbols for GDB usage', 'with_debug', configGroup)
globalEnv.configReport.add('Enable run-time optimizations', 'with_optimizations', configGroup)
globalEnv.configReport.add('Build DragonScript Interpreter', 'build_dsi', configGroup)

# build scripts
SConscript(dirs='src/scriptengine', variant_dir='src/scriptengine/build', duplicate=0, exports='globalEnv' )

packages = []
#packages.append('src/packages/collections')
#packages.append('src/packages/dsbench')
#packages.append('src/packages/dsunit')
#packages.append('src/packages/exceptions')
packages.append('src/packages/introspection')
packages.append('src/packages/math')
packages.append('src/packages/utils')

for p in packages:
	SConscript(dirs=p, variant_dir='{}/build'.format(p), duplicate=0, exports='globalEnv' )

SConscript(dirs='src/dsi', variant_dir='src/dsi/build', duplicate=0, exports='globalEnv' )

SConscript(dirs='archive', variant_dir='archive/build', duplicate=0, exports='globalEnv' )

# build all target
targets = []
for t in globalEnv.targetManager.targets.values():
	try:
		targets.extend(t.build)
	except:
		pass
globalEnv.targetManager.add('build', globalEnv.targetManager.Target('Build All', globalEnv.Alias('build', targets)))

# install all target
targets = []
for t in globalEnv.targetManager.targets.values():
	try:
		targets.extend(t.install)
	except:
		pass
globalEnv.targetManager.add('install', globalEnv.targetManager.Target('Install All', globalEnv.Alias('install', targets)))

# by default just build
Default('build')

# finish
globalEnv.targetManager.updateHelp()
print(globalEnv.configReport)
globalEnv.configReport.save()

"""
archive = {}
for t in globalEnv.targetManager.targets.values():
	try:
		archive.update(t.archive)
	except:
		pass
for key in sorted(archive.keys()):
	print(key)
"""
