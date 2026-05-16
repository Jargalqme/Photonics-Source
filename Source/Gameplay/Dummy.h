#pragma once

#include "Common/Transform.h"
#include "Gameplay/Combat/ICombatTarget.h"
#include <DirectXCollision.h>
#include <algorithm>

class RenderCommandQueue;
struct SceneContext;

class Dummy : public ICombatTarget
{
public:
    explicit Dummy(SceneContext& context);

    void initialize();
    void update(float deltaTime);
    void submitRender(RenderCommandQueue& queue) const;
    void finalize();

    void spawn(const Vector3& startPos, DirectX::GeometricPrimitive* mesh = nullptr);
    void deactivate();

    // --- ICombatTarget ---
    void collectHitColliders(std::vector<CombatHitCollider>& out) override;
    void onHit(const CombatHit& hit) override;

    bool isActive() const { return m_active; }
    bool isRespawning() const { return !m_active && m_respawnTimer > 0.0f; }
    bool isInvulnerable() const { return m_invulnerable; }

    Vector3 getPosition() const { return m_transform.position; }
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    float getRespawnTimer() const { return m_respawnTimer; }
    float getRespawnDelay() const { return m_respawnDelay; }

    void setPosition(const Vector3& pos);
    void setMoving(bool enabled, float amplitude = 3.0f, float frequency = 1.0f);
    void setInvulnerable(bool enabled) { m_invulnerable = enabled; }
    void setRespawnDelay(float delay) { m_respawnDelay = std::max(0.0f, delay); }

private:
    static constexpr float HIT_FLASH_DURATION = 0.15f;
    static constexpr float DEFAULT_HEALTH = 100.0f;
    static constexpr float DEFAULT_RESPAWN_DELAY = 1.0f;

    // 当たり判定コライダ（collectHitColliders で使用）
    static constexpr float BODY_OFFSET_Y = 1.0f;
    static constexpr float BODY_RADIUS   = 1.0f;
    static constexpr float HEAD_OFFSET_Y = 2.0f;
    static constexpr float HEAD_RADIUS   = 0.5f;
    static constexpr float WEAK_POINT_MULT = 2.0f;

    void respawn();

    SceneContext* m_context = nullptr;
    DirectX::GeometricPrimitive* m_mesh = nullptr;

    Vector3 m_spawnPosition = Vector3::Zero;
    Vector3 m_basePosition = Vector3::Zero;

    float m_health = DEFAULT_HEALTH;
    float m_maxHealth = DEFAULT_HEALTH;
    float m_respawnDelay = DEFAULT_RESPAWN_DELAY;
    float m_respawnTimer = 0.0f;

    bool m_moving = false;
    float m_moveAmplitude = 3.0f;
    float m_moveFrequency = 1.0f;
    float m_moveTime = 0.0f;

    bool m_invulnerable = false;

    Color m_originalColor = Color(0.95f, 0.92f, 0.78f);
    Color m_hitColor = Color(1.0f, 0.55f, 0.0f);
    Color m_color = m_originalColor;
    float m_hitFlashTimer = 0.0f;

    // 旧 GameObject 由来
    Transform m_transform;
    bool m_active = true;
};
