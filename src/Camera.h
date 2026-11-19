#pragma once
#include "Types.h"
#include "Config.h"

class Camera {
public:
    Vec3 target;         // Tâm trục quay ở giữa sàn (x=2.5, y=5.0, z=2.5)
    float yaw;           // Góc quay quanh trục Y (độ)
    float pitch;         // Góc nghiêng trên/dưới (độ)
    float distance;      // Khoảng cách từ camera tới tâm trục

    // Giá trị đích để chuyển động mượt mà (Smooth Damping)
    float targetYaw;
    float targetPitch;
    float targetDistance;

    // Độ nhạy chuột
    float mouseSensitivity;
    float zoomSensitivity;

    Camera();

    // Cập nhật nội suy làm mượt camera mỗi frame (khắc phục lỗi lộn ngược khi qua mốc 0/360 độ)
    void update(float dt);

    // Kéo chuột để xoay quanh trục ở giữa
    void onMouseDrag(float deltaX, float deltaY);

    // Cuộn chuột để phóng to / thu nhỏ
    void onMouseWheel(float wheelDelta);

    // Quay nhanh góc yaw một lượng deltaAngle (ví dụ: +90 hoặc -90 độ)
    void rotateYawBy(float deltaAngle);

    // Đặt lại các góc nhìn chuẩn
    void setPresetIsometric();
    void setPresetTopDown();
    void setPresetFront();

    // Tính toán tọa độ mắt (Eye Position) của Camera
    Vec3 getEyePosition() const;

    // Thiết lập ma trận góc nhìn OpenGL (gluLookAt)
    void applyView() const;
};
