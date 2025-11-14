cmake = import('cmake')

if target_machine.system() != 'windows'
	error('OS @0@ is not compatible with the Kinect SDK'.format(target_machine.system()))
endif

kinect_sdk_path = get_option('kinect_sdk_path')
kinect_sdk_dll = 'Microsoft.Kinect.dll'
kinect_sdk_inc = include_directories(kinect_sdk_path/'inc')
kinect_sdk_dll_path = kinect_sdk_path/'Assemblies'
if target_machine.cpu_family() == 'x86'
	kinect_sdk_lib_path = kinect_sdk_path/'lib'/'x86'
elif target_machine.cpu_family() == 'x86_64'
	kinect_sdk_lib_path = kinect_sdk_path/'lib'/'amd64'
else
	error('CPU arch @0@ is not compatible with the Kinect SDK'.format(target_machine.cpu_family()))
endif

cc = meson.get_compiler('c')
kinect_sdk_dep = declare_dependency(
	dependencies: cc.find_library('Kinect10', dirs: kinect_sdk_lib_path),
	include_directories: kinect_sdk_inc,
)

openvr_opts = cmake.subproject_options()

openvr_opts.add_cmake_defines({
	'BUILD_SHARED': 'OFF',
})
openvr_opts.set_install(false)
if get_option('b_vscrt') == 'mt'
	openvr_opts.append_compile_args('cpp', '/MT')
elif get_option('b_vscrt') == 'mtd'
	openvr_opts.append_compile_args('cpp', '/MTd')
elif get_option('b_vscrt') == 'static_from_buildtype'
	if get_option('buildtype') == 'debug'
		openvr_opts.append_compile_args('cpp', '/MTd')
	else
		openvr_opts.append_compile_args('cpp', '/MT')
	endif
elif get_option('b_vscrt') == 'md'
	openvr_opts.append_compile_args('cpp', '/MD')
elif get_option('b_vscrt') == 'mdd'
	openvr_opts.append_compile_args('cpp', '/MDd')
elif get_option('b_vscrt') == 'from_buildtype'
	if get_option('buildtype') == 'debug'
		openvr_opts.append_compile_args('cpp', '/MDd')
	else
		openvr_opts.append_compile_args('cpp', '/MD')
	endif
elif get_option('b_vscrt') != 'none'
	error('unsupported vscrt link mode')
endif

openvr_subprj = cmake.subproject('openvr', options: openvr_opts)

if host_machine.system() == 'windows'
	openvr_dep = openvr_subprj.dependency('openvr_api64').as_link_whole()
else
	openvr_dep = openvr_subprj.dependency('openvr_api').as_link_whole()
endif
