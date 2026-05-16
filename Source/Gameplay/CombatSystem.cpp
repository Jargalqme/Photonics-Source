#include "pch.h"
#include "CombatSystem.h"

#include "Gameplay/Bullet.h"
#include "Gameplay/BulletPool.h"
#include "Gameplay/Combat/CollisionSystem.h"
#include "Gameplay/Combat/ShotIntent.h"
#include "Gameplay/EventBus.h"
#include "Gameplay/EventTypes.h"

void CombatSystem::update(
    float deltaTime,
    std::vector<ICombatTarget*>& targets,
    BulletPool& bullets,
    std::vector<ShotIntent>& shotIntents)
{
    bullets.update(deltaTime);
    collectColliders(targets);
    resolveShotIntents(shotIntents);
    resolveBullets(bullets);
}

void CombatSystem::collectColliders(std::vector<ICombatTarget*>& targets)
{
    m_colliders.clear();
    for (ICombatTarget* target : targets)
    {
        if (target)
        {
            target->collectHitColliders(m_colliders);
        }
    }
}

void CombatSystem::resolveShotIntents(std::vector<ShotIntent>& intents)
{
    for (const auto& intent : intents)
    {
        float closest = intent.maxRange;
        const CombatHitCollider* hitCollider = nullptr;

        for (const auto& collider : m_colliders)
        {
            if (collider.faction == intent.faction)
            {
                continue;  // フレンドリーファイア除外
            }

            float dist = 0.0f;
            if (CollisionSystem::checkRaySphere(intent.hitScanOrigin, intent.hitScanDirection, collider.bounds, dist)
                && dist < closest)
            {
                closest = dist;
                hitCollider = &collider;
            }
        }

        const Vector3 hitPoint = hitCollider
            ? (intent.hitScanOrigin + intent.hitScanDirection * closest)
            : (intent.hitScanOrigin + intent.hitScanDirection * intent.maxRange);

        if (hitCollider)
        {
            CombatHit hit;
            hit.part = hitCollider->part;
            hit.point = hitPoint;
            hit.direction = intent.hitScanDirection;
            hit.rawDamage = intent.damage;
            hit.finalDamage = intent.damage * hitCollider->damageMultiplier;
            hitCollider->target->onHit(hit);
        }

        // ヒット有無に関わらずトレーサー発行（外れたら最大射程まで線を描く）
        EventBus::publish(ShotResolvedEvent{ intent.tracerStart, hitPoint, intent.tracerColor });
    }

    intents.clear();
}

void CombatSystem::resolveBullets(BulletPool& bullets)
{
    Bullet* arr = bullets.getBullets();
    const int n = bullets.getMaxBullets();

    for (int i = 0; i < n; i++)
    {
        if (!arr[i].isActive())
        {
            continue;
        }

        const CombatFaction bulletFaction = arr[i].getFaction();

        for (const auto& collider : m_colliders)
        {
            if (collider.faction == bulletFaction)
            {
                continue;  // フレンドリーファイア除外
            }

            if (CollisionSystem::checkSphereSphere(arr[i].getBoundingSphere(), collider.bounds))
            {
                CombatHit hit;
                hit.part = collider.part;
                hit.point = arr[i].getPosition();
                hit.direction = arr[i].getDirection();
                hit.rawDamage = arr[i].getDamage();
                hit.finalDamage = arr[i].getDamage() * collider.damageMultiplier;
                collider.target->onHit(hit);
                arr[i].deactivate();
                break;  // 1発の弾は1ターゲットにのみヒット
            }
        }
    }
}
