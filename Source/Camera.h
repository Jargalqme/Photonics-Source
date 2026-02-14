#pragma once

#include <memory>

class InputManager;

using namespace DirectX;
using namespace DirectX::SimpleMath;

enum class CameraMode {
    Free,    // FPS-Style (for debugging)
    Follow   // TPS-Style (for gameplay)
};

class Camera {
public:
	Camera();
	~Camera() = default;

    // Mode
    void SetMode(CameraMode mode);
    CameraMode GetMode() const { return m_mode; }
    const char* GetModeName() const;

    // Follow mode target
    void SetFollowTarget(Vector3* position, float* rotation);

    // Settings
    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    void SetMouseSensitivity(float sens) { m_mouseSensitivity = sens; }
    void SetFollowDistance(float dist) { m_followDistance = dist; }
    void SetFollowHeight(float height) { m_followHeight = height; }

    // Update - delegates to controller
    void Update(float deltaTime, InputManager* input);

    // Core setters (called by controllers)
    void SetPosition(const Vector3& pos) { m_position = pos; }
    void SetLookAt(const Vector3& target);
    void SetRotation(float pitch, float yaw, float roll = 0.0f);

    // Matrices
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
    Matrix GetViewMatrix() const { return m_viewMatrix; }
    Matrix GetProjectionMatrix() const { return m_projectionMatrix; }

    // Getters (controllers need these)
    Vector3 GetPosition() const { return m_position; }
    Vector3 GetForward() const { return m_forward; }
    Vector3 GetRight() const { return m_right; }
    Vector3 GetUp() const { return m_up; }
    float GetPitch() const { return m_pitch; }
    float GetYaw() const { return m_yaw; }

    // Projection
    void SetProjectionParameters(float fov, float aspectRatio, float nearPlane, float farPlane);

    // Movement helpers (used by FreeCameraController)
    void MoveForward(float distance);
    void MoveRight(float distance);
    void MoveUp(float distance);
    void AddPitch(float degrees);
    void AddYaw(float degrees);

    // Screen Shake
    void TriggerShake(float intensity, float duration);
    bool IsShaking() const { return m_currentShakeIntensity > 0.01f; }

    // Zoom
    void SetZoom(bool zooming);
    void SetZoomFOV(float fov) { m_zoomFOV = XMConvertToRadians(fov); }
    void SetZoomSpeed(float speed) { m_zoomSpeed = speed; }
    bool IsZooming() const { return m_isZooming; }

private:
    // Mode
    CameraMode m_mode = CameraMode::Free;

    // Free mode settings
    float m_moveSpeed = 25.0f;
    float m_mouseSensitivity = 50.0f;

    // Follow mode settings
    Vector3* m_followTargetPos = nullptr;
    float* m_followTargetRot = nullptr;
    float m_followDistance = 9.0f;
    float m_followHeight = 3.0f;
    float m_followSmoothSpeed = 5.0f;
    Vector3 m_smoothedPosition;

    // Internal update methods
    void UpdateFreeMode(float deltaTime, InputManager* input);
    void UpdateFollowMode(float deltaTime, InputManager* input);

    // Transform
    Vector3 m_position = {0.0f, 0.0f, 5.0f};
    Vector3 m_forward;
    Vector3 m_right;
    Vector3 m_up;

    // Rotation (degrees)
    float m_pitch; // yes  - nodding (up/down)
    float m_yaw;   // no   - shaking (left/right)
    float m_roll;  // what - tilting head sideways

    // Matrices
    Matrix m_viewMatrix;
    Matrix m_projectionMatrix;

    // Projection
    float m_fov;          // field of view (how wide the view is)
    float m_aspectRatio;  // width / height of screen
    float m_nearPlane;    // closest visible distance
    float m_farPlane;     // farthest visible distance

    // Screen Shake
    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeTimer = 0.0f;
    float m_currentShakeIntensity = 0.0f;
    void UpdateShake(float deltaTime);
    Vector3 GetShakeOffset() const;

    // Zoom
    bool m_isZooming = false;
    float m_normalFOV = XMConvertToRadians(45.0f);
    float m_zoomFOV = XMConvertToRadians(25.0f);
    float m_currentFOV = XMConvertToRadians(45.0f);
    float m_zoomSpeed = 5.0f;
    void UpdateZoom(float deltaTime);

    void UpdateDirectionVectors();
};
