#ifndef OPENVR_WRAPPER_HPP
#define OPENVR_WRAPPER_HPP

#include <lak/string.hpp>

#include <openvr.h>

#include <glm/mat3x4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

void init_openvr();
void init_cameras();

glm::mat4 to_z_up(const glm::mat4 &transform);
glm::mat3x3 hmd_to_glm(const vr::HmdMatrix33_t &m);
vr::HmdMatrix33_t glm_to_hmd(const glm::mat3x3 &m);
glm::mat4x3 hmd_to_glm(const vr::HmdMatrix34_t &m);
vr::HmdMatrix34_t glm_to_hmd(const glm::mat4x3 &m);
glm::mat4x4 hmd_to_glm(const vr::HmdMatrix44_t &m);
vr::HmdMatrix44_t glm_to_hmd(const glm::mat4x4 &m);
glm::vec3 hmd_to_glm(const vr::HmdVector3_t &m);
vr::HmdVector3_t glm_to_hmd(const glm::vec3 &m);
glm::vec4 hmd_to_glm(const vr::HmdVector4_t &m);
vr::HmdVector4_t glm_to_hmd(const glm::vec4 &m);

lak::astring get_tracked_device_string(
  vr::TrackedDeviceIndex_t device,
  vr::TrackedDeviceProperty prop,
  vr::TrackedPropertyError *error = nullptr);

#endif
