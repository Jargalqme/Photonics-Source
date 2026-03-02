#include "pch.h"
#include "Camera.h"
#include "InputManager.h"

CameraState CameraState::Lerp(const CameraState& a, const CameraState& b, float t) {
    CameraState result;
    result.distance = a.distance + (b.distance - a.distance) * t;
    result.height = a.height + (b.height - a.height) * t;
    result.shoulderOffset = a.shoulderOffset + (b.shoulderOffset - a.shoulderOffset) * t;
    result.fov = a.fov + (b.fov - a.fov) * t;
    return result;
}

Camera::Camera()
    : m_position(0.0f, 0.0f, 5.0f)
    , m_forward(0.0f, 0.0f, -1.0f)
    , m_right(1.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_pitch(0.0f)
    , m_yaw(0.0f)
    , m_roll(0.0f)
    , m_fov(XMConvertToRadians(45.0f))
    , m_aspectRatio(16.0f / 9.0f)
    , m_nearZ(0.1f)
    , m_farZ(1000.0f)
{
    // Initialize camera state presets
    m_followState = { 10.0f, 4.0f, 1.5f, 60.0f };
    m_aimState = { 6.0f, 2.5f, 1.0f, 40.0f };
    m_current = m_followState;

    updateViewMatrix();
    updateProjectionMatrix();
}

void Camera::setLookAt(const Vector3& target)
{
    m_forward = target - m_position;
    m_forward.Normalize();

    Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
    m_right = worldUp.Cross(m_forward);
    m_right.Normalize();

    m_up = m_forward.Cross(m_right);
    m_up.Normalize();

    m_yaw = XMConvertToDegrees(atan2f(m_forward.x, m_forward.z));
    m_pitch = XMConvertToDegrees(asinf(-m_forward.y));
}

void Camera::setRotation(float pitch, float yaw, float roll)
{
    m_pitch = pitch;
    m_yaw = yaw;
    m_roll = roll;
    updateDirectionVectors();
}

void Camera::moveForward(float distance)
{
    m_position += m_forward * distance;
}

void Camera::moveRight(float distance)
{
    m_position += m_right * distance;
}

void Camera::moveUp(float distance)
{
    m_position += m_up * distance;
}

void Camera::addPitch(float degrees)
{
    m_pitch += degrees;

    // OTS pitch clamp
    if (m_pitch > 60.0f)
        m_pitch = 60.0f;
    if (m_pitch < -30.0f)
        m_pitch = -30.0f;

    updateDirectionVectors();
}

void Camera::addYaw(float degrees)
{
    m_yaw += degrees;

    // Wrap yaw around
    if (m_yaw > 360.0f)
        m_yaw -= 360.0f;
    if (m_yaw < 0.0f)
        m_yaw += 360.0f;

    updateDirectionVectors();
}

void Camera::updateDirectionVectors()
{
    // Convert degrees to radians
    float pitchRad = XMConvertToRadians(m_pitch);
    float yawRad = XMConvertToRadians(m_yaw);
    float rollRad = XMConvertToRadians(m_roll);

    // Calculate forward vector
    m_forward.x = sinf(yawRad) * cosf(pitchRad);
    m_forward.y = -sinf(pitchRad);
    m_forward.z = cosf(yawRad) * cosf(pitchRad);
    m_forward.Normalize();

    // Calculate right vector (assuming world up is Y)
    Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
    m_right = worldUp.Cross(m_forward);
    m_right.Normalize();

    // Calculate up vector
    m_up = m_forward.Cross(m_right);
    m_up.Normalize();

    // Apply roll if needed
    if (fabs(m_roll) > 0.001f)
    {
        Matrix rollMatrix = Matrix::CreateFromAxisAngle(m_forward, rollRad);
        m_right = Vector3::Transform(m_right, rollMatrix);
        m_up = Vector3::Transform(m_up, rollMatrix);
    }
}

void Camera::updateViewMatrix()
{
    Vector3 shakeOffset = getShakeOffset();
    Vector3 eyePos = m_position + shakeOffset;
    Vector3 target = eyePos + m_forward;

    m_viewMatrix = XMMatrixLookAtLH(eyePos, target, m_up);
}

void Camera::setProjectionParameters(float fov, float aspectRatio, float nearPlane, float farPlane)
{
    m_fov = XMConvertToRadians(fov);
    m_aspectRatio = aspectRatio;
    m_nearZ = nearPlane;
    m_farZ = farPlane;
    updateProjectionMatrix();
}

void Camera::updateProjectionMatrix()
{
    m_projectionMatrix = XMMatrixPerspectiveFovLH(
        m_fov,
        m_aspectRatio,
        m_nearZ,
        m_farZ
    );
}

void Camera::update(float deltaTime, InputManager* input)
{
    // Follow mode freezes entirely when cursor is visible (gameplay pause)
    if (input->isCursorVisible() && m_mode == CameraMode::Follow)
        return;

    // Mode toggle
    if (input->isKeyPressed(Keyboard::Keys::F1))
        setMode(CameraMode::Free);
    if (input->isKeyPressed(Keyboard::Keys::F2))
        setMode(CameraMode::Follow);

    switch (m_mode) {
    case CameraMode::Free:
        updateFreeMode(deltaTime, input);
        break;
    case CameraMode::Follow:
        updateFollowMode(deltaTime, input);
        break;
    }
    updateShake(deltaTime);
    updateViewMatrix();
}

void Camera::setMode(CameraMode mode)
{
    m_mode = mode;
    if (mode == CameraMode::Follow && m_followTargetPos)
    {
        m_smoothedPosition = *m_followTargetPos;
    }
}

const char* Camera::getModeName() const
{
    switch (m_mode) {
    case CameraMode::Free:   return "Free Camera";
    case CameraMode::Follow: return "Follow Camera";
    default:                 return "Unknown";
    }
}

void Camera::setFollowTarget(Vector3* position, float* rotation)
{
    m_followTargetPos = position;
    m_followTargetRot = rotation;

    if (m_followTargetPos)
    {
        m_smoothedPosition = *m_followTargetPos;
    }
}

void Camera::updateFreeMode(float deltaTime, InputManager* input)
{
    float speed = m_moveSpeed * deltaTime;

    // Sprint
    if (input->isKeyDown(Keyboard::Keys::LeftShift))
        speed *= 2.0f;

    // WASD movement
    if (input->isKeyDown(Keyboard::Keys::W))
        moveForward(speed);
    if (input->isKeyDown(Keyboard::Keys::S))
        moveForward(-speed);
    if (input->isKeyDown(Keyboard::Keys::A))
        moveRight(-speed);
    if (input->isKeyDown(Keyboard::Keys::D))
        moveRight(speed);

    // Vertical movement
    if (input->isKeyDown(Keyboard::Keys::Space))
        moveUp(speed);
    if (input->isKeyDown(Keyboard::Keys::LeftControl))
        moveUp(-speed);

    // Mouse look (skip when cursor visible — UI mode)
    if (!input->isCursorVisible())
    {
        Vector2 look = input->getLookInput();
        addYaw(look.x * m_mouseSensitivity);
        addPitch(look.y * m_mouseSensitivity);
    }
}

void Camera::updateFollowMode(float deltaTime, InputManager* input)
{
    if (!m_followTargetPos)
        return;

    // Blend m_current toward target state (follow or aim)
    CameraState& target = m_isAiming ? m_aimState : m_followState;
    float t = std::min(1.0f, m_blendSpeed * deltaTime);
    m_current = CameraState::Lerp(m_current, target, t);

    // FPS-style mouse look
    Vector2 look = input->getLookInput();
    addYaw(look.x * m_mouseSensitivity);
    addPitch(look.y * m_mouseSensitivity);

    // Smooth player position tracking
    m_smoothedPosition = Vector3::Lerp(m_smoothedPosition, *m_followTargetPos,
        m_followSmoothSpeed * deltaTime);

    // Camera position: orbit around player using blended state
    float yawRad = XMConvertToRadians(m_yaw);
    float pitchRad = XMConvertToRadians(m_pitch);

    Vector3 offset;
    offset.x = -sinf(yawRad) * cosf(pitchRad) * m_current.distance;
    offset.y = m_current.height + sinf(pitchRad) * m_current.distance;
    offset.z = -cosf(yawRad) * cosf(pitchRad) * m_current.distance;

    // Right side (shoulder) offset
    Vector3 rightDir;
    rightDir.x = cosf(yawRad);
    rightDir.z = -sinf(yawRad);
    rightDir.y = 0.0f;

    offset += rightDir * m_current.shoulderOffset;

    // Clamp camera above the ground
    Vector3 camPos = m_smoothedPosition + offset;
    if (camPos.y < 1.0f)
        camPos.y = 1.0f;

    setPosition(camPos);

    // Update FOV from blended state
    m_fov = XMConvertToRadians(m_current.fov);
    updateProjectionMatrix();
}

void Camera::triggerShake(float intensity, float duration)
{
    // allow new shakes to add to existing ones
    if (m_shakeTimer < m_shakeDuration)
    {
        // add intensities together, but cap at reasonable maximum
        m_shakeIntensity = std::min(m_shakeIntensity + intensity * 0.5f, 30.0f);
        m_shakeDuration = std::max(m_shakeDuration, duration);
    }
    else
    {
        // no shake active, start fresh
        m_shakeIntensity = intensity;
        m_shakeDuration = duration;
        m_shakeTimer = 0.0f;
    }
}

void Camera::updateShake(float deltaTime)
{
    if (m_shakeTimer < m_shakeDuration)
    {
        m_shakeTimer += deltaTime;

        // exponential falloff
        float progress = m_shakeTimer / m_shakeDuration;
        float falloff = 1.0f - progress;
        falloff = falloff * falloff;

        m_currentShakeIntensity = m_shakeIntensity * falloff;
    }
    else
    {
        m_currentShakeIntensity = 0.0f;
        m_shakeIntensity = 0.0f;
    }
}

Vector3 Camera::getShakeOffset() const
{
    if (m_currentShakeIntensity <= 0.01f)
        return Vector3::Zero;

    // Random offset based on intensity
    float x = ((rand() % 200 - 100) / 100.0f) * m_currentShakeIntensity * 0.1f;
    float y = ((rand() % 200 - 100) / 100.0f) * m_currentShakeIntensity * 0.1f;
    float z = ((rand() % 200 - 100) / 100.0f) * m_currentShakeIntensity * 0.05f;

    return Vector3(x, y, z);
}

void Camera::setAiming(bool aiming)
{
    m_isAiming = aiming;
}
