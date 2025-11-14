project(
	'VT-LM-3D-scanner',
	['c', 'cpp'],
	default_options: [
		'b_vscrt=static_from_buildtype',
	],
)

lak_subprj = subproject('lak')

c_cpp_args = lak_subprj.get_variable('c_cpp_args')
cpp_args = lak_subprj.get_variable('cpp_args')
c_args = lak_subprj.get_variable('c_args')
version = lak_subprj.get_variable('version')
lak_dep = lak_subprj.get_variable('lak_dep')

c_cpp_args += [
	'-DGLM_FORCE_SIZE_T_LENGTH',
]

cxx = meson.get_compiler('cpp')

if cxx.get_id() == 'msvc'
	c_cpp_args += [
		'-bigobj',
	]
endif

add_project_arguments(c_cpp_args + cpp_args, language: ['cpp'])
add_project_arguments(c_cpp_args + c_args, language: ['c'])

if host_machine.system() == 'windows'
  git_header = custom_target(
    'kinetic_generate_git_file',
    output: 'kinetic_git.hpp',
    command: ['generate_git_file.bat', '@OUTPUT@'] ,
    build_by_default: true,
  )
else
  git_header = custom_target(
    'kinetic_generate_git_file',
    output: 'kinetic_git.hpp',
    command: ['./generate_git_file.sh', '@OUTPUT@'] ,
    build_by_default: true,
  )
endif

subdir('include')
subdir('src')

fs = import('fs')
fs.copyfile(kinect_sdk_dll_path/kinect_sdk_dll, kinect_sdk_dll)

executable(
	'kinetic',
	[
		kinetic_files,
		git_header,
	],
	override_options: lak_subprj.get_variable('override_options_w3'),
	link_with: [
		openvr_wrapper,
	],
	dependencies: [
		lak_dep,
		openvr_dep,
		kinect_sdk_dep,
	],
)
