#include "Camera.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <algorithm>

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG2RAD = PI / 180.0f;

Camera::Camera()
    : target(2.5f, 5.0f, 2.5f), // Tâm hình học chính xác của sàn 5x5 và chiều cao 10
      yaw(45.0f),
      pitch(28.0f),
      distance(17.5f),
      targetYaw(45.0f),
      targetPitch(28.0f),
      targetDistance(17.5f),
      mouseSensitivity(0.32f),
      zoomSensitivity(1.2f)
{
}

void Camera::update(float dt) {
    // Làm mượt góc nhìn bằng nội suy tuyến tính (Lerp)
    float blend = std::min(1.0f, dt * 14.0f);

    // Tính khoảng cách góc ngắn nhất theo cung tròn để tránh bị xoay 360 độ giật cục khi qua mốc 0/360
    float diffYaw = targetYaw - yaw;
    while (diffYaw < -180.0f) diffYaw += 360.0f;
    while (diffYaw > 180.0f) diffYaw -= 360.0f;
    yaw += diffYaw * blend;
    while (yaw < 0.0f) yaw += 360.0f;
    while (yaw >= 360.0f) yaw -= 360.0f;

    pitch += (targetPitch - pitch) * blend;
    distance += (targetDistance - distance) * blend;
}

void Camera::onMouseDrag(float deltaX, float deltaY) {
    targetYaw += deltaX * mouseSensitivity;
    while (targetYaw >= 360.0f) targetYaw -= 360.0f;
    while (targetYaw < 0.0f) targetYaw += 360.0f;

    targetPitch += deltaY * mouseSensitivity;
    // Giới hạn góc pitch từ -5 đến 85 độ (nhìn ngang đến nhìn thẳng từ trên xuống)
    targetPitch = std::max(-5.0f, std::min(85.0f, targetPitch));
}

void Camera::onMouseWheel(float wheelDelta) {
    targetDistance -= wheelDelta * zoomSensitivity;
    targetDistance = std::max(8.0f, std::min(32.0f, targetDistance));
}

void Camera::rotateYawBy(float deltaAngle) {
    targetYaw += deltaAngle;
    while (targetYaw >= 360.0f) targetYaw -= 360.0f;
    while (targetYaw < 0.0f) targetYaw += 360.0f;
}

void Camera::setPresetIsometric() {
    targetYaw = 45.0f;
    targetPitch = 30.0f;
    targetDistance = 17.5f;
}

void Camera::setPresetTopDown() {
    targetYaw = 0.0f;
    targetPitch = 85.0f;
    targetDistance = 15.0f;
}

void Camera::setPresetFront() {
    targetYaw = 0.0f;
    targetPitch = 15.0f;
    targetDistance = 16.5f;
}

Vec3 Camera::getEyePosition() const {
    float radYaw = yaw * DEG2RAD;
    float radPitch = pitch * DEG2RAD;
    float eyeX = target.x + distance * std::cos(radPitch) * std::sin(radYaw);
    float eyeY = target.y + distance * std::sin(radPitch);
    float eyeZ = target.z + distance * std::cos(radPitch) * std::cos(radYaw);
    return Vec3(eyeX, eyeY, eyeZ);
}

void Camera::applyView() const {
    Vec3 eye = getEyePosition();
    gluLookAt(
        eye.x, eye.y, eye.z,           // Tọa độ mắt
        target.x, target.y, target.z,  // Tọa độ trục quay trung tâm
        0.0f, 1.0f, 0.0f               // Vector hướng lên (Up vector)
    );
}
