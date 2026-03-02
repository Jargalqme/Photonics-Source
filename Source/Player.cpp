#include "pch.h"
#include "Player.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Player::Player(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_boundingSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 0.5f)
    , m_speed(15.0f)
    , m_boosting(false)
    , m_boostTimer(0.0f)
    , m_boostDuration(1.5f)
    , m_boostSpeedMultiplier(2.0f)
    , m_health(100.0f)
    , m_maxHealth(100.0f)
    , m_color(Colors::Red)
    , m_isGrounded(true)
    , m_verticalVelocity(0.0f)
    , m_jumpForce(15.0f)
    , m_gravity(40.0f)
    , m_groundLevel(0.0f)
{
}

void Player::initialize()
{
    buildPlayer();
}

void Player::render(const Matrix& view, const Matrix& projection)
{
    // Vehicle base transform (position + rotation in world)
    Matrix playerWorld = m_transform.getMatrix();

    // Draw each part with its local transform
    for (const auto& part : m_parts)
    {
        // Local transform: rotation then position (relative to vehicle center)
        Matrix localParts = 
              Matrix::CreateRotationX(part.localRotation.x)
            * Matrix::CreateRotationY(part.localRotation.y)
            * Matrix::CreateRotationZ(part.localRotation.z)
            * Matrix::CreateTranslation(part.localPosition);

        // Final world = local * player
        Matrix world = localParts * playerWorld;

        part.mesh->Draw(world, view, projection, part.color);
    }
}

void Player::onDeviceLost()
{
    m_parts.clear();
}

void Player::takeDamage(float amount)
{
    m_health -= amount;
    if (m_health < 0.0f)
        m_health = 0.0f;
}

void Player::activateBoost()
{
    if (!m_boosting)
    {
        m_boosting = true;
        m_boostTimer = m_boostDuration;
    }
}

void Player::reset()
{
    m_transform.position = Vector3(0.0f, 0.0f, -20.0f);
    m_transform.rotation = Vector3::Zero;
    m_health = m_maxHealth;
    m_boosting = false;
    m_boostTimer = 0.0f;
}

Vector3 Player::getForward() const
{
    return Vector3(std::sin(m_transform.rotation.y), 0.0f, std::cos(m_transform.rotation.y));
}

void Player::moveInDirection(const Vector3& moveDirection, float aimYaw, float deltaTime)
{
    // Handle boost timer
    if (m_boosting)
    {
        m_boostTimer -= deltaTime;
        if (m_boostTimer <= 0.0f)
        {
            m_boosting = false;
            m_boostTimer = 0.0f;
        }
    }

    // MOVEMENT: Move in WASD direction
    Vector3 moveDir = moveDirection;
    moveDir.y = 0.0f;  // Keep on XZ plane

    // FACING: Always face camera yaw (crosshair direction)
    m_transform.rotation.y = aimYaw;

    if (moveDir.LengthSquared() > 0.001f)
    {
        moveDir.Normalize();
        // Apply speed (with boost if active)
        float currentSpeed = m_speed;
        if (m_boosting)
        {
            currentSpeed *= m_boostSpeedMultiplier;
        }

        m_transform.position += moveDir * currentSpeed * deltaTime;
    }

    // GRAVITY & JUMP
    if (!m_isGrounded)
    {
        // Apply gravity (reduce vertical velocity over time)
        m_verticalVelocity -= m_gravity * deltaTime;

        // Move vertically
        m_transform.position.y += m_verticalVelocity * deltaTime;

        // Check if landed
        if (m_transform.position.y <= m_groundLevel)
        {
            m_transform.position.y = m_groundLevel;
            m_verticalVelocity = 0.0f;
            m_isGrounded = true;
        }
    }

    // Update bounding sphere
    m_boundingSphere.Center.x = m_transform.position.x;
    m_boundingSphere.Center.y = m_transform.position.y;
    m_boundingSphere.Center.z = m_transform.position.z;

    // Clamp to arena
    //float distanceFromCenter = m_transform.position.Length();
    //if (distanceFromCenter > m_arenaRadius)
    //{
    //    m_transform.position.Normalize();
    //    m_transform.position *= m_arenaRadius;
    //}
}

void Player::jump()
{
    // Can only jump if on ground
    if (m_isGrounded)
    {
        m_verticalVelocity = m_jumpForce;
        m_isGrounded = false;
    }
}

void Player::updateIdle(float aimYaw, float deltaTime)
{
    // Handle boost timer
    if (m_boosting)
    {
        m_boostTimer -= deltaTime;
        if (m_boostTimer <= 0.0f)
        {
            m_boosting = false;
            m_boostTimer = 0.0f;
        }
    }

    // Always face camera yaw (crosshair direction)
    m_transform.rotation.y = aimYaw;

    // Apply gravity (if jumping)
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

    // Update bounding sphere
    m_boundingSphere.Center.x = m_transform.position.x;
    m_boundingSphere.Center.y = m_transform.position.y;
    m_boundingSphere.Center.z = m_transform.position.z;
}

void Player::buildPlayer()
{
    auto ctx = m_deviceResources->GetD3DDeviceContext();
    m_parts.clear();

    m_parts.push_back(MeshPart::CreateIcosahedron(ctx,
        1.3f,
        { 0.0f, 0.0f, 0.0f },
        m_color));

    m_parts.push_back(MeshPart::CreateOctahedron(ctx,
        0.42f,
        { 0.0f, 2.15f, 0.0f },
        m_color));

    m_parts.push_back(MeshPart::CreateSphere(ctx,
        0.24f,
        { 0.0f, 2.15f, 0.45f },
        m_color));

    // Update bounding sphere for collision
    m_boundingSphere.Radius = 1.5f;
}
