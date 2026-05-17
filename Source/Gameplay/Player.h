#pragma once

#include <DirectXCollision.h>
#include <memory>
#include <vector>

#include "Common/Transform.h"
#include "Gameplay/Combat/ICombatTarget.h"
#include "Gameplay/Combat/Weapon.h"
#include "Gameplay/Combat/WeaponAnimator.h"
#include "Gameplay/PlayerViewmodel.h"

struct SceneContext;
class InputManager;
class RenderCommandQueue;

class Player : public ICombatTarget
{
public:
    explicit Player(SceneContext& context);

    void initialize();
    void finalize();
    void reset();
    void update(InputManager& input, float deltaTime, std::vector<ShotIntent>& outIntents);
    void clearInputState();

    void renderWeapon(RenderCommandQueue& queue, const Matrix& view);

    float getLookYaw() const { return m_lookYaw; }
    float getLookPitch() const { return m_lookPitch; }
    Vector3 getLookForward() const;
    Vector3 getLookRight() const;
    Vector3 getForward() const;

    Vector3 getPosition() const { return m_transform.position; }
    Vector3 getEyePosition() const { return m_transform.position + Vector3(0.0f, EYE_HEIGHT, 0.0f); }
    Vector3* getPositionPtr() { return &m_transform.position; }

    bool isGrounded() const { return m_isGrounded; }
    bool isAiming() const { return m_isAiming; }
    bool isDead() const { return m_health <= 0.0f; }
    bool isInvincible() const { return m_invincibleTimer > 0.0f; }
    bool isReloading() const { return m_weapon->isReloading(); }
    void takeDamage(float amount);

    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    int getAmmo() const { return m_weapon->getAmmoCount(); }
    int getMaxAmmo() const { return m_weapon->getClipSize(); }

    float getMouseSensitivity() const { return m_mouseSensitivity; }
    float* getMouseSensitivityPtr() { return &m_mouseSensitivity; }
    Transform* getTransformPtr() { return &m_transform; }
    WeaponAnimator* getViewmodelAnimator() { return &m_viewmodelAnimator; }

    bool hasImportedRifleViewmodel() const { return m_viewmodelMesh.hasImportedRifle(); }
    ImportedRifleViewmodelSettings& getImportedRifleViewmodelSettings() { return m_viewmodelMesh.settings(); }
    const ImportedRifleViewmodelSettings& getImportedRifleViewmodelSettings() const { return m_viewmodelMesh.settings(); }
    void resetImportedRifleViewmodelSettings() { m_viewmodelMesh.resetSettings(); }

    void setPosition(const Vector3& position) { m_transform.position = position; }
    void setHealth(float health) { m_health = health; }

    void collectHitColliders(std::vector<CombatHitCollider>& out) override;
    void onHit(const CombatHit& hit) override;

private:
    static constexpr float INVINCIBLE_DURATION = 0.5f;
    static constexpr float MOVE_THRESHOLD      = 0.001f;
    static constexpr float ARENA_HALF_SIZE     = 195.0f;
    static constexpr float LOOK_PITCH_CLAMP    = 89.0f;
    static constexpr float EYE_HEIGHT          = 2.3f;

    static constexpr const char* IMPORTED_RIFLE_VIEWMODEL_PATH = "Assets/Weapons/Rifle/rifle.glb";

    void updateInvincibility(float deltaTime);
    void updateVerticalMovement(float deltaTime);
    void updateWeaponAnimation(float deltaTime, const Vector2& lookDeltaDegrees);
    void setAiming(bool aiming) { m_isAiming = aiming; }
    void setFiring(bool firing);
    void reload() { m_weapon->reload(); }
    void jump();
    Vector2 applyLookDelta(float deltaYaw, float deltaPitch);
    void applyMovement(const Vector3& direction, float aimYaw, float deltaTime);
    void updateWeapon(
        float deltaTime,
        const Vector3& hitScanOrigin,
        const Vector3& hitScanDirection,
        const Vector3& tracerStart,
        std::vector<ShotIntent>& outIntents);
    Vector3 getMuzzlePosition() const;
    float calculateMoveSpeed01(float deltaTime);

    Matrix createGameplayCameraWorldMatrix() const;
    Matrix createViewmodelRootWorldMatrix(const Matrix& cameraWorld) const;

    SceneContext* m_context = nullptr;
    PlayerViewmodel m_viewmodelMesh;
    WeaponAnimator m_viewmodelAnimator;
    std::unique_ptr<Weapon> m_weapon;

    Transform m_transform;
    Vector3 m_lastPosition = Vector3::Zero;

    float m_speed = 15.0f;
    float m_lookYaw = 0.0f;
    float m_lookPitch = 0.0f;
    float m_mouseSensitivity = 0.05f;

    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
    float m_invincibleTimer = 0.0f;

    bool m_isGrounded = true;
    bool m_isAiming = false;
    float m_verticalVelocity = 0.0f;
    float m_jumpForce = 15.0f;
    float m_gravity = 40.0f;
    float m_groundLevel = 0.0f;
};
