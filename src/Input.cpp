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

    // Cơ chế DAS & ARR cho di chuyển mượt mà
    const float DAS_DELAY = 0.16f;
    const float ARR_INTERVAL = 0.045f;

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
    float y = std::fmod(camera.yaw, 360.0f);
    if (y < 0.0f) y += 360.0f;

    int dx = 0, dz = 0;

    if (y >= 315.0f || y < 45.0f) {
        dz -= inputForward;
        dx += inputRight;
    } else if (y >= 45.0f && y < 135.0f) {
        dx -= inputForward;
        dz -= inputRight;
    } else if (y >= 135.0f && y < 225.0f) {
        dz += inputForward;
        dx -= inputRight;
    } else {
        dx += inputForward;
        dz += inputRight;
    }

    game.moveActiveBlock(dx, dz);
}

void InputManager::handleKeyDown(HWND hwnd, WPARAM wParam, GameLogic& game, Camera& camera) {
    // Nếu Menu Cheat đang mở, ưu tiên xử lý các phím số 1-9 và phím đóng K / ESC
    if (game.isCheatMenuOpen) {
        switch (wParam) {
            case 'K':
            case VK_ESCAPE:
                game.isCheatMenuOpen = false;
                break;
            case '1':
            case VK_NUMPAD1:
                game.cheatStack10GameOver();
                break;
            case '2':
            case VK_NUMPAD2:
                game.cheatStack9Tiers();
                break;
            case '3':
            case VK_NUMPAD3:
                game.cheatClearLayerInstant();
                break;
            case '4':
            case VK_NUMPAD4:
                game.cheatFillLayerAlmost();
                break;
            case '5':
            case VK_NUMPAD5:
                game.cheatCycleDifficulty();
                break;
            case '6':
            case VK_NUMPAD6:
                game.cheatTrigger15Obstacle();
                break;
            case '7':
            case VK_NUMPAD7:
                game.cheatSetTime5Seconds();
                break;
            case '8':
            case VK_NUMPAD8:
                game.cheatAddScore();
                break;
            case '9':
            case VK_NUMPAD9:
                game.cheatClearAllGrid();
                break;
            default:
                break;
        }
        return;
    }

    switch (wParam) {
        case VK_SPACE:
        case VK_RETURN:
            if (game.state == GameState::START_SCREEN ||
                game.state == GameState::GAME_OVER_TIMEOUT ||
                game.state == GameState::GAME_OVER_HEIGHT) {
                game.resetGame();
            } else if (game.state == GameState::PLAYING) {
                game.hardDrop();
            }
            break;

        case VK_SHIFT:
        case 'X':
            if (game.state == GameState::PLAYING) {
                holdSoftDrop = true;
                softDropTimer = 0.0f;
                game.softDrop();
            }
            break;

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

        case 'W':
        case VK_UP:
            if (game.state == GameState::PLAYING) {
                holdForward = 1;
                dasTimer = 0.0f;
                arrTimer = 0.0f;
                handleCameraRelativeMove(1, 0, game, camera);
            }
            break;

        case 'S':
        case VK_DOWN:
            if (game.state == GameState::PLAYING) {
                holdForward = -1;
                dasTimer = 0.0f;
                arrTimer = 0.0f;
                handleCameraRelativeMove(-1, 0, game, camera);
            }
            break;

        case 'A':
        case VK_LEFT:
            if (game.state == GameState::PLAYING) {
                holdRight = -1;
                dasTimer = 0.0f;
                arrTimer = 0.0f;
                handleCameraRelativeMove(0, -1, game, camera);
            }
            break;

        case 'D':
        case VK_RIGHT:
            if (game.state == GameState::PLAYING) {
                holdRight = 1;
                dasTimer = 0.0f;
                arrTimer = 0.0f;
                handleCameraRelativeMove(0, 1, game, camera);
            }
            break;

        case VK_OEM_4: // '['
        case VK_OEM_COMMA:
            camera.rotateYawBy(-90.0f);
            break;

        case VK_OEM_6: // ']'
        case VK_OEM_PERIOD:
            camera.rotateYawBy(90.0f);
            break;

        // Phím 1, 2, 3: Chọn thời gian ở màn hình chính, hoặc chọn khay khối khi đang chơi
        case '1':
        case VK_NUMPAD1:
            if (game.state == GameState::START_SCREEN) {
                game.setTimeMode(TimeMode::MIN_3);
            } else if (game.state == GameState::PLAYING) {
                game.selectRackSlot(0);
            }
            break;

        case '2':
        case VK_NUMPAD2:
            if (game.state == GameState::START_SCREEN) {
                game.setTimeMode(TimeMode::MIN_15);
            } else if (game.state == GameState::PLAYING) {
                game.selectRackSlot(1);
            }
            break;

        case '3':
        case VK_NUMPAD3:
            if (game.state == GameState::START_SCREEN) {
                game.setTimeMode(TimeMode::ENDLESS);
            } else if (game.state == GameState::PLAYING) {
                game.selectRackSlot(2);
            }
            break;

        // Phím 4, 5, 6: Chọn độ khó ở màn hình chính
        case '4':
        case VK_NUMPAD4:
            if (game.state == GameState::START_SCREEN) {
                game.setDifficulty(Difficulty::EASY);
            }
            break;

        case '5':
        case VK_NUMPAD5:
            if (game.state == GameState::START_SCREEN) {
                game.setDifficulty(Difficulty::NORMAL);
            }
            break;

        case '6':
        case VK_NUMPAD6:
            if (game.state == GameState::START_SCREEN) {
                game.setDifficulty(Difficulty::HARD);
            }
            break;

        case 'K': // Bật / Tắt Menu Cheat & Test Demo nhanh
            game.toggleCheatMenu();
            break;

        case 'M': // Đổi chế độ / Quay về menu chính
            if (game.state == GameState::GAME_OVER_TIMEOUT ||
                game.state == GameState::GAME_OVER_HEIGHT ||
                game.state == GameState::PAUSED) {
                game.state = GameState::START_SCREEN;
            }
            break;

        case 'P':
            if (game.state == GameState::PLAYING) {
                game.state = GameState::PAUSED;
            } else if (game.state == GameState::PAUSED) {
                game.state = GameState::PLAYING;
            }
            break;

        case 'V':
            {
                static int viewIdx = 0;
                viewIdx = (viewIdx + 1) % 3;
                if (viewIdx == 0) camera.setPresetIsometric();
                else if (viewIdx == 1) camera.setPresetTopDown();
                else camera.setPresetFront();
            }
            break;

        case VK_ESCAPE:
            if (game.isCheatMenuOpen) {
                game.isCheatMenuOpen = false;
            } else {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
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

    // Chuột phải (button == 1) luôn dùng riêng để xoay góc nhìn camera 360 độ
    if (button == 1) {
        isDraggingMouse = true;
        dragButton = 1;
        lastMouseX = x;
        lastMouseY = y;
        return;
    }

    // Xử lý click chuột trái (button == 0)
    if (game.isCheatMenuOpen) {
        float mW = 720.0f;
        float mH = 500.0f;
        float mX = ((float)winW - mW) * 0.5f;
        float mY = ((float)winH - mH) * 0.5f;
        float colW = 315.0f;
        float rowH = 62.0f;
        float col1X = mX + 30.0f;
        float col2X = mX + mW - 30.0f - colW;
        float startY = mY + 80.0f;
        float gapY = 70.0f;

        // Cột trái [1..5]
        for (int i = 0; i < 5; ++i) {
            float by = startY + i * gapY;
            if (x >= col1X && x <= col1X + colW && y >= by && y <= by + rowH) {
                if (i == 0) game.cheatStack10GameOver();
                else if (i == 1) game.cheatStack9Tiers();
                else if (i == 2) game.cheatClearLayerInstant();
                else if (i == 3) game.cheatFillLayerAlmost();
                else if (i == 4) game.cheatCycleDifficulty();
                return;
            }
        }

        // Cột phải [6..9, K]
        for (int i = 0; i < 5; ++i) {
            float by = startY + i * gapY;
            if (x >= col2X && x <= col2X + colW && y >= by && y <= by + rowH) {
                if (i == 0) game.cheatTrigger15Obstacle();
                else if (i == 1) game.cheatSetTime5Seconds();
                else if (i == 2) game.cheatAddScore();
                else if (i == 3) game.cheatClearAllGrid();
                else if (i == 4) game.isCheatMenuOpen = false;
                return;
            }
        }

        // Click bên ngoài modal -> đóng menu
        if (x < mX || x > mX + mW || y < mY || y > mY + mH) {
            game.isCheatMenuOpen = false;
        }
        return;
    }

    if (game.state == GameState::START_SCREEN) {
        float sW = 740.0f, sH = 540.0f;
        float sX = (winW - sW) * 0.5f, sY = (winH - sH) * 0.5f;
        float startX = sX + 35.0f;
        float btnW = 210.0f, btnH = 46.0f, gap = 20.0f;
        float btnY_time = sY + 95.0f + 24.0f;

        // 1. Click các nút chọn Thời gian
        if (y >= btnY_time && y <= btnY_time + btnH) {
            if (x >= startX && x <= startX + btnW) {
                game.setTimeMode(TimeMode::MIN_3);
                return;
            }
            if (x >= startX + btnW + gap && x <= startX + (btnW + gap) + btnW) {
                game.setTimeMode(TimeMode::MIN_15);
                return;
            }
            if (x >= startX + (btnW + gap) * 2 && x <= startX + (btnW + gap) * 2 + btnW) {
                game.setTimeMode(TimeMode::ENDLESS);
                return;
            }
        }

        // 2. Click các nút chọn Độ khó
        float btnY_diff = btnY_time + btnH + 22.0f + 24.0f;
        float btnH_diff = 54.0f;
        if (y >= btnY_diff && y <= btnY_diff + btnH_diff) {
            if (x >= startX && x <= startX + btnW) {
                game.setDifficulty(Difficulty::EASY);
                return;
            }
            if (x >= startX + btnW + gap && x <= startX + (btnW + gap) + btnW) {
                game.setDifficulty(Difficulty::NORMAL);
                return;
            }
            if (x >= startX + (btnW + gap) * 2 && x <= startX + (btnW + gap) * 2 + btnW) {
                game.setDifficulty(Difficulty::HARD);
                return;
            }
        }

        // 3. Click nút Bắt đầu chơi
        float startBtnW = 400.0f, startBtnH = 52.0f;
        float startBtnX = sX + (sW - startBtnW) * 0.5f;
        float startBtnY = sY + sH - startBtnH - 24.0f;
        if (x >= startBtnX && x <= startBtnX + startBtnW && y >= startBtnY && y <= startBtnY + startBtnH) {
            game.resetGame();
            return;
        }
    } else if (game.state == GameState::GAME_OVER_TIMEOUT || game.state == GameState::GAME_OVER_HEIGHT) {
        float mW = 640.0f, mH = 410.0f;
        float mX = (winW - mW) * 0.5f, mY = (winH - mH) * 0.5f;
        float btnW = 260.0f, btnH = 50.0f;
        float btnY = mY + 310.0f;
        float btnX1 = mX + 45.0f;
        float btnX2 = mX + mW - 45.0f - btnW;

        if (y >= btnY && y <= btnY + btnH) {
            if (x >= btnX1 && x <= btnX1 + btnW) {
                game.resetGame(); // Chơi lại
                return;
            }
            if (x >= btnX2 && x <= btnX2 + btnW) {
                game.state = GameState::START_SCREEN; // Đổi chế độ
                return;
            }
        }
    } else if (game.state == GameState::PAUSED) {
        float pW = 480.0f, pH = 220.0f;
        float pX = (winW - pW) * 0.5f, pY = (winH - pH) * 0.5f;
        float btnW = 190.0f, btnH = 46.0f;
        float btnY = pY + 125.0f;

        if (y >= btnY && y <= btnY + btnH) {
            if (x >= pX + 35.0f && x <= pX + 35.0f + btnW) {
                game.state = GameState::PLAYING;
                return;
            }
            if (x >= pX + pW - 35.0f - btnW && x <= pX + pW - 35.0f) {
                game.state = GameState::START_SCREEN;
                return;
            }
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

        // 2. Click các nút thao tác camera nhanh góc dưới bên phải
        float camBtnsX = winW - 365.0f;
        float camBtnsY = winH - 56.0f;

        if (x >= camBtnsX && x <= camBtnsX + 110.0f && y >= camBtnsY && y <= camBtnsY + 40.0f) {
            camera.rotateYawBy(-90.0f);
            return;
        }
        if (x >= camBtnsX + 118.0f && x <= camBtnsX + 228.0f && y >= camBtnsY && y <= camBtnsY + 40.0f) {
            camera.rotateYawBy(90.0f);
            return;
        }
        if (x >= camBtnsX + 236.0f && x <= camBtnsX + 350.0f && y >= camBtnsY && y <= camBtnsY + 40.0f) {
            static int viewIdx = 0;
            viewIdx = (viewIdx + 1) % 3;
            if (viewIdx == 0) camera.setPresetIsometric();
            else if (viewIdx == 1) camera.setPresetTopDown();
            else camera.setPresetFront();
            return;
        }
    }

    // Nếu không click trúng nút UI, kéo chuột xoay 3D
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
        camera.onMouseDrag(dx, dy);
        lastMouseX = x;
        lastMouseY = y;
    }
}

void InputManager::handleMouseWheel(HWND hwnd, short delta, Camera& camera) {
    float wheelSteps = (float)delta / 120.0f;
    camera.onMouseWheel(wheelSteps);
}
