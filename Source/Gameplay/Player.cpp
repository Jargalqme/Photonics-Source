#include "pch.h"
#include "Gameplay/Player.h"
#include "Gameplay/Combat/WeaponRifle.h"
#include "Gameplay/Events/EventBus.h"
#include "Gameplay/Events/EventTypes.h"
#include "Services/SceneContext.h"
#include "Render/MeshCache.h"
#include "Render/ImportedModel.h"
#include "Render/ImportedModelCache.h"
#include "Render/RenderCommandQueue.h"
#include "ThirdParty/json.hpp"
#include <fstream>
#include <limits>

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace
{
    constexpr const char* IMPORTED_RIFLE_VIEWMODEL_PATH = "Assets/Weapons/Rifle/rifle.glb";

    struct ImportedModelBounds
    {
        Vector3 center = Vector3::Zero;
        float longestSide = 1.0f;
    };

    ImportedModelBounds CalculateImportedModelBounds(const ImportedModel& model)
    {
        const auto& vertices = model.data().vertices;
        if (vertices.empty())
        {
            return ImportedModelBounds{};
        }

        const float maxFloat = std::numeric_limits<float>::max();
        Vector3 minBounds(maxFloat, maxFloat, maxFloat);
        Vector3 maxBounds(-maxFloat, -maxFloat, -maxFloat);

        for (const auto& vertex : vertices)
        {
            minBounds.x = std::min(minBounds.x, vertex.position.x);
            minBounds.y = std::min(minBounds.y, vertex.position.y);
            minBounds.z = std::min(minBounds.z, vertex.position.z);
            maxBounds.x = std::max(maxBounds.x, vertex.position.x);
            maxBounds.y = std::max(maxBounds.y, vertex.position.y);
            maxBounds.z = std::max(maxBounds.z, vertex.position.z);
        }

        const Vector3 size = maxBounds - minBounds;
        const float longestSide = std::max({ size.x, size.y, size.z });
        ImportedModelBounds bounds;
        bounds.center = (minBounds + maxBounds) * 0.5f;
        bounds.longestSide = longestSide > 0.001f ? longestSide : 1.0f;
        return bounds;
    }

    Matrix CreateImportedRifleViewmodelLocalMatrix(
        const Vector3& center,
        float longestSide,
        const ImportedRifleViewmodelSettings& settings)
    {
        const float scale = settings.targetLength / std::max(longestSide, 0.001f);
        const Vector3 rotationRadians(
            XMConvertToRadians(settings.rotationDegrees.x),
            XMConvertToRadians(settings.rotationDegrees.y),
            XMConvertToRadians(settings.rotationDegrees.z));

        return Matrix::CreateTranslation(-center)
            * Matrix::CreateScale(scale)
            * Matrix::CreateFromYawPitchRoll(
                rotationRadians.y,
                rotationRadians.x,
                rotationRadians.z)
            * Matrix::CreateTranslation(settings.position);
    }
}

// === 生成・初期化 ===

Player::Player(SceneContext& context)
    : m_context(&context)
    , m_speed(15.0f)
    , m_health(100.0f)
    , m_maxHealth(100.0f)
    , m_isGrounded(true)
    , m_verticalVelocity(0.0f)
    , m_jumpForce(15.0f)
    , m_gravity(40.0f)
    , m_groundLevel(0.0f)
    , m_invincibleTimer(0.0f)
    , m_weapon(std::make_unique<WeaponRifle>())
{
}

void Player::initialize()
{
    loadGunFromJson("Assets/Weapons/gun.json");
    loadImportedRifleViewmodel(IMPORTED_RIFLE_VIEWMODEL_PATH);
    m_weapon->initialize();
}

bool Player::loadGunFromJson(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    nlohmann::json data;
    file >> data;

    MeshCache* meshes = m_context->meshes;

    for (const auto& part : data["parts"])
    {
        GunPart gp;
        const std::string type = part["type"];
        float scaleCorrection = 1.0f;

        if (type == "box")
        {
            gp.primitive = meshes->getCube();
        }
        else if (type == "cylinder")
        {
            gp.primitive = meshes->getCylinder();
            scaleCorrection = 2.0f;
        }
        else if (type == "sphere")
        {
            gp.primitive = meshes->getSphere();        // tess 16 デフォルト
            scaleCorrection = 2.0f;
        }
        else if (type == "torus")
        {
            gp.primitive = meshes->getTorus(1.0f, 0.333f);  // tess 32 デフォルト
            scaleCorrection = 2.0f;
        }
        else
        {
            continue;   // 未対応タイプはスキップ
        }

        const auto& pos = part["pos"];
        const auto& rot = part["rot"];
        const auto& sc = part["scale"];

        gp.localTransform.position = Vector3(
            -(float)pos[0], // X is flipped to match LH system
             (float)pos[1],
             (float)pos[2]);

        gp.localTransform.rotation = Vector3(
            XMConvertToRadians((float)rot[0]),
            XMConvertToRadians((float)rot[1]),
            XMConvertToRadians((float)rot[2]));

        gp.localTransform.scale = Vector3(
            (float)sc[0] * scaleCorrection,
            (float)sc[1] * scaleCorrection,
            (float)sc[2] * scaleCorrection);

        m_gunParts.push_back(std::move(gp));
    }

    return true;
}

bool Player::loadImportedRifleViewmodel(const std::string& path)
{
    m_importedRifleViewmodel = nullptr;
    m_importedRifleViewmodelCenter = Vector3::Zero;
    m_importedRifleViewmodelLongestSide = 1.0f;

    if (!m_context || !m_context->importedModels)
    {
        return false;
    }

    const ImportedModel* model = m_context->importedModels->get(path);
    if (!model)
    {
        return false;
    }

    m_importedRifleViewmodel = model;
    const ImportedModelBounds bounds = CalculateImportedModelBounds(*model);
    m_importedRifleViewmodelCenter = bounds.center;
    m_importedRifleViewmodelLongestSide = bounds.longestSide;
    return true;
}

// === 1フレーム駆動 ===

void Player::applyLookDelta(float deltaYaw, float deltaPitch)
{
    // ヨー：[0, 360) 巻き戻し（sway デルタの連続性を保つ）
    m_lookYaw += deltaYaw;
    if (m_lookYaw > 360.0f)
    {
        m_lookYaw -= 360.0f;
    }
    if (m_lookYaw < 0.0f)
    {
        m_lookYaw += 360.0f;
    }

    // ピッチ：±89° クランプ
    m_lookPitch += deltaPitch;
    if (m_lookPitch > LOOK_PITCH_CLAMP)
    {
        m_lookPitch = LOOK_PITCH_CLAMP;
    }
    if (m_lookPitch < -LOOK_PITCH_CLAMP)
    {
        m_lookPitch = -LOOK_PITCH_CLAMP;
    }
}

void Player::applyMovement(const Vector3& direction, float aimYaw, float deltaTime)
{
    // 視点方向に向く
    m_transform.rotation.y = aimYaw;

    // 入力方向に移動（0 ベクトルなら回転と重力のみ＝旧 updateIdle と等価）
    Vector3 moveDir = direction;
    moveDir.y = 0.0f;
    if (moveDir.LengthSquared() > MOVE_THRESHOLD)
    {
        moveDir.Normalize();
        m_transform.position += moveDir * m_speed * deltaTime;
    }

    // アリーナ境界クランプ
    m_transform.position.x = std::clamp(m_transform.position.x, -ARENA_HALF_SIZE, ARENA_HALF_SIZE);
    m_transform.position.z = std::clamp(m_transform.position.z, -ARENA_HALF_SIZE, ARENA_HALF_SIZE);

    updateGravity(deltaTime);
}

void Player::tick(float deltaTime)
{
    updateTimers(deltaTime);
    updateGravity(deltaTime);

    // ADS ブレンド補間
    float target = m_isAiming ? 1.0f : 0.0f;
    m_adsBlend += (target - m_adsBlend) * std::min(1.0f, ADS_BLEND_SPEED * deltaTime);

    // ビューモデルへゲーム状態を供給（bob 用）
    Vector3 delta = m_transform.position - m_lastPosition;
    delta.y = 0.0f;
    float horizSpeed = (deltaTime > 0.0f) ? delta.Length() / deltaTime : 0.0f;
    m_viewmodel.setGameParams({ horizSpeed, m_isGrounded });
    m_lastPosition = m_transform.position;

    m_viewmodel.update(deltaTime);
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
    // 無敵時間
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
            m_viewmodel.onLand(-m_verticalVelocity);   // 下向き速度を正値に
            m_verticalVelocity = 0.0f;
            m_isGrounded = true;
        }
    }
}

// === ICombatTarget ===

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

    takeDamage(hit.finalDamage);  // 内部で iframe をセット + 体力クランプ
    EventBus::publish(PlayerDamagedEvent{
        m_transform.position,
        hit.finalDamage,
        m_health,
        m_maxHealth });
}

// === ダメージ ===

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

// === 描画 ===

void Player::submitViewmodel(RenderCommandQueue& queue, const Matrix& view)
{
    float r = GUN_OFFSET_RIGHT + (ADS_OFFSET_RIGHT - GUN_OFFSET_RIGHT) * m_adsBlend;
    float d = GUN_OFFSET_DOWN + (ADS_OFFSET_DOWN - GUN_OFFSET_DOWN) * m_adsBlend;
    float f = GUN_OFFSET_FORWARD + (ADS_OFFSET_FORWARD - GUN_OFFSET_FORWARD) * m_adsBlend;

    Vector3 proc = m_viewmodel.getOffset().pos;
    r += proc.x;
    d += proc.y;
    f += proc.z;

    Matrix camWorld = view.Invert();
    Matrix gunOffset = Matrix::CreateTranslation(r, d, f);
    Matrix gunWorld = gunOffset * camWorld;
    m_gunWorldMatrix = gunWorld;

    if (m_importedRifleViewmodel)
    {
        ImportedModelCommand command;
        command.model = m_importedRifleViewmodel;
        command.world = CreateImportedRifleViewmodelLocalMatrix(
            m_importedRifleViewmodelCenter,
            m_importedRifleViewmodelLongestSide,
            m_importedRifleViewmodelSettings) * gunWorld;
        command.color = Color(1.0f, 1.0f, 1.0f, 1.0f);
        queue.submit(command);
        return;
    }

    for (const auto& gp : m_gunParts)
    {
        if (!gp.primitive)
        {
            continue;
        }

        MeshCommand command;
        command.mesh = gp.primitive;
        command.world = gp.localTransform.getMatrix() * gunWorld;
        command.color = m_gunColor;
        queue.submit(command);
    }
}

void Player::finalize()
{
    m_gunParts.clear();
    m_importedRifleViewmodel = nullptr;
    m_importedRifleViewmodelCenter = Vector3::Zero;
    m_importedRifleViewmodelLongestSide = 1.0f;
}

// === 視点 ===

void Player::resetImportedRifleViewmodelSettings()
{
    m_importedRifleViewmodelSettings = ImportedRifleViewmodelSettings{};
}

Vector3 Player::getLookForward() const
{
    float yawRad = XMConvertToRadians(m_lookYaw);
    float pitchRad = XMConvertToRadians(m_lookPitch);
    Vector3 forward;
    forward.x = sinf(yawRad) * cosf(pitchRad);
    forward.y = -sinf(pitchRad);
    forward.z = cosf(yawRad) * cosf(pitchRad);
    forward.Normalize();
    return forward;
}

Vector3 Player::getLookRight() const
{
    Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
    Vector3 right = worldUp.Cross(getLookForward());
    right.Normalize();
    return right;
}

// === ユーティリティ ===

Vector3 Player::getMuzzlePosition() const
{
    // ヒップ／ADS のブレンド（submitViewmodel と同じ式）
    float r = GUN_OFFSET_RIGHT   + (ADS_OFFSET_RIGHT   - GUN_OFFSET_RIGHT)   * m_adsBlend;
    float d = GUN_OFFSET_DOWN    + (ADS_OFFSET_DOWN    - GUN_OFFSET_DOWN)    * m_adsBlend;
    float f = GUN_OFFSET_FORWARD + (ADS_OFFSET_FORWARD - GUN_OFFSET_FORWARD) * m_adsBlend;

    // プロシージャル sway / bob / recoil（submitViewmodel と同じ）
    Vector3 proc = m_viewmodel.getOffset().pos;
    r += proc.x;
    d += proc.y;
    f += proc.z;

    // 視点空間で組み立て（CryEngine の ShootFromHelper 相当：visual barrel）
    Vector3 fwd   = getLookForward();
    Vector3 right = getLookRight();
    Vector3 up    = fwd.Cross(right);
    up.Normalize();

    return getEyePosition() + right * r + up * d + fwd * (f + GUN_TIP_Z);
}

Vector3 Player::getGunTip(const Matrix& view) const
{
    (void)view;
    Vector3 tip = Vector3::Transform(Vector3(0.0f, 0.0f, GUN_TIP_Z), m_gunWorldMatrix);
    return tip;
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
    m_adsBlend = 0.0f;
    m_lookYaw = 0.0f;
    m_lookPitch = 0.0f;
    m_viewmodel.reset();
    m_weapon->initialize();
}
