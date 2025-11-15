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
