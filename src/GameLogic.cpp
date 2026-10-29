#include "GameLogic.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>

GameLogic::GameLogic()
    : state(GameState::START_SCREEN),
      roundTimeRemaining(Config::ROUND_TIME_LIMIT),
      gravityTimer(0.0f),
      gravityInterval(Config::DEFAULT_GRAVITY_INTERVAL),
      clearAnimTimer(0.0f),
      score(0),
      highScore(0),
      roundCount(1),
      totalLayersCleared(0),
      gameOverReason(""),
      onPlaySound(nullptr)
{
    std::memset(grid, 0, sizeof(grid));
    loadHighScore();
}

void GameLogic::resetGame() {
    std::memset(grid, 0, sizeof(grid));
    for (int x = 0; x < Config::GRID_X; ++x) {
        for (int y = 0; y < Config::MAX_HEIGHT + 6; ++y) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                gridColors[x][y][z] = Color4f(0, 0, 0, 0);
            }
        }
    }

    score = 0;
    roundCount = 1;
    totalLayersCleared = 0;
    roundTimeRemaining = Config::ROUND_TIME_LIMIT;
    gravityTimer = 0.0f;
    gravityInterval = Config::DEFAULT_GRAVITY_INTERVAL;
    clearedLayers.clear();
    gameOverReason = "";

    // Sinh 3 khối đầu tiên cho ván chơi
    rack.generateNewBatch();
    state = GameState::PLAYING;

    if (onPlaySound) onPlaySound(1); // Start sound
}

void GameLogic::loadHighScore() {
    highScore = 0;
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void GameLogic::saveHighScore() {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

int GameLogic::getCurrentMaxHeight() const {
    int maxH = 0;
    for (int y = Config::MAX_HEIGHT + 5; y >= 0; --y) {
        for (int x = 0; x < Config::GRID_X; ++x) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                if (grid[x][y][z] != 0) {
                    return y + 1; // Chiều cao thực tế (1-indexed)
                }
            }
        }
    }
    return 0;
}

bool GameLogic::isValidPosition(const Block& b, const Vec3i& testPos) const {
    std::vector<Vec3i> voxels = b.getWorldVoxelsAt(testPos);
    for (const auto& v : voxels) {
        // Kiểm tra biên sàn 5x5
        if (v.x < 0 || v.x >= Config::GRID_X) return false;
        if (v.z < 0 || v.z >= Config::GRID_Z) return false;
        // Kiểm tra đáy sàn
        if (v.y < 0) return false;
        // Kiểm tra va chạm với ô đã có sẵn
        if (v.y < Config::MAX_HEIGHT + 6) {
            if (grid[v.x][v.y][v.z] != 0) {
                return false;
            }
        }
    }
    return true;
}

Vec3i GameLogic::calculateGhostPosition() const {
    const Block* active = rack.getActiveBlock();
    if (!active) return Vec3i(0, 0, 0);

    Vec3i ghostPos = active->pos;
    while (true) {
        Vec3i nextPos = ghostPos;
        nextPos.y--;
        if (isValidPosition(*active, nextPos)) {
            ghostPos = nextPos;
        } else {
            break;
        }
    }
    return ghostPos;
}

bool GameLogic::moveActiveBlock(int dx, int dz) {
    if (state != GameState::PLAYING) return false;
    Block* active = rack.getActiveBlock();
    if (!active) return false;

    Vec3i newPos = active->pos;
    newPos.x += dx;
    newPos.z += dz;

    if (isValidPosition(*active, newPos)) {
        active->pos = newPos;
        if (onPlaySound) onPlaySound(2); // Move sound
        return true;
    }
    return false;
}

bool GameLogic::rotateActiveBlock(RotationAxis axis, bool clockwise) {
    if (state != GameState::PLAYING) return false;
    Block* active = rack.getActiveBlock();
    if (!active) return false;

    Block testBlock = *active;
    testBlock.rotate(axis, clockwise);

    // Thử vị trí hiện tại
    if (isValidPosition(testBlock, testBlock.pos)) {
        *active = testBlock;
        if (onPlaySound) onPlaySound(3); // Rotate sound
        return true;
    }

    // Cơ chế Wall-kick mở rộng (nếu chạm thành kính hoặc khối bên cạnh thì thử dịch 1 hoặc 2 ô để lọt)
    const Vec3i kicks[] = {
        {-1, 0, 0}, {1, 0, 0}, {0, 0, -1}, {0, 0, 1},
        {-2, 0, 0}, {2, 0, 0}, {0, 0, -2}, {0, 0, 2},
        {-1, 0, -1}, {1, 0, 1}, {-1, 0, 1}, {1, 0, -1},
        {0, 1, 0}, {0, 2, 0},
        {-1, 1, 0}, {1, 1, 0}, {0, 1, -1}, {0, 1, 1}
    };
    for (const auto& k : kicks) {
        Vec3i kickPos = testBlock.pos + k;
        if (isValidPosition(testBlock, kickPos)) {
            testBlock.pos = kickPos;
            *active = testBlock;
            if (onPlaySound) onPlaySound(3); // Rotate sound
            return true;
        }
    }

    return false;
}

void GameLogic::softDrop() {
    if (state != GameState::PLAYING) return;
    Block* active = rack.getActiveBlock();
    if (!active) return;

    Vec3i nextPos = active->pos;
    nextPos.y--;
    if (isValidPosition(*active, nextPos)) {
        active->pos = nextPos;
        gravityTimer = 0.0f;
    } else {
        lockCurrentBlock();
    }
}

void GameLogic::hardDrop() {
    if (state != GameState::PLAYING) return;
    Block* active = rack.getActiveBlock();
    if (!active) return;

    Vec3i ghost = calculateGhostPosition();
    active->pos = ghost;
    lockCurrentBlock();
}

bool GameLogic::selectRackSlot(int slotIndex) {
    if (state != GameState::PLAYING) return false;
    bool success = rack.selectSlot(slotIndex);
    if (success && onPlaySound) {
        onPlaySound(2); // Select sound
    }
    return success;
}

void GameLogic::lockCurrentBlock() {
    Block* active = rack.getActiveBlock();
    if (!active) return;

    std::vector<Vec3i> worldVoxels = active->getWorldVoxels();
    bool heightExceeded = false;

    // Ghi các ô vào mảng grid
    for (const auto& v : worldVoxels) {
        if (v.x >= 0 && v.x < Config::GRID_X &&
            v.z >= 0 && v.z < Config::GRID_Z &&
            v.y >= 0 && v.y < Config::MAX_HEIGHT + 6)
        {
            grid[v.x][v.y][v.z] = active->colorId;
            gridColors[v.x][v.y][v.z] = active->color;

            // KIỂM TRA ĐIỀU KIỆN THUA: Nếu xếp độ cao vượt quá 10 (tức là v.y >= 10)
            if (v.y >= Config::MAX_HEIGHT) {
                heightExceeded = true;
            }
        }
    }

    score += Config::SCORE_PLACE_BLOCK;
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }

    if (onPlaySound) onPlaySound(4); // Lock block sound

    // Đánh dấu khối này trong khay là đã sử dụng
    rack.markActiveAsUsed();

    // Nếu vượt quá độ cao 10 -> Xử thua ngay lập tức!
    if (heightExceeded) {
        state = GameState::GAME_OVER_HEIGHT;
        gameOverReason = "ĐỘ CAO VƯỢT QUÁ 10 TẦNG! (HEIGHT OVERFLOW)";
        if (onPlaySound) onPlaySound(6); // Game over sound
        return;
    }

    // Kiểm tra các tầng đầy (full 5x5 = 25 ô)
    checkFullLayers();

    // Nếu không có tầng nào cần xóa và tất cả 3 khối trong đợt đã đặt xong:
    if (clearedLayers.empty() && rack.areAllUsed()) {
        // Hoàn thành xuất sắc đợt 3 khối trong 3 phút!
        score += Config::SCORE_ROUND_BONUS;
        roundCount++;
        roundTimeRemaining = Config::ROUND_TIME_LIMIT; // Đặt lại 3 phút cho đợt tiếp theo
        rack.generateNewBatch();
        if (onPlaySound) onPlaySound(5); // Round clear fanfare
    }
}

void GameLogic::checkFullLayers() {
    clearedLayers.clear();

    for (int y = 0; y < Config::MAX_HEIGHT + 6; ++y) {
        bool full = true;
        for (int x = 0; x < Config::GRID_X; ++x) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                if (grid[x][y][z] == 0) {
                    full = false;
                    break;
                }
            }
            if (!full) break;
        }
        if (full) {
            clearedLayers.push_back(y);
        }
    }

    if (!clearedLayers.empty()) {
        state = GameState::CLEARING_ANIMATION;
        clearAnimTimer = 0.45f;
        if (onPlaySound) onPlaySound(5); // Line clear sound
    }
}

void GameLogic::applyClearLayers() {
    if (clearedLayers.empty()) return;

    // Sắp xếp các tầng giảm dần để xóa từ trên xuống dưới
    std::sort(clearedLayers.rbegin(), clearedLayers.rend());

    for (int layerY : clearedLayers) {
        for (int y = layerY; y < Config::MAX_HEIGHT + 5; ++y) {
            for (int x = 0; x < Config::GRID_X; ++x) {
                for (int z = 0; z < Config::GRID_Z; ++z) {
                    grid[x][y][z] = grid[x][y + 1][z];
                    gridColors[x][y][z] = gridColors[x][y + 1][z];
                }
            }
        }
        // Xóa sạch tầng trên cùng
        for (int x = 0; x < Config::GRID_X; ++x) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                grid[x][Config::MAX_HEIGHT + 5][z] = 0;
                gridColors[x][Config::MAX_HEIGHT + 5][z] = Color4f(0, 0, 0, 0);
            }
        }
    }

    int count = (int)clearedLayers.size();
    totalLayersCleared += count;

    int points = 0;
    if (count == 1) points = Config::SCORE_CLEAR_1_LAYER;
    else if (count == 2) points = Config::SCORE_CLEAR_2_LAYERS;
    else if (count == 3) points = Config::SCORE_CLEAR_3_LAYERS;
    else points = Config::SCORE_CLEAR_4_LAYERS;

    score += points;
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }

    clearedLayers.clear();

    // Kiểm tra xem đã hết 3 khối trong đợt chưa
    if (rack.areAllUsed()) {
        score += Config::SCORE_ROUND_BONUS;
        roundCount++;
        roundTimeRemaining = Config::ROUND_TIME_LIMIT; // Đặt lại 3 phút cho đợt 3 khối mới
        rack.generateNewBatch();
    }

    state = GameState::PLAYING;
}

void GameLogic::update(float dt) {
    if (state == GameState::CLEARING_ANIMATION) {
        clearAnimTimer -= dt;
        if (clearAnimTimer <= 0.0f) {
            applyClearLayers();
        }
        return;
    }

    if (state != GameState::PLAYING) return;

    // Cập nhật hoạt họa xoay nhẹ khối trong khay
    rack.updateAnimation(dt);

    // ĐẾM NGƯỢC THỜI GIAN 3 PHÚT (180s) CHO ĐỢT 3 KHỐI HIỆN TẠI
    roundTimeRemaining -= dt;
    if (roundTimeRemaining <= 0.0f) {
        roundTimeRemaining = 0.0f;
        // Nếu hết thời gian mà chưa sắp xong 3 khối vào sàn -> XỬ THUA!
        if (!rack.areAllUsed()) {
            state = GameState::GAME_OVER_TIMEOUT;
            gameOverReason = "HẾT THỜI GIAN 3 PHÚT! (TIMEOUT DEFEAT)";
            if (onPlaySound) onPlaySound(6); // Game over sound
            return;
        }
    }

    // Rơi tự nhiên của khối đang điều khiển
    Block* active = rack.getActiveBlock();
    if (active) {
        gravityTimer += dt;
        if (gravityTimer >= gravityInterval) {
            gravityTimer = 0.0f;
            Vec3i nextPos = active->pos;
            nextPos.y--;
            if (isValidPosition(*active, nextPos)) {
                active->pos = nextPos;
            } else {
                lockCurrentBlock();
            }
        }
    }
}
