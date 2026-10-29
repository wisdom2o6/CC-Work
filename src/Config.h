#pragma once

// ============================================================================
// CONFIGURATION & GAME CONSTANTS - TETRIS 3D
// ============================================================================

namespace Config {
    // Sàn lưới 5x5
    constexpr int GRID_X = 5;
    constexpr int GRID_Z = 5;

    // Chiều cao tối đa cho phép là 10. Vượt quá 10 sẽ bị xử thua!
    constexpr int MAX_HEIGHT = 10;

    // Số khối trong mỗi đợt random
    constexpr int BLOCKS_PER_ROUND = 3;

    // Thời gian giới hạn cho mỗi đợt 3 khối (3 phút = 180 giây)
    constexpr float ROUND_TIME_LIMIT = 180.0f;

    // Tốc độ rơi tự nhiên mặc định (giây mỗi ô)
    constexpr float DEFAULT_GRAVITY_INTERVAL = 1.0f;
    constexpr float SOFT_DROP_INTERVAL = 0.08f;

    // Điểm số
    constexpr int SCORE_PLACE_BLOCK = 50;
    constexpr int SCORE_CLEAR_1_LAYER = 1000;
    constexpr int SCORE_CLEAR_2_LAYERS = 2500;
    constexpr int SCORE_CLEAR_3_LAYERS = 5000;
    constexpr int SCORE_CLEAR_4_LAYERS = 10000;
    constexpr int SCORE_ROUND_BONUS = 500;

    // Kích thước cửa sổ mặc định
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 800;
    constexpr const char* WINDOW_TITLE = "Tetris 3D - Sàn 5x5 - OpenGL";
}
