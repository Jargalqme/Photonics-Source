#include "pch.h"
#include "Camera.h"
#include "Gameplay/Player.h"
#include "Gameplay/Combat/WeaponAnimator.h"

using namespace DirectX;

CameraState CameraState::Lerp(const CameraState& a, const CameraState& b, float t)
{
    CameraState result;
    result.height = a.height + (b.height - a.height) * t;
    result.fov = a.fov + (b.fov - a.fov) * t;
    return result;
}

// === 初期化 ===

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
    // カメラ状態プリセット（高さ, FOV）
    m_followState = { 2.3f, 70.0f };
    m_aimState = { 2.3f, 45.0f };
    m_current = m_followState;

    m_shakeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_shakeNoise.SetFrequency(SHAKE_NOISE_FREQ);

    updateViewMatrix();
    updateProjectionMatrix();
}

// === 向き制御 ===

void Camera::setRotation(float pitch, float yaw, float roll)
{
    m_pitch = pitch;
    m_yaw = yaw;
    m_roll = roll;
    updateDirectionVectors();
}

void Camera::updateDirectionVectors()
{
    float pitchRad = XMConvertToRadians(m_pitch);
    float yawRad = XMConvertToRadians(m_yaw);
    float rollRad = XMConvertToRadians(m_roll);

    // 前方ベクトル計算
    m_forward.x = sinf(yawRad) * cosf(pitchRad);
    m_forward.y = -sinf(pitchRad);
    m_forward.z = cosf(yawRad) * cosf(pitchRad);
    m_forward.Normalize();

    // 右ベクトル（ワールドY軸を基準）
    Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
    m_right = worldUp.Cross(m_forward);
    m_right.Normalize();

    // 上ベクトル
    m_up = m_forward.Cross(m_right);
    m_up.Normalize();

    // ロール適用
    if (fabs(m_roll) > 0.001f)
    {
        Matrix rollMatrix = Matrix::CreateFromAxisAngle(m_forward, rollRad);
        m_right = Vector3::Transform(m_right, rollMatrix);
        m_up = Vector3::Transform(m_up, rollMatrix);
    }
}

// === 行列更新 ===

void Camera::updateViewMatrix()
{
    // 位置シェイク
    Vector3 eyePos = m_position + Vector3(m_shakeOffsetX, m_shakeOffsetY, m_shakeOffsetZ);

    Vector3 fwd = m_forward;
    Vector3 up = m_up;

    float punchPitch = m_viewmodel ? m_viewmodel->getOffset().rotDeg.x : 0.0f;

    // 回転シェイク + エイムパンチ
    if (m_currentShakeIntensity > SHAKE_THRESHOLD || fabsf(punchPitch) > 0.001f)
    {
        Matrix rot = Matrix::CreateFromYawPitchRoll(
            XMConvertToRadians(m_shakeYaw),
            XMConvertToRadians(m_shakePitch + punchPitch),
            XMConvertToRadians(m_shakeRoll)
        );
        fwd = Vector3::TransformNormal(fwd, rot);
        up = Vector3::TransformNormal(up, rot);
    }

    Vector3 target = eyePos + fwd;
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
        m_farZ
    );
}

void Camera::updateViewModelProjection()
{
    m_viewModelProjection = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(VIEWMODEL_FOV),
        m_aspectRatio,
        VIEWMODEL_NEAR,
        VIEWMODEL_FAR
    );
}

// === 更新 ===

void Camera::update(const Player& player, float deltaTime)
{
    // プレイヤーの視点を反映（マウスルックは PlayerSystem 経由で Player::applyLookDelta が処理済み）
    m_yaw = player.getLookYaw();
    m_pitch = player.getLookPitch();
    updateDirectionVectors();

    // 目標状態へブレンド（通常 or ADS）
    CameraState& target = player.isAiming() ? m_aimState : m_followState;
    float t = std::min(1.0f, m_blendSpeed * deltaTime);
    m_current = CameraState::Lerp(m_current, target, t);

    // FPS — プレイヤー位置に直接ロック（CryEngine CDefaultCameraMode::UpdateView と同じ）
    Vector3 camPos = player.getPosition() + Vector3(0.0f, m_current.height, 0.0f);
    if (camPos.y < MIN_CAMERA_HEIGHT)
    {
        camPos.y = MIN_CAMERA_HEIGHT;
    }

    setPosition(camPos);

    // ブレンド状態からFOV更新
    m_fov = XMConvertToRadians(m_current.fov);
    updateProjectionMatrix();

    // 視点の変化量を武器へ送る（sway 用）
    if (m_viewmodel)
    {
        m_viewmodel->addLookDelta(m_yaw - m_lastYaw, m_pitch - m_lastPitch);
    }
    m_lastYaw   = m_yaw;
    m_lastPitch = m_pitch;

    updateShake(deltaTime);
    updateViewMatrix();
}

// === 画面シェイク ===

void Camera::triggerShake(float intensity, float duration)
{
    // 既存のシェイクに加算（上限あり）
    if (m_shakeTimer < m_shakeDuration)
    {
        m_shakeIntensity = std::min(m_shakeIntensity + intensity * 0.5f, MAX_COMBINED_SHAKE);
        m_shakeDuration = std::max(m_shakeDuration, duration);
    }
    else
    {
        // シェイク未発動 — 新規開始
        m_shakeIntensity = intensity;
        m_shakeDuration = duration;
        m_shakeTimer = 0.0f;
    }
}

// Squirrel Eiserloh GDC 2016 — Math for Game Programmers: Juicing Your Cameras with Math
void Camera::updateShake(float deltaTime)
{
    if (m_shakeTimer < m_shakeDuration)
    {
        m_shakeTimer += deltaTime;
        m_shakeTime += deltaTime * SHAKE_TIME_SCALE;

        // 指数減衰
        float progress = m_shakeTimer / m_shakeDuration;
        float falloff = 1.0f - progress;
        m_currentShakeIntensity = m_shakeIntensity * falloff * falloff;

        // 6DOFノイズ — 各チャンネルは異なるノイズ領域をサンプリング
        float shake = m_currentShakeIntensity;
        m_shakeYaw    = m_shakeMaxYaw    * shake * m_shakeNoise.GetNoise(m_shakeTime, 0.0f);
        m_shakePitch  = m_shakeMaxPitch  * shake * m_shakeNoise.GetNoise(m_shakeTime, 100.0f);
        m_shakeRoll   = m_shakeMaxRoll   * shake * m_shakeNoise.GetNoise(m_shakeTime, 200.0f);
        m_shakeOffsetX = m_shakeMaxOffset * shake * m_shakeNoise.GetNoise(m_shakeTime, 300.0f);
        m_shakeOffsetY = m_shakeMaxOffset * shake * m_shakeNoise.GetNoise(m_shakeTime, 400.0f);
        m_shakeOffsetZ = m_shakeMaxOffset * shake * m_shakeNoise.GetNoise(m_shakeTime, 500.0f) * SHAKE_Z_ATTENUATION;
    }
    else
    {
        m_currentShakeIntensity = 0.0f;
        m_shakeIntensity = 0.0f;
        m_shakeYaw = m_shakePitch = m_shakeRoll = 0.0f;
        m_shakeOffsetX = m_shakeOffsetY = m_shakeOffsetZ = 0.0f;
    }
}
