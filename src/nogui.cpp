#define WIN32_LEAN_AND_MEAN
#include <Ole2.h>
#include <WinSock2.h>
#include <Windows.h>
#include <comdef.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#pragma comment(lib, "kinect10.lib")

#include <NuiApi.h>

#include <fstream>
#include <ios>
#include <iostream>
#include <thread>

#include <lak/vec.h>

#include "debug.h"

INuiSensor *nui_sensor = nullptr;

int opengl_major;
int opengl_minor;

template<typename T>
std::string win32_error(T err)
{
  _com_error com_err(err);
  return std::string(com_err.ErrorMessage());
}

std::string last_wsa_error()
{
  return win32_error(WSAGetLastError());
}

void lak_terminate_handler()
{
  ABORT();
}

std::string error_string(HRESULT error)
{
  std::string result;
  switch (error)
  {
    case S_OK: result = "S_OK"; break;
    case S_FALSE: result = "S_FALSE"; break;

    case E_FAIL: result = "E_FAIL"; break;
    case E_INVALIDARG: result = "E_INVALIDARG"; break;
    case E_OUTOFMEMORY: result = "E_OUTOFMEMORY"; break;
    case E_POINTER: result = "E_POINTER"; break;

    case E_NUI_DEVICE_NOT_CONNECTED:
      result = "E_NUI_DEVICE_NOT_CONNECTED";
      break;
    case E_NUI_DEVICE_NOT_READY: result = "E_NUI_DEVICE_NOT_READY"; break;
    case E_NUI_FEATURE_NOT_INITIALIZED:
      result = "E_NUI_FEATURE_NOT_INITIALIZED";
      break;
    case E_NUI_IMAGE_STREAM_IN_USE:
      result = "E_NUI_IMAGE_STREAM_IN_USE";
      break;
    case E_NUI_FRAME_NO_DATA: result = "E_NUI_FRAME_NO_DATA"; break;

    default: result = std::to_string(error); break;
  }
  return result;
}

int main()
{
  std::set_terminate(&lak_terminate_handler);

  /* --- Initialise Nui --- */

  if (const auto result = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH |
                                        NUI_INITIALIZE_FLAG_USES_COLOR);
      FAILED(result))
  {
    ERROR("Failed to initialise NUI (" + error_string(result) + ")");
    return 1;
  }

  /* --- Open Nui streams --- */

  HANDLE depth_stream;
  HANDLE colour_stream;

  auto try_open = [](HANDLE *stream,
                     NUI_IMAGE_TYPE type,
                     NUI_IMAGE_RESOLUTION res) -> HRESULT {
    HRESULT result;
    size_t max_tries = 10000;
    do
    {
      result = NuiImageStreamOpen(type, res, NULL, 2, NULL, stream);
      std::this_thread::yield();
    } while (FAILED(result) && --max_tries);
    return result;
  };

  if (const auto depth_open_result = try_open(
        &depth_stream, NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480);
      FAILED(depth_open_result))
  {
    ERROR("Failed to open depth stream (" + error_string(depth_open_result) +
          ")");
    NuiShutdown();
    return 1;
  }

  if (const auto colour_open_result = try_open(
        &colour_stream, NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_1280x960);
      FAILED(colour_open_result))
  {
    ERROR("Failed to open colour stream (" + error_string(colour_open_result) +
          ")");
    NuiShutdown();
    return 1;
  }

  auto read_frame =
    [&](HANDLE stream, size_t element_size, auto callback) -> bool {
    const NUI_IMAGE_FRAME *frame = nullptr;
    const auto get_frame_result =
      NuiImageStreamGetNextFrame(stream, 1000, &frame);

    if (FAILED(get_frame_result))
    {
      ERROR("Failed to get next frame from colour stream (" +
            error_string(get_frame_result) + ")");
      return false;
    }

    if (frame != nullptr)
    {
      INuiFrameTexture *texture = frame->pFrameTexture;
      NUI_LOCKED_RECT locked_rect;

      texture->LockRect(0, &locked_rect, nullptr, 0);
      if (locked_rect.Pitch != 0)
      {
        callback(lak::vec2i_t(locked_rect.Pitch / element_size,
                              locked_rect.size / locked_rect.Pitch),
                 locked_rect.pBits);
      }
      else
      {
        ERROR("Buffer length of received texture is bogus");
        return false;
      }
      texture->UnlockRect(0);

      NuiImageStreamReleaseFrame(stream, frame);
    }
    else
    {
      ERROR("No frame");
      return false;
    }

    return true;
  };

  /* --- Test Nui streams --- */

  if (!read_frame(
        colour_stream, 4, [&](lak::vec2i_t size, const byte *frame) {}))
  {
    ERROR("Reading colour frame failed");
    NuiShutdown();
    return 1;
  }

  if (!read_frame(
        depth_stream, 2, [&](lak::vec2i_t size, const byte *frame) {}))
  {
    ERROR("Reading depth frame failed");
    NuiShutdown();
    return 1;
  }

  WSADATA wsaData;
  SOCKET connect_socket = INVALID_SOCKET;

  if (auto err = WSAStartup(MAKEWORD(2, 2), &wsaData); err != 0)
  {
    ERROR("WSAStartup failed (0x" << err << ")");
    return 1;
  }

  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // struct addrinfo *result;

  // if (auto err = getaddrinfo("localhost", "13269", &hints, &result); err !=
  // 0)
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
    ERROR("socket failed (" << last_wsa_error() << ")");
    WSACleanup();
    return 1;
  }

  sockaddr_in addr;
  ZeroMemory(&addr, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port        = htons(13269);
  if (addr.sin_addr.s_addr == -1)
  {
    ERROR("Failed to get address");
    closesocket(connect_socket);
    WSACleanup();
    return 1;
  }
  if (auto err = connect(connect_socket, (SOCKADDR *)&addr, sizeof(addr));
      err == SOCKET_ERROR)
  {
    ERROR("Failed to connect to socket (" << last_wsa_error() << ")");
    closesocket(connect_socket);
    connect_socket = INVALID_SOCKET;
  }

  if (connect_socket == INVALID_SOCKET)
  {
    ERROR("Unable to connect to server");
    WSACleanup();
    return 1;
  }

  auto send_frame = [&connect_socket](lak::vec2i_t size,
                                      size_t element_size,
                                      const byte *buffer) -> bool {
    std::vector<char> buf;
    buf.resize(sizeof(size.x) + sizeof(size.y) +
               (size.x * size.y * element_size));
    memcpy(buf.data(), &size.x, sizeof(size.x));
    memcpy(buf.data() + sizeof(size.x), &size.y, sizeof(size.y));
    memcpy(buf.data() + sizeof(size.x) + sizeof(size.y),
           buffer,
           size.x * size.y * element_size);
    auto err = send(connect_socket, buf.data(), (int)buf.size(), 0);
    if (err == SOCKET_ERROR)
    {
      ERROR("send failed (" << last_wsa_error() << ")");
      return false;
    }
    return true;
  };

  CHECKPOINT();

  for (bool running = true; running;)
  {
    if (!read_frame(
          colour_stream, 4, [&](lak::vec2i_t size, const byte *frame) {
            if (!send_frame(size, 4, frame))
            {
              ERROR("Aborting");
              running = false;
            }
          }))
    {
      ERROR("Reading colour frame failed");
      break;
    }

    if (!read_frame(
          depth_stream, 2, [&](lak::vec2i_t size, const byte *frame) {
            if (!send_frame(size, 2, frame))
            {
              ERROR("Aborting");
              running = false;
            }
          }))
    {
      ERROR("Reading depth frame failed");
      break;
    }
  }

  CHECKPOINT();

  NuiShutdown();

  auto err = shutdown(connect_socket, SD_SEND | SD_RECEIVE);
  closesocket(connect_socket);
  WSACleanup();
  if (err == SOCKET_ERROR)
  {
    ERROR("shutdown failed (" << last_wsa_error() << ")");
    return 1;
  }
}

#include <strconv/strconv.cpp>

#include "debug.cpp"