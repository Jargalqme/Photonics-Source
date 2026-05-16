#include "pch.h"
#include "Camera.h"
#include "Gameplay/Player.h"

using namespace DirectX;

CameraState CameraState::Lerp(const CameraState& a, const CameraState& b, float t)
{
    CameraState result;
    result.height = a.height + (b.height - a.height) * t;
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
    m_followState = { 2.3f, 70.0f };
    m_aimState = { 2.3f, 45.0f };
    m_current = m_followState;

    m_shakeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_shakeNoise.SetFrequency(SHAKE_NOISE_FREQ);

    updateViewMatrix();
    updateProjectionMatrix();
    updateViewModelProjection();
}

void Camera::setRotation(float pitch, float yaw, float roll)
{
    m_pitch = pitch;
    m_yaw = yaw;
    m_roll = roll;
    updateDirectionVectors();
}

void Camera::updateDirectionVectors()
{
    const float pitchRad = XMConvertToRadians(m_pitch);
    const float yawRad = XMConvertToRadians(m_yaw);
    const float rollRad = XMConvertToRadians(m_roll);

    m_forward.x = sinf(yawRad) * cosf(pitchRad);
    m_forward.y = -sinf(pitchRad);
    m_forward.z = cosf(yawRad) * cosf(pitchRad);
    m_forward.Normalize();

    const Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
    m_right = worldUp.Cross(m_forward);
    m_right.Normalize();

    m_up = m_forward.Cross(m_right);
    m_up.Normalize();

    if (fabsf(m_roll) > 0.001f)
    {
        const Matrix rollMatrix = Matrix::CreateFromAxisAngle(m_forward, rollRad);
        m_right = Vector3::Transform(m_right, rollMatrix);
        m_up = Vector3::Transform(m_up, rollMatrix);
    }
}

void Camera::updateViewMatrix()
{
    const Vector3 eyePos = m_position + Vector3(m_shakeOffsetX, m_shakeOffsetY, m_shakeOffsetZ);

    Vector3 forward = m_forward;
    Vector3 up = m_up;

    if (m_currentShakeIntensity > SHAKE_THRESHOLD)
    {
        const Matrix rotation = Matrix::CreateFromYawPitchRoll(
            XMConvertToRadians(m_shakeYaw),
            XMConvertToRadians(m_shakePitch),
            XMConvertToRadians(m_shakeRoll));

        forward = Vector3::TransformNormal(forward, rotation);
        up = Vector3::TransformNormal(up, rotation);
    }

    const Vector3 target = eyePos + forward;
    m_viewMatrix = XMMatrixLookAtLH(eyePos, target, up);
}

void Camera::setProjectionParameters(float fov, float aspectRatio, float nearPlane, float farPlane)
{
    m_fov = XMConvertToRadians(fov);
    m_aspectRatio = aspectRatio;
    m_nearZ = nearPlane;
    m_farZ = farPlane;
    updateProjectionMatrix();
    updateViewModelProjection();
}

void Camera::updateProjectionMatrix()
{
    m_projectionMatrix = XMMatrixPerspectiveFovLH(
        m_fov,
        m_aspectRatio,
        m_nearZ,
        m_farZ);
}

void Camera::updateViewModelProjection()
{
    m_viewModelProjection = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(VIEWMODEL_FOV),
        m_aspectRatio,
        VIEWMODEL_NEAR,
        VIEWMODEL_FAR);
}

void Camera::update(const Player& player, float deltaTime)
{
    m_yaw = player.getLookYaw();
    m_pitch = player.getLookPitch();
    updateDirectionVectors();

    const CameraState& targetState = player.isAiming() ? m_aimState : m_followState;
    const float blend = std::min(1.0f, m_blendSpeed * deltaTime);
    m_current = CameraState::Lerp(m_current, targetState, blend);

    Vector3 cameraPosition = player.getPosition() + Vector3(0.0f, m_current.height, 0.0f);
    if (cameraPosition.y < MIN_CAMERA_HEIGHT)
    {
        cameraPosition.y = MIN_CAMERA_HEIGHT;
    }

    setPosition(cameraPosition);

    m_fov = XMConvertToRadians(m_current.fov);
    updateProjectionMatrix();

    updateShake(deltaTime);
    updateViewMatrix();
}

void Camera::triggerShake(float intensity, float duration)
{
    if (m_shakeTimer < m_shakeDuration)
    {
        m_shakeIntensity = std::min(m_shakeIntensity + intensity * 0.5f, MAX_COMBINED_SHAKE);
        m_shakeDuration = std::max(m_shakeDuration, duration);
    }
    else
    {
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
        m_shakeTime += deltaTime * SHAKE_TIME_SCALE;

        const float progress = m_shakeTimer / m_shakeDuration;
        const float falloff = 1.0f - progress;
        m_currentShakeIntensity = m_shakeIntensity * falloff * falloff;

        const float shake = m_currentShakeIntensity;
        m_shakeYaw = m_shakeMaxYaw * shake * m_shakeNoise.GetNoise(m_shakeTime, 0.0f);
        m_shakePitch = m_shakeMaxPitch * shake * m_shakeNoise.GetNoise(m_shakeTime, 100.0f);
        m_shakeRoll = m_shakeMaxRoll * shake * m_shakeNoise.GetNoise(m_shakeTime, 200.0f);
        m_shakeOffsetX = m_shakeMaxOffset * shake * m_shakeNoise.GetNoise(m_shakeTime, 300.0f);
        m_shakeOffsetY = m_shakeMaxOffset * shake * m_shakeNoise.GetNoise(m_shakeTime, 400.0f);
        m_shakeOffsetZ = m_shakeMaxOffset * shake * m_shakeNoise.GetNoise(m_shakeTime, 500.0f) * SHAKE_Z_ATTENUATION;
    }
    else
    {
        m_currentShakeIntensity = 0.0f;
        m_shakeIntensity = 0.0f;
        m_shakeYaw = 0.0f;
        m_shakePitch = 0.0f;
        m_shakeRoll = 0.0f;
        m_shakeOffsetX = 0.0f;
        m_shakeOffsetY = 0.0f;
        m_shakeOffsetZ = 0.0f;
    }
}
