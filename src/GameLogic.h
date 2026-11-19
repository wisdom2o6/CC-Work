#pragma once
#include "Config.h"
#include "Types.h"
#include "BlockLogic.h"
#include <vector>
#include <string>

// Trạng thái của trò chơi
enum class GameState {
    START_SCREEN,          // Màn hình chọn chế độ và bắt đầu
    PLAYING,               // Đang chơi bình thường
    CLEARING_ANIMATION,    // Hoạt họa xóa tầng
    PAUSED,                // Tạm dừng
    GAME_OVER_TIMEOUT,     // Thua vì hết thời gian
    GAME_OVER_HEIGHT       // Thua vì độ cao đạt/vượt quá 10 tầng
};

class GameLogic {
public:
    // Lưới không gian 3D: grid[x][y][z]
    // 0 = ô trống, >0 = ID màu của ô lập phương
    int grid[Config::GRID_X][Config::MAX_HEIGHT + 6][Config::GRID_Z];
    Color4f gridColors[Config::GRID_X][Config::MAX_HEIGHT + 6][Config::GRID_Z];

    // Khay 3 khối ngẫu nhiên
    BlockRack rack;

    // Chế độ chơi và độ khó
    TimeMode currentTimeMode;
    Difficulty currentDifficulty;

    // Bảng lưu điểm kỷ lục: 3 mốc thời gian x 3 độ khó = 9 kỷ lục
    int highScores[3][3];

    // Trạng thái và bộ đếm thời gian
    GameState state;
    float matchTimeRemaining;     // Đếm ngược cho 3 phút (180s) và 15 phút (900s)
    float matchTimeElapsed;       // Đếm xuôi thời gian chơi cho Endless mode
    float gravityTimer;           // Bộ đếm thời gian rơi tự nhiên
    float gravityInterval;        // Tốc độ rơi theo độ khó (giây/ô)
    float clearAnimTimer;         // Thời gian hoạt họa xóa tầng
    std::vector<int> clearedLayers; // Danh sách các tầng Y đang được xóa

    // Thống kê ván chơi
    int score;
    int highScore;
    int roundCount;               // Đợt chơi thứ mấy
    int totalLayersCleared;       // Tổng số tầng đã xóa
    int blocksPlacedCount;        // Đếm số khối đã đặt (dùng cho mốc 15 khối ở chế độ khó)

    // Thông báo chướng ngại vật xuất hiện
    float obstacleNotifyTimer;
    std::string obstacleNotifyMessage;

    // Lý do thua cuộc hiển thị lên UI
    std::string gameOverReason;

    // Menu Cheat & Test Demo Nhanh (Phím K)
    bool isCheatMenuOpen;

    GameLogic();

    // Thiết lập chế độ và độ khó
    void setTimeMode(TimeMode m);
    void setDifficulty(Difficulty d);
    int getScoreMultiplier() const;
    int getHighScoreForCurrentMode() const;

    // Khởi tạo ván chơi mới
    void resetGame();

    // Cập nhật logic theo delta time (dt)
    void update(float dt);

    // Điều khiển khối đang hoạt động
    bool moveActiveBlock(int dx, int dz);
    bool rotateActiveBlock(RotationAxis axis, bool clockwise = true);
    void softDrop();
    void hardDrop();

    // Chuyển đổi khối trong khay (phím 1, 2, 3)
    bool selectRackSlot(int slotIndex);

    // Tính vị trí bóng đổ (Ghost Block)
    Vec3i calculateGhostPosition() const;

    // Kiểm tra tính hợp lệ của khối tại vị trí thử nghiệm
    bool isValidPosition(const Block& b, const Vec3i& testPos) const;

    // Khóa khối hiện tại vào sàn lưới
    void lockCurrentBlock();

    // Kiểm tra và xóa các tầng đầy (5x5 = 25 ô)
    void checkFullLayers();

    // Áp dụng việc xóa tầng sau khi hoạt họa kết thúc
    void applyClearLayers();

    // Sinh ô chướng ngại vật ở vị trí thấp nhất (dành cho chế độ khó mỗi 15 khối)
    void spawnHardModeObstacle();

    // Lấy chiều cao lớn nhất hiện tại của các khối trên sàn
    int getCurrentMaxHeight() const;

    // Các tính năng Cheat Mode phục vụ Demo nhanh (Phím K)
    void toggleCheatMenu();
    void cheatStack10GameOver();
    void cheatStack9Tiers();
    void cheatClearLayerInstant();
    void cheatFillLayerAlmost();
    void cheatCycleDifficulty();
    void cheatTrigger15Obstacle();
    void cheatSetTime5Seconds();
    void cheatAddScore(int amount = 5000);
    void cheatClearAllGrid();

    // Lưu và nạp điểm cao kỷ lục
    void loadHighScore();
    void saveHighScore();

    // Callback âm thanh
    void (*onPlaySound)(int soundId);
};
