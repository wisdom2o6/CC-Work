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

    // Thiết lập nguồn sáng
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    GLfloat light0_pos[] = { 10.0f, 20.0f, 15.0f, 1.0f };
    GLfloat light0_ambient[] = { 0.25f, 0.28f, 0.35f, 1.0f };
    GLfloat light0_diffuse[] = { 0.85f, 0.90f, 0.95f, 1.0f };
    GLfloat light0_specular[] = { 0.50f, 0.50f, 0.50f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    GLfloat light1_pos[] = { -10.0f, -5.0f, -10.0f, 1.0f };
    GLfloat light1_ambient[] = { 0.10f, 0.10f, 0.15f, 1.0f };
    GLfloat light1_diffuse[] = { 0.30f, 0.35f, 0.45f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);

    GLfloat mat_specular[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat mat_shininess[] = { 32.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // Màu nền Cyberpunk Dark Slate sang trọng
    glClearColor(0.04f, 0.06f, 0.10f, 1.0f);
}

void Renderer::reshape(int width, int height) {
    windowWidth = width;
    windowHeight = height > 0 ? height : 1;
    glViewport(0, 0, windowWidth, windowHeight);
}

void Renderer::render(const GameLogic& game, const Camera& camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Vẽ không gian 3D (Sàn 5x5, Tháp lồng kính, Khối lập phương, Trục xoay)
    renderScene3D(game, camera);

    // 2. Vẽ giao diện HUD 2D (Thời gian 3 phút, Điểm, Khay 3 khối, Nút điều khiển nhanh)
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

    // Vẽ đánh dấu trục quay trung tâm
    renderCenterAxisMarker();

    // Vẽ sàn lưới 5x5
    renderFloorGrid();

    // Vẽ lồng tháp kính 5x5x10 với vạch cảnh báo đỏ ở độ cao 10
    renderGlassTower(game.getCurrentMaxHeight());

    // Vẽ bóng chiếu của khối đang rơi xuống mặt sàn (Floor Footprint)
    renderFloorFootprint(game);

    // Vẽ các khối đã cố định trên sàn
    renderPlacedBlocks(game);

    // Vẽ bóng đổ (Ghost Block) để người chơi ngắm vị trí rơi
    renderGhostBlock(game);

    // Vẽ khối đang hoạt động (đang rơi)
    renderActiveBlock(game);
}

void Renderer::renderCenterAxisMarker() {
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);

    // Trục trung tâm thẳng đứng ở x=2.5, z=2.5 từ y=-0.5 đến y=10.5
    glBegin(GL_LINES);
    glColor4f(0.0f, 0.8f, 1.0f, 0.25f);
    glVertex3f(2.5f, -0.5f, 2.5f);
    glVertex3f(2.5f, 10.5f, 2.5f);
    glEnd();

    glEnable(GL_LIGHTING);
}

void Renderer::renderFloorGrid() {
    glDisable(GL_LIGHTING);

    // Vẽ các ô nền 5x5 với họa tiết bàn cờ công nghệ cao
    for (int x = 0; x < Config::GRID_X; ++x) {
        for (int z = 0; z < Config::GRID_Z; ++z) {
            float brightness = ((x + z) % 2 == 0) ? 0.08f : 0.12f;
            glColor4f(brightness, brightness * 1.3f, brightness * 1.8f, 0.95f);

            glBegin(GL_QUADS);
            glVertex3f((float)x, 0.0f, (float)z);
            glVertex3f((float)x + 1.0f, 0.0f, (float)z);
            glVertex3f((float)x + 1.0f, 0.0f, (float)z + 1.0f);
            glVertex3f((float)x, 0.0f, (float)z + 1.0f);
            glEnd();
        }
    }

    // Các đường lưới phát sáng Neon Cyan
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor4f(0.0f, 0.9f, 1.0f, 0.7f);
    for (int i = 0; i <= Config::GRID_X; ++i) {
        glVertex3f((float)i, 0.005f, 0.0f);
        glVertex3f((float)i, 0.005f, (float)Config::GRID_Z);
    }
    for (int j = 0; j <= Config::GRID_Z; ++j) {
        glVertex3f(0.0f, 0.005f, (float)j);
        glVertex3f((float)Config::GRID_X, 0.005f, (float)j);
    }
    glEnd();

    // Viền phát sáng ngoài cùng của sàn 5x5
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glColor4f(0.0f, 1.0f, 0.85f, 0.9f);
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

    // 1. Bốn cột trụ phát sáng ở 4 góc tháp
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glColor4f(0.0f, 0.85f, 1.0f, 0.6f);
    // Cột 1
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, maxH, 0.0f);
    // Cột 2
    glVertex3f(gx, 0.0f, 0.0f);
    glVertex3f(gx, maxH, 0.0f);
    // Cột 3
    glVertex3f(gx, 0.0f, gz);
    glVertex3f(gx, maxH, gz);
    // Cột 4
    glVertex3f(0.0f, 0.0f, gz);
    glVertex3f(0.0f, maxH, gz);
    glEnd();

    // 2. Các vòng chia tầng từ 1 đến 9
    glLineWidth(1.0f);
    for (int y = 1; y < Config::MAX_HEIGHT; ++y) {
        float fy = (float)y;
        glBegin(GL_LINE_LOOP);
        if (y <= currentMaxHeight) {
            glColor4f(0.0f, 0.8f, 1.0f, 0.35f); // Tầng đã có khối
        } else {
            glColor4f(0.3f, 0.4f, 0.5f, 0.18f); // Tầng còn trống
        }
        glVertex3f(0.0f, fy, 0.0f);
        glVertex3f(gx, fy, 0.0f);
        glVertex3f(gx, fy, gz);
        glVertex3f(0.0f, fy, gz);
        glEnd();
    }

    // 3. VẠCH ĐỎ CẢNH BÁO TẦNG 10 (ĐỘ CAO TỐI ĐA - VƯỢT QUÁ SẼ THUA!)
    glLineWidth(3.5f);
    glBegin(GL_LINE_LOOP);
    glColor4f(1.0f, 0.15f, 0.15f, 0.95f);
    glVertex3f(0.0f, maxH, 0.0f);
    glVertex3f(gx, maxH, 0.0f);
    glVertex3f(gx, maxH, gz);
    glVertex3f(0.0f, maxH, gz);
    glEnd();

    // Vách kính mờ ảo (Glass walls)
    glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.25f, 0.45f, 0.05f);
    // Vách trái (x = 0)
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, maxH, 0.0f);
    glVertex3f(0.0f, maxH, gz);
    glVertex3f(0.0f, 0.0f, gz);
    // Vách phải (x = gx)
    glVertex3f(gx, 0.0f, 0.0f);
    glVertex3f(gx, maxH, 0.0f);
    glVertex3f(gx, maxH, gz);
    glVertex3f(gx, 0.0f, gz);
    // Vách sau (z = 0)
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(gx, 0.0f, 0.0f);
    glVertex3f(gx, maxH, 0.0f);
    glVertex3f(0.0f, maxH, 0.0f);
    glEnd();
    glDepthMask(GL_TRUE);

    glEnable(GL_LIGHTING);
}

void Renderer::renderFloorFootprint(const GameLogic& game) {
    if (game.state != GameState::PLAYING) return;
    const Block* active = game.rack.getActiveBlock();
    if (!active) return;

    // Chiếu bóng các ô của khối đang điều khiển xuống mặt sàn (y = 0.015)
    std::vector<Vec3i> voxels = active->getWorldVoxels();
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);

    for (const auto& v : voxels) {
        if (v.x < 0 || v.x >= Config::GRID_X || v.z < 0 || v.z >= Config::GRID_Z) continue;

        // Vẽ ô phát sáng tinh tế trên sàn
        glColor4f(active->color.r, active->color.g, active->color.b, 0.25f);
        glBegin(GL_QUADS);
        glVertex3f((float)v.x + 0.05f, 0.015f, (float)v.z + 0.05f);
        glVertex3f((float)v.x + 0.95f, 0.015f, (float)v.z + 0.05f);
        glVertex3f((float)v.x + 0.95f, 0.015f, (float)v.z + 0.95f);
        glVertex3f((float)v.x + 0.05f, 0.015f, (float)v.z + 0.95f);
        glEnd();

        // Viền neon nổi bật
        glLineWidth(2.0f);
        glColor4f(active->color.r, active->color.g, active->color.b, 0.75f);
        glBegin(GL_LINE_LOOP);
        glVertex3f((float)v.x + 0.05f, 0.016f, (float)v.z + 0.05f);
        glVertex3f((float)v.x + 0.95f, 0.016f, (float)v.z + 0.05f);
        glVertex3f((float)v.x + 0.95f, 0.016f, (float)v.z + 0.95f);
        glVertex3f((float)v.x + 0.05f, 0.016f, (float)v.z + 0.95f);
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
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);

        // Mặt dưới (-Y)
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
        glVertex3f(x0, y0, z1);

        // Mặt trước (+Z)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);

        // Mặt sau (-Z)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y0, z0);

        // Mặt trái (-X)
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);

        // Mặt phải (+X)
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y0, z1);
        glEnd();
    }

    // Vẽ viền cạnh sắc nét (Outlines)
    glDisable(GL_LIGHTING);
    glLineWidth(1.8f);
    if (wireframeOnly) {
        glColor4f(color.r, color.g, color.b, 0.8f);
    } else {
        glColor4f(color.r * 1.3f, color.g * 1.3f, color.b * 1.3f, 0.9f);
    }

    glBegin(GL_LINE_LOOP); // Vòng đáy
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
    glEnd();

    glBegin(GL_LINE_LOOP); // Vòng đỉnh
    glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
    glEnd();

    glBegin(GL_LINES); // 4 cột nối
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
                        col = Color4f(1.0f, 0.95f, 0.4f, 0.9f).blend(Color4f(1.0f, 1.0f, 1.0f, 1.0f), flash);
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
    Color4f ghostCol = active->color.withAlpha(0.28f);

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
    drawRectOutline(x, y, w, h, Color4f(colFill.r, colFill.g, colFill.b, 0.8f), 1.0f);
}

void Renderer::drawButton(float x, float y, float w, float h, const std::string& label, const Color4f& baseCol, float fontSize) {
    drawRect(x, y, w, h, baseCol);
    drawRectOutline(x, y, w, h, Color4f(baseCol.r * 1.5f, baseCol.g * 1.5f, baseCol.b * 1.5f, 0.9f), 1.5f);
    FontRenderer::drawText(x + w * 0.5f, y + (h - fontSize) * 0.5f - 1.0f, label, Color4f(1.0f, 1.0f, 1.0f), fontSize, true, true);
}

void Renderer::renderHUD2D(const GameLogic& game, const Camera& camera) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (double)windowWidth, (double)windowHeight, 0.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 1. Thanh trạng thái phía trên
    drawTopPanel(game);

    // 2. Bảng hướng dẫn phím ở góc dưới bên trái
    drawControlsGuide();

    // 3. Khay 3 khối ở giữa cạnh dưới
    drawBottomRackHUD(game);

    // 4. Các nút điều khiển camera góc dưới bên phải
    drawCameraControlsHUD(camera);

    // 5. Màn hình tạm dừng / Game Over / Bắt đầu
    if (game.state == GameState::START_SCREEN) {
        drawStartScreen();
    } else if (game.state == GameState::PAUSED) {
        drawPauseScreen();
    } else if (game.state == GameState::GAME_OVER_TIMEOUT || game.state == GameState::GAME_OVER_HEIGHT) {
        drawGameOverModal(game);
    }
}

void Renderer::drawTopPanel(const GameLogic& game) {
    float topH = 72.0f;
    drawRect(0, 0, (float)windowWidth, topH, Color4f(0.06f, 0.09f, 0.16f, 0.88f));
    drawRectOutline(0, 0, (float)windowWidth, topH, Color4f(0.0f, 0.85f, 1.0f, 0.4f), 1.5f);

    // Tiêu đề game Tiếng Việt sắc nét
    FontRenderer::drawText(20, 12, "TETRIS 3D", Color4f(0.0f, 1.0f, 0.9f), 26.0f);
    FontRenderer::drawText(20, 44, "SÀN 5x5 | TRỤC XOAY 360°", Color4f(0.6f, 0.8f, 1.0f), 13.0f);

    // ĐẾM NGƯỢC THỜI GIAN 3 PHÚT (180 GIÂY)
    int totalSec = (int)std::ceil(game.roundTimeRemaining);
    int minutes = totalSec / 60;
    int seconds = totalSec % 60;
    char timeStr[32];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", minutes, seconds);

    float timerX = (float)windowWidth * 0.33f;
    Color4f timerCol = Color4f(0.1f, 1.0f, 0.4f);
    if (game.roundTimeRemaining < 30.0f) {
        float blink = (float)((int)(game.roundTimeRemaining * 4.0f) % 2);
        timerCol = (blink > 0.5f) ? Color4f(1.0f, 0.2f, 0.2f) : Color4f(1.0f, 0.9f, 0.2f);
    } else if (game.roundTimeRemaining < 60.0f) {
        timerCol = Color4f(1.0f, 0.85f, 0.2f);
    }

    FontRenderer::drawText(timerX, 10, "THỜI GIAN CÒN LẠI (3 PHÚT):", Color4f(0.8f, 0.9f, 1.0f), 12.0f);
    FontRenderer::drawText(timerX, 28, timeStr, timerCol, 26.0f);

    float timeRatio = game.roundTimeRemaining / Config::ROUND_TIME_LIMIT;
    drawProgressBar(timerX + 105, 36, 150, 14, timeRatio, timerCol, Color4f(0.15f, 0.18f, 0.25f, 0.8f));

    // ĐIỂM SỐ & KỶ LỤC
    float scoreX = (float)windowWidth * 0.63f;
    char scoreStr[32], highStr[32];
    snprintf(scoreStr, sizeof(scoreStr), "ĐIỂM: %d", game.score);
    snprintf(highStr, sizeof(highStr), "KỶ LỤC: %d", game.highScore);
    FontRenderer::drawText(scoreX, 12, scoreStr, Color4f(1.0f, 1.0f, 1.0f), 18.0f);
    FontRenderer::drawText(scoreX, 38, highStr, Color4f(1.0f, 0.85f, 0.1f), 16.0f);

    // ĐỘ CAO HIỆN TẠI (TỐI ĐA 10 TẦNG)
    int maxH = game.getCurrentMaxHeight();
    float heightX = (float)windowWidth - 215.0f;
    char heightStr[32];
    snprintf(heightStr, sizeof(heightStr), "ĐỘ CAO: %d / 10", maxH);

    Color4f hCol = Color4f(0.2f, 0.9f, 0.5f);
    if (maxH >= 8) hCol = Color4f(1.0f, 0.2f, 0.2f);
    else if (maxH >= 6) hCol = Color4f(1.0f, 0.8f, 0.2f);

    FontRenderer::drawText(heightX, 12, heightStr, hCol, 16.0f);
    drawProgressBar(heightX, 38, 175, 14, (float)maxH / 10.0f, hCol, Color4f(0.15f, 0.18f, 0.25f, 0.8f));
}

void Renderer::drawBottomRackHUD(const GameLogic& game) {
    float rackW = 480.0f;
    float rackH = 95.0f;
    float rackX = ((float)windowWidth - rackW) * 0.5f;
    float rackY = (float)windowHeight - rackH - 15.0f;

    drawRect(rackX, rackY, rackW, rackH, Color4f(0.06f, 0.09f, 0.16f, 0.88f));
    drawRectOutline(rackX, rackY, rackW, rackH, Color4f(0.0f, 0.85f, 1.0f, 0.5f), 1.5f);

    char batchInfo[64];
    snprintf(batchInfo, sizeof(batchInfo), "KHAY 3 KHỐI [ĐỢT %d] - CÒN LẠI: %d/3 KHỐI", game.roundCount, game.rack.getRemainingCount());
    FontRenderer::drawText(rackX + 16, rackY + 6, batchInfo, Color4f(0.7f, 0.9f, 1.0f), 13.0f);

    float slotW = 140.0f;
    float slotH = 60.0f;
    float slotGap = 15.0f;
    float startX = rackX + 15.0f;
    float startY = rackY + 26.0f;

    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        float sx = startX + i * (slotW + slotGap);
        float sy = startY;

        bool isUsed = game.rack.slots[i].used;
        bool isActive = (game.rack.activeSlotIndex == i && !isUsed);

        Color4f bgCol = isUsed ? Color4f(0.08f, 0.08f, 0.10f, 0.6f) :
                        isActive ? Color4f(0.10f, 0.26f, 0.42f, 0.95f) :
                        Color4f(0.10f, 0.14f, 0.22f, 0.7f);

        Color4f borderCol = isUsed ? Color4f(0.3f, 0.3f, 0.35f, 0.4f) :
                            isActive ? Color4f(0.0f, 1.0f, 0.85f, 1.0f) :
                            Color4f(0.3f, 0.5f, 0.7f, 0.6f);

        drawRect(sx, sy, slotW, slotH, bgCol);
        drawRectOutline(sx, sy, slotW, slotH, borderCol, isActive ? 2.5f : 1.2f);

        char keyLabel[16];
        snprintf(keyLabel, sizeof(keyLabel), "[%d]", i + 1);
        FontRenderer::drawText(sx + 8, sy + 6, keyLabel, isActive ? Color4f(0.0f, 1.0f, 0.9f) : Color4f(0.8f, 0.8f, 0.8f), 14.0f);

        if (isUsed) {
            FontRenderer::drawText(sx + 40, sy + 18, "ĐÃ XẾP", Color4f(0.5f, 0.5f, 0.5f), 15.0f);
        } else {
            Color4f blkCol = game.rack.slots[i].block.color;
            drawRect(sx + 36, sy + 7, 14, 14, blkCol);
            drawRectOutline(sx + 36, sy + 7, 14, 14, Color4f(1, 1, 1, 0.8f), 1.0f);

            if (isActive) {
                FontRenderer::drawText(sx + 56, sy + 6, "ĐANG CHỌN", Color4f(0.2f, 1.0f, 0.5f), 12.0f);
            } else {
                FontRenderer::drawText(sx + 56, sy + 6, "CHỜ XẾP", Color4f(0.8f, 0.8f, 0.9f), 12.0f);
            }

            // Tên khối tiếng Việt sắc nét (vừa vặn đẹp mắt trong ô khay)
            FontRenderer::drawText(sx + 10, sy + 34, game.rack.slots[i].block.name, blkCol, 12.0f);
        }
    }
}

void Renderer::drawControlsGuide() {
    float boxW = 290.0f;
    float boxH = 280.0f;
    float boxX = 20.0f;
    float boxY = (float)windowHeight - boxH - 15.0f;

    drawRect(boxX, boxY, boxW, boxH, Color4f(0.05f, 0.08f, 0.14f, 0.88f));
    drawRectOutline(boxX, boxY, boxW, boxH, Color4f(0.0f, 0.7f, 1.0f, 0.45f), 1.2f);

    FontRenderer::drawText(boxX + 14, boxY + 10, "HƯỚNG DẪN ĐIỀU KHIỂN:", Color4f(0.0f, 0.95f, 1.0f), 14.0f);

    float textY = boxY + 34.0f;
    float dy = 20.0f;

    FontRenderer::drawText(boxX + 14, textY, "Chuột phải/kéo: Xoay 360° quanh trục", Color4f(1.0f, 0.9f, 0.3f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Cuộn chuột: Thu phóng camera", Color4f(0.8f, 0.9f, 1.0f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "W, A, S, D: Di chuyển (theo góc nhìn)", Color4f(0.8f, 0.9f, 1.0f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Q / E: Xoay ngang (Trục Y)", Color4f(0.3f, 1.0f, 0.6f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "R / F: Xoay dọc (Trục X)", Color4f(0.3f, 1.0f, 0.6f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Z / C: Xoay nghiêng (Trục Z)", Color4f(0.3f, 1.0f, 0.6f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Space: Thả rơi ngay (Hard Drop)", Color4f(1.0f, 0.4f, 0.8f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Shift / X: Rơi nhanh (Soft Drop)", Color4f(1.0f, 0.6f, 0.2f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Phím [ / ]: Xoay nhanh góc 90°", Color4f(0.2f, 0.9f, 1.0f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "Phím 1, 2, 3: Chọn khối trong khay", Color4f(0.0f, 0.9f, 1.0f), 12.0f);
    textY += dy;
    FontRenderer::drawText(boxX + 14, textY, "P: Tạm dừng | Esc: Thoát", Color4f(0.7f, 0.7f, 0.8f), 12.0f);
}

void Renderer::drawCameraControlsHUD(const Camera& camera) {
    float camBtnsX = (float)windowWidth - 365.0f;
    float camBtnsY = (float)windowHeight - 56.0f;

    // Góc quay camera hiện tại
    char angleStr[48];
    snprintf(angleStr, sizeof(angleStr), "Góc Camera: %d° quanh trục", (int)std::round(camera.yaw));
    FontRenderer::drawText(camBtnsX, camBtnsY - 22.0f, angleStr, Color4f(0.5f, 0.85f, 1.0f), 13.0f);

    // 3 nút điều khiển nhanh
    drawButton(camBtnsX, camBtnsY, 110.0f, 38.0f, "↺ Xoay 90°", Color4f(0.10f, 0.18f, 0.30f, 0.9f), 13.0f);
    drawButton(camBtnsX + 118.0f, camBtnsY, 110.0f, 38.0f, "Xoay 90° ↻", Color4f(0.10f, 0.18f, 0.30f, 0.9f), 13.0f);
    drawButton(camBtnsX + 236.0f, camBtnsY, 114.0f, 38.0f, "Góc nhìn: V", Color4f(0.12f, 0.24f, 0.38f, 0.9f), 13.0f);
}

void Renderer::drawGameOverModal(const GameLogic& game) {
    drawRect(0, 0, (float)windowWidth, (float)windowHeight, Color4f(0.02f, 0.03f, 0.06f, 0.85f));

    float mW = 620.0f;
    float mH = 370.0f;
    float mX = ((float)windowWidth - mW) * 0.5f;
    float mY = ((float)windowHeight - mH) * 0.5f;

    drawRect(mX, mY, mW, mH, Color4f(0.08f, 0.10f, 0.18f, 0.95f));
    drawRectOutline(mX, mY, mW, mH, Color4f(1.0f, 0.2f, 0.3f, 0.9f), 2.5f);

    FontRenderer::drawText(mX + mW * 0.5f, mY + 28.0f, "TRÒ CHƠI KẾT THÚC", Color4f(1.0f, 0.2f, 0.25f), 28.0f, true, true);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 68.0f, "GAME OVER", Color4f(1.0f, 0.6f, 0.6f), 18.0f, true, true);

    // Lý do thua hiển thị trọn vẹn tiếng Việt có dấu
    drawRect(mX + 30, mY + 105, mW - 60, 48, Color4f(0.2f, 0.05f, 0.08f, 0.8f));
    drawRectOutline(mX + 30, mY + 105, mW - 60, 48, Color4f(1.0f, 0.2f, 0.2f, 0.6f), 1.5f);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 118.0f, game.gameOverReason, Color4f(1.0f, 0.85f, 0.3f), 16.0f, true, true);

    char scoreInfo[64], highInfo[64];
    snprintf(scoreInfo, sizeof(scoreInfo), "ĐIỂM ĐẠT ĐƯỢC: %d", game.score);
    snprintf(highInfo, sizeof(highInfo), "KỶ LỤC ĐIỂM SỐ: %d", game.highScore);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 180.0f, scoreInfo, Color4f(1.0f, 1.0f, 1.0f), 20.0f, true, true);
    FontRenderer::drawText(mX + mW * 0.5f, mY + 215.0f, highInfo, Color4f(1.0f, 0.85f, 0.1f), 18.0f, true, true);

    // Nút Chơi Lại
    float btnW = 360.0f;
    float btnH = 54.0f;
    float btnX = mX + (mW - btnW) * 0.5f;
    float btnY = mY + 280.0f;
    drawButton(btnX, btnY, btnW, btnH, "CHƠI LẠI (NHẤN SPACE HOẶC R)", Color4f(0.0f, 0.65f, 0.5f, 0.95f), 17.0f);
}

void Renderer::drawStartScreen() {
    drawRect(0, 0, (float)windowWidth, (float)windowHeight, Color4f(0.02f, 0.04f, 0.08f, 0.88f));

    float sW = 700.0f;
    float sH = 430.0f;
    float sX = ((float)windowWidth - sW) * 0.5f;
    float sY = ((float)windowHeight - sH) * 0.5f;

    drawRect(sX, sY, sW, sH, Color4f(0.07f, 0.11f, 0.20f, 0.95f));
    drawRectOutline(sX, sY, sW, sH, Color4f(0.0f, 0.85f, 1.0f, 0.8f), 2.5f);

    FontRenderer::drawText(sX + sW * 0.5f, sY + 28.0f, "TETRIS 3D", Color4f(0.0f, 1.0f, 0.9f), 34.0f, true, true);
    FontRenderer::drawText(sX + sW * 0.5f, sY + 70.0f, "SÀN 5x5 - TRỤC XOAY 360°", Color4f(0.6f, 0.8f, 1.0f), 16.0f, true, true);

    float textY = sY + 115.0f;
    float dy = 30.0f;
    FontRenderer::drawText(sX + 45, textY, "• Sàn chơi 5x5, chiều cao tối đa 10 tầng trong tháp kính.", Color4f(0.9f, 0.9f, 0.9f), 14.0f);
    textY += dy;
    FontRenderer::drawText(sX + 45, textY, "• Mỗi lượt xuất hiện 3 khối, có 3 phút để sắp xếp vào sàn.", Color4f(0.9f, 0.9f, 0.9f), 14.0f);
    textY += dy;
    FontRenderer::drawText(sX + 45, textY, "• Lấp đầy 1 tầng 5x5 (25 ô) để xóa tầng và ghi điểm kỷ lục.", Color4f(0.2f, 1.0f, 0.5f), 14.0f);
    textY += dy;
    FontRenderer::drawText(sX + 45, textY, "• Nếu vượt quá độ cao 10 tầng hoặc hết giờ 3 phút sẽ bị xử thua!", Color4f(1.0f, 0.4f, 0.4f), 14.0f);
    textY += dy;
    FontRenderer::drawText(sX + 45, textY, "• Giữ chuột phải hoặc kéo chuột để xoay 360° quanh trục trung tâm.", Color4f(1.0f, 0.9f, 0.3f), 14.0f);
    textY += dy;
    FontRenderer::drawText(sX + 45, textY, "• Phím W, A, S, D tự động đổi hướng di chuyển theo góc nhìn camera.", Color4f(0.4f, 0.85f, 1.0f), 14.0f);

    float btnW = 340.0f;
    float btnH = 55.0f;
    float btnX = sX + (sW - btnW) * 0.5f;
    float btnY = sY + 340.0f;
    drawButton(btnX, btnY, btnW, btnH, "NHẤN SPACE ĐỂ BẮT ĐẦU", Color4f(0.0f, 0.7f, 0.45f, 0.95f), 18.0f);
}

void Renderer::drawPauseScreen() {
    drawRect(0, 0, (float)windowWidth, (float)windowHeight, Color4f(0.0f, 0.0f, 0.0f, 0.65f));

    float pW = 420.0f;
    float pH = 170.0f;
    float pX = ((float)windowWidth - pW) * 0.5f;
    float pY = ((float)windowHeight - pH) * 0.5f;

    drawRect(pX, pY, pW, pH, Color4f(0.08f, 0.12f, 0.20f, 0.95f));
    drawRectOutline(pX, pY, pW, pH, Color4f(0.0f, 0.8f, 1.0f, 0.8f), 2.0f);

    FontRenderer::drawText(pX + pW * 0.5f, pY + 35.0f, "TẠM DỪNG", Color4f(1.0f, 0.85f, 0.1f), 28.0f, true, true);
    FontRenderer::drawText(pX + pW * 0.5f, pY + 95.0f, "NHẤN PHÍM P ĐỂ TIẾP TỤC", Color4f(0.8f, 0.9f, 1.0f), 16.0f, true, true);
}
