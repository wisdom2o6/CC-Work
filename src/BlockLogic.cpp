#include "BlockLogic.h"
#include <cstdlib>
#include <algorithm>
#include <climits>

// ============================================================================
// BLOCK IMPLEMENTATION
// ============================================================================

Block::Block() : shapeId(0), colorId(0), color(1.0f, 1.0f, 1.0f), name("None"), pos(0, 0, 0) {}

Block::Block(int sId, int cId, const Color4f& col, const std::string& n, const std::vector<Vec3i>& v)
    : shapeId(sId), colorId(cId), color(col), name(n), voxels(v), pos(0, 0, 0) {
    normalize();
}

void Block::normalize() {
    if (voxels.empty()) return;
    int minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX;
    for (const auto& v : voxels) {
        if (v.x < minX) minX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.z < minZ) minZ = v.z;
    }
    for (auto& v : voxels) {
        v.x -= minX;
        v.y -= minY;
        v.z -= minZ;
    }
}

void Block::rotate(RotationAxis axis, bool clockwise) {
    for (auto& v : voxels) {
        int x = v.x, y = v.y, z = v.z;
        switch (axis) {
            case RotationAxis::X: // Xoay dọc quanh trục X (Pitch)
                if (clockwise) {
                    v.y = -z;
                    v.z = y;
                } else {
                    v.y = z;
                    v.z = -y;
                }
                break;
            case RotationAxis::Y: // Xoay ngang quanh trục Y (Yaw)
                if (clockwise) {
                    v.x = z;
                    v.z = -x;
                } else {
                    v.x = -z;
                    v.z = x;
                }
                break;
            case RotationAxis::Z: // Xoay nghiêng quanh trục Z (Roll)
                if (clockwise) {
                    v.x = -y;
                    v.y = x;
                } else {
                    v.x = y;
                    v.y = -x;
                }
                break;
        }
    }
    normalize();
}

Vec3i Block::getSize() const {
    if (voxels.empty()) return Vec3i(0, 0, 0);
    int maxX = 0, maxY = 0, maxZ = 0;
    for (const auto& v : voxels) {
        if (v.x > maxX) maxX = v.x;
        if (v.y > maxY) maxY = v.y;
        if (v.z > maxZ) maxZ = v.z;
    }
    return Vec3i(maxX + 1, maxY + 1, maxZ + 1);
}

std::vector<Vec3i> Block::getWorldVoxels() const {
    return getWorldVoxelsAt(pos);
}

std::vector<Vec3i> Block::getWorldVoxelsAt(const Vec3i& testPos) const {
    std::vector<Vec3i> res;
    res.reserve(voxels.size());
    for (const auto& v : voxels) {
        res.push_back(testPos + v);
    }
    return res;
}

// ============================================================================
// RANDOM BLOCK GENERATION - 7 KHỐI TETRIS KINH ĐIỂN (1 Ô CAO MẶC ĐỊNH)
// ============================================================================

// Bảng màu sắc hiện đại, dịu mắt (không chói gắt), dễ quan sát
static const Color4f TETRIS_PALETTE[] = {
    Color4f(0.28f, 0.78f, 0.92f), // 0: Soft Cyan (Thanh I)
    Color4f(0.96f, 0.74f, 0.22f), // 1: Warm Amber (Khối vuông O)
    Color4f(0.66f, 0.42f, 0.90f), // 2: Soft Purple (Khối chữ T)
    Color4f(0.96f, 0.54f, 0.22f), // 3: Soft Orange (Khối chữ L)
    Color4f(0.30f, 0.62f, 0.94f), // 4: Soft Blue (Khối chữ J)
    Color4f(0.22f, 0.82f, 0.52f), // 5: Soft Mint (Khối chữ S)
    Color4f(0.94f, 0.38f, 0.42f)  // 6: Soft Coral (Khối chữ Z)
};

Color4f Block::getTetrisColor(int colorId) {
    if (colorId < 0) colorId = 0;
    return TETRIS_PALETTE[colorId % 7];
}

Block BlockRack::createBlockByShape(int shape) {
    if (shape < 0 || shape >= 7) shape = 0;
    std::vector<Vec3i> v;
    std::string name;

    switch (shape) {
        case 0: // Thanh dài 4 ô I
            name = "Thanh I (4 ô)";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {3,0,0} };
            break;
        case 1: // Khối vuông phẳng 2x2 O
            name = "Vuông O (2x2)";
            v = { {0,0,0}, {1,0,0}, {0,0,1}, {1,0,1} };
            break;
        case 2: // Khối chữ T
            name = "Chữ T (4 ô)";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {1,0,1} };
            break;
        case 3: // Khối chữ L
            name = "Chữ L (4 ô)";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {2,0,1} };
            break;
        case 4: // Khối chữ J
            name = "Chữ J (4 ô)";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {0,0,1} };
            break;
        case 5: // Khối chữ S (Zigzag phải)
            name = "Chữ S (4 ô)";
            v = { {1,0,0}, {2,0,0}, {0,0,1}, {1,0,1} };
            break;
        case 6: // Khối chữ Z (Zigzag trái)
            name = "Chữ Z (4 ô)";
            v = { {0,0,0}, {1,0,0}, {1,0,1}, {2,0,1} };
            break;
    }

    int colorId = shape + 1;
    Color4f col = TETRIS_PALETTE[shape % 7];
    Block b(shape, colorId, col, name, v);

    // Căn giữa vị trí ban đầu trên đỉnh sàn (sàn 5x5, tọa độ x: 0..4, z: 0..4)
    Vec3i size = b.getSize();
    b.pos.x = std::max(0, (Config::GRID_X - size.x) / 2);
    b.pos.z = std::max(0, (Config::GRID_Z - size.z) / 2);
    b.pos.y = Config::MAX_HEIGHT - size.y; // Đặt ở đỉnh giếng (y = 9 cho khối cao 1 ô)

    return b;
}

Block BlockRack::createRandomBlock() {
    return createBlockByShape(rand() % 7);
}

// ============================================================================
// BLOCK RACK IMPLEMENTATION
// ============================================================================

BlockRack::BlockRack() : activeSlotIndex(0) {
    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        slots[i].used = true;
        slots[i].previewRotY = 0.0f;
        slots[i].previewRotX = 15.0f;
    }
}

void BlockRack::generateNewBatch() {
    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        slots[i].block = createRandomBlock();
        slots[i].used = false;
        slots[i].previewRotY = (float)(rand() % 360);
        slots[i].previewRotX = 18.0f;
    }
    // Chọn khối đầu tiên còn khả dụng
    activeSlotIndex = 0;
}

bool BlockRack::areAllUsed() const {
    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        if (!slots[i].used) return false;
    }
    return true;
}

int BlockRack::getRemainingCount() const {
    int count = 0;
    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        if (!slots[i].used) count++;
    }
    return count;
}

Block* BlockRack::getActiveBlock() {
    if (activeSlotIndex >= 0 && activeSlotIndex < Config::BLOCKS_PER_ROUND) {
        if (!slots[activeSlotIndex].used) {
            return &slots[activeSlotIndex].block;
        }
    }
    return nullptr;
}

const Block* BlockRack::getActiveBlock() const {
    if (activeSlotIndex >= 0 && activeSlotIndex < Config::BLOCKS_PER_ROUND) {
        if (!slots[activeSlotIndex].used) {
            return &slots[activeSlotIndex].block;
        }
    }
    return nullptr;
}

bool BlockRack::selectSlot(int index) {
    if (index >= 0 && index < Config::BLOCKS_PER_ROUND) {
        if (!slots[index].used) {
            activeSlotIndex = index;
            // Căn giữa khối theo X và Z khi vừa chọn
            Block* b = &slots[activeSlotIndex].block;
            Vec3i sz = b->getSize();
            b->pos.x = std::max(0, (Config::GRID_X - sz.x) / 2);
            b->pos.z = std::max(0, (Config::GRID_Z - sz.z) / 2);
            b->pos.y = Config::MAX_HEIGHT - sz.y;
            return true;
        }
    }
    return false;
}

void BlockRack::markActiveAsUsed() {
    if (activeSlotIndex >= 0 && activeSlotIndex < Config::BLOCKS_PER_ROUND) {
        slots[activeSlotIndex].used = true;
    }
    // Tự động nhảy sang ô chưa dùng kế tiếp
    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        int next = (activeSlotIndex + 1 + i) % Config::BLOCKS_PER_ROUND;
        if (!slots[next].used) {
            selectSlot(next);
            return;
        }
    }
}

void BlockRack::updateAnimation(float dt) {
    for (int i = 0; i < Config::BLOCKS_PER_ROUND; ++i) {
        if (!slots[i].used) {
            slots[i].previewRotY += 35.0f * dt;
            if (slots[i].previewRotY >= 360.0f) {
                slots[i].previewRotY -= 360.0f;
            }
        }
    }
}
