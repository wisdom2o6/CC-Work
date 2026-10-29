#pragma once
#include <windows.h>
#include "GameLogic.h"
#include "Camera.h"

class InputManager {
public:
    static bool isDraggingMouse;
    static int dragButton; // 0 = Left, 1 = Right, 2 = Middle
    static int lastMouseX;
    static int lastMouseY;

    // Quản lý DAS (Delayed Auto Shift) & ARR (Auto Repeat Rate) cho chuyển động mượt mà
    static int holdForward;
    static int holdRight;
    static bool holdSoftDrop;
    static float dasTimer;
    static float arrTimer;
    static float softDropTimer;

    // Cập nhật lặp phím DAS/ARR mỗi frame
    static void update(float dt, GameLogic& game, const Camera& camera);

    static void handleKeyDown(HWND hwnd, WPARAM wParam, GameLogic& game, Camera& camera);
    static void handleKeyUp(HWND hwnd, WPARAM wParam);
    static void handleMouseDown(HWND hwnd, int x, int y, int button, GameLogic& game, Camera& camera);
    static void handleMouseUp(HWND hwnd, int x, int y, int button);
    static void handleMouseMove(HWND hwnd, int x, int y, Camera& camera);
    static void handleMouseWheel(HWND hwnd, short delta, Camera& camera);

    static void handleCameraRelativeMove(int inputForward, int inputRight, GameLogic& game, const Camera& camera);
};
