#include "pch.h"
#include "Gameplay/Player.h"
#include "Gameplay/Combat/WeaponRifle.h"
#include "Gameplay/EventBus.h"
#include "Gameplay/EventTypes.h"
#include "Services/SceneContext.h"
#include "Render/RenderCommandQueue.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Player::Player(SceneContext& context)
    : m_context(&context)
    , m_weapon(std::make_unique<WeaponRifle>())
{
}

void Player::initialize()
{
    m_viewmodelMesh.loadImportedRifle(*m_context, IMPORTED_RIFLE_VIEWMODEL_PATH);
    m_weapon->initialize();
}

Vector2 Player::applyLookDelta(float deltaYaw, float deltaPitch)
{
    const float previousPitch = m_lookPitch;

    m_lookYaw += deltaYaw;
    if (m_lookYaw > 360.0f)
    {
        m_lookYaw -= 360.0f;
    }
    if (m_lookYaw < 0.0f)
    {
        m_lookYaw += 360.0f;
    }

    m_lookPitch += deltaPitch;
    if (m_lookPitch > LOOK_PITCH_CLAMP)
    {
        m_lookPitch = LOOK_PITCH_CLAMP;
    }
    if (m_lookPitch < -LOOK_PITCH_CLAMP)
    {
        m_lookPitch = -LOOK_PITCH_CLAMP;
    }

    return Vector2(deltaYaw, m_lookPitch - previousPitch);
}

void Player::applyMovement(const Vector3& direction, float aimYaw, float deltaTime)
{
    m_transform.rotation.y = aimYaw;

    Vector3 moveDir = direction;
    moveDir.y = 0.0f;
    if (moveDir.LengthSquared() > MOVE_THRESHOLD)
    {
        moveDir.Normalize();
        m_transform.position += moveDir * m_speed * deltaTime;
    }

    m_transform.position.x = std::clamp(m_transform.position.x, -ARENA_HALF_SIZE, ARENA_HALF_SIZE);
    m_transform.position.z = std::clamp(m_transform.position.z, -ARENA_HALF_SIZE, ARENA_HALF_SIZE);
}

void Player::tick(float deltaTime, const Vector2& weaponLookDeltaDeg)
{
    updateTimers(deltaTime);
    updateGravity(deltaTime);

    Vector3 delta = m_transform.position - m_lastPosition;
    delta.y = 0.0f;
    const float horizSpeed = (deltaTime > 0.0f) ? delta.Length() / deltaTime : 0.0f;
    m_lastPosition = m_transform.position;

    WeaponAnimationInput animationInput;
    animationInput.deltaTime = deltaTime;
    animationInput.lookDeltaDegrees = weaponLookDeltaDeg;
    animationInput.moveSpeed01 = m_speed > 0.0f ? horizSpeed / m_speed : 0.0f;
    animationInput.isGrounded = m_isGrounded;
    animationInput.isAiming = m_isAiming;

    m_viewmodelAnimator.update(animationInput);
}

void Player::updateWeapon(
    float deltaTime,
    const Vector3& hitScanOrigin,
    const Vector3& hitScanDirection,
    const Vector3& tracerStart,
    std::vector<ShotIntent>& outIntents)
{
    if (m_weapon->update(deltaTime, hitScanOrigin, hitScanDirection, tracerStart, outIntents))
    {
        m_viewmodelAnimator.onWeaponFired();
    }
}

void Player::jump()
{
    if (m_isGrounded)
    {
        m_verticalVelocity = m_jumpForce;
        m_isGrounded = false;
    }
}

void Player::updateTimers(float deltaTime)
{
    if (m_invincibleTimer > 0.0f)
    {
        m_invincibleTimer -= deltaTime;
        if (m_invincibleTimer < 0.0f)
        {
            m_invincibleTimer = 0.0f;
        }
    }
}

void Player::updateGravity(float deltaTime)
{
    if (!m_isGrounded)
    {
        m_verticalVelocity -= m_gravity * deltaTime;
        m_transform.position.y += m_verticalVelocity * deltaTime;

        if (m_transform.position.y <= m_groundLevel)
        {
            m_transform.position.y = m_groundLevel;
            m_verticalVelocity = 0.0f;
            m_isGrounded = true;
        }
    }
}

void Player::collectHitColliders(std::vector<CombatHitCollider>& out)
{
    if (isDead() || isInvincible())
    {
        return;
    }

    CombatHitCollider c;
    c.target = this;
    c.faction = CombatFaction::Player;
    c.part = HitPart::Body;
    c.bounds.Center = XMFLOAT3(
        m_transform.position.x,
        m_transform.position.y + 1.2f,
        m_transform.position.z);
    c.bounds.Radius = 1.2f;
    c.damageMultiplier = 1.0f;
    out.push_back(c);
}

void Player::onHit(const CombatHit& hit)
{
    if (isDead() || isInvincible())
    {
        return;
    }

    takeDamage(hit.finalDamage);
    EventBus::publish(PlayerDamagedEvent{
        m_transform.position,
        hit.finalDamage,
        m_health,
        m_maxHealth });
}

void Player::takeDamage(float amount)
{
    if (isInvincible())
    {
        return;
    }

    m_health -= amount;
    if (m_health < 0.0f)
    {
        m_health = 0.0f;
    }

    m_invincibleTimer = INVINCIBLE_DURATION;
}

void Player::submitViewmodel(RenderCommandQueue& queue, const Matrix& view)
{
    const Matrix cameraWorld = view.Invert();
    m_viewmodelMesh.submit(queue, createViewmodelRootWorldMatrix(cameraWorld));
}

void Player::finalize()
{
    m_viewmodelMesh.finalize();
}

Matrix Player::createGameplayCameraWorldMatrix() const
{
    const Vector3 eyePosition = getEyePosition();
    const Vector3 forward = getLookForward();
    const Vector3 right = getLookRight();
    Vector3 up = forward.Cross(right);
    up.Normalize();

    const Matrix view = XMMatrixLookAtLH(eyePosition, eyePosition + forward, up);
    return view.Invert();
}

Matrix Player::createViewmodelRootWorldMatrix(const Matrix& cameraWorld) const
{
    const WeaponAnimationOutput animation = m_viewmodelAnimator.getOutput();
    const Vector3 rotationRadians(
        XMConvertToRadians(animation.rotationDegrees.x),
        XMConvertToRadians(animation.rotationDegrees.y),
        XMConvertToRadians(animation.rotationDegrees.z));

    const Matrix animationRotation = Matrix::CreateFromYawPitchRoll(
        rotationRadians.y,
        rotationRadians.x,
        rotationRadians.z);

    return animationRotation
        * Matrix::CreateTranslation(animation.position)
        * cameraWorld;
}

Vector3 Player::getLookForward() const
{
    const float yawRad = XMConvertToRadians(m_lookYaw);
    const float pitchRad = XMConvertToRadians(m_lookPitch);
    Vector3 forward;
    forward.x = sinf(yawRad) * cosf(pitchRad);
    forward.y = -sinf(pitchRad);
    forward.z = cosf(yawRad) * cosf(pitchRad);
    forward.Normalize();
    return forward;
}

Vector3 Player::getLookRight() const
{
    const Vector3 worldUp(0.0f, 1.0f, 0.0f);
    Vector3 right = worldUp.Cross(getLookForward());
    right.Normalize();
    return right;
}

Vector3 Player::getMuzzlePosition() const
{
    const Matrix cameraWorld = createGameplayCameraWorldMatrix();
    const Matrix rootWorld = createViewmodelRootWorldMatrix(cameraWorld);
    const Matrix modelWorld = m_viewmodelMesh.buildModelWorldMatrix(rootWorld);
    return Vector3::Transform(m_viewmodelMesh.getMuzzleLocalPosition(), modelWorld);
}

Vector3 Player::getForward() const
{
    return Vector3(std::sin(m_transform.rotation.y), 0.0f, std::cos(m_transform.rotation.y));
}

void Player::reset()
{
    m_transform.position = Vector3(0.0f, 0.0f, -20.0f);
    m_transform.rotation = Vector3::Zero;
    m_health = m_maxHealth;
    m_invincibleTimer = 0.0f;
    m_isAiming = false;
    m_lookYaw = 0.0f;
    m_lookPitch = 0.0f;
    m_viewmodelAnimator.reset();
    m_weapon->initialize();
}
