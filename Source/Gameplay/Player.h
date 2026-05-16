#pragma once
#include <DirectXCollision.h>
#include "Common/Transform.h"
#include "Gameplay/Combat/WeaponAnimator.h"
#include "Gameplay/Combat/Weapon.h"
#include "Gameplay/Combat/ICombatTarget.h"
#include "Gameplay/PlayerViewmodel.h"
#include <memory>

struct SceneContext;
class RenderCommandQueue;

class Player : public ICombatTarget
{
public:
    Player(SceneContext& context);

    void initialize();
    void submitViewmodel(RenderCommandQueue& queue, const Matrix& view);
    void finalize();

    Vector2 applyLookDelta(float deltaYaw, float deltaPitch);
    void applyMovement(const Vector3& direction, float aimYaw, float deltaTime);
    void tick(float deltaTime, const Vector2& weaponLookDeltaDeg);
    void jump();
    bool isGrounded() const { return m_isGrounded; }

    // --- Look ---
    float getLookYaw() const { return m_lookYaw; }
    float getLookPitch() const { return m_lookPitch; }
    Vector3 getLookForward() const;
    Vector3 getLookRight() const;
    float getMouseSensitivity() const { return m_mouseSensitivity; }
    float* getMouseSensitivityPtr() { return &m_mouseSensitivity; }

    // --- Weapon ---
    void startFire() { m_weapon->startFire(); }
    void stopFire() { m_weapon->stopFire(); }
    void updateWeapon(
        float deltaTime,
        const Vector3& hitScanOrigin,
        const Vector3& hitScanDirection,
        const Vector3& tracerStart,
        std::vector<ShotIntent>& outIntents);
    Weapon* getWeapon() { return m_weapon.get(); }
    const Weapon* getWeapon() const { return m_weapon.get(); }
    void reload() { m_weapon->reload(); }
    bool isReloading() const { return m_weapon->isReloading(); }
    int getAmmo() const { return m_weapon->getAmmoCount(); }
    int getMaxAmmo() const { return m_weapon->getClipSize(); }

    // --- Damage ---
    void takeDamage(float amount);
    bool isDead() const { return m_health <= 0; }
    bool isInvincible() const { return m_invincibleTimer > 0.0f; }

    // --- ICombatTarget ---
    void collectHitColliders(std::vector<CombatHitCollider>& out) override;
    void onHit(const CombatHit& hit) override;

    // --- Accessors ---
    Vector3 getPosition() const { return m_transform.position; }
    Vector3 getEyePosition() const { return m_transform.position + Vector3(0.0f, EYE_HEIGHT, 0.0f); }
    Vector3 getMuzzlePosition() const;
    Vector3 getForward() const;
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    bool isAiming() const { return m_isAiming; }

    Vector3* getPositionPtr() { return &m_transform.position; }
    Transform* getTransformPtr() { return &m_transform; }
    WeaponAnimator* getViewmodelAnimator() { return &m_viewmodelAnimator; }
    WeaponAnimator* getViewmodel() { return &m_viewmodelAnimator; }
    PlayerViewmodel& getViewmodelMesh() { return m_viewmodelMesh; }
    const PlayerViewmodel& getViewmodelMesh() const { return m_viewmodelMesh; }
    bool hasImportedRifleViewmodel() const { return m_viewmodelMesh.hasImportedRifle(); }
    ImportedRifleViewmodelSettings& getImportedRifleViewmodelSettings() { return m_viewmodelMesh.settings(); }
    const ImportedRifleViewmodelSettings& getImportedRifleViewmodelSettings() const { return m_viewmodelMesh.settings(); }
    void resetImportedRifleViewmodelSettings() { m_viewmodelMesh.resetSettings(); }

    void setPosition(const Vector3& pos) { m_transform.position = pos; }
    void setHealth(float health) { m_health = health; }
    void setAiming(bool aiming) { m_isAiming = aiming; }
    void reset();

private:
    static constexpr float INVINCIBLE_DURATION = 0.5f;
    static constexpr float MOVE_THRESHOLD      = 0.001f;
    static constexpr float ARENA_HALF_SIZE     = 195.0f;
    static constexpr float LOOK_PITCH_CLAMP    = 89.0f;
    static constexpr float EYE_HEIGHT          = 2.3f;

    static constexpr const char* IMPORTED_RIFLE_VIEWMODEL_PATH = "Assets/Weapons/Rifle/rifle.glb";

    void updateTimers(float deltaTime);
    void updateGravity(float deltaTime);
    Matrix createGameplayCameraWorldMatrix() const;
    Matrix createViewmodelRootWorldMatrix(const Matrix& cameraWorld) const;

    SceneContext* m_context = nullptr;
    PlayerViewmodel m_viewmodelMesh;
    WeaponAnimator m_viewmodelAnimator;
    std::unique_ptr<Weapon> m_weapon;

    float m_speed = 15.0f;
    Vector3 m_lastPosition = Vector3::Zero;

    float m_lookYaw          = 0.0f;
    float m_lookPitch        = 0.0f;
    float m_mouseSensitivity = 50.0f;

    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
    float m_invincibleTimer = 0.0f;

    bool m_isGrounded = true;
    float m_verticalVelocity = 0.0f;
    float m_jumpForce = 15.0f;
    float m_gravity = 40.0f;
    float m_groundLevel = 0.0f;

    bool  m_isAiming = false;

    // 旧 GameObject 由来
    Transform m_transform;
};
