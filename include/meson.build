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
