#include "Input.h"
#include "Config.h"
#include <cmath>

bool InputManager::isDraggingMouse = false;
int InputManager::dragButton = 0;
int InputManager::lastMouseX = 0;
int InputManager::lastMouseY = 0;

int InputManager::holdForward = 0;
int InputManager::holdRight = 0;
bool InputManager::holdSoftDrop = false;
float InputManager::dasTimer = 0.0f;
float InputManager::arrTimer = 0.0f;
float InputManager::softDropTimer = 0.0f;

void InputManager::update(float dt, GameLogic& game, const Camera& camera) {
    if (game.state != GameState::PLAYING) return;

    // Cơ chế DAS (Delayed Auto Shift) & ARR (Auto Repeat Rate)
    const float DAS_DELAY = 0.16f; // Sau 160ms giữ phím sẽ bắt đầu tự động lặp
    const float ARR_INTERVAL = 0.045f; // Tần suất dịch ô 45ms mỗi bước

    if (holdForward != 0 || holdRight != 0) {
        dasTimer += dt;
        if (dasTimer >= DAS_DELAY) {
            arrTimer += dt;
            if (arrTimer >= ARR_INTERVAL) {
                arrTimer = 0.0f;
                handleCameraRelativeMove(holdForward, holdRight, game, camera);
            }
        }
    } else {
        dasTimer = 0.0f;
        arrTimer = 0.0f;
    }

    if (holdSoftDrop) {
        softDropTimer += dt;
        if (softDropTimer >= Config::SOFT_DROP_INTERVAL) {
            softDropTimer = 0.0f;
            game.softDrop();
        }
    } else {
        softDropTimer = 0.0f;
    }
}

void InputManager::handleCameraRelativeMove(int inputForward, int inputRight, GameLogic& game, const Camera& camera) {
    // Chuẩn hóa góc quay camera quanh trục Y (0..360)
    float y = std::fmod(camera.yaw, 360.0f);
    if (y < 0.0f) y += 360.0f;

    // Xác định góc nhìn chính gần nhất (4 hướng: Bắc, Đông, Nam, Tây)
    // Để khi người chơi nhấn W luôn đi xa mắt, S đi gần mắt, A sang trái màn hình, D sang phải màn hình
    int dx = 0, dz = 0;

    if (y >= 315.0f || y < 45.0f) {
        // Camera nhìn từ phía Nam (+Z) hướng về Bắc (-Z)
        dz -= inputForward;
        dx += inputRight;
    } else if (y >= 45.0f && y < 135.0f) {
        // Camera nhìn từ phía Đông (+X) hướng về Tây (-X)
        dx -= inputForward;
        dz -= inputRight;
    } else if (y >= 135.0f && y < 225.0f) {
        // Camera nhìn từ phía Bắc (-Z) hướng về Nam (+Z)
        dz += inputForward;
        dx -= inputRight;
    } else {
        // Camera nhìn từ phía Tây (-X) hướng về Đông (+X)
        dx += inputForward;
        dz += inputRight;
    }

    game.moveActiveBlock(dx, dz);
}

void InputManager::handleKeyDown(HWND hwnd, WPARAM wParam, GameLogic& game, Camera& camera) {
    switch (wParam) {
        case VK_SPACE:
            if (game.state == GameState::START_SCREEN ||
                game.state == GameState::GAME_OVER_TIMEOUT ||
                game.state == GameState::GAME_OVER_HEIGHT) {
                game.resetGame();
            } else if (game.state == GameState::PLAYING) {
                game.hardDrop(); // Thả rơi tức thì (Hard Drop)
            }
            break;

        case VK_SHIFT:
        case 'X': // Soft drop (Rơi nhanh)
            if (game.state == GameState::PLAYING) {
                holdSoftDrop = true;
                softDropTimer = 0.0f;
                game.softDrop();
            }
            break;

        // Xoay khối quanh 3 trục
        case 'R':
            if (game.state == GameState::GAME_OVER_TIMEOUT ||
                game.state == GameState::GAME_OVER_HEIGHT) {
                game.resetGame();
            } else if (game.state == GameState::PLAYING) {
                game.rotateActiveBlock(RotationAxis::X, true); // Xoay dọc trục X
            }
            break;

        case 'F':
            if (game.state == GameState::PLAYING) {
                game.rotateActiveBlock(RotationAxis::X, false);
            }
            break;

        case 'Q':
            if (game.state == GameState::PLAYING) {
                game.rotateActiveBlock(RotationAxis::Y, false); // Xoay ngang trục Y
            }
            break;

        case 'E':
            if (game.state == GameState::PLAYING) {
                game.rotateActiveBlock(RotationAxis::Y, true);
            }
            break;

        case 'Z':
            if (game.state == GameState::PLAYING) {
                game.rotateActiveBlock(RotationAxis::Z, false); // Xoay nghiêng trục Z
            }
            break;

        case 'C':
            if (game.state == GameState::PLAYING) {
                game.rotateActiveBlock(RotationAxis::Z, true);
            }
            break;

        // Di chuyển trên mặt sàn 5x5 (hỗ trợ giữ phím mượt mà với DAS/ARR)
        case 'W':
        case VK_UP:
            holdForward = 1;
            dasTimer = 0.0f;
            arrTimer = 0.0f;
            handleCameraRelativeMove(1, 0, game, camera);
            break;

        case 'S':
        case VK_DOWN:
            holdForward = -1;
            dasTimer = 0.0f;
            arrTimer = 0.0f;
            handleCameraRelativeMove(-1, 0, game, camera);
            break;

        case 'A':
        case VK_LEFT:
            holdRight = -1;
            dasTimer = 0.0f;
            arrTimer = 0.0f;
            handleCameraRelativeMove(0, -1, game, camera);
            break;

        case 'D':
        case VK_RIGHT:
            holdRight = 1;
            dasTimer = 0.0f;
            arrTimer = 0.0f;
            handleCameraRelativeMove(0, 1, game, camera);
            break;

        // Phím xoay nhanh góc camera 90 độ ([ hoặc phím phẩy ',' / ] hoặc phím chấm '.')
        case VK_OEM_4: // '['
        case VK_OEM_COMMA:
            camera.rotateYawBy(-90.0f);
            break;

        case VK_OEM_6: // ']'
        case VK_OEM_PERIOD:
            camera.rotateYawBy(90.0f);
            break;

        // Chọn các khối trong khay 3 khối
        case '1':
        case VK_NUMPAD1:
            game.selectRackSlot(0);
            break;

        case '2':
        case VK_NUMPAD2:
            game.selectRackSlot(1);
            break;

        case '3':
        case VK_NUMPAD3:
            game.selectRackSlot(2);
            break;

        case 'P': // Tạm dừng
            if (game.state == GameState::PLAYING) {
                game.state = GameState::PAUSED;
            } else if (game.state == GameState::PAUSED) {
                game.state = GameState::PLAYING;
            }
            break;

        case 'V': // Đổi góc camera định sẵn (Isometric, TopDown, Front)
            {
                static int viewIdx = 0;
                viewIdx = (viewIdx + 1) % 3;
                if (viewIdx == 0) camera.setPresetIsometric();
                else if (viewIdx == 1) camera.setPresetTopDown();
                else camera.setPresetFront();
            }
            break;

        case VK_ESCAPE:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
    }
}

void InputManager::handleKeyUp(HWND hwnd, WPARAM wParam) {
    switch (wParam) {
        case 'W':
        case VK_UP:
            if (holdForward == 1) holdForward = 0;
            break;

        case 'S':
        case VK_DOWN:
            if (holdForward == -1) holdForward = 0;
            break;

        case 'A':
        case VK_LEFT:
            if (holdRight == -1) holdRight = 0;
            break;

        case 'D':
        case VK_RIGHT:
            if (holdRight == 1) holdRight = 0;
            break;

        case VK_SHIFT:
        case 'X':
            holdSoftDrop = false;
            break;
    }
}

void InputManager::handleMouseDown(HWND hwnd, int x, int y, int button, GameLogic& game, Camera& camera) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int winW = rc.right - rc.left;
    int winH = rc.bottom - rc.top;

    // Chuột phải (button == 1) luôn dùng riêng để xoay góc nhìn camera 360 độ mà không lo bấm nhầm UI
    if (button == 1) {
        isDraggingMouse = true;
        dragButton = 1;
        lastMouseX = x;
        lastMouseY = y;
        return;
    }

    // Xử lý click chuột trái (button == 0) vào các nút giao diện (HUD Buttons)
    if (game.state == GameState::START_SCREEN) {
        float sW = 700.0f, sH = 430.0f;
        float sX = (winW - sW) * 0.5f, sY = (winH - sH) * 0.5f;
        float btnW = 340.0f, btnH = 55.0f;
        float btnX = sX + (sW - btnW) * 0.5f, btnY = sY + 340.0f;
        if (x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH) {
            game.resetGame();
            return;
        }
    } else if (game.state == GameState::GAME_OVER_TIMEOUT || game.state == GameState::GAME_OVER_HEIGHT) {
        float mW = 620.0f, mH = 370.0f;
        float mX = (winW - mW) * 0.5f, mY = (winH - mH) * 0.5f;
        float btnW = 360.0f, btnH = 54.0f;
        float btnX = mX + (mW - btnW) * 0.5f, btnY = mY + 280.0f;
        if (x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH) {
            game.resetGame();
            return;
        }
    } else if (game.state == GameState::PLAYING) {
        // 1. Click vào các ô trong khay 3 khối
        float rackW = 480.0f, rackH = 95.0f;
        float rackX = (winW - rackW) * 0.5f, rackY = winH - rackH - 15.0f;
        float slotW = 140.0f, slotH = 60.0f, slotGap = 15.0f;
        float startX = rackX + 15.0f, startY = rackY + 26.0f;

        for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
            float sx = startX + i * (slotW + slotGap);
            float sy = startY;
            if (x >= sx && x <= sx + slotW && y >= sy && y <= sy + slotH) {
                game.selectRackSlot(i);
                return;
            }
        }

        // 2. Click các nút thao tác camera nhanh ở góc dưới bên phải
        float camBtnsX = winW - 365.0f;
        float camBtnsY = winH - 56.0f;

        // Nút xoay trái 90°
        if (x >= camBtnsX && x <= camBtnsX + 110.0f && y >= camBtnsY && y <= camBtnsY + 40.0f) {
            camera.rotateYawBy(-90.0f);
            return;
        }
        // Nút xoay phải 90°
        if (x >= camBtnsX + 118.0f && x <= camBtnsX + 228.0f && y >= camBtnsY && y <= camBtnsY + 40.0f) {
            camera.rotateYawBy(90.0f);
            return;
        }
        // Nút đổi góc nhìn Camera (V)
        if (x >= camBtnsX + 236.0f && x <= camBtnsX + 350.0f && y >= camBtnsY && y <= camBtnsY + 40.0f) {
            static int viewIdx = 0;
            viewIdx = (viewIdx + 1) % 3;
            if (viewIdx == 0) camera.setPresetIsometric();
            else if (viewIdx == 1) camera.setPresetTopDown();
            else camera.setPresetFront();
            return;
        }
    }

    // Nếu không click trúng nút giao diện nào, kích hoạt kéo chuột xoay 3D
    isDraggingMouse = true;
    dragButton = 0;
    lastMouseX = x;
    lastMouseY = y;
}

void InputManager::handleMouseUp(HWND hwnd, int x, int y, int button) {
    if (button == dragButton) {
        isDraggingMouse = false;
    }
}

void InputManager::handleMouseMove(HWND hwnd, int x, int y, Camera& camera) {
    if (isDraggingMouse) {
        float dx = (float)(x - lastMouseX);
        float dy = (float)(y - lastMouseY);
        // Xoay góc nhìn camera quanh trục thẳng đứng ở giữa sàn
        camera.onMouseDrag(dx, dy);
        lastMouseX = x;
        lastMouseY = y;
    }
}

void InputManager::handleMouseWheel(HWND hwnd, short delta, Camera& camera) {
    float wheelSteps = (float)delta / 120.0f;
    camera.onMouseWheel(wheelSteps);
}
