#pragma once
#include "Types.h"
#include "Config.h"
#include "GameLogic.h"
#include "Camera.h"
#include "FontRenderer.h"

class Renderer {
public:
    int windowWidth;
    int windowHeight;

    Renderer();

    // Khởi tạo các thông số OpenGL (Depth test, Alpha blend, Lighting, Material)
    void initGL(int width, int height);

    // Cập nhật khi cửa sổ thay đổi kích thước
    void reshape(int width, int height);

    // Vẽ toàn bộ khung hình trò chơi
    void render(const GameLogic& game, const Camera& camera);

private:
    // Vẽ các thành phần 3D
    void renderScene3D(const GameLogic& game, const Camera& camera);
    void renderFloorGrid();
    void renderGlassTower(int currentMaxHeight);
    void renderPlacedBlocks(const GameLogic& game);
    void renderGhostBlock(const GameLogic& game);
    void renderActiveBlock(const GameLogic& game);
    void renderFloorFootprint(const GameLogic& game);
    void renderCenterAxisMarker();

    // Hàm phụ trợ vẽ 1 ô lập phương 3D có vát cạnh và viền phát sáng
    void drawStyledCube(float x, float y, float z, const Color4f& color, float scale = 0.96f, bool wireframeOnly = false);

    // Vẽ giao diện người dùng 2D (HUD)
    void renderHUD2D(const GameLogic& game, const Camera& camera);
    void drawTopPanel(const GameLogic& game);
    void drawBottomRackHUD(const GameLogic& game);
    void drawControlsGuide();
    void drawCameraControlsHUD(const Camera& camera);
    void drawGameOverModal(const GameLogic& game);
    void drawStartScreen();
    void drawPauseScreen();

    // Hàm phụ trợ vẽ hình chữ nhật phẳng 2D và thanh đo
    void drawRect(float x, float y, float w, float h, const Color4f& col);
    void drawRectOutline(float x, float y, float w, float h, const Color4f& col, float lineWidth = 1.5f);
    void drawProgressBar(float x, float y, float w, float h, float ratio, const Color4f& colFill, const Color4f& colBg);
    void drawButton(float x, float y, float w, float h, const std::string& label, const Color4f& baseCol, float fontSize = 16.0f);
};
