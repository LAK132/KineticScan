#include "openvr_wrapper.hpp"

#include <lak/array.hpp>
#include <lak/debug.hpp>

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 to_z_up(const glm::mat4 &transform)
{
	static const auto root_transform = glm::rotate(
	  glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::vec3 offset(transform[3]);
	return glm::translate(root_transform * glm::translate(transform, -offset),
	                      offset);
}

glm::mat3x3 hmd_to_glm(const vr::HmdMatrix33_t &m)
{
	glm::mat3x3 result;
	for (size_t i = 0; i < 3; ++i)
		for (size_t j = 0; j < 3; ++j) result[i][j] = m.m[j][i];
	return result;
}

vr::HmdMatrix33_t glm_to_hmd(const glm::mat3x3 &m)
{
	vr::HmdMatrix33_t result;
	for (size_t i = 0; i < 3; ++i)
		for (size_t j = 0; j < 3; ++j) result.m[i][j] = m[j][i];
	return result;
}

glm::mat4x3 hmd_to_glm(const vr::HmdMatrix34_t &m)
{
	glm::mat4x3 result;
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 3; ++j) result[i][j] = m.m[j][i];
	return result;
}

vr::HmdMatrix34_t glm_to_hmd(const glm::mat4x3 &m)
{
	vr::HmdMatrix34_t result;
	for (size_t i = 0; i < 3; ++i)
		for (size_t j = 0; j < 4; ++j) result.m[i][j] = m[j][i];
	return result;
}

glm::mat4x4 hmd_to_glm(const vr::HmdMatrix44_t &m)
{
	glm::mat4x4 result;
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j) result[i][j] = m.m[j][i];
	return result;
}

vr::HmdMatrix44_t glm_to_hmd(const glm::mat4x4 &m)
{
	vr::HmdMatrix44_t result;
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j) result.m[i][j] = m[j][i];
	return result;
}

glm::vec3 hmd_to_glm(const vr::HmdVector3_t &m)
{
	glm::vec3 result;
	for (size_t i = 0; i < 3; ++i) result[i] = m.v[i];
	return result;
}

vr::HmdVector3_t glm_to_hmd(const glm::vec3 &m)
{
	vr::HmdVector3_t result;
	for (size_t i = 0; i < 3; ++i) result.v[i] = m[i];
	return result;
}

glm::vec4 hmd_to_glm(const vr::HmdVector4_t &m)
{
	glm::vec4 result;
	for (size_t i = 0; i < 4; ++i) result[i] = m.v[i];
	return result;
}

vr::HmdVector4_t glm_to_hmd(const glm::vec4 &m)
{
	vr::HmdVector4_t result;
	for (size_t i = 0; i < 4; ++i) result.v[i] = m[i];
	return result;
}

lak::astring get_tracked_device_string(vr::TrackedDeviceIndex_t device,
                                       vr::TrackedDeviceProperty prop,
                                       vr::TrackedPropertyError *error)
{
	uint32_t buffer_length = vr::VRSystem()->GetStringTrackedDeviceProperty(
	  device, prop, NULL, 0, error);
	if (buffer_length == 0) return "";

	lak::array<char> buffer;
	buffer.resize(buffer_length, '\0');
	buffer_length = vr::VRSystem()->GetStringTrackedDeviceProperty(
	  device, prop, buffer.data(), buffer_length, error);
	return buffer.data();
}

void init_openvr()
{
	vr::EVRInitError init_error = vr::VRInitError_None;
	vr::VR_Init(&init_error, vr::VRApplication_Scene);

	if (init_error != vr::VRInitError_None)
		FATAL("Unable to init VR runtime: ",
		      vr::VR_GetVRInitErrorAsEnglishDescription(init_error));
}
