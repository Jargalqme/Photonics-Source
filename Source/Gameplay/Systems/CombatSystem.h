#pragma once

#include "Gameplay/Combat/ICombatTarget.h"

#include <vector>

class BulletPool;
struct ShotIntent;

class CombatSystem
{
public:
    CombatSystem() = default;

    void update(
        float deltaTime,
        std::vector<ICombatTarget*>& targets,
        BulletPool& bullets,
        std::vector<ShotIntent>& shotIntents);

private:
    void collectColliders(std::vector<ICombatTarget*>& targets);
    void resolveShotIntents(std::vector<ShotIntent>& intents);
    void resolveBullets(BulletPool& bullets);

    std::vector<CombatHitCollider> m_colliders;  // フレーム内再利用、毎フレームクリア
};
