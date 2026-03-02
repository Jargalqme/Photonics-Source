#pragma once

#include <memory>

class InputManager;

using namespace DirectX;
using namespace DirectX::SimpleMath;

enum class CameraMode {
    Free,    // FPS-Style (for debugging)
    Follow   // TPS-Style (for gameplay)
};

struct CameraState {
    float distance = 4.5f;   // distance behind player
    float height = 1.5f;   // height above player pivot
    float shoulderOffset = 1.5f;   // right-side offset
    float fov = 45.0f;  // field of view (degrees)

    static CameraState Lerp(const CameraState& a, const CameraState& b, float t);
};

class Camera {
public:
    Camera();
    ~Camera() = default;

    // Mode
    void setMode(CameraMode mode);
    CameraMode getMode() const { return m_mode; }
    const char* getModeName() const;

    // Follow mode target
    void setFollowTarget(Vector3* position, float* rotation);

    // Settings
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }
    void setMouseSensitivity(float sens) { m_mouseSensitivity = sens; }
    // Update - delegates to controller
    void update(float deltaTime, InputManager* input);

    // Core setters (called by controllers)
    void setPosition(const Vector3& pos) { m_position = pos; }
    void setLookAt(const Vector3& target);
    void setRotation(float pitch, float yaw, float roll = 0.0f);

    // Matrices
    void updateViewMatrix();
    void updateProjectionMatrix();
    Matrix getViewMatrix() const { return m_viewMatrix; }
    Matrix getProjectionMatrix() const { return m_projectionMatrix; }

    // Getters (controllers need these)
    Vector3 getPosition() const { return m_position; }
    Vector3 getForward() const { return m_forward; }
    Vector3 getRight() const { return m_right; }
    Vector3 getUp() const { return m_up; }
    float getPitch() const { return m_pitch; }
    float getYaw() const { return m_yaw; }

    // Projection
    void setProjectionParameters(float fov, float aspectRatio, float nearPlane, float farPlane);

    // Movement helpers (used by FreeCameraController)
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void addPitch(float degrees);
    void addYaw(float degrees);

    // Screen Shake
    void triggerShake(float intensity, float duration);
    bool isShaking() const { return m_currentShakeIntensity > 0.01f; }

    // Aiming (OTS state blend)
    void setAiming(bool aiming);
    bool isAiming() const { return m_isAiming; }
    float getShoulderOffset() const { return m_current.shoulderOffset; }

private:
    // Mode
    CameraMode m_mode = CameraMode::Free;

    // Free mode settings
    float m_moveSpeed = 25.0f;
    float m_mouseSensitivity = 50.0f;

    // Follow mode settings
    Vector3* m_followTargetPos = nullptr;
    float* m_followTargetRot = nullptr;
    float m_followSmoothSpeed = 10.0f;
    Vector3 m_smoothedPosition;

    // Camera state presets
    CameraState m_followState;    // normal gameplay
    CameraState m_aimState;       // right-click aim
    CameraState m_current;        // blended live state
    float m_blendSpeed = 8.0f;    // transition speed between states
    bool m_isAiming = false;      // which state to blend toward

    // Internal update methods
    void updateFreeMode(float deltaTime, InputManager* input);
    void updateFollowMode(float deltaTime, InputManager* input);

    // Transform
    Vector3 m_position = { 0.0f, 0.0f, 5.0f };
    Vector3 m_right = { 1.0f, 0.0f, 0.0f };
    Vector3 m_up = { 0.0f, 1.0f, 0.0f };
    Vector3 m_forward = { 0.0f, 0.0f, 1.0f };

    // Rotation (degrees)
    float m_pitch;   // yes  - nodding (up/down)
    float m_yaw;   // no   - shaking (left/right)
    float m_roll;   // what - tilting head sideways

    // Matrices
    Matrix m_viewMatrix;
    Matrix m_projectionMatrix;

    // Projection
    float m_fov;          // field of view (how wide the view is)
    float m_aspectRatio;  // width / height of screen
    float m_nearZ;    // closest visible distance
    float m_farZ;     // farthest visible distance

    // Screen Shake
    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeTimer = 0.0f;
    float m_currentShakeIntensity = 0.0f;
    void updateShake(float deltaTime);
    Vector3 getShakeOffset() const;

    void updateDirectionVectors();
};
