#pragma once

#include <memory>
#include "ThirdParty/FastNoiseLite.h"

class Player;
class WeaponAnimator;

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Matrix;

/// カメラ状態パラメータ（通常/ADS間のブレンド用）
struct CameraState
{
    float height = 1.5f;           // プレイヤー基点からの高さ
    float fov = 45.0f;             // 視野角（度）

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

    void setViewmodel(WeaponAnimator* vm) { m_viewmodel = vm; }

    // デバッグUI用ポインタ
    CameraState* getFollowStatePtr() { return &m_followState; }
    CameraState* getAimStatePtr() { return &m_aimState; }

private:
    // === 定数 ===
    static constexpr float MIN_CAMERA_HEIGHT    = 1.0f;        // カメラの最低高さ
    static constexpr float SHAKE_THRESHOLD      = 0.01f;       // シェイク閾値
    static constexpr float SHAKE_TIME_SCALE     = 60.0f;       // シェイクノイズの時間スケール
    static constexpr float MAX_COMBINED_SHAKE   = 30.0f;       // シェイク合算の上限
    static constexpr float SHAKE_NOISE_FREQ     = 5.0f;        // ノイズ周波数
    static constexpr float SHAKE_Z_ATTENUATION  = 0.5f;        // Z軸シェイクの減衰率

    static constexpr float VIEWMODEL_FOV  = 70.0f;
    static constexpr float VIEWMODEL_NEAR = 0.01f;
    static constexpr float VIEWMODEL_FAR  = 10.0f;

    // === カメラ状態プリセット ===
    CameraState m_followState;    // 通常ゲームプレイ
    CameraState m_aimState;       // ADS時
    CameraState m_current;        // ブレンド中のライブ状態
    float m_blendSpeed = 8.0f;

    // === 内部更新メソッド ===
    void updateViewModelProjection();

    // === トランスフォーム ===
    Vector3 m_position = { 0.0f, 0.0f, 5.0f };
    Vector3 m_right    = { 1.0f, 0.0f, 0.0f };
    Vector3 m_up       = { 0.0f, 1.0f, 0.0f };
    Vector3 m_forward  = { 0.0f, 0.0f, 1.0f };

    // 回転（度）
    float m_pitch;  // 縦回転（上下）
    float m_yaw;    // 横回転（左右）
    float m_roll;   // 傾き

    // sway 用：前フレームとの差分を武器へ渡す
    float m_lastYaw = 0.0f;
    float m_lastPitch = 0.0f;

    // === 行列 ===
    Matrix m_viewMatrix;
    Matrix m_projectionMatrix;
    Matrix m_viewModelProjection;

    // === プロジェクション ===
    float m_fov;
    float m_aspectRatio;
    float m_nearZ;
    float m_farZ;

    // === 画面シェイク ===
    FastNoiseLite m_shakeNoise;
    float m_shakeTime = 0.0f;

    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeTimer = 0.0f;
    float m_currentShakeIntensity = 0.0f;

    // チャンネルごとの最大値
    float m_shakeMaxYaw = 0.5f;
    float m_shakeMaxPitch = 0.3f;
    float m_shakeMaxRoll = 1.0f;
    float m_shakeMaxOffset = 0.03f;

    // シェイク計算結果（毎フレーム更新）
    float m_shakeYaw = 0.0f;
    float m_shakePitch = 0.0f;
    float m_shakeRoll = 0.0f;
    float m_shakeOffsetX = 0.0f;
    float m_shakeOffsetY = 0.0f;
    float m_shakeOffsetZ = 0.0f;
    void updateShake(float deltaTime);

    // === 武器ビューモデル（エイムパンチ読み取り用） ===
    WeaponAnimator* m_viewmodel = nullptr;

    void updateDirectionVectors();
};
