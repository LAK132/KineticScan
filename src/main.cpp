#include "main.hpp"

#include "openvr_wrapper.hpp"

#include <lak/array.hpp>
#include <lak/binary_writer.hpp>
#include <lak/debug.hpp>
#include <lak/system/file.hpp>

#include <iostream>

#define LAK_BASIC_PROGRAM_IMGUI_WINDOW_IMPL
#include <lak/basic_single_window_program.inl>

#include <lak/system/windowing/window.hpp>

#include <lak/system/win32/wrapper.hpp>

#include <Ole2.h>
#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#include <NuiApi.h>

vr::TrackedDevicePose_t tracked_device_poses[vr::k_unMaxTrackedDeviceCount];
glm::mat4 device_poses[vr::k_unMaxTrackedDeviceCount];
vr::ETrackedDeviceClass device_classes[vr::k_unMaxTrackedDeviceCount];

INuiSensor *nui_sensor = nullptr;

SOCKET connect_socket = INVALID_SOCKET;

std::error_code last_wsa_error()
{
	return lak::winapi::make_win32_error(WSAGetLastError());
}

struct nui_error
{
	HRESULT value;

	inline lak::astring to_string() const
	{
		switch (value)
		{
			case E_NUI_DEVICE_NOT_CONNECTED:
				return "E_NUI_DEVICE_NOT_CONNECTED";
				break;
			case E_NUI_DEVICE_NOT_READY:
				return "E_NUI_DEVICE_NOT_READY";
				break;
			case E_NUI_ALREADY_INITIALIZED:
				return "E_NUI_ALREADY_INITIALIZED";
				break;
			case E_NUI_NO_MORE_ITEMS:
				return "E_NUI_NO_MORE_ITEMS";
				break;

			case E_NUI_FRAME_NO_DATA:
				return "E_NUI_FRAME_NO_DATA";
				break;
			case E_NUI_STREAM_NOT_ENABLED:
				return "E_NUI_STREAM_NOT_ENABLED";
				break;
			case E_NUI_IMAGE_STREAM_IN_USE:
				return "E_NUI_IMAGE_STREAM_IN_USE";
				break;
			case E_NUI_FRAME_LIMIT_EXCEEDED:
				return "E_NUI_FRAME_LIMIT_EXCEEDED";
				break;
			case E_NUI_FEATURE_NOT_INITIALIZED:
				return "E_NUI_FEATURE_NOT_INITIALIZED";
				break;
			case E_NUI_NOTGENUINE:
				return "E_NUI_NOTGENUINE";
				break;
			case E_NUI_INSUFFICIENTBANDWIDTH:
				return "E_NUI_INSUFFICIENTBANDWIDTH";
				break;
			case E_NUI_NOTSUPPORTED:
				return "E_NUI_NOTSUPPORTED";
				break;
			case E_NUI_DEVICE_IN_USE:
				return "E_NUI_DEVICE_IN_USE";
				break;

			case E_NUI_DATABASE_NOT_FOUND:
				return "E_NUI_DATABASE_NOT_FOUND";
				break;
			case E_NUI_DATABASE_VERSION_MISMATCH:
				return "E_NUI_DATABASE_VERSION_MISMATCH";
				break;
			case E_NUI_HARDWARE_FEATURE_UNAVAILABLE:
				return "E_NUI_HARDWARE_FEATURE_UNAVAILABLE";
				break;
			case E_NUI_NOTCONNECTED:
				return "E_NUI_NOTCONNECTED";
				break;
			case E_NUI_NOTREADY:
				return "E_NUI_NOTREADY";
				break;
			case E_NUI_SKELETAL_ENGINE_BUSY:
				return "E_NUI_SKELETAL_ENGINE_BUSY";
				break;
			case E_NUI_NOTPOWERED:
				return "E_NUI_NOTPOWERED";
				break;
			case E_NUI_BADIINDEX:
				return "E_NUI_BADIINDEX";
				break;

			default:
				static_assert(sizeof(HRESULT) == sizeof(unsigned long));
				return lak::to_astring(
				  lak::streamify(static_cast<unsigned long>(value)));
				break;
		}
	}
};

template<typename CHAR>
std::basic_ostream<CHAR> &operator<<(std::basic_ostream<CHAR> &strm,
                                     const nui_error &err)
{
	return strm << lak::string_view{err.to_string()};
}

template<typename T = lak::monostate>
using nui_result = lak::result<T, nui_error>;

nui_result<> as_nui_result(HRESULT val)
{
	if (FAILED(val))
		return lak::err_t{nui_error{val}};
	else
		return lak::ok_t{};
}

// NUI errors are Win32 errors

template<typename T>
bool send_image(const lak::image<T> &buffer)
{
	lak::binary_array_writer strm;
	strm.reserve((sizeof(uint32_t) * 2) + buffer.contig_size_bytes());
	strm.write(static_cast<uint32_t>(buffer.size().x));
	strm.write(static_cast<uint32_t>(buffer.size().y));
	strm.write(lak::span(buffer.data(), buffer.contig_size()));
	auto buf = strm.release();
	auto err = send(connect_socket,
	                reinterpret_cast<const char *>(buf.data()),
	                static_cast<int>(buf.size()),
	                0);
	if (err == SOCKET_ERROR)
	{
		ERROR("send failed (", last_wsa_error(), ")");
		return false;
	}
	return true;
}

bool send_colour(const lak::image<lak::color4_t> &buffer)
{
	static_assert(sizeof(lak::color4_t) == sizeof(uint8_t) * 4);
	return send_image(buffer);
}

bool send_depth(const lak::image<uint8_t> &buffer)
{
	return send_image(buffer);
}

void imgui_matrix(const char *str, const glm::mat4 &matrix)
{
	ImGui::Text("%s", str);
	auto m = glm::transpose(matrix);
	ImGui::SliderFloat4("", &m[0][0], -1.f, 1.f, "%+.3f");
	ImGui::SliderFloat4("", &m[1][0], -1.f, 1.f, "%+.3f");
	ImGui::SliderFloat4("", &m[2][0], -1.f, 1.f, "%+.3f");
	ImGui::SliderFloat4("", &m[3][0], -1.f, 1.f, "%+.3f");
}

void update_poses()
{
	vr::VRCompositor()->WaitGetPoses(
	  tracked_device_poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	for (uint32_t nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount;
	     ++nDevice)
	{
		if (tracked_device_poses[nDevice].bPoseIsValid)
		{
			device_poses[nDevice] = to_z_up(lak::openvr::to_glm(
			  tracked_device_poses[nDevice].mDeviceToAbsoluteTracking));
			if (device_classes[nDevice] == vr::TrackedDeviceClass_Invalid)
				device_classes[nDevice] =
				  vr::VRSystem()->GetTrackedDeviceClass(nDevice);
		}
	}
}

lak::optional<int> LAK_BASIC_PROGRAM(program_preinit)(lak::span<char *>)
{
	return lak::nullopt;
}

lak::optional<int> LAK_BASIC_SINGLE_WINDOW_PROGRAM(program_init)()
{
	basic_window_target_framerate                = 30;
	basic_window_opengl_settings.major           = 3;
	basic_window_opengl_settings.minor           = 2;
	basic_window_opengl_settings.double_buffered = true;
	basic_window_clear_colour                    = {0.0f, 0.0f, 0.0f, 1.0f};

	basic_imgui_main_window_flags =
	  ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar |
	  ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings |
	  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;

	lak::openvr::init(vr::VRApplication_Other).UNWRAP();

	if (false)
	{
		WSADATA wsa_data;

		if_let_err (auto err,
		            lak::winapi::result_from_win32(
		              WSAStartup(MAKEWORD(2, 2), &wsa_data)))
		{
			ERROR("WSAStartup failed (", err, ")");
			return EXIT_FAILURE;
		}

		struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family   = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		// struct addrinfo *result;

		// if (auto err = getaddrinfo("localhost", "13279", &hints, &result); err
		// != 0)
		// {
		//   ERROR("getaddrinfo failed (0x" << err << ")");
		//   WSACleanup();
		//   return 1;
		// }

		// for (struct addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next)
		// {
		//   connect_socket = socket(ptr->ai_family, ptr->ai_socktype,
		//   ptr->ai_protocol); if (connect_socket == INVALID_SOCKET)
		//   {
		//     ERROR("socket failed (0x" << WSAGetLastError() << ")");
		//     WSACleanup();
		//     return 1;
		//   }

		//   if (auto err = connect(connect_socket, ptr->ai_addr,
		//   (int)ptr->ai_addrlen);
		//       err != SOCKET_ERROR)
		//     break; // we got a connection!

		//   closesocket(connect_socket);
		//   connect_socket = INVALID_SOCKET;
		// }

		// freeaddrinfo(result);

		connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (connect_socket == INVALID_SOCKET)
		{
			ERROR("socket failed (", connect_socket, ")");
			WSACleanup();
			return EXIT_FAILURE;
		}

		sockaddr_in addr;
		ZeroMemory(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(132'79);
		if (auto err = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr); err == 0)
		{
			ERROR("invalid IPv4 address");
			closesocket(connect_socket);
			WSACleanup();
			return EXIT_FAILURE;
		}
		else if (err == -1)
		{
			ERROR("failed to get address (", last_wsa_error(), ")");
			closesocket(connect_socket);
			WSACleanup();
			return EXIT_FAILURE;
		}

		if (connect(connect_socket,
		            reinterpret_cast<SOCKADDR *>(&addr),
		            sizeof(addr)) == SOCKET_ERROR)
		{
			ERROR("failed to connecto to socket (", last_wsa_error(), ")");
			closesocket(connect_socket);
			connect_socket = INVALID_SOCKET;
		}

		if (connect_socket == INVALID_SOCKET)
		{
			ERROR("unable to connect to server");
			WSACleanup();
			return EXIT_FAILURE;
		}
	}

	{
		if_let_err (auto err,
		            as_nui_result(NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH |
		                                        NUI_INITIALIZE_FLAG_USES_COLOR)))
		{
			ERROR("failed to initialise NUI (", err, ")");
			return EXIT_FAILURE;
		}
	}

	return lak::nullopt;
}

int LAK_BASIC_PROGRAM(program_quit)()
{
	vr::VR_Shutdown();

	return EXIT_SUCCESS;
}

void LAK_BASIC_PROGRAM(window_init)(lak::window &window)
{
	window.set_title(L"" APP_NAME);
}

void LAK_BASIC_PROGRAM(window_handle_event)(lak::window *window,
                                            lak::event &event)
{
	switch (event.type)
	{
		case lak::event_type::close_window:
			ASSERT(!!window);
			basic_destroy_window(*window);
			break;

		case lak::event_type::quit_program:
			// Need to rework this, causes a crash.
			// ASSERT(!!basic_single_window_window);
			// basic_destroy_window(*basic_single_window_window);
			break;

		default:
			break;
	}
}

void LAK_BASIC_PROGRAM(window_loop)(lak::window &window,
                                    uint64_t counter_delta)
{
	LAK_UNUSED(window);
	LAK_UNUSED(counter_delta);

	/* --- Handle SteamVR events --- */

	for (vr::VREvent_t event;
	     vr::VRSystem()->PollNextEvent(&event, sizeof(event));)
	{
		switch (event.eventType)
		{
			case vr::VREvent_TrackedDeviceDeactivated:
			{
				DEBUG("Device ", event.trackedDeviceIndex, " detached.");
			}
			break;
			case vr::VREvent_TrackedDeviceActivated:
			{
				DEBUG("Device ", event.trackedDeviceIndex, " activated.");
			}
			break;
			case vr::VREvent_TrackedDeviceUpdated:
			{
				DEBUG("Device ", event.trackedDeviceIndex, " updated.");
			}
			break;
		}
	}

	update_poses();

	for (size_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
	{
		switch (device_classes[i])
		{
			case vr::TrackedDeviceClass_Controller:
				ImGui::Separator();
				imgui_matrix("Controller", device_poses[i]);
				break;
			case vr::TrackedDeviceClass_HMD:
				ImGui::Separator();
				imgui_matrix("HMD", device_poses[i]);
				break;
			case vr::TrackedDeviceClass_GenericTracker:
				ImGui::Separator();
				imgui_matrix("Generic Tracker", device_poses[i]);
				break;
			case vr::TrackedDeviceClass_TrackingReference:
				ImGui::Separator();
				imgui_matrix("Tracking Reference", device_poses[i]);
				break;
		}
	}
}

void LAK_BASIC_PROGRAM(window_quit)(lak::window &)
{
	//
}
