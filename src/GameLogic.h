#pragma once
#include "Config.h"
#include "Types.h"
#include "BlockLogic.h"
#include <vector>
#include <string>

// Trạng thái của trò chơi
enum class GameState {
    START_SCREEN,          // Màn hình chào mừng / Bắt đầu
    PLAYING,               // Đang chơi bình thường
    CLEARING_ANIMATION,    // Hoạt họa xóa tầng đang diễn ra
    PAUSED,                // Tạm dừng
    GAME_OVER_TIMEOUT,     // Thua vì hết 3 phút mà chưa xếp xong 3 khối
    GAME_OVER_HEIGHT       // Thua vì độ cao khối vượt quá 10
};

class GameLogic {
public:
    // Lưới không gian 3D: grid[x][y][z]
    // 0 = ô trống, >0 = ID màu của ô lập phương
    int grid[Config::GRID_X][Config::MAX_HEIGHT + 6][Config::GRID_Z];
    Color4f gridColors[Config::GRID_X][Config::MAX_HEIGHT + 6][Config::GRID_Z];

    // Khay 3 khối ngẫu nhiên
    BlockRack rack;

    // Trạng thái và thời gian
    GameState state;
    float roundTimeRemaining;    // Đếm ngược 180s cho đợt 3 khối hiện tại
    float gravityTimer;          // Bộ đếm thời gian cho rơi tự nhiên
    float gravityInterval;       // Chu kỳ rơi tự nhiên (giây)
    float clearAnimTimer;        // Thời gian hoạt họa xóa tầng
    std::vector<int> clearedLayers; // Danh sách các tầng Y vừa được lấp đầy đang bị xóa

    // Điểm số và kỷ lục
    int score;
    int highScore;
    int roundCount;              // Đợt chơi thứ mấy
    int totalLayersCleared;      // Tổng số tầng đã xóa

    // Lý do thua cuộc hiển thị lên UI
    std::string gameOverReason;

    GameLogic();

    // Khởi tạo ván chơi mới
    void resetGame();

    // Cập nhật logic theo khung thời gian dt (delta time)
    void update(float dt);

    // Điều khiển khối đang hoạt động
    bool moveActiveBlock(int dx, int dz);
    bool rotateActiveBlock(RotationAxis axis, bool clockwise = true);
    void softDrop();
    void hardDrop();

    // Chuyển đổi khối trong khay (phím 1, 2, 3)
    bool selectRackSlot(int slotIndex);

    // Tính vị trí bóng đổ (Ghost Block) để người chơi biết khối sẽ rơi vào đâu
    Vec3i calculateGhostPosition() const;

    // Kiểm tra tính hợp lệ của khối tại vị trí thử nghiệm
    bool isValidPosition(const Block& b, const Vec3i& testPos) const;

    // Khóa khối hiện tại vào sàn lưới
    void lockCurrentBlock();

    // Kiểm tra và xóa các tầng đầy (5x5 = 25 ô)
    void checkFullLayers();

    // Áp dụng việc xóa tầng sau khi hoạt họa kết thúc
    void applyClearLayers();

    // Lấy chiều cao lớn nhất hiện tại của các khối trên sàn
    int getCurrentMaxHeight() const;

    // Lưu và nạp điểm cao kỷ lục
    void loadHighScore();
    void saveHighScore();

    // Callback âm thanh (sẽ được gán từ AudioManager)
    void (*onPlaySound)(int soundId);
};
