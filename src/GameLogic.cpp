#include "GameLogic.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>

GameLogic::GameLogic()
    : currentTimeMode(TimeMode::MIN_3),
      currentDifficulty(Difficulty::NORMAL),
      state(GameState::START_SCREEN),
      matchTimeRemaining(Config::TIME_LIMIT_3MIN),
      matchTimeElapsed(0.0f),
      gravityTimer(0.0f),
      gravityInterval(Config::GRAVITY_NORMAL),
      clearAnimTimer(0.0f),
      score(0),
      highScore(0),
      roundCount(1),
      totalLayersCleared(0),
      blocksPlacedCount(0),
      obstacleNotifyTimer(0.0f),
      obstacleNotifyMessage(""),
      gameOverReason(""),
      isCheatMenuOpen(false),
      onPlaySound(nullptr)
{
    std::memset(grid, 0, sizeof(grid));
    for (int t = 0; t < 3; ++t) {
        for (int d = 0; d < 3; ++d) {
            highScores[t][d] = 0;
        }
    }
    loadHighScore();
}

void GameLogic::setTimeMode(TimeMode m) {
    currentTimeMode = m;
    highScore = getHighScoreForCurrentMode();
}

void GameLogic::setDifficulty(Difficulty d) {
    currentDifficulty = d;
    highScore = getHighScoreForCurrentMode();
}

int GameLogic::getScoreMultiplier() const {
    switch (currentDifficulty) {
        case Difficulty::EASY: return Config::MULT_EASY;     // x1
        case Difficulty::NORMAL: return Config::MULT_NORMAL; // x8
        case Difficulty::HARD: return Config::MULT_HARD;     // x20
    }
    return Config::MULT_NORMAL;
}

int GameLogic::getHighScoreForCurrentMode() const {
    int t = (int)currentTimeMode;
    int d = (int)currentDifficulty;
    if (t >= 0 && t < 3 && d >= 0 && d < 3) {
        return highScores[t][d];
    }
    return 0;
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
    blocksPlacedCount = 0;
    obstacleNotifyTimer = 0.0f;
    obstacleNotifyMessage = "";
    gameOverReason = "";
    isCheatMenuOpen = false;
    clearedLayers.clear();

    // Thiết lập thời gian theo chế độ
    if (currentTimeMode == TimeMode::MIN_3) {
        matchTimeRemaining = Config::TIME_LIMIT_3MIN;
    } else if (currentTimeMode == TimeMode::MIN_15) {
        matchTimeRemaining = Config::TIME_LIMIT_15MIN;
    } else {
        matchTimeRemaining = 0.0f; // Endless
    }
    matchTimeElapsed = 0.0f;

    // Thiết lập tốc độ rơi theo độ khó
    if (currentDifficulty == Difficulty::EASY) {
        gravityInterval = Config::GRAVITY_EASY;     // 2.0s (chậm 1/2)
    } else if (currentDifficulty == Difficulty::NORMAL) {
        gravityInterval = Config::GRAVITY_NORMAL;   // 1.0s (bằng hiện tại)
    } else {
        gravityInterval = Config::GRAVITY_HARD;     // 0.5s (nhanh gấp đôi)
    }
    gravityTimer = 0.0f;

    highScore = getHighScoreForCurrentMode();

    // Sinh 3 khối đầu tiên cho đợt 1
    rack.generateNewBatch();
    state = GameState::PLAYING;

    if (onPlaySound) onPlaySound(1); // Start sound
}

void GameLogic::loadHighScore() {
    for (int t = 0; t < 3; ++t) {
        for (int d = 0; d < 3; ++d) {
            highScores[t][d] = 0;
        }
    }
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        int val = 0;
        int idx = 0;
        while (file >> val && idx < 9) {
            highScores[idx / 3][idx % 3] = val;
            idx++;
        }
        file.close();
    }
    highScore = getHighScoreForCurrentMode();
}

void GameLogic::saveHighScore() {
    int t = (int)currentTimeMode;
    int d = (int)currentDifficulty;
    if (t >= 0 && t < 3 && d >= 0 && d < 3) {
        highScores[t][d] = highScore;
    }
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                file << highScores[i][j] << " ";
            }
            file << "\n";
        }
        file.close();
    }
}

int GameLogic::getCurrentMaxHeight() const {
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
        // Kiểm tra va chạm với ô đã có sẵn trong giếng
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

    // Cơ chế Wall-kick 3D
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
    if (success) {
        Block* active = rack.getActiveBlock();
        if (active && !isValidPosition(*active, active->pos)) {
            // Không thể đặt khối mới do sàn đã đầy lên đỉnh giếng
            state = GameState::GAME_OVER_HEIGHT;
            gameOverReason = "KHÔNG CÒN CHỖ TRỐNG ĐỂ SINH KHỐI MỚI (10/10)!";
            if (onPlaySound) onPlaySound(6);
            return false;
        }
        if (onPlaySound) onPlaySound(2); // Select sound
    }
    return success;
}

void GameLogic::spawnHardModeObstacle() {
    // CƠ CHẾ CHẾ ĐỘ KHÓ (Mỗi 15 khối):
    // Sinh 1 khối ngẫu nhiên từ các khối có sẵn (I, O, T, L, J, S, Z)
    // Đặt ở vị trí thấp nhất trên sàn, đảm bảo bên dưới không có ô trống
    // Ràng buộc quan trọng: Khối sinh ra KHÔNG ĐƯỢC TỰ HOÀN THÀNH TẦNG (không làm bất kỳ tầng nào đạt 25/25 ô)

    struct CandidatePlacement {
        Block block;
        Vec3i pos;
        int maxVy;
    };

    std::vector<CandidatePlacement> validCandidates;
    int minCandidateY = Config::MAX_HEIGHT + 1;

    // Thứ tự thử các hình dạng ngẫu nhiên (7 khối có sẵn)
    std::vector<int> shapes = {0, 1, 2, 3, 4, 5, 6};
    for (size_t i = 0; i < shapes.size(); ++i) {
        size_t j = i + rand() % (shapes.size() - i);
        std::swap(shapes[i], shapes[j]);
    }

    for (int shapeId : shapes) {
        Block baseBlock = BlockRack::createBlockByShape(shapeId);

        // Thử các biến thể xoay quanh trục Y (Yaw) và trục X (Pitch)
        std::vector<Block> rotatedVariants;
        for (int ry = 0; ry < 4; ++ry) {
            Block bY = baseBlock;
            for (int r = 0; r < ry; ++r) bY.rotate(RotationAxis::Y, true);
            rotatedVariants.push_back(bY);

            // Thử lật đứng theo trục X
            Block bX = bY;
            bX.rotate(RotationAxis::X, true);
            rotatedVariants.push_back(bX);
        }

        for (const auto& testBlock : rotatedVariants) {
            Vec3i sz = testBlock.getSize();
            if (sz.x > Config::GRID_X || sz.z > Config::GRID_Z || sz.y > Config::MAX_HEIGHT) continue;

            for (int px = 0; px <= Config::GRID_X - sz.x; ++px) {
                for (int pz = 0; pz <= Config::GRID_Z - sz.z; ++pz) {
                    for (int py = 0; py <= Config::MAX_HEIGHT - sz.y; ++py) {
                        Vec3i testPos(px, py, pz);
                        std::vector<Vec3i> worldVoxels = testBlock.getWorldVoxelsAt(testPos);

                        // 1. Kiểm tra va chạm với các ô đã có trên sàn và giới hạn trần
                        bool overlaps = false;
                        for (const auto& v : worldVoxels) {
                            if (v.y >= Config::MAX_HEIGHT || grid[v.x][v.y][v.z] != 0) {
                                overlaps = true;
                                break;
                            }
                        }
                        if (overlaps) continue;

                        // 2. Đảm bảo bên dưới không có ô trống:
                        // Với mỗi cột (v.x, v.z) của khối, các ô từ y=0 đến v.y-1 phải được lấp đầy
                        bool solidBelow = true;
                        for (const auto& v : worldVoxels) {
                            for (int yBelow = 0; yBelow < v.y; ++yBelow) {
                                bool inBlock = false;
                                for (const auto& other : worldVoxels) {
                                    if (other.x == v.x && other.z == v.z && other.y == yBelow) {
                                        inBlock = true;
                                        break;
                                    }
                                }
                                if (!inBlock && grid[v.x][yBelow][v.z] == 0) {
                                    solidBelow = false;
                                    break;
                                }
                            }
                            if (!solidBelow) break;
                        }
                        if (!solidBelow) continue;

                        // 3. RÀNG BUỘC: KHÔNG TỰ HOÀN THÀNH TẦNG
                        // Kiểm tra xem khối này có làm cho bất kỳ tầng nào đầy đủ 25/25 ô không
                        bool completesAnyLayer = false;
                        for (int y = 0; y < Config::MAX_HEIGHT; ++y) {
                            int currentCount = 0;
                            for (int x = 0; x < Config::GRID_X; ++x) {
                                for (int z = 0; z < Config::GRID_Z; ++z) {
                                    if (grid[x][y][z] != 0) currentCount++;
                                }
                            }
                            int addedCount = 0;
                            for (const auto& v : worldVoxels) {
                                if (v.y == y) addedCount++;
                            }
                            if (currentCount + addedCount >= Config::GRID_X * Config::GRID_Z) {
                                completesAnyLayer = true;
                                break;
                            }
                        }
                        if (completesAnyLayer) continue;

                        // Hợp lệ! Tính độ cao lớn nhất của khối này
                        int maxVy = 0;
                        for (const auto& v : worldVoxels) {
                            if (v.y > maxVy) maxVy = v.y;
                        }

                        if (maxVy < minCandidateY) {
                            minCandidateY = maxVy;
                            validCandidates.clear();
                            validCandidates.push_back({testBlock, testPos, maxVy});
                        } else if (maxVy == minCandidateY) {
                            validCandidates.push_back({testBlock, testPos, maxVy});
                        }

                        // Vì py duyệt từ thấp lên cao (0 -> MAX), py đầu tiên hợp lệ chính là vị trí thấp nhất cho (px, pz) này
                        break;
                    }
                }
            }
        }

        if (!validCandidates.empty()) break; // Đã tìm được khối hợp lệ ở độ cao thấp nhất
    }

    if (!validCandidates.empty()) {
        int idx = rand() % (int)validCandidates.size();
        const auto& chosen = validCandidates[idx];
        std::vector<Vec3i> worldVoxels = chosen.block.getWorldVoxelsAt(chosen.pos);

        // Ghi các ô của khối chướng ngại vật ngẫu nhiên vào sàn
        for (const auto& v : worldVoxels) {
            grid[v.x][v.y][v.z] = 99; // ID chướng ngại vật
            gridColors[v.x][v.y][v.z] = Color4f(0.95f, 0.28f, 0.35f, 1.0f); // Soft Ruby Red
        }

        obstacleNotifyTimer = 3.5f;
        obstacleNotifyMessage = "CHƯỚNG NGẠI VẬT! (Cứ 15 khối sinh 1 khối " + chosen.block.name + " ở đáy)";

        if (onPlaySound) onPlaySound(7); // Âm thanh cảnh báo chướng ngại vật
        // LƯU Ý: Tuyệt đối KHÔNG gọi checkFullLayers() vì khối này đã đảm bảo không tự hoàn thành tầng
    }
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

            // KIỂM TRA ĐIỀU KIỆN THUA: Nếu khối vượt quá chiều cao tối đa 10
            if (v.y >= Config::MAX_HEIGHT) {
                heightExceeded = true;
            }
        }
    }

    // Mỗi khối đặt thành công: 5 điểm cơ bản + bonus dựa vào độ khó (5 * multiplier)
    int placeScore = Config::BASE_SCORE_PLACE_BLOCK * getScoreMultiplier();
    score += placeScore;
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }

    if (onPlaySound) onPlaySound(4); // Lock block sound

    // Đánh dấu khối này trong khay là đã sử dụng
    rack.markActiveAsUsed();

    // Nếu xếp vượt quá đỉnh giếng (v.y >= 10) -> Xử thua ngay lập tức!
    if (heightExceeded) {
        state = GameState::GAME_OVER_HEIGHT;
        gameOverReason = "ĐỘ CAO VƯỢT QUÁ 10 TẦNG! (HEIGHT OVERFLOW)";
        if (onPlaySound) onPlaySound(6); // Game over sound
        return;
    }

    // CƠ CHẾ CHẾ ĐỘ KHÓ: Cứ mỗi 15 khối được đặt thành công, sinh 1 khối ngẫu nhiên ở vị trí thấp nhất
    blocksPlacedCount++;
    if (currentDifficulty == Difficulty::HARD && (blocksPlacedCount % Config::HARD_MODE_OBSTACLE_INTERVAL == 0)) {
        spawnHardModeObstacle();
    }

    // Kiểm tra các tầng đầy (full 5x5 = 25 ô)
    checkFullLayers();

    // Nếu không có tầng nào được xóa
    if (clearedLayers.empty()) {
        // KIỂM TRA ĐIỀU KIỆN THUA: Nếu độ cao đã chạm ngưỡng 10/10 mà không xóa được tầng
        if (getCurrentMaxHeight() >= Config::MAX_HEIGHT) {
            state = GameState::GAME_OVER_HEIGHT;
            gameOverReason = "CÁC KHỐI ĐÃ ĐẠT ĐỘ CAO TỐI ĐA 10/10! (GAME OVER)";
            if (onPlaySound) onPlaySound(6);
            return;
        }

        // Nếu tất cả 3 khối trong khay đã đặt xong -> Thưởng hoàn thành đợt và sinh 3 khối mới
        if (rack.areAllUsed()) {
            score += Config::BASE_SCORE_ROUND_BONUS * getScoreMultiplier();
            if (score > highScore) {
                highScore = score;
                saveHighScore();
            }
            roundCount++;
            rack.generateNewBatch();
            if (onPlaySound) onPlaySound(5); // Round clear sound
        }

        // KIỂM TRA VA CHẠM KHỐI MỚI SINH RA (SPAWN COLLISION)
        // Đảm bảo khối mới không bao giờ sinh ra chồng đè lên khối cũ
        Block* nextActive = rack.getActiveBlock();
        if (nextActive && !isValidPosition(*nextActive, nextActive->pos)) {
            state = GameState::GAME_OVER_HEIGHT;
            gameOverReason = "KHÔNG CÒN CHỖ TRỐNG ĐỂ SINH KHỐI MỚI! (BLOCKED)";
            if (onPlaySound) onPlaySound(6);
            return;
        }
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
        clearAnimTimer = 0.40f;
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

    // Tính điểm thưởng xóa tầng: mặc định 100 điểm + bonus thêm điểm dựa vào độ khó
    int baseLayerScore = 0;
    if (count == 1) baseLayerScore = Config::BASE_SCORE_CLEAR_1_LAYER;       // 100
    else if (count == 2) baseLayerScore = Config::BASE_SCORE_CLEAR_2_LAYERS; // 250
    else if (count == 3) baseLayerScore = Config::BASE_SCORE_CLEAR_3_LAYERS; // 500
    else baseLayerScore = Config::BASE_SCORE_CLEAR_4_LAYERS;                 // 1000

    int points = baseLayerScore * getScoreMultiplier();
    score += points;
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }

    clearedLayers.clear();

    // KIỂM TRA ĐỘ CAO SAU KHI XÓA TẦNG
    if (getCurrentMaxHeight() >= Config::MAX_HEIGHT) {
        state = GameState::GAME_OVER_HEIGHT;
        gameOverReason = "CÁC KHỐI ĐÃ ĐẠT ĐỘ CAO TỐI ĐA 10/10! (GAME OVER)";
        if (onPlaySound) onPlaySound(6);
        return;
    }

    // Kiểm tra xem đã dùng hết 3 khối trong khay chưa
    if (rack.areAllUsed()) {
        score += Config::BASE_SCORE_ROUND_BONUS * getScoreMultiplier();
        if (score > highScore) {
            highScore = score;
            saveHighScore();
        }
        roundCount++;
        rack.generateNewBatch();
    }

    // Kiểm tra tính hợp lệ khi sinh khối mới
    Block* nextActive = rack.getActiveBlock();
    if (nextActive && !isValidPosition(*nextActive, nextActive->pos)) {
        state = GameState::GAME_OVER_HEIGHT;
        gameOverReason = "KHÔNG CÒN CHỖ TRỐNG ĐỂ SINH KHỐI MỚI! (BLOCKED)";
        if (onPlaySound) onPlaySound(6);
        return;
    }

    state = GameState::PLAYING;
}

void GameLogic::update(float dt) {
    if (isCheatMenuOpen) {
        // Tạm dừng cập nhật thời gian và rơi khối khi đang mở Menu Cheat để người chơi demo
        return;
    }

    if (obstacleNotifyTimer > 0.0f) {
        obstacleNotifyTimer -= dt;
        if (obstacleNotifyTimer <= 0.0f) {
            obstacleNotifyTimer = 0.0f;
            obstacleNotifyMessage = "";
        }
    }

    if (state == GameState::CLEARING_ANIMATION) {
        clearAnimTimer -= dt;
        if (clearAnimTimer <= 0.0f) {
            applyClearLayers();
        }
        return;
    }

    if (state != GameState::PLAYING) return;

    // Cập nhật hoạt họa khay 3 khối
    rack.updateAnimation(dt);

    // XỬ LÝ THỜI GIAN THEO CHẾ ĐỘ CHƠI
    if (currentTimeMode == TimeMode::ENDLESS) {
        matchTimeElapsed += dt; // Vô tận: đếm xuôi thời gian, không bao giờ thua do hết giờ
    } else {
        matchTimeRemaining -= dt;
        if (matchTimeRemaining <= 0.0f) {
            matchTimeRemaining = 0.0f;
            state = GameState::GAME_OVER_TIMEOUT;
            gameOverReason = "HẾT THỜI GIAN LƯỢT CHƠI! (TIMEOUT)";
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

void GameLogic::toggleCheatMenu() {
    isCheatMenuOpen = !isCheatMenuOpen;
}

void GameLogic::cheatStack10GameOver() {
    // 1. Chồng các khối lên kín 10 tầng (y = 0..9) và xử thua Game Over ngay
    for (int y = 0; y < Config::MAX_HEIGHT; ++y) {
        for (int x = 1; x <= 3; ++x) {
            for (int z = 1; z <= 3; ++z) {
                grid[x][y][z] = (y % 7) + 1;
                gridColors[x][y][z] = Block::getTetrisColor(y % 7);
            }
        }
    }
    isCheatMenuOpen = false;
    state = GameState::GAME_OVER_HEIGHT;
    gameOverReason = "CÁC KHỐI ĐÃ ĐẠT ĐỘ CAO TỐI ĐA 10/10! (DEMO THUA)";
    if (onPlaySound) onPlaySound(6);
}

void GameLogic::cheatStack9Tiers() {
    // 2. Dựng sẵn tháp 9 tầng (y = 0..8), chừa tầng 10 để người chơi tự thả 1 khối chạm trần 10/10
    std::memset(grid, 0, sizeof(grid));
    for (int x = 0; x < Config::GRID_X; ++x) {
        for (int y = 0; y < Config::MAX_HEIGHT + 6; ++y) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                gridColors[x][y][z] = Color4f(0, 0, 0, 0);
            }
        }
    }
    for (int y = 0; y < Config::MAX_HEIGHT - 1; ++y) {
        for (int x = 1; x <= 3; ++x) {
            for (int z = 1; z <= 3; ++z) {
                grid[x][y][z] = (y % 7) + 1;
                gridColors[x][y][z] = Block::getTetrisColor(y % 7);
            }
        }
    }
    if (state != GameState::PLAYING) {
        state = GameState::PLAYING;
    }
    isCheatMenuOpen = false;
    obstacleNotifyTimer = 3.5f;
    obstacleNotifyMessage = "ĐÃ DỰNG THÁP 9 TẦNG! (Thả thêm 1 khối để chạm trần 10/10 demo thua)";
    if (onPlaySound) onPlaySound(2);
}

void GameLogic::cheatClearLayerInstant() {
    // 3. Lấp đầy 25/25 ô ở tầng thấp nhất còn trống để kích hoạt tự xóa tầng và cộng điểm
    if (state != GameState::PLAYING) {
        state = GameState::PLAYING;
    }
    int targetY = 0;
    for (int y = 0; y < Config::MAX_HEIGHT; ++y) {
        bool hasEmpty = false;
        for (int x = 0; x < Config::GRID_X; ++x) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                if (grid[x][y][z] == 0) { hasEmpty = true; break; }
            }
            if (hasEmpty) break;
        }
        if (hasEmpty) { targetY = y; break; }
    }
    for (int x = 0; x < Config::GRID_X; ++x) {
        for (int z = 0; z < Config::GRID_Z; ++z) {
            grid[x][targetY][z] = 1;
            gridColors[x][targetY][z] = Block::getTetrisColor(0); // Soft Cyan
        }
    }
    isCheatMenuOpen = false;
    checkFullLayers();
}

void GameLogic::cheatFillLayerAlmost() {
    // 4. Lấp 24/25 ô tầng 0 (chừa ô tâm x=2, z=2) để người chơi tự đặt khối vào xóa tầng
    if (state != GameState::PLAYING) {
        state = GameState::PLAYING;
    }
    for (int x = 0; x < Config::GRID_X; ++x) {
        for (int z = 0; z < Config::GRID_Z; ++z) {
            if (x == 2 && z == 2) {
                grid[x][0][z] = 0;
                gridColors[x][0][z] = Color4f(0, 0, 0, 0);
            } else {
                grid[x][0][z] = 2;
                gridColors[x][0][z] = Block::getTetrisColor(1); // Warm Amber
            }
        }
    }
    isCheatMenuOpen = false;
    obstacleNotifyTimer = 3.5f;
    obstacleNotifyMessage = "ĐÃ LẤP 24/25 Ô TẦNG ĐÁY! (Chừa ô tâm x=2, z=2 để bạn tự xếp khối xóa tầng)";
    if (onPlaySound) onPlaySound(2);
}

void GameLogic::cheatCycleDifficulty() {
    // 5. Xoay vòng độ khó: Dễ -> Vừa -> Khó
    if (currentDifficulty == Difficulty::EASY) {
        setDifficulty(Difficulty::NORMAL);
    } else if (currentDifficulty == Difficulty::NORMAL) {
        setDifficulty(Difficulty::HARD);
    } else {
        setDifficulty(Difficulty::EASY);
    }
    obstacleNotifyTimer = 2.5f;
    obstacleNotifyMessage = std::string("ĐÃ ĐỔI ĐỘ KHÓ: ") + getDifficultyName(currentDifficulty) +
                            " (Hệ số x" + std::to_string(getScoreMultiplier()) + ")";
    if (onPlaySound) onPlaySound(2);
}

void GameLogic::cheatTrigger15Obstacle() {
    // 6. Demo ngay cơ chế 15 khối: sinh khối ngẫu nhiên từ 7 khối có sẵn ở đáy (không tự hoàn thành tầng)
    if (state != GameState::PLAYING) {
        state = GameState::PLAYING;
    }
    blocksPlacedCount += 15;
    spawnHardModeObstacle();
    isCheatMenuOpen = false;
}

void GameLogic::cheatSetTime5Seconds() {
    // 7. Chỉnh thời gian còn 5 giây để demo hết giờ (Timeout)
    if (state != GameState::PLAYING) {
        state = GameState::PLAYING;
    }
    if (currentTimeMode == TimeMode::ENDLESS) {
        currentTimeMode = TimeMode::MIN_3;
    }
    matchTimeRemaining = 5.0f;
    isCheatMenuOpen = false;
    obstacleNotifyTimer = 2.5f;
    obstacleNotifyMessage = "ĐÃ ĐẶT THỜI GIAN CÒN 5 GIÂY! (Chuẩn bị demo Timeout)";
    if (onPlaySound) onPlaySound(2);
}

void GameLogic::cheatAddScore(int amount) {
    // 8. Cộng điểm nhanh
    score += amount;
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }
    obstacleNotifyTimer = 2.5f;
    obstacleNotifyMessage = std::string("ĐÃ CỘNG +") + std::to_string(amount) + " ĐIỂM THƯỞNG!";
    if (onPlaySound) onPlaySound(4);
}

void GameLogic::cheatClearAllGrid() {
    // 9. Dọn sạch toàn bộ sàn lưới 5x5
    std::memset(grid, 0, sizeof(grid));
    for (int x = 0; x < Config::GRID_X; ++x) {
        for (int y = 0; y < Config::MAX_HEIGHT + 6; ++y) {
            for (int z = 0; z < Config::GRID_Z; ++z) {
                gridColors[x][y][z] = Color4f(0, 0, 0, 0);
            }
        }
    }
    totalLayersCleared = 0;
    if (state == GameState::GAME_OVER_HEIGHT || state == GameState::GAME_OVER_TIMEOUT) {
        state = GameState::PLAYING;
    }
    isCheatMenuOpen = false;
    obstacleNotifyTimer = 2.5f;
    obstacleNotifyMessage = "ĐÃ XÓA SẠCH SÀN CHƠI! (Lưới 5x5 đã trống hoàn toàn)";
    if (onPlaySound) onPlaySound(2);
}
