#include "pch.h"
#include "Player.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

LightCycle::LightCycle(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_boundingSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 0.5f)
    , m_speed(0.0f)
    , m_maxSpeed(15.0f)
    , m_acceleration(20.0f)
    , m_brakeForce(30.0f)
    , m_turnRate(2.5f)
    , m_friction(5.0f)
    , m_arenaRadius(100.0f)
    , m_boosting(false)
    , m_boostTimer(0.0f)
    , m_boostDuration(1.5f)
    , m_boostSpeedMultiplier(2.0f)
    , m_health(100.0f)
    , m_maxHealth(100.0f)
    , m_primaryColor(Colors::Red)
    , m_carbonColor(Color(0.15f, 0.15f, 0.15f))
    , m_glowColor(Colors::White)
    , m_isGrounded(true)
    , m_verticalVelocity(0.0f)
    , m_jumpForce(15.0f)
    , m_gravity(40.0f)
    , m_groundLevel(0.0f)
{
}

void LightCycle::Initialize()
{
    BuildWeapon();
}

void LightCycle::Update(float deltaTime)
{
    // Apply friction to forward speed
    if (m_speed > 0.0f)
    {
        m_speed -= m_friction * deltaTime;
        if (m_speed < 0.0f) m_speed = 0.0f;
    }
    else if (m_speed < 0.0f)
    {
        m_speed += m_friction * deltaTime;
        if (m_speed > 0.0f) m_speed = 0.0f;
    }

    if (m_boosting)
    {
        m_boostTimer -= deltaTime;
        if (m_boostTimer <= 0.0f)
        {
            m_boosting = false;
            m_boostTimer = 0.0f;
        }
    }

    // Move in facing direction (free movement)
    Vector3 forward = GetForward();
    float currentSpeed = m_speed;
    if (m_boosting)
    {
        currentSpeed *= m_boostSpeedMultiplier;
    }
    m_transform.position += forward * currentSpeed * deltaTime;

    // Update bounding sphere
    m_boundingSphere.Center.x = m_transform.position.x;
    m_boundingSphere.Center.y = m_transform.position.y;
    m_boundingSphere.Center.z = m_transform.position.z;

    // Clamp pos to circular arena
    float distanceFromCenter = m_transform.position.Length(); // dist from 0.0.0
    if (distanceFromCenter > m_arenaRadius)
    {
        // Normalize and scale back to edge
        m_transform.position.Normalize();
        m_transform.position *= m_arenaRadius;

        // optional: kill speed when hitting wall
        //m_speed *= 0.5;
    }
}

void LightCycle::Render(const Matrix& view, const Matrix& projection)
{
    // Vehicle base transform (position + rotation in world)
    Matrix playerWorld = m_transform.GetMatrix();

    // Draw each part with its local transform
    for (const auto& part : m_parts)
    {
        // Local transform: rotation then position (relative to vehicle center)
        Matrix localParts = Matrix::CreateRotationX(part.localRotation.x)
            * Matrix::CreateRotationY(part.localRotation.y)
            * Matrix::CreateRotationZ(part.localRotation.z)
            * Matrix::CreateTranslation(part.localPosition);

        // Final world = local * player
        Matrix world = localParts * playerWorld;

        part.mesh->Draw(world, view, projection, part.color);
    }
}

void LightCycle::OnDeviceLost()
{
    m_parts.clear();
}

void LightCycle::Accelerate(float deltaTime)
{
    m_speed += m_acceleration * deltaTime;
    if (m_speed > m_maxSpeed) m_speed = m_maxSpeed;
}

void LightCycle::Brake(float deltaTime)
{
    m_speed -= m_brakeForce * deltaTime;
    if (m_speed < -m_maxSpeed * 0.3f) m_speed = -m_maxSpeed * 0.3f; // Reverse is slower
}

void LightCycle::Turn(float direction, float deltaTime)
{
    // Original rotation logic (free steering)
    if (std::abs(m_speed) > 0.1f)
    {
        float turnAmount = direction * m_turnRate * deltaTime;
        turnAmount *= std::min(1.0f, std::abs(m_speed) / 5.0f);
        m_transform.rotation.y += turnAmount;
    }
}

void LightCycle::TakeDamage(float amount)
{
    m_health -= amount;
    if (m_health < 0.0f)
        m_health = 0.0f;
}

void LightCycle::ActivateBoost()
{
    if (!m_boosting)
    {
        m_boosting = true;
        m_boostTimer = m_boostDuration;
    }
}

void LightCycle::Reset()
{
    m_transform.position = Vector3(0.0f, 0.0f, -20.0f);
    m_transform.rotation = Vector3::Zero;
    m_speed = 0.0f;
    m_health = m_maxHealth;
    m_boosting = false;
    m_boostTimer = 0.0f;
}

Vector3 LightCycle::GetForward() const
{
    return Vector3(std::sin(m_transform.rotation.y), 0.0f, std::cos(m_transform.rotation.y));
}

void LightCycle::MoveInDirection(const Vector3& moveDirection, const Vector3& facingDirection, float deltaTime)
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

    // FACING: Face movement direction when moving, camera direction when stationary
    if (moveDir.LengthSquared() > 0.001f)
    {
        // Moving: face the direction of travel
        Vector3 faceDir = moveDir;
        faceDir.Normalize();
        m_transform.rotation.y = atan2f(faceDir.x, faceDir.z);
    }
    //else
    //{
    //    // Stationary (e.g. shooting): face camera direction
    //    Vector3 faceDir = facingDirection;
    //    faceDir.y = 0.0f;
    //    if (faceDir.LengthSquared() > 0.001f)
    //    {
    //        faceDir.Normalize();
    //        m_transform.rotation.y = atan2f(faceDir.x, faceDir.z);
    //    }
    //}

    if (moveDir.LengthSquared() > 0.001f)
    {
        moveDir.Normalize();

        // Apply speed (with boost if active)
        float currentSpeed = m_maxSpeed;
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
    float distanceFromCenter = m_transform.position.Length();
    if (distanceFromCenter > m_arenaRadius)
    {
        m_transform.position.Normalize();
        m_transform.position *= m_arenaRadius;
    }
}

void LightCycle::Jump()
{
    // Can only jump if on ground
    if (m_isGrounded)
    {
        m_verticalVelocity = m_jumpForce;
        m_isGrounded = false;
    }
}

void LightCycle::UpdateIdle(float deltaTime)
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

    // NO rotation change - player keeps facing same direction
}

void LightCycle::BuildWeapon()
{
    auto ctx = m_deviceResources->GetD3DDeviceContext();
    m_parts.clear();

    // Pulse Rifle colors
    Color bodyColor  = Color(0.16f, 0.16f, 0.25f);   // Dark blue-gray metal
    Color darkColor  = Color(0.10f, 0.10f, 0.16f);   // Darker stock/grip
    Color glowColor  = Color(0.0f, 0.94f, 1.0f);     // Cyan energy glow
    Color greenColor = Color(0.0f, 1.0f, 0.53f);     // Green charge indicator

    // === MAIN BARREL — long cylinder ===
    m_parts.push_back(MeshPart::CreateCylinder(ctx,
        2.2f, 0.12f,                          // height, radius
        Vector3(0.0f, 0.0f, 0.1f),            // position (centered forward)
        Vector3(XM_PIDIV2, 0.0f, 0.0f),       // rotation (lay flat along Z)
        bodyColor));

    // === REAR STOCK — shorter cylinder ===
    m_parts.push_back(MeshPart::CreateCylinder(ctx,
        0.8f, 0.10f,
        Vector3(0.0f, 0.0f, -1.1f),
        Vector3(XM_PIDIV2, 0.0f, 0.0f),
        darkColor));

    // === MUZZLE CONE — emitter tip ===
    m_parts.push_back(MeshPart::CreateCone(ctx,
        0.5f, 0.2f,                           // height, radius
        Vector3(0.0f, 0.0f, 1.45f),
        Vector3(XM_PIDIV2, 0.0f, 0.0f),
        glowColor));

    // === GRIP / RECEIVER — box ===
    m_parts.push_back(MeshPart::CreateBox(ctx,
        XMFLOAT3(0.14f, 0.45f, 0.2f),
        Vector3(0.0f, -0.32f, -0.3f),
        Vector3(0.15f, 0.0f, 0.0f),           // slight angle
        darkColor));

    // === ENERGY RING — front torus ===
    m_parts.push_back(MeshPart::CreateTorus(ctx,
        0.64f, 0.025f,                        // diameter, thickness
        Vector3(0.0f, 0.0f, 0.5f),
        Vector3(XM_PIDIV2, 0.0f, 0.0f),
        glowColor));

    // === ENERGY RING — rear torus ===
    m_parts.push_back(MeshPart::CreateTorus(ctx,
        0.56f, 0.025f,
        Vector3(0.0f, 0.0f, 0.9f),
        Vector3(XM_PIDIV2, 0.0f, 0.0f),
        glowColor));

    // === CHARGE INDICATOR — small sphere ===
    m_parts.push_back(MeshPart::CreateSphere(ctx,
        0.12f,                                 // diameter
        Vector3(0.0f, 0.16f, -0.1f),
        greenColor));

    // Update bounding sphere for collision
    m_boundingSphere.Radius = 1.5f;
}