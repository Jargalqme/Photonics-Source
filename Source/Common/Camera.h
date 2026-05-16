#pragma once

#include <memory>
#include "ThirdParty/FastNoiseLite.h"

class Player;

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Matrix;

struct CameraState
{
    float height = 1.5f;
    float fov = 45.0f;

    static CameraState Lerp(const CameraState& a, const CameraState& b, float t);
};

class Camera
{
public:
    Camera();
    ~Camera() = default;

    void update(const Player& player, float deltaTime);

    void setPosition(const Vector3& pos) { m_position = pos; }
    void setRotation(float pitch, float yaw, float roll = 0.0f);

    void updateViewMatrix();
    void updateProjectionMatrix();
    Matrix getViewMatrix() const { return m_viewMatrix; }
    Matrix getProjectionMatrix() const { return m_projectionMatrix; }
    Matrix getViewModelProjection() const { return m_viewModelProjection; }

    Vector3 getPosition() const { return m_position; }
    Vector3 getForward() const { return m_forward; }
    Vector3 getRight() const { return m_right; }
    Vector3 getUp() const { return m_up; }
    float getPitch() const { return m_pitch; }
    float getYaw() const { return m_yaw; }

    void setProjectionParameters(float fov, float aspectRatio, float nearPlane, float farPlane);

    void triggerShake(float intensity, float duration);
    bool isShaking() const { return m_currentShakeIntensity > SHAKE_THRESHOLD; }

    CameraState* getFollowStatePtr() { return &m_followState; }
    CameraState* getAimStatePtr() { return &m_aimState; }

private:
    static constexpr float MIN_CAMERA_HEIGHT = 1.0f;
    static constexpr float SHAKE_THRESHOLD = 0.01f;
    static constexpr float SHAKE_TIME_SCALE = 60.0f;
    static constexpr float MAX_COMBINED_SHAKE = 30.0f;
    static constexpr float SHAKE_NOISE_FREQ = 5.0f;
    static constexpr float SHAKE_Z_ATTENUATION = 0.5f;
    static constexpr float VIEWMODEL_FOV = 70.0f;
    static constexpr float VIEWMODEL_NEAR = 0.01f;
    static constexpr float VIEWMODEL_FAR = 10.0f;

    void updateViewModelProjection();
    void updateDirectionVectors();
    void updateShake(float deltaTime);

    CameraState m_followState;
    CameraState m_aimState;
    CameraState m_current;
    float m_blendSpeed = 8.0f;

    Vector3 m_position;
    Vector3 m_right;
    Vector3 m_up;
    Vector3 m_forward;

    float m_pitch;
    float m_yaw;
    float m_roll;

    Matrix m_viewMatrix;
    Matrix m_projectionMatrix;
    Matrix m_viewModelProjection;

    float m_fov;
    float m_aspectRatio;
    float m_nearZ;
    float m_farZ;

    FastNoiseLite m_shakeNoise;
    float m_shakeTime = 0.0f;

    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeTimer = 0.0f;
    float m_currentShakeIntensity = 0.0f;

    float m_shakeMaxYaw = 0.5f;
    float m_shakeMaxPitch = 0.3f;
    float m_shakeMaxRoll = 1.0f;
    float m_shakeMaxOffset = 0.03f;

    float m_shakeYaw = 0.0f;
    float m_shakePitch = 0.0f;
    float m_shakeRoll = 0.0f;
    float m_shakeOffsetX = 0.0f;
    float m_shakeOffsetY = 0.0f;
    float m_shakeOffsetZ = 0.0f;
};
