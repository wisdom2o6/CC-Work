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
            case RotationAxis::X: // Xoay quanh trục X
                if (clockwise) {
                    v.y = -z;
                    v.z = y;
                } else {
                    v.y = z;
                    v.z = -y;
                }
                break;
            case RotationAxis::Y: // Xoay quanh trục Y
                if (clockwise) {
                    v.x = z;
                    v.z = -x;
                } else {
                    v.x = -z;
                    v.z = x;
                }
                break;
            case RotationAxis::Z: // Xoay quanh trục Z
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
// RANDOM BLOCK GENERATION
// ============================================================================

// Bảng màu 20 màu rực rỡ phong cách Cyberpunk / Neon cho các khối 3D
static const Color4f PALETTE[] = {
    Color4f(0.00f, 0.90f, 1.00f), // 0: Neon Cyan (Thanh dài I)
    Color4f(1.00f, 0.75f, 0.00f), // 1: Neon Amber (Khối vuông O)
    Color4f(1.00f, 0.45f, 0.00f), // 2: Bright Orange (Khối chữ L)
    Color4f(0.20f, 0.50f, 1.00f), // 3: Electric Blue (Khối chữ J)
    Color4f(0.75f, 0.15f, 1.00f), // 4: Neon Purple (Khối chữ T)
    Color4f(0.10f, 0.90f, 0.40f), // 5: Emerald Green (Khối chữ Z)
    Color4f(1.00f, 0.15f, 0.35f), // 6: Ruby Crimson (Góc 3D Tripod - Cao 2)
    Color4f(1.00f, 0.20f, 0.70f), // 7: Hot Pink (Nhánh xoắn 3D - Cao 2)
    Color4f(0.40f, 1.00f, 0.20f), // 8: Lime Green (Góc nhỏ 3 ô)
    Color4f(0.00f, 0.85f, 0.75f), // 9: Aqua Mint (Thanh ngắn 3 ô)
    Color4f(1.00f, 0.90f, 0.15f), // 10: Cyber Yellow (Bậc Thang 2 Tầng - Cao 2)
    Color4f(1.00f, 0.35f, 0.15f), // 11: Flame Orange (Cổng Vòm 2 Tầng - Cao 2)
    Color4f(0.85f, 0.15f, 0.90f), // 12: Magenta Neon (Chữ U Đứng 2 Tầng - Cao 2)
    Color4f(0.15f, 0.85f, 0.95f), // 13: Sky Blue (Chữ T Đứng 2 Tầng - Cao 2)
    Color4f(0.95f, 0.60f, 0.10f), // 14: Golden Amber (Tường Góc 2 Tầng - Cao 2)
    Color4f(0.95f, 0.30f, 0.65f), // 15: Neon Rose (Rắn Xoắn 2 Tầng - Cao 2)
    Color4f(0.60f, 0.40f, 1.00f), // 16: Deep Lavender (Kim Tự Tháp 2 Tầng - Cao 2)
    Color4f(0.25f, 0.95f, 0.65f), // 17: Mint Neon (Chữ Z Đứng 2 Tầng - Cao 2)
    Color4f(1.00f, 0.25f, 0.45f), // 18: Radiant Coral (Cột Cao 3 Tầng - Cao 3)
    Color4f(0.45f, 0.85f, 1.00f)  // 19: Ice Blue (Chữ L Đứng 3 Tầng - Cao 3)
};

Block BlockRack::createRandomBlock() {
    int shape = rand() % 20;
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
        case 2: // Khối chữ L
            name = "Chữ L (4 ô)";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {2,0,1} };
            break;
        case 3: // Khối chữ J
            name = "Chữ J (4 ô)";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {0,0,1} };
            break;
        case 4: // Khối chữ T
            name = "Chữ T (4 ô)";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {1,0,1} };
            break;
        case 5: // Khối chữ Z (Zigzag)
            name = "Chữ Z (4 ô)";
            v = { {0,0,0}, {1,0,0}, {1,0,1}, {2,0,1} };
            break;
        case 6: // Khối góc 3D Tripod (Cao 2 tầng)
            name = "Góc 3D Tripod";
            v = { {0,0,0}, {1,0,0}, {0,1,0}, {0,0,1} };
            break;
        case 7: // Khối nhánh xoắn 3D (Cao 2 tầng)
            name = "Nhánh Xoắn 3D";
            v = { {0,0,0}, {1,0,0}, {1,1,0}, {1,1,1} };
            break;
        case 8: // Góc phẳng 3 ô
            name = "Góc Nhỏ 3 ô";
            v = { {0,0,0}, {1,0,0}, {0,0,1} };
            break;
        case 9: // Thanh 3 ô
            name = "Thanh Ngắn 3 ô";
            v = { {0,0,0}, {1,0,0}, {2,0,0} };
            break;

        // --- CÁC KHỐI CAO 2 TẦNG & TĂNG ĐỘ KHÓ KHÔNG GIAN 3D ---
        case 10: // Bậc thang 2 tầng
            name = "Bậc Thang 2 Tầng";
            v = { {0,0,0}, {1,0,0}, {1,1,0}, {2,1,0} };
            break;
        case 11: // Cổng vòm 2 tầng (có lỗ hổng 1 ô ở đáy)
            name = "Cổng Vòm 2 Tầng";
            v = { {0,0,0}, {0,1,0}, {1,1,0}, {2,1,0}, {2,0,0} };
            break;
        case 12: // Khối chữ U ngửa cao 2 tầng
            name = "Chữ U Đứng 2 Tầng";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {0,1,0}, {2,1,0} };
            break;
        case 13: // Khối chữ T đứng có trụ nhô lên tầng 2
            name = "Chữ T Đứng 2 Tầng";
            v = { {0,0,0}, {1,0,0}, {2,0,0}, {1,1,0} };
            break;
        case 14: // Bức tường góc chữ L cao 2 tầng
            name = "Tường Góc 2 Tầng";
            v = { {0,0,0}, {1,0,0}, {0,0,1}, {0,1,0}, {0,1,1} };
            break;
        case 15: // Khối rắn uốn lượn 3D qua 2 tầng
            name = "Rắn Xoắn 2 Tầng";
            v = { {0,0,0}, {1,0,0}, {1,1,0}, {1,1,1}, {2,1,1} };
            break;
        case 16: // Khối kim tự tháp (đế 2x2, đỉnh 1 ô tầng 2)
            name = "Kim Tự Tháp 2 Tầng";
            v = { {0,0,0}, {1,0,0}, {0,0,1}, {1,0,1}, {0,1,0} };
            break;
        case 17: // Chữ Z uốn vuông góc lên tầng trên
            name = "Chữ Z Đứng 2 Tầng";
            v = { {0,0,0}, {1,0,0}, {1,1,0}, {1,1,1} };
            break;

        // --- CÁC KHỐI CAO 3 TẦNG SIÊU THÁCH THỨC ---
        case 18: // Cột tháp đứng thẳng 3 tầng
            name = "Cột Cao 3 Tầng";
            v = { {0,0,0}, {0,1,0}, {0,2,0} };
            break;
        case 19: // Chữ L đứng có trụ cao 3 tầng
            name = "Chữ L Đứng 3 Tầng";
            v = { {0,0,0}, {1,0,0}, {0,1,0}, {0,2,0} };
            break;
    }

    int colorId = shape;
    Color4f col = PALETTE[colorId % 20];
    Block b(shape, colorId + 1, col, name, v);

    // Căn giữa vị trí ban đầu trên đỉnh sàn (sàn 5x5, tọa độ x: 0..4, z: 0..4)
    Vec3i size = b.getSize();
    b.pos.x = std::max(0, (Config::GRID_X - size.x) / 2);
    b.pos.z = std::max(0, (Config::GRID_Z - size.z) / 2);
    b.pos.y = Config::MAX_HEIGHT - size.y; // Đặt ở gần đỉnh giếng

    return b;
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
        slots[i].previewRotX = 20.0f;
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
            // Đảm bảo khối được căn giữa theo X và Z khi vừa chọn
            Block* b = &slots[activeSlotIndex].block;
            Vec3i sz = b->getSize();
            b->pos.x = std::max(0, (Config::GRID_X - sz.x) / 2);
            b->pos.z = std::max(0, (Config::GRID_Z - sz.z) / 2);
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
            slots[i].previewRotY += 45.0f * dt;
            if (slots[i].previewRotY >= 360.0f) {
                slots[i].previewRotY -= 360.0f;
            }
        }
    }
}
