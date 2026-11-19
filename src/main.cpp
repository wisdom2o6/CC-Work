#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <chrono>

#include "Config.h"
#include "Types.h"
#include "BlockLogic.h"
#include "GameLogic.h"
#include "Camera.h"
#include "Renderer.h"
#include "FontRenderer.h"
#include "Audio.h"
#include "Input.h"

// Biến toàn cục chính
static GameLogic g_game;
static Camera g_camera;
static Renderer g_renderer;
static HDC g_hDC = NULL;
static HGLRC g_hRC = NULL;
static bool g_running = true;

// Hàm thiết lập Pixel Format cho OpenGL
static bool SetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,                           // 32-bit color
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                           // 24-bit depth buffer
        8,                            // 8-bit stencil buffer
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (!pixelFormat) return false;
    return SetPixelFormat(hdc, pixelFormat, &pfd);
}

// Window Procedure xử lý sự kiện Win32
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            g_renderer.reshape(width, height);
            return 0;
        }

        case WM_KEYDOWN:
            InputManager::handleKeyDown(hwnd, wParam, g_game, g_camera);
            return 0;

        case WM_KEYUP:
            InputManager::handleKeyUp(hwnd, wParam);
            return 0;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int btn = (msg == WM_LBUTTONDOWN) ? 0 : 1;
            SetCapture(hwnd);
            InputManager::handleMouseDown(hwnd, x, y, btn, g_game, g_camera);
            return 0;
        }

        case WM_LBUTTONUP:
        case WM_RBUTTONUP: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int btn = (msg == WM_LBUTTONUP) ? 0 : 1;
            ReleaseCapture();
            InputManager::handleMouseUp(hwnd, x, y, btn);
            return 0;
        }

        case WM_MOUSEMOVE: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            InputManager::handleMouseMove(hwnd, x, y, g_camera);
            return 0;
        }

        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            InputManager::handleMouseWheel(hwnd, delta, g_camera);
            return 0;
        }

        case WM_CLOSE:
            g_running = false;
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Khởi tạo DPI awareness
    SetProcessDPIAware();

    // Khởi tạo bộ sinh số ngẫu nhiên
    srand((unsigned int)time(NULL));

    // Khởi tạo âm thanh
    AudioManager::init();
    g_game.onPlaySound = AudioManager::playSound;

    // Đăng ký lớp cửa sổ
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "Tetris3D_WindowClass";
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Không thể đăng ký lớp cửa sổ Win32!", "Lỗi", MB_ICONERROR);
        return 1;
    }

    // Tính kích thước cửa sổ để vùng client đạt đúng Config::WINDOW_WIDTH x Config::WINDOW_HEIGHT
    RECT wr = { 0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    int winWidth = wr.right - wr.left;
    int winHeight = wr.bottom - wr.top;

    int posX = (GetSystemMetrics(SM_CXSCREEN) - winWidth) / 2;
    int posY = (GetSystemMetrics(SM_CYSCREEN) - winHeight) / 2;

    HWND hwnd = CreateWindowEx(
        0,
        "Tetris3D_WindowClass",
        Config::WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        posX, posY, winWidth, winHeight,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "Không thể tạo cửa sổ Win32!", "Lỗi", MB_ICONERROR);
        return 1;
    }

    // Thiết lập OpenGL Context
    g_hDC = GetDC(hwnd);
    if (!SetupPixelFormat(g_hDC)) {
        MessageBox(hwnd, "Không thể thiết lập Pixel Format cho OpenGL!", "Lỗi", MB_ICONERROR);
        return 1;
    }

    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC || !wglMakeCurrent(g_hDC, g_hRC)) {
        MessageBox(hwnd, "Không thể tạo OpenGL Rendering Context!", "Lỗi", MB_ICONERROR);
        return 1;
    }

    // Khởi tạo đồ họa Renderer và FontRenderer (Segoe UI tiếng Việt sắc nét)
    g_renderer.initGL(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
    FontRenderer::init(g_hDC, L"Segoe UI", 36);

    // Vòng lặp chính Game Loop với bộ đo thời gian hiệu năng cao
    LARGE_INTEGER freq, lastTime, curTime;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&lastTime);

    MSG msg = {};
    while (g_running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!g_running) break;

        // Tính delta time (dt)
        QueryPerformanceCounter(&curTime);
        float dt = (float)(curTime.QuadPart - lastTime.QuadPart) / (float)freq.QuadPart;
        lastTime = curTime;

        // Giới hạn dt tối đa để tránh giật hình khi chuyển cửa sổ
        if (dt > 0.1f) dt = 0.1f;

        // 1. Cập nhật input DAS/ARR, camera và logic
        InputManager::update(dt, g_game, g_camera);
        g_camera.update(dt);
        g_game.update(dt);

        // 2. Vẽ khung hình
        g_renderer.render(g_game, g_camera);

        // 3. Đổi bộ đệm Double Buffer
        SwapBuffers(g_hDC);

        // Nghỉ 1ms để giảm tải CPU
        Sleep(1);
    }

    // Dọn dẹp tài nguyên
    FontRenderer::cleanup();

    if (g_hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }
    if (g_hDC) {
        ReleaseDC(hwnd, g_hDC);
        g_hDC = NULL;
    }
    AudioManager::cleanup();

    return 0;
}
