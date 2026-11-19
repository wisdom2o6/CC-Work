#include "Renderer.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>
#include <cstdio>
#include <algorithm>

Renderer::Renderer() : windowWidth(Config::WINDOW_WIDTH), windowHeight(Config::WINDOW_HEIGHT) {}

void Renderer::initGL(int width, int height) {
    windowWidth = width;
    windowHeight = height;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Thiết lập nguồn sáng dịu mắt (không gây chói gắt, không specular cháy sáng)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Light 0: Nguồn sáng chính phía trên phải - sắc thái ánh sáng tự nhiên dịu mát
    GLfloat light0_pos[] = { 10.0f, 18.0f, 14.0f, 1.0f };
    GLfloat light0_ambient[] = { 0.32f, 0.35f, 0.42f, 1.0f };
    GLfloat light0_diffuse[] = { 0.72f, 0.76f, 0.82f, 1.0f };
    GLfloat light0_specular[] = { 0.20f, 0.22f, 0.25f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    // Light 1: Nguồn sáng phụ hắt nhẹ từ phía dưới đối diện
    GLfloat light1_pos[] = { -10.0f, -4.0f, -10.0f, 1.0f };
    GLfloat light1_ambient[] = { 0.14f, 0.16f, 0.20f, 1.0f };
    GLfloat light1_diffuse[] = { 0.30f, 0.34f, 0.40f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);

    // Độ bóng vật liệu mờ satin (Matte acrylic), tránh bóng nhẫy chói mắt
    GLfloat mat_specular[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    GLfloat mat_shininess[] = { 16.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // Màu nền Slate Dark Navy sang trọng, thư giãn mắt
    glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
}

void Renderer::reshape(int width, int height) {
    windowWidth = width;
    windowHeight = height > 0 ? height : 1;
    glViewport(0, 0, windowWidth, windowHeight);
}

void Renderer::render(const GameLogic& game, const Camera& camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Vẽ không gian 3D
    renderScene3D(game, camera);

    // 2. Vẽ giao diện HUD 2D
    renderHUD2D(game, camera);
}

void Renderer::renderScene3D(const GameLogic& game, const Camera& camera) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 120.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.applyView();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // Trục trung tâm
    renderCenterAxisMarker();

    // Sàn lưới 5x5
    renderFloorGrid();

    // Lồng tháp kính 5x5x10
    renderGlassTower(game.getCurrentMaxHeight());

    // Bóng chiếu xuống mặt sàn
    renderFloorFootprint(game);

    // Các khối đã cố định trên sàn
    renderPlacedBlocks(game);

    // Bóng đổ (Ghost Block)
    renderGhostBlock(game);

    // Khối đang rơi
    renderActiveBlock(game);
}

void Renderer::renderCenterAxisMarker() {
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
    glColor4f(0.35f, 0.65f, 0.85f, 0.18f);
    glVertex3f(2.5f, -0.4f, 2.5f);
    glVertex3f(2.5f, 10.4f, 2.5f);
    glEnd();

    glEnable(GL_LIGHTING);
}

void Renderer::renderFloorGrid() {
    glDisable(GL_LIGHTING);

    // Bàn cờ sàn 5x5 với gam màu xanh thẫm slate nhã nhặn
    for (int x = 0; x < Config::GRID_X; ++x) {
        for (int z = 0; z < Config::GRID_Z; ++z) {
            float b = ((x + z) % 2 == 0) ? 0.10f : 0.14f;
            glColor4f(b * 0.9f, b * 1.1f, b * 1.5f, 0.95f);

            glBegin(GL_QUADS);
            glVertex3f((float)x, 0.0f, (float)z);
            glVertex3f((float)x + 1.0f, 0.0f, (float)z);
            glVertex3f((float)x + 1.0f, 0.0f, (float)z + 1.0f);
            glVertex3f((float)x, 0.0f, (float)z + 1.0f);
            glEnd();
        }
    }

    // Các đường lưới sàn màu xanh slate mềm
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glColor4f(0.28f, 0.52f, 0.68f, 0.40f);
    for (int i = 0; i <= Config::GRID_X; ++i) {
        glVertex3f((float)i, 0.005f, 0.0f);
        glVertex3f((float)i, 0.005f, (float)Config::GRID_Z);
    }
    for (int j = 0; j <= Config::GRID_Z; ++j) {
        glVertex3f(0.0f, 0.005f, (float)j);
        glVertex3f((float)Config::GRID_X, 0.005f, (float)j);
    }
    glEnd();

    // Viền sàn
    glLineWidth(2.2f);
    glBegin(GL_LINE_LOOP);
    glColor4f(0.30f, 0.70f, 0.80f, 0.65f);
    glVertex3f(0.0f, 0.01f, 0.0f);
    glVertex3f((float)Config::GRID_X, 0.01f, 0.0f);
    glVertex3f((float)Config::GRID_X, 0.01f, (float)Config::GRID_Z);
    glVertex3f(0.0f, 0.01f, (float)Config::GRID_Z);
    glEnd();

    glEnable(GL_LIGHTING);
}

void Renderer::renderGlassTower(int currentMaxHeight) {
    glDisable(GL_LIGHTING);

    float gx = (float)Config::GRID_X;
    float gz = (float)Config::GRID_Z;
    float maxH = (float)Config::MAX_HEIGHT;

    // 1. Bốn cột trụ góc lồng tháp
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor4f(0.32f, 0.62f, 0.78f, 0.45f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, maxH, 0.0f);
    glVertex3f(gx, 0.0f, 0.0f);   glVertex3f(gx, maxH, 0.0f);
    glVertex3f(gx, 0.0f, gz);     glVertex3f(gx, maxH, gz);
    glVertex3f(0.0f, 0.0f, gz);   glVertex3f(0.0f, maxH, gz);
    glEnd();

    // 2. Các vòng chia tầng từ 1 đến 9
    glLineWidth(1.0f);
    for (int y = 1; y < Config::MAX_HEIGHT; ++y) {
        float fy = (float)y;
        glBegin(GL_LINE_LOOP);
        if (y <= currentMaxHeight) {
            glColor4f(0.35f, 0.65f, 0.85f, 0.32f);
        } else {
            glColor4f(0.25f, 0.32f, 0.42f, 0.16f);
        }
        glVertex3f(0.0f, fy, 0.0f);
        glVertex3f(gx, fy, 0.0f);
        glVertex3f(gx, fy, gz);
        glVertex3f(0.0f, fy, gz);
        glEnd();
    }

    // 3. VÒNG CẢNH BÁO TẦNG 10 (ĐỘ CAO TỐI ĐA - MÀU CORAL DỊU MẮT)
    glLineWidth(2.2f);
    glBegin(GL_LINE_LOOP);
    glColor4f(0.92f, 0.35f, 0.38f, 0.75f);
    glVertex3f(0.0f, maxH, 0.0f);
    glVertex3f(gx, maxH, 0.0f);
    glVertex3f(gx, maxH, gz);
    glVertex3f(0.0f, maxH, gz);
    glEnd();

    // Vách kính mờ
    glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
    glColor4f(0.08f, 0.16f, 0.28f, 0.04f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, maxH, 0.0f);
    glVertex3f(0.0f, maxH, gz);   glVertex3f(0.0f, 0.0f, gz);
    glVertex3f(gx, 0.0f, 0.0f);   glVertex3f(gx, maxH, 0.0f);
    glVertex3f(gx, maxH, gz);     glVertex3f(gx, 0.0f, gz);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(gx, 0.0f, 0.0f);
    glVertex3f(gx, maxH, 0.0f);   glVertex3f(0.0f, maxH, 0.0f);
    glEnd();
    glDepthMask(GL_TRUE);

    glEnable(GL_LIGHTING);
}

void Renderer::renderFloorFootprint(const GameLogic& game) {
    if (game.state != GameState::PLAYING) return;
    const Block* active = game.rack.getActiveBlock();
    if (!active) return;

    std::vector<Vec3i> voxels = active->getWorldVoxels();
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);

    for (const auto& v : voxels) {
        if (v.x < 0 || v.x >= Config::GRID_X || v.z < 0 || v.z >= Config::GRID_Z) continue;

        // Vùng chiếu sáng nhẹ dưới sàn
        glColor4f(active->color.r, active->color.g, active->color.b, 0.16f);
        glBegin(GL_QUADS);
        glVertex3f((float)v.x + 0.06f, 0.015f, (float)v.z + 0.06f);
        glVertex3f((float)v.x + 0.94f, 0.015f, (float)v.z + 0.06f);
        glVertex3f((float)v.x + 0.94f, 0.015f, (float)v.z + 0.94f);
        glVertex3f((float)v.x + 0.06f, 0.015f, (float)v.z + 0.94f);
        glEnd();

        // Viền thanh thoát
        glLineWidth(1.6f);
        glColor4f(active->color.r, active->color.g, active->color.b, 0.55f);
        glBegin(GL_LINE_LOOP);
        glVertex3f((float)v.x + 0.06f, 0.016f, (float)v.z + 0.06f);
        glVertex3f((float)v.x + 0.94f, 0.016f, (float)v.z + 0.06f);
        glVertex3f((float)v.x + 0.94f, 0.016f, (float)v.z + 0.94f);
        glVertex3f((float)v.x + 0.06f, 0.016f, (float)v.z + 0.94f);
        glEnd();
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void Renderer::drawStyledCube(float x, float y, float z, const Color4f& color, float scale, bool wireframeOnly) {
    float offset = (1.0f - scale) * 0.5f;
    float x0 = x + offset, x1 = x + 1.0f - offset;
    float y0 = y + offset, y1 = y + 1.0f - offset;
    float z0 = z + offset, z1 = z + 1.0f - offset;

    if (!wireframeOnly) {
        glEnable(GL_LIGHTING);
        glColor4f(color.r, color.g, color.b, color.a);

        glBegin(GL_QUADS);
        // Mặt trên (+Y)
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x0, y1, z0); glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1); glVertex3f(x1, y1, z0);

        // Mặt dưới (-Y)
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);

        // Mặt trước (+Z)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);

        // Mặt sau (-Z)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0); glVertex3f(x1, y0, z0);

        // Mặt trái (-X)
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(x0, y0, z0); glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1); glVertex3f(x0, y1, z0);

        // Mặt phải (+X)
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1); glVertex3f(x1, y0, z1);
        glEnd();
    }

    // Viền cạnh lập phương thanh lịch
    glDisable(GL_LIGHTING);
    glLineWidth(1.4f);

    if (wireframeOnly) {
        glColor4f(color.r, color.g, color.b, 0.70f);
    } else {
        glColor4f(color.r * 1.15f, color.g * 1.15f, color.b * 1.15f, 0.85f);
    }

    glBegin(GL_LINE_LOOP);
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0);
    glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1);
    glEnd();

    glEnable(GL_LIGHTING);
}

void Renderer::renderPlacedBlocks(const GameLogic& game) {
    for (int y = 0; y < Config::MAX_HEIGHT + 4; ++y) {
        bool isClearing = false;
        for (int cy : game.clearedLayers) {
            if (cy == y) {
                isClearing = true;
                break;
            }
        }

        for (int x = 0; x < Config::GRID_X; ++x) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                if (game.grid[x][y][z] != 0) {
                    Color4f col = game.gridColors[x][y][z];
                    if (isClearing) {
                        float flash = 0.5f + 0.5f * std::sin(game.clearAnimTimer * 28.0f);
                        col = Color4f(0.98f, 0.90f, 0.45f, 0.9f).blend(Color4f(1.0f, 1.0f, 1.0f, 1.0f), flash);
                    }
                    drawStyledCube((float)x, (float)y, (float)z, col, 0.95f);
                }
            }
        }
    }
}

void Renderer::renderGhostBlock(const GameLogic& game) {
    if (game.state != GameState::PLAYING) return;
    const Block* active = game.rack.getActiveBlock();
    if (!active) return;

    Vec3i ghostPos = game.calculateGhostPosition();
    if (ghostPos == active->pos) return;

    std::vector<Vec3i> ghostVoxels = active->getWorldVoxelsAt(ghostPos);
    Color4f ghostCol = active->color.withAlpha(0.24f);

    glDepthMask(GL_FALSE);
    for (const auto& v : ghostVoxels) {
        drawStyledCube((float)v.x, (float)v.y, (float)v.z, ghostCol, 0.92f);
    }
    glDepthMask(GL_TRUE);
}

void Renderer::renderActiveBlock(const GameLogic& game) {
    if (game.state != GameState::PLAYING) return;
    const Block* active = game.rack.getActiveBlock();
    if (!active) return;

    std::vector<Vec3i> voxels = active->getWorldVoxels();
    for (const auto& v : voxels) {
        drawStyledCube((float)v.x, (float)v.y, (float)v.z, active->color, 0.95f);
    }
}

// ============================================================================
// 2D HUD AND UI RENDERING
// ============================================================================

void Renderer::drawRect(float x, float y, float w, float h, const Color4f& col) {
    glColor4f(col.r, col.g, col.b, col.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Renderer::drawRectOutline(float x, float y, float w, float h, const Color4f& col, float lineWidth) {
    glLineWidth(lineWidth);
    glColor4f(col.r, col.g, col.b, col.a);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Renderer::drawProgressBar(float x, float y, float w, float h, float ratio, const Color4f& colFill, const Color4f& colBg) {
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    drawRect(x, y, w, h, colBg);
    if (ratio > 0.0f) {
        drawRect(x, y, w * ratio, h, colFill);
    }
    drawRectOutline(x, y, w, h, Color4f(colFill.r, colFill.g, colFill.b, 0.65f), 1.0f);
}

void Renderer::drawButton(float x, float y, float w, float h, const std::string& label, const Color4f& baseCol, float fontSize, bool active) {
    Color4f fillCol = baseCol;
    Color4f outlineCol = Color4f(baseCol.r * 1.35f, baseCol.g * 1.35f, baseCol.b * 1.35f, 0.85f);
    Color4f textCol = Color4f(0.92f, 0.94f, 0.97f);

    if (active) {
        fillCol = Color4f(baseCol.r * 1.4f, baseCol.g * 1.4f, baseCol.b * 1.4f, 0.98f);
        outlineCol = Color4f(0.28f, 0.85f, 0.95f, 1.0f);
        textCol = Color4f(1.0f, 1.0f, 1.0f);
    }

    drawRect(x, y, w, h, fillCol);
    drawRectOutline(x, y, w, h, outlineCol, active ? 2.2f : 1.2f);
    FontRenderer::drawText(x + w * 0.5f, y + (h - fontSize) * 0.5f - 1.0f, label, textCol, fontSize, true, true);
}

void Renderer::renderHUD2D(const GameLogic& game, const Camera& camera) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (double)windowWidth, (double)windowHeight, 0.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 1. Thanh trạng thái trên cùng
    drawTopPanel(game);

    // 2. Bảng hướng dẫn phím góc dưới trái
    drawControlsGuide();

    // 3. Khay 3 khối ở đáy
    drawBottomRackHUD(game);

    // 4. Nút camera góc dưới phải
    drawCameraControlsHUD(camera);

    // 5. Thông báo chướng ngại vật xuất hiện (Hard Mode)
    drawObstacleToast(game);

    // 6. Màn hình tạm dừng / Game Over / Bắt đầu
    if (game.state == GameState::START_SCREEN) {
        drawStartScreen(game);
    } else if (game.state == GameState::PAUSED) {
        drawPauseScreen(game);
    } else if (game.state == GameState::GAME_OVER_TIMEOUT || game.state == GameState::GAME_OVER_HEIGHT) {
        drawGameOverModal(game);
    }

    // 7. Menu Cheat & Test Demo Nhanh (Phím K)
    if (game.isCheatMenuOpen) {
        drawCheatMenuModal(game);
    }
}

void Renderer::drawTopPanel(const GameLogic& game) {
    float topH = 72.0f;
    drawRect(0, 0, (float)windowWidth, topH, Color4f(0.07f, 0.09f, 0.14f, 0.90f));
    drawRectOutline(0, 0, (float)windowWidth, topH, Color4f(0.25f, 0.42f, 0.60f, 0.35f), 1.2f);

    // Tiêu đề game & Tag Chế độ + Độ khó
    FontRenderer::drawText(20, 10, "TETRIS 3D", Color4f(0.35f, 0.85f, 0.95f), 24.0f);

    char modeBadge[64];
    snprintf(modeBadge, sizeof(modeBadge), "[%s | %s (x%d)]",
             getTimeModeName(game.currentTimeMode),
             getDifficultyName(game.currentDifficulty),
             game.getScoreMultiplier());
    FontRenderer::drawText(20, 42, modeBadge, Color4f(0.96f, 0.74f, 0.24f), 13.0f);

    // THỜI GIAN
    float timerX = (float)windowWidth * 0.31f;
    if (game.currentTimeMode == TimeMode::ENDLESS) {
        int totalSec = (int)game.matchTimeElapsed;
        int m = totalSec / 60;
        int s = totalSec % 60;
        char timeStr[32];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", m, s);

        FontRenderer::drawText(timerX, 10, "THỜI GIAN ĐÃ CHƠI (VÔ TẬN):", Color4f(0.70f, 0.80f, 0.92f), 12.0f);
        FontRenderer::drawText(timerX, 28, timeStr, Color4f(0.35f, 0.78f, 0.95f), 24.0f);
        FontRenderer::drawText(timerX + 100, 32, "∞ KHÔNG GIỚI HẠN", Color4f(0.50f, 0.85f, 0.65f), 13.0f);
    } else {
        float limit = (game.currentTimeMode == TimeMode::MIN_3) ? Config::TIME_LIMIT_3MIN : Config::TIME_LIMIT_15MIN;
        int totalSec = (int)std::ceil(game.matchTimeRemaining);
        int m = totalSec / 60;
        int s = totalSec % 60;
        char timeStr[32];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", m, s);

        Color4f timerCol = Color4f(0.28f, 0.85f, 0.55f);
        if (game.matchTimeRemaining < 30.0f) {
            float blink = (float)((int)(game.matchTimeRemaining * 4.0f) % 2);
            timerCol = (blink > 0.5f) ? Color4f(0.95f, 0.35f, 0.38f) : Color4f(0.96f, 0.74f, 0.22f);
        } else if (game.matchTimeRemaining < 60.0f) {
            timerCol = Color4f(0.96f, 0.74f, 0.22f);
        }

        char titleTime[48];
        snprintf(titleTime, sizeof(titleTime), "THỜI GIAN CÒN LẠI (%s):", getTimeModeName(game.currentTimeMode));
        FontRenderer::drawText(timerX, 10, titleTime, Color4f(0.70f, 0.80f, 0.92f), 12.0f);
        FontRenderer::drawText(timerX, 28, timeStr, timerCol, 24.0f);

        float timeRatio = game.matchTimeRemaining / limit;
        drawProgressBar(timerX + 95, 36, 140, 13, timeRatio, timerCol, Color4f(0.12f, 0.16f, 0.22f, 0.8f));
    }

    // ĐIỂM SỐ & KỶ LỤC CỦA CHẾ ĐỘ HIỆN TẠI
    float scoreX = (float)windowWidth * 0.62f;
    char scoreStr[40], highStr[40];
    snprintf(scoreStr, sizeof(scoreStr), "ĐIỂM: %d", game.score);
    snprintf(highStr, sizeof(highStr), "KỶ LỤC: %d", game.highScore);
    FontRenderer::drawText(scoreX, 12, scoreStr, Color4f(0.94f, 0.95f, 0.98f), 18.0f);
    FontRenderer::drawText(scoreX, 38, highStr, Color4f(0.96f, 0.78f, 0.24f), 15.0f);

    // ĐỘ CAO HIỆN TẠI (TỐI ĐA 10 TẦNG)
    int maxH = game.getCurrentMaxHeight();
    float heightX = (float)windowWidth - 215.0f;
    char heightStr[32];
    snprintf(heightStr, sizeof(heightStr), "ĐỘ CAO: %d / 10", maxH);

    Color4f hCol = Color4f(0.28f, 0.82f, 0.52f);
    if (maxH >= 9) hCol = Color4f(0.95f, 0.35f, 0.38f);
    else if (maxH >= 6) hCol = Color4f(0.96f, 0.72f, 0.22f);

    FontRenderer::drawText(heightX, 12, heightStr, hCol, 16.0f);
    drawProgressBar(heightX, 38, 175, 13, (float)maxH / 10.0f, hCol, Color4f(0.12f, 0.16f, 0.22f, 0.8f));
}

void Renderer::drawBottomRackHUD(const GameLogic& game) {
    float rackW = 480.0f;
    float rackH = 95.0f;
    float rackX = ((float)windowWidth - rackW) * 0.5f;
    float rackY = (float)windowHeight - rackH - 15.0f;

    drawRect(rackX, rackY, rackW, rackH, Color4f(0.07f, 0.10f, 0.16f, 0.90f));
    drawRectOutline(rackX, rackY, rackW, rackH, Color4f(0.25f, 0.45f, 0.65f, 0.40f), 1.2f);

    char batchInfo[64];
    snprintf(batchInfo, sizeof(batchInfo), "KHAY 3 KHỐI [ĐỢT %d] - CÒN LẠI: %d/3 KHỐI", game.roundCount, game.rack.getRemainingCount());
    FontRenderer::drawText(rackX + 16, rackY + 6, batchInfo, Color4f(0.70f, 0.85f, 0.98f), 13.0f);

    float slotW = 140.0f;
    float slotH = 60.0f;
    float slotGap = 15.0f;
    float startX = rackX + 15.0f;
    float startY = rackY + 26.0f;

    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        float sx = startX + i * (slotW + slotGap);
        float sy = startY;

        if (game.rack.slots[i].used) {
            drawRect(sx, sy, slotW, slotH, Color4f(0.05f, 0.07f, 0.10f, 0.65f));
            drawRectOutline(sx, sy, slotW, slotH, Color4f(0.20f, 0.25f, 0.35f, 0.35f), 1.0f);

            char slotLabel[16];
            snprintf(slotLabel, sizeof(slotLabel), "[%d]", i + 1);
            FontRenderer::drawText(sx + 10, sy + 10, slotLabel, Color4f(0.40f, 0.45f, 0.55f), 12.0f);
            FontRenderer::drawText(sx + 42, sy + 22, "ĐÃ XẾP", Color4f(0.40f, 0.45f, 0.55f), 13.0f);
        } else {
            bool isActive = (i == game.rack.activeSlotIndex);
            Color4f bgCol = isActive ? Color4f(0.10f, 0.16f, 0.26f, 0.92f) : Color4f(0.06f, 0.09f, 0.14f, 0.80f);
            Color4f outCol = isActive ? Color4f(0.30f, 0.85f, 0.95f, 0.90f) : Color4f(0.25f, 0.38f, 0.52f, 0.40f);

            drawRect(sx, sy, slotW, slotH, bgCol);
            drawRectOutline(sx, sy, slotW, slotH, outCol, isActive ? 2.0f : 1.0f);

            char slotLabel[16];
            snprintf(slotLabel, sizeof(slotLabel), "[%d]", i + 1);
            FontRenderer::drawText(sx + 8, sy + 8, slotLabel, isActive ? Color4f(1.0f, 1.0f, 1.0f) : Color4f(0.65f, 0.72f, 0.82f), 12.0f);

            Color4f blkCol = game.rack.slots[i].block.color;
            drawRect(sx + 32, sy + 10, 10, 10, blkCol);

            if (isActive) {
                FontRenderer::drawText(sx + 48, sy + 8, "ĐANG CHỌN", Color4f(0.28f, 0.90f, 0.60f), 11.0f);
            }

            FontRenderer::drawText(sx + 10, sy + 34, game.rack.slots[i].block.name, blkCol, 12.0f);
        }
    }
}

void Renderer::drawControlsGuide() {
    float boxW = 290.0f;
    float boxH = 302.0f;
    float boxX = 20.0f;
    float boxY = (float)windowHeight - boxH - 15.0f;

    drawRect(boxX, boxY, boxW, boxH, Color4f(0.06f, 0.09f, 0.14f, 0.88f));
    drawRectOutline(boxX, boxY, boxW, boxH, Color4f(0.25f, 0.45f, 0.65f, 0.40f), 1.2f);

    FontRenderer::drawText(boxX + 14, boxY + 10, "HƯỚNG DẪN ĐIỀU KHIỂN:", Color4f(0.35f, 0.85f, 0.95f), 14.0f);

    float textY = boxY + 34.0f;
    float dy = 20.0f;

    FontRenderer::drawText(boxX + 14, textY, "Chuột phải/kéo: Xoay 360° quanh trục", Color4f(0.96f, 0.80f, 0.35f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Cuộn chuột: Thu phóng camera", Color4f(0.75f, 0.85f, 0.95f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "W, A, S, D: Di chuyển (theo góc nhìn)", Color4f(0.75f, 0.85f, 0.95f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Q / E: Xoay ngang (Trục Y)", Color4f(0.35f, 0.85f, 0.60f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "R / F: Xoay dọc (Trục X)", Color4f(0.35f, 0.85f, 0.60f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Z / C: Xoay nghiêng (Trục Z)", Color4f(0.35f, 0.85f, 0.60f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Space: Thả rơi ngay (Hard Drop)", Color4f(0.95f, 0.50f, 0.75f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Shift / X: Rơi nhanh (Soft Drop)", Color4f(0.96f, 0.65f, 0.30f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Phím [ / ]: Xoay nhanh góc 90°", Color4f(0.35f, 0.80f, 0.95f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Phím 1, 2, 3: Chọn khối trong khay", Color4f(0.35f, 0.80f, 0.95f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Phím K: Menu Cheat (Demo nhanh)", Color4f(0.96f, 0.78f, 0.24f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "P: Tạm dừng | Esc: Thoát", Color4f(0.65f, 0.70f, 0.80f), 12.0f);
}

void Renderer::drawCameraControlsHUD(const Camera& camera) {
    float camBtnsX = (float)windowWidth - 365.0f;
    float camBtnsY = (float)windowHeight - 56.0f;

    char angleStr[48];
    snprintf(angleStr, sizeof(angleStr), "Góc Camera: %d° quanh trục", (int)std::round(camera.yaw));
    FontRenderer::drawText(camBtnsX, camBtnsY - 22.0f, angleStr, Color4f(0.60f, 0.78f, 0.92f), 13.0f);

    drawButton(camBtnsX, camBtnsY, 110.0f, 38.0f, "↺ Xoay 90°", Color4f(0.10f, 0.15f, 0.24f, 0.9f), 13.0f);
    drawButton(camBtnsX + 118.0f, camBtnsY, 110.0f, 38.0f, "Xoay 90° ↻", Color4f(0.10f, 0.15f, 0.24f, 0.9f), 13.0f);
    drawButton(camBtnsX + 236.0f, camBtnsY, 114.0f, 38.0f, "Góc nhìn: V", Color4f(0.12f, 0.18f, 0.28f, 0.9f), 13.0f);
}

void Renderer::drawObstacleToast(const GameLogic& game) {
    if (game.obstacleNotifyTimer <= 0.0f) return;

    float toastW = 520.0f;
    float toastH = 40.0f;
    float toastX = ((float)windowWidth - toastW) * 0.5f;
    float toastY = 82.0f;

    float alpha = std::min(1.0f, game.obstacleNotifyTimer * 2.0f);
    drawRect(toastX, toastY, toastW, toastH, Color4f(0.30f, 0.08f, 0.12f, 0.90f * alpha));
    drawRectOutline(toastX, toastY, toastW, toastH, Color4f(0.95f, 0.35f, 0.40f, 0.85f * alpha), 1.5f);

    FontRenderer::drawText(toastX + toastW * 0.5f, toastY + 11.0f, game.obstacleNotifyMessage,
                           Color4f(1.0f, 0.88f, 0.40f, alpha), 14.0f, true, true);
}

void Renderer::drawStartScreen(const GameLogic& game) {
    drawRect(0, 0, (float)windowWidth, (float)windowHeight, Color4f(0.04f, 0.06f, 0.09f, 0.88f));

    float sW = 740.0f;
    float sH = 540.0f;
    float sX = ((float)windowWidth - sW) * 0.5f;
    float sY = ((float)windowHeight - sH) * 0.5f;

    drawRect(sX, sY, sW, sH, Color4f(0.08f, 0.11f, 0.17f, 0.96f));
    drawRectOutline(sX, sY, sW, sH, Color4f(0.28f, 0.55f, 0.80f, 0.60f), 2.0f);

    // Tiêu đề
    FontRenderer::drawText(sX + sW * 0.5f, sY + 22.0f, "TETRIS 3D", Color4f(0.35f, 0.88f, 0.98f), 32.0f, true, true);
    FontRenderer::drawText(sX + sW * 0.5f, sY + 62.0f, "SÀN 5x5 | TRỤC XOAY 360° | 7 KHỐI KINH ĐIỂN", Color4f(0.65f, 0.78f, 0.92f), 13.0f, true, true);

    // BƯỚC 1: CHỌN THỜI GIAN MỖI LƯỢT CHƠI
    float section1Y = sY + 95.0f;
    FontRenderer::drawText(sX + 35, section1Y, "1. CHỌN THỜI GIAN MỖI LƯỢT CHƠI:", Color4f(0.35f, 0.85f, 0.95f), 14.0f);

    float btnW = 210.0f;
    float btnH = 46.0f;
    float gap = 20.0f;
    float startX = sX + 35.0f;
    float btnY_time = section1Y + 24.0f;

    bool sel3 = (game.currentTimeMode == TimeMode::MIN_3);
    bool sel15 = (game.currentTimeMode == TimeMode::MIN_15);
    bool selEnd = (game.currentTimeMode == TimeMode::ENDLESS);

    drawButton(startX, btnY_time, btnW, btnH, "3 PHÚT (Phím 1)", Color4f(0.12f, 0.16f, 0.25f, 0.9f), 14.0f, sel3);
    drawButton(startX + btnW + gap, btnY_time, btnW, btnH, "15 PHÚT (Phím 2)", Color4f(0.12f, 0.16f, 0.25f, 0.9f), 14.0f, sel15);
    drawButton(startX + (btnW + gap) * 2, btnY_time, btnW, btnH, "VÔ TẬN (Phím 3)", Color4f(0.12f, 0.16f, 0.25f, 0.9f), 14.0f, selEnd);

    // BƯỚC 2: CHỌN TIẾP ĐỘ KHÓ
    float section2Y = btnY_time + btnH + 22.0f;
    FontRenderer::drawText(sX + 35, section2Y, "2. CHỌN TIẾP ĐỘ KHÓ:", Color4f(0.35f, 0.85f, 0.95f), 14.0f);

    float btnY_diff = section2Y + 24.0f;
    float btnH_diff = 54.0f;

    bool selEasy = (game.currentDifficulty == Difficulty::EASY);
    bool selNorm = (game.currentDifficulty == Difficulty::NORMAL);
    bool selHard = (game.currentDifficulty == Difficulty::HARD);

    drawButton(startX, btnY_diff, btnW, btnH_diff, "DỄ (Phím 4)\nTốc độ 0.5x | Điểm x1", Color4f(0.12f, 0.16f, 0.25f, 0.9f), 13.0f, selEasy);
    drawButton(startX + btnW + gap, btnY_diff, btnW, btnH_diff, "TRUNG BÌNH (Phím 5)\nTốc độ 1.0x | Điểm x8", Color4f(0.12f, 0.16f, 0.25f, 0.9f), 13.0f, selNorm);
    drawButton(startX + (btnW + gap) * 2, btnY_diff, btnW, btnH_diff, "KHÓ (Phím 6)\nTốc độ 2.0x | Điểm x20", Color4f(0.12f, 0.16f, 0.25f, 0.9f), 13.0f, selHard);

    // BẢNG THÔNG TIN KỶ LỤC & QUY TẮC
    float infoY = btnY_diff + btnH_diff + 24.0f;
    char highRecordStr[80];
    snprintf(highRecordStr, sizeof(highRecordStr), "KỶ LỤC CHO [%s - %s]: %d ĐIỂM",
             getTimeModeName(game.currentTimeMode),
             getDifficultyName(game.currentDifficulty),
             game.getHighScoreForCurrentMode());
    FontRenderer::drawText(sX + sW * 0.5f, infoY, highRecordStr, Color4f(0.96f, 0.78f, 0.24f), 15.0f, true, true);

    float tipY = infoY + 28.0f;
    if (game.currentDifficulty == Difficulty::HARD) {
        FontRenderer::drawText(sX + sW * 0.5f, tipY, "* Chế độ Khó: Cứ mỗi 15 khối đặt thành công sẽ sinh 1 ô ngẫu nhiên ở đáy!",
                               Color4f(0.95f, 0.45f, 0.45f), 13.0f, true, true);
    } else {
        FontRenderer::drawText(sX + sW * 0.5f, tipY, "* Mỗi khối đặt thành công +5đ, xóa 1 tầng +100đ (nhân theo hệ số độ khó).",
                               Color4f(0.35f, 0.85f, 0.60f), 13.0f, true, true);
    }

    float ctrlTipY = tipY + 24.0f;
    FontRenderer::drawText(sX + sW * 0.5f, ctrlTipY, "Chuột phải xoay 360° | W,A,S,D di chuyển | Q/E, R/F, Z/C xoay 3D",
                           Color4f(0.65f, 0.72f, 0.82f), 12.0f, true, true);

    // NÚT BẮT ĐẦU CHƠI
    float startBtnW = 400.0f;
    float startBtnH = 52.0f;
    float startBtnX = sX + (sW - startBtnW) * 0.5f;
    float startBtnY = sY + sH - startBtnH - 24.0f;
    drawButton(startBtnX, startBtnY, startBtnW, startBtnH, "BẮT ĐẦU CHƠI (SPACE / ENTER)", Color4f(0.12f, 0.45f, 0.38f, 0.95f), 16.0f, true);
}

void Renderer::drawPauseScreen(const GameLogic& game) {
    drawRect(0, 0, (float)windowWidth, (float)windowHeight, Color4f(0.0f, 0.0f, 0.0f, 0.65f));

    float pW = 480.0f;
    float pH = 220.0f;
    float pX = ((float)windowWidth - pW) * 0.5f;
    float pY = ((float)windowHeight - pH) * 0.5f;

    drawRect(pX, pY, pW, pH, Color4f(0.08f, 0.11f, 0.17f, 0.96f));
    drawRectOutline(pX, pY, pW, pH, Color4f(0.30f, 0.55f, 0.80f, 0.60f), 2.0f);

    FontRenderer::drawText(pX + pW * 0.5f, pY + 28.0f, "TẠM DỪNG", Color4f(0.96f, 0.78f, 0.24f), 26.0f, true, true);

    char modeStr[64];
    snprintf(modeStr, sizeof(modeStr), "Chế độ: %s  |  Độ khó: %s (x%d)",
             getTimeModeName(game.currentTimeMode),
             getDifficultyName(game.currentDifficulty),
             game.getScoreMultiplier());
    FontRenderer::drawText(pX + pW * 0.5f, pY + 70.0f, modeStr, Color4f(0.75f, 0.85f, 0.95f), 14.0f, true, true);

    float btnW = 190.0f, btnH = 46.0f;
    float btnY = pY + 125.0f;
    drawButton(pX + 35.0f, btnY, btnW, btnH, "TIẾP TỤC (P)", Color4f(0.12f, 0.42f, 0.35f, 0.95f), 14.0f);
    drawButton(pX + pW - 35.0f - btnW, btnY, btnW, btnH, "MENU CHÍNH (M)", Color4f(0.20f, 0.25f, 0.36f, 0.95f), 14.0f);
}

void Renderer::drawGameOverModal(const GameLogic& game) {
    drawRect(0, 0, (float)windowWidth, (float)windowHeight, Color4f(0.04f, 0.05f, 0.08f, 0.88f));

    float mW = 640.0f;
    float mH = 410.0f;
    float mX = ((float)windowWidth - mW) * 0.5f;
    float mY = ((float)windowHeight - mH) * 0.5f;

    drawRect(mX, mY, mW, mH, Color4f(0.09f, 0.11f, 0.17f, 0.96f));
    drawRectOutline(mX, mY, mW, mH, Color4f(0.95f, 0.35f, 0.38f, 0.75f), 2.2f);

    FontRenderer::drawText(mX + mW * 0.5f, mY + 24.0f, "TRÒ CHƠI KẾT THÚC", Color4f(0.95f, 0.35f, 0.38f), 26.0f, true, true);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 60.0f, "GAME OVER", Color4f(0.85f, 0.60f, 0.65f), 16.0f, true, true);

    // Lý do thua
    drawRect(mX + 30, mY + 95, mW - 60, 46, Color4f(0.22f, 0.08f, 0.10f, 0.8f));
    drawRectOutline(mX + 30, mY + 95, mW - 60, 46, Color4f(0.95f, 0.35f, 0.38f, 0.5f), 1.2f);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 110.0f, game.gameOverReason, Color4f(0.98f, 0.82f, 0.35f), 15.0f, true, true);

    char scoreInfo[64], highInfo[64], modeInfo[80];
    snprintf(scoreInfo, sizeof(scoreInfo), "ĐIỂM ĐẠT ĐƯỢC: %d", game.score);
    snprintf(highInfo, sizeof(highInfo), "KỶ LỤC [%s - %s]: %d",
             getTimeModeName(game.currentTimeMode),
             getDifficultyName(game.currentDifficulty),
             game.highScore);
    snprintf(modeInfo, sizeof(modeInfo), "Số tầng đã xóa: %d  |  Số đợt hoàn thành: %d", game.totalLayersCleared, game.roundCount);

    FontRenderer::drawText(mX + mW * 0.5f, mY + 170.0f, scoreInfo, Color4f(1.0f, 1.0f, 1.0f), 20.0f, true, true);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 205.0f, highInfo, Color4f(0.96f, 0.78f, 0.24f), 17.0f, true, true);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 242.0f, modeInfo, Color4f(0.70f, 0.80f, 0.92f), 13.0f, true, true);

    // 2 Nút: Chơi lại & Chọn chế độ khác
    float btnW = 260.0f;
    float btnH = 50.0f;
    float btnY = mY + 310.0f;
    float btnX1 = mX + 45.0f;
    float btnX2 = mX + mW - 45.0f - btnW;

    drawButton(btnX1, btnY, btnW, btnH, "CHƠI LẠI (SPACE)", Color4f(0.12f, 0.45f, 0.38f, 0.95f), 15.0f, true);
    drawButton(btnX2, btnY, btnW, btnH, "ĐỔI CHẾ ĐỘ (PHÍM M)", Color4f(0.20f, 0.26f, 0.38f, 0.95f), 15.0f);
}

void Renderer::drawCheatMenuModal(const GameLogic& game) {
    // Backdrop bán trong suốt
    drawRect(0, 0, (float)windowWidth, (float)windowHeight, Color4f(0.02f, 0.04f, 0.07f, 0.85f));

    float mW = 720.0f;
    float mH = 500.0f;
    float mX = ((float)windowWidth - mW) * 0.5f;
    float mY = ((float)windowHeight - mH) * 0.5f;

    // Khung chính viền vàng sang trọng
    drawRect(mX, mY, mW, mH, Color4f(0.08f, 0.11f, 0.17f, 0.98f));
    drawRectOutline(mX, mY, mW, mH, Color4f(0.96f, 0.78f, 0.24f, 0.85f), 2.2f);

    // Tiêu đề
    FontRenderer::drawText(mX + mW * 0.5f, mY + 24.0f, "MENU CHEAT & TEST DEMO NHANH", Color4f(0.96f, 0.78f, 0.24f), 22.0f, true, true);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 52.0f, "Nhấn phím tắt [1-9] hoặc CLICK CHUỘT vào các chức năng bên dưới:", Color4f(0.40f, 0.80f, 0.95f), 13.0f, true, true);

    // Bố cục 2 cột x 5 nút (Tổng cộng 10 nút chức năng)
    float colW = 315.0f;
    float rowH = 62.0f;
    float col1X = mX + 30.0f;
    float col2X = mX + mW - 30.0f - colW;
    float startY = mY + 80.0f;
    float gapY = 70.0f;

    struct CheatItem {
        std::string key;
        std::string title;
        std::string desc;
        Color4f col;
    };

    CheatItem leftItems[5] = {
        {"1", "Thua ngay 10/10", "Chồng khối lên trần 10/10 -> Xử thua Game Over", Color4f(0.95f, 0.35f, 0.38f)},
        {"2", "Dựng tháp 9 tầng", "Lập tháp 9/10, bạn thả 1 khối là thua 10/10", Color4f(0.96f, 0.65f, 0.25f)},
        {"3", "Tự xóa 1 tầng ngay", "Lấp kín 25/25 ô tầng đáy -> Xóa & cộng điểm", Color4f(0.28f, 0.82f, 0.52f)},
        {"4", "Gần đầy tầng (24/25 ô)", "Chừa 1 ô đáy để bạn tự đặt khối xóa tầng", Color4f(0.35f, 0.78f, 0.95f)},
        {"5", "Đổi Độ khó nhanh", "Xoay vòng Dễ (x1) -> Vừa (x8) -> Khó (x20)", Color4f(0.66f, 0.50f, 0.95f)}
    };

    CheatItem rightItems[5] = {
        {"6", "Demo khối ngẫu nhiên", "Sinh 1 khối ngẫu nhiên ở đáy (không tự hoàn thành tầng)", Color4f(0.95f, 0.40f, 0.30f)},
        {"7", "Thời gian còn 5 giây", "Đếm ngược 5s để demo thua vì Timeout", Color4f(0.96f, 0.78f, 0.24f)},
        {"8", "Cộng +5,000 Điểm", "Tăng nhanh điểm số & kiểm tra Kỷ lục HighScore", Color4f(0.98f, 0.85f, 0.35f)},
        {"9", "Xóa sạch sàn 5x5", "Dọn trống toàn bộ các khối để test lại từ đầu", Color4f(0.45f, 0.75f, 0.95f)},
        {"K", "ĐÓNG MENU CHEAT", "Nhấn K hoặc Esc để quay lại ván chơi", Color4f(0.55f, 0.60f, 0.70f)}
    };

    // Vẽ cột bên trái
    for (int i = 0; i < 5; ++i) {
        float by = startY + i * gapY;
        drawRect(col1X, by, colW, rowH, Color4f(0.10f, 0.14f, 0.21f, 0.95f));
        drawRectOutline(col1X, by, colW, rowH, leftItems[i].col, 1.2f);

        // Huy hiệu phím tắt
        drawRect(col1X + 8.0f, by + 12.0f, 36.0f, 38.0f, Color4f(0.16f, 0.22f, 0.32f, 0.95f));
        drawRectOutline(col1X + 8.0f, by + 12.0f, 36.0f, 38.0f, leftItems[i].col, 1.2f);
        FontRenderer::drawText(col1X + 26.0f, by + 23.0f, leftItems[i].key, leftItems[i].col, 16.0f, true, true);

        // Tiêu đề & mô tả
        FontRenderer::drawText(col1X + 54.0f, by + 14.0f, leftItems[i].title, leftItems[i].col, 14.0f);
        FontRenderer::drawText(col1X + 54.0f, by + 36.0f, leftItems[i].desc, Color4f(0.70f, 0.78f, 0.88f), 11.0f);
    }

    // Vẽ cột bên phải
    for (int i = 0; i < 5; ++i) {
        float by = startY + i * gapY;
        drawRect(col2X, by, colW, rowH, Color4f(0.10f, 0.14f, 0.21f, 0.95f));
        drawRectOutline(col2X, by, colW, rowH, rightItems[i].col, 1.2f);

        // Huy hiệu phím tắt
        drawRect(col2X + 8.0f, by + 12.0f, 36.0f, 38.0f, Color4f(0.16f, 0.22f, 0.32f, 0.95f));
        drawRectOutline(col2X + 8.0f, by + 12.0f, 36.0f, 38.0f, rightItems[i].col, 1.2f);
        FontRenderer::drawText(col2X + 26.0f, by + 23.0f, rightItems[i].key, rightItems[i].col, 16.0f, true, true);

        // Tiêu đề & mô tả
        FontRenderer::drawText(col2X + 54.0f, by + 14.0f, rightItems[i].title, rightItems[i].col, 14.0f);
        FontRenderer::drawText(col2X + 54.0f, by + 36.0f, rightItems[i].desc, Color4f(0.70f, 0.78f, 0.88f), 11.0f);
    }

    // Ghi chú chân trang
    FontRenderer::drawText(mX + mW * 0.5f, mY + mH - 24.0f, "Mẹo: Menu cheat có thể bật ở bất kỳ chế độ nào (3 phút, 15 phút, Endless).", Color4f(0.60f, 0.70f, 0.82f), 12.0f, true, true);
}
