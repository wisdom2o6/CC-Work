#pragma once

// ============================================================================
// CONFIGURATION & GAME CONSTANTS - TETRIS 3D
// ============================================================================

namespace Config {
    // Sàn lưới 5x5
    constexpr int GRID_X = 5;
    constexpr int GRID_Z = 5;

    // Chiều cao tối đa cho phép là 10 tầng
    constexpr int MAX_HEIGHT = 10;

    // Số khối trong mỗi đợt trong khay
    constexpr int BLOCKS_PER_ROUND = 3;

    // Các mốc thời gian giới hạn (giây)
    constexpr float TIME_LIMIT_3MIN = 180.0f;   // 3 phút
    constexpr float TIME_LIMIT_15MIN = 900.0f;  // 15 phút

    // Tốc độ rơi tự nhiên theo độ khó (giây mỗi ô)
    constexpr float GRAVITY_EASY = 2.0f;        // Dễ: chậm bằng 1/2 hiện tại (2.0s/ô)
    constexpr float GRAVITY_NORMAL = 1.0f;      // Trung bình: 1.0s/ô
    constexpr float GRAVITY_HARD = 0.5f;        // Khó: gấp đôi hiện tại (0.5s/ô)
    constexpr float SOFT_DROP_INTERVAL = 0.06f;

    // Hệ số nhân điểm theo độ khó
    constexpr int MULT_EASY = 1;                // x1
    constexpr int MULT_NORMAL = 8;              // x8
    constexpr int MULT_HARD = 20;               // x20

    // Điểm số cơ bản (sẽ nhân với hệ số độ khó)
    constexpr int BASE_SCORE_PLACE_BLOCK = 5;   // 5 điểm mỗi khối
    constexpr int BASE_SCORE_CLEAR_1_LAYER = 100; // Mặc định 100 điểm khi xóa 1 tầng
    constexpr int BASE_SCORE_CLEAR_2_LAYERS = 250;
    constexpr int BASE_SCORE_CLEAR_3_LAYERS = 500;
    constexpr int BASE_SCORE_CLEAR_4_LAYERS = 1000;
    constexpr int BASE_SCORE_ROUND_BONUS = 50;  // Thưởng hoàn thành 3 khối trong khay

    // Chu kỳ sinh ô chướng ngại vật ở chế độ khó (mỗi 15 khối đặt thành công)
    constexpr int HARD_MODE_OBSTACLE_INTERVAL = 15;

    // Kích thước cửa sổ mặc định
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 800;
    constexpr const char* WINDOW_TITLE = "Tetris 3D - Sàn 5x5 - OpenGL";
}
