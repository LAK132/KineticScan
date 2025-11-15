#ifndef OPENVR_WRAPPER_HPP
#define OPENVR_WRAPPER_HPP

#include <lak/string.hpp>

#include <lak/openvr/openvr.hpp>

#include <glm/mat3x4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

glm::mat4 to_z_up(const glm::mat4 &transform);

lak::astring get_tracked_device_string(
  vr::TrackedDeviceIndex_t device,
  vr::TrackedDeviceProperty prop,
  vr::TrackedPropertyError *error = nullptr);

#endif
