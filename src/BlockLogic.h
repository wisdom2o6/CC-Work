#pragma once
#include "Types.h"
#include "Config.h"
#include <vector>
#include <string>

// Enum hướng xoay 3D
enum class RotationAxis {
    X, // Xoay quanh trục X (dọc / pitch)
    Y, // Xoay quanh trục Y (ngang / yaw)
    Z  // Xoay quanh trục Z (nghiêng / roll)
};

// Cấu trúc một khối 3D (Polycube Block)
struct Block {
    int shapeId;                  // Mã hình dạng
    int colorId;                  // Mã màu
    Color4f color;                // Màu sắc hiển thị
    std::string name;             // Tên mô tả (I, L, T, Corner-3D, ...)
    std::vector<Vec3i> voxels;    // Tọa độ cục bộ các ô lập phương tạo nên khối
    Vec3i pos;                    // Vị trí mỏ neo trên sàn 5x5x10

    Block();
    Block(int shapeId, int colorId, const Color4f& color, const std::string& name, const std::vector<Vec3i>& voxels);

    // Xoay khối 90 độ quanh trục chỉ định (X, Y, hoặc Z) và chuẩn hóa về gốc (0,0,0)
    void rotate(RotationAxis axis, bool clockwise = true);

    // Tính kích thước khối sau xoay (kích thước theo X, Y, Z)
    Vec3i getSize() const;

    // Lấy danh sách tọa độ thế giới (world coordinates) của tất cả các voxel
    std::vector<Vec3i> getWorldVoxels() const;

    // Lấy tọa độ thế giới với độ lệch vị trí (dùng để kiểm tra va chạm / ghost)
    std::vector<Vec3i> getWorldVoxelsAt(const Vec3i& testPos) const;

    // Chuẩn hóa tọa độ cục bộ sao cho min(x)=0, min(y)=0, min(z)=0
    void normalize();
};

// Cấu trúc một ô trong khay 3 khối (Rack Slot)
struct RackSlot {
    Block block;
    bool used;               // Đã xếp vào sàn chưa
    float previewRotY;       // Góc xoay hiển thị hoạt họa 3D trong khay
    float previewRotX;
};

// Quản lý khay 3 khối mỗi đợt
class BlockRack {
public:
    RackSlot slots[Config::BLOCKS_PER_ROUND];
    int activeSlotIndex;     // Ô đang chọn (0, 1 hoặc 2)

    BlockRack();

    // Sinh ngẫu nhiên 3 khối mới cho đợt chơi mới
    void generateNewBatch();

    // Kiểm tra xem cả 3 khối đã được đặt hết chưa
    bool areAllUsed() const;

    // Đếm số khối còn lại chưa đặt (0..3)
    int getRemainingCount() const;

    // Lấy con trỏ tới khối đang được chọn để điều khiển
    Block* getActiveBlock();
    const Block* getActiveBlock() const;

    // Chọn ô (0, 1, 2)
    bool selectSlot(int index);

    // Đánh dấu ô hiện tại là đã dùng và tự động chuyển sang ô chưa dùng tiếp theo
    void markActiveAsUsed();

    // Cập nhật góc quay tự động cho các khối trong khay để vẽ đẹp mắt
    void updateAnimation(float dt);

    // Sinh 1 khối ngẫu nhiên
    static Block createRandomBlock();
};
