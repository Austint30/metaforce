#include "VISIRenderer.hpp"
#include <Windows.h>
#include <WinUser.h>
#include <Shlwapi.h>
#include <strsafe.h>
#include "athena/Global.hpp"
#include "logvisor/logvisor.hpp"
#include "../version.h"
#include <thread>

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* /*file*/, const char*, int /*line*/,
                      fmt::string_view fmt, fmt::format_args args) {
  AthenaLog.vreport(logvisor::Level(level), fmt, args);
}

static float s_Percent = 0.f;
static DWORD s_mainThreadId;
static void UpdatePercent(float percent) {
  s_Percent = percent;
  PostThreadMessage(s_mainThreadId, WM_USER + 1, 0, 0);
}

static const DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
static VISIRenderer* s_Renderer;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == WM_SIZING) {
    RECT& dragRect = reinterpret_cast<RECT&>(lParam);
    RECT tmpRect = dragRect;
    tmpRect.bottom = tmpRect.top + 512;
    tmpRect.right = tmpRect.left + 768;
    AdjustWindowRect(&tmpRect, dwStyle, FALSE);
    dragRect = tmpRect;
    return TRUE;
  } else if (uMsg == WM_CLOSE) {
    s_Renderer->Terminate();
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int wmain(int argc, const hecl::SystemChar** argv) {
  if (argc > 1 && !wcscmp(argv[1], L"--dlpackage")) {
    fmt::print(FMT_STRING("{}\n"), METAFORCE_DLPACKAGE);
    return 100;
  }

  logvisor::RegisterStandardExceptions();
  logvisor::RegisterConsoleLogger();
  atSetExceptionHandler(AthenaExc);
  VISIRenderer renderer(argc, argv);
  s_Renderer = &renderer;

  int instIdx = -1;
  if (argc > 3)
    instIdx = _wtoi(argv[3]);

  WNDCLASS wndClass = {CS_NOCLOSE, WindowProc, 0, 0, GetModuleHandle(nullptr), 0, 0, 0, 0, L"VISIGenWindow"};
  RegisterClassW(&wndClass);

  RECT clientRect = {0, 0, 768, 512};
  AdjustWindowRect(&clientRect, dwStyle, FALSE);

  int x = 0;
  int y = 0;
  if (instIdx != -1) {
    x = (instIdx & 1) != 0;
    y = (instIdx & 2) != 0;
  }

  HWND window = CreateWindowW(L"VISIGenWindow", L"VISIGen", dwStyle, x, y, clientRect.right - clientRect.left,
                              clientRect.bottom - clientRect.top, NULL, NULL, NULL, NULL);

  PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR),
                               1,
                               PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL, // Flags
                               PFD_TYPE_RGBA,                           // The kind of framebuffer. RGBA or palette.
                               32,                                      // Colordepth of the framebuffer.
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               24, // Number of bits for the depthbuffer
                               8,  // Number of bits for the stencilbuffer
                               0,  // Number of Aux buffers in the framebuffer.
                               PFD_MAIN_PLANE,
                               0,
                               0,
                               0,
                               0};

  HDC deviceContext = GetDC(window);
  int pf = ChoosePixelFormat(deviceContext, &pfd);
  SetPixelFormat(deviceContext, pf, &pfd);
  HGLRC glContext = wglCreateContext(deviceContext);
  ShowWindow(window, SW_SHOW);

  s_mainThreadId = GetCurrentThreadId();

  /* Spawn client thread */
  std::thread clientThread([&]() {
    wglMakeCurrent(deviceContext, glContext);
    renderer.Run(UpdatePercent);
    PostThreadMessage(s_mainThreadId, WM_USER, 0, 0);
  });

  /* Pump messages */
  MSG msg = {0};
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!msg.hwnd) {
      /* PostThreadMessage events */
      switch (msg.message) {
      case WM_USER:
        /* Quit message from client thread */
        PostQuitMessage(0);
        continue;
      case WM_USER + 1: {
        /* Update window title from client thread */
        std::wstring title = fmt::format(FMT_STRING(L"VISIGen [{:g}%]"), s_Percent * 100.f);
        SetWindowTextW(window, title.c_str());
        continue;
      }
      default:
        break;
      }
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  renderer.Terminate();
  if (clientThread.joinable())
    clientThread.join();

  wglDeleteContext(glContext);

  return renderer.ReturnVal();
}
