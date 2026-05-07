#pragma once
#include "GeometricPrimitive.h"
#include "Effects.h"
#include <DirectXCollision.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Combat/WeaponAnimator.h"
#include "Gameplay/Combat/Weapon.h"
#include "Gameplay/Combat/ICombatTarget.h"
#include <memory>

struct SceneContext;
class RenderCommandQueue;
class ImportedModel;

struct GunPart
{
    DirectX::GeometricPrimitive* primitive = nullptr;  // MeshCache から借用（非所有）
    Transform localTransform;                          // gun_root からの相対変換
};

struct ImportedRifleViewmodelSettings
{
    float targetLength = 1.68f;
    Vector3 position = Vector3(-0.015f, -0.195, 0.180f);
    Vector3 rotationDegrees = Vector3(0.0f, 180.0f, 0.0f);
};

class Player : public GameObject, public ICombatTarget
{
public:
    Player(SceneContext& context);

    void initialize() override;
    void submitViewmodel(RenderCommandQueue& queue, const Matrix& view);
    void finalize() override;

    void applyLookDelta(float deltaYaw, float deltaPitch);
    void applyMovement(const Vector3& direction, float aimYaw, float deltaTime);
    void tick(float deltaTime);
    void jump();
    bool isGrounded() const { return m_isGrounded; }

    // --- 視点 ---
    float getLookYaw() const { return m_lookYaw; }
    float getLookPitch() const { return m_lookPitch; }
    Vector3 getLookForward() const;
    Vector3 getLookRight() const;
    float getMouseSensitivity() const { return m_mouseSensitivity; }
    float* getMouseSensitivityPtr() { return &m_mouseSensitivity; }

    // --- 射撃 ---
    void startFire() { m_weapon->startFire(); }
    void stopFire() { m_weapon->stopFire(); }
    void updateWeapon(
        float deltaTime,
        const Vector3& hitScanOrigin,
        const Vector3& hitScanDirection,
        const Vector3& tracerStart,
        std::vector<ShotIntent>& outIntents)
    {
        m_weapon->update(deltaTime, hitScanOrigin, hitScanDirection, tracerStart, outIntents);
    }
    Weapon* getWeapon() { return m_weapon.get(); }
    bool canFire() const { return m_weapon->canFire(); }
    void reload() { m_weapon->reload(); }
    bool isReloading() const { return m_weapon->isReloading(); }
    int getAmmo() const { return m_weapon->getAmmoCount(); }
    int getMaxAmmo() const { return m_weapon->getClipSize(); }

    // --- ダメージ ---
    void takeDamage(float amount);
    bool isDead() const { return m_health <= 0; }
    bool isInvincible() const { return m_invincibleTimer > 0.0f; }

    // --- ICombatTarget ---
    void collectHitColliders(std::vector<CombatHitCollider>& out) override;
    void onHit(const CombatHit& hit) override;

    // --- ゲッター ---
    Vector3 getPosition() const { return m_transform.position; }
    Vector3 getEyePosition() const { return m_transform.position + Vector3(0.0f, EYE_HEIGHT, 0.0f); }
    Vector3 getMuzzlePosition() const;
    Vector3 getForward() const;
    Vector3 getGunTip(const Matrix& view) const;
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    bool isAiming() const { return m_isAiming; }

    // --- External live tracking pointers ---
    Vector3* getPositionPtr() { return &m_transform.position; }
    Transform* getTransformPtr() { return &m_transform; }
    WeaponAnimator* getViewmodel() { return &m_viewmodel; }
    bool hasImportedRifleViewmodel() const { return m_importedRifleViewmodel != nullptr; }
    ImportedRifleViewmodelSettings& getImportedRifleViewmodelSettings() { return m_importedRifleViewmodelSettings; }
    const ImportedRifleViewmodelSettings& getImportedRifleViewmodelSettings() const { return m_importedRifleViewmodelSettings; }
    void resetImportedRifleViewmodelSettings();

    // --- セッター ---
    void setPosition(const Vector3& pos) { m_transform.position = pos; }
    void setHealth(float health) { m_health = health; }
    void setAiming(bool aiming) { m_isAiming = aiming; }
    void reset();

private:
    // --- 定数 ---
    static constexpr float INVINCIBLE_DURATION = 0.5f;
    static constexpr float MOVE_THRESHOLD      = 0.001f;
    static constexpr float ARENA_HALF_SIZE     = 195.0f;
    static constexpr float LOOK_PITCH_CLAMP    = 89.0f;
    static constexpr float EYE_HEIGHT          = 2.3f;

    // ビューモデル（銃）のオフセット: 右、下、前
    static constexpr float GUN_OFFSET_RIGHT   =  0.3f;
    static constexpr float GUN_OFFSET_DOWN    = -0.25f;
    static constexpr float GUN_OFFSET_FORWARD =  0.7f;

    // ADS ビューモデルオフセット：中央、やや上、前方
    static constexpr float ADS_OFFSET_RIGHT   = 0.0f;
    static constexpr float ADS_OFFSET_DOWN    = -0.085f;
    static constexpr float ADS_OFFSET_FORWARD = 0.35f;
    static constexpr float ADS_BLEND_SPEED    = 10.0f;

    static constexpr float GUN_TIP_Z = 0.3f;

    // --- 内部メソッド ---
    bool loadGunFromJson(const std::string& path);
    bool loadImportedRifleViewmodel(const std::string& path);
    void updateTimers(float deltaTime);
    void updateGravity(float deltaTime);

    // --- メンバ変数 ---
    SceneContext* m_context;

    // 描画
    std::vector<GunPart> m_gunParts;
    Color m_gunColor = Color(0.043f, 0.059f, 0.078f, 1.0f);
    const ImportedModel* m_importedRifleViewmodel = nullptr;
    ImportedRifleViewmodelSettings m_importedRifleViewmodelSettings;
    Vector3 m_importedRifleViewmodelCenter = Vector3::Zero;
    float m_importedRifleViewmodelLongestSide = 1.0f;

    Matrix m_gunWorldMatrix;

    // ビューモデルのプロシージャルアニメーション
    WeaponAnimator m_viewmodel;

    std::unique_ptr<Weapon> m_weapon;

    // 移動
    float m_speed;
    Vector3 m_lastPosition = Vector3::Zero;

    // 視点（マウスルック）
    float m_lookYaw          = 0.0f;
    float m_lookPitch        = 0.0f;
    float m_mouseSensitivity = 50.0f;

    // 体力
    float m_health;
    float m_maxHealth;
    float m_invincibleTimer = 0.0f;

    // ジャンプ
    bool m_isGrounded;
    float m_verticalVelocity;
    float m_jumpForce;
    float m_gravity;
    float m_groundLevel;

    // ADS
    bool  m_isAiming = false;
    float m_adsBlend = 0.0f;
};
