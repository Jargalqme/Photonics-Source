#include "pch.h"
#include "EnemyBoss.h"

EnemyBoss::EnemyBoss(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_boundingSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 2.0f)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_color(Colors::Black)
{
}

void EnemyBoss::initialize()
{
    buildBoss();
}


void EnemyBoss::update(float deltaTime)
{
    m_boundingSphere.Center.x = m_position.x;
    m_boundingSphere.Center.y = m_position.y;
    m_boundingSphere.Center.z = m_position.z;
}

void EnemyBoss::render(const Matrix& view, const Matrix& projection)
{
    Matrix towerWorld = Matrix::CreateTranslation(m_position);

    for (const auto& part : m_parts)
    {
        Matrix local = Matrix::CreateRotationX(part.localRotation.x)
            * Matrix::CreateRotationY(part.localRotation.y)
            * Matrix::CreateRotationZ(part.localRotation.z)
            * Matrix::CreateTranslation(part.localPosition);

        Matrix world = local * towerWorld;
        part.mesh->Draw(world, view, projection, part.color);
    }
}

void EnemyBoss::onDeviceLost()
{
    m_parts.clear();
}

void EnemyBoss::buildBoss()
{
    auto ctx = m_deviceResources->GetD3DDeviceContext();
    m_parts.clear();

    // Colors - brutalist concrete + energy glow
    Color concrete = Color(0.12f, 0.12f, 0.14f);      // Dark concrete
    Color concreteDark = Color(0.06f, 0.06f, 0.08f);  // Darker accents
    Color glowColor = Color(1.0f, 0.0f, 0.3f);        // Menacing red/pink
    Color coreColor = Color(1.0f, 0.2f, 0.4f);        // Core glow+

    // === BASE PLATFORM ===
    //m_parts.push_back(MeshPart::CreateBox(ctx,
    //    XMFLOAT3(8.0f, 0.5f, 8.0f),
    //    Vector3(0, 0.25f, 0),
    //    Vector3::Zero,
    //    concreteDark));

    // === SUPPORT COLUMNS (4 corners) ===
    //float columnOffset = 1.8f;
    //float columnPositions[4][2] = {
    //    {-columnOffset, -columnOffset},
    //    { columnOffset, -columnOffset},
    //    {-columnOffset,  columnOffset},
    //    { columnOffset,  columnOffset}
    //};

    //for (int i = 0; i < 4; i++)
    //{
    //    m_parts.push_back(MeshPart::CreateCylinder(ctx,
    //        3.0f, 0.3f,
    //        Vector3(columnPositions[i][0], 2.0f, columnPositions[i][1]),
    //        Vector3::Zero,
    //        concrete));
    //}

    // === INVERTED PYRAMIDS (Tokyo Big Sight style) ===
    // 4 pyramids arranged in a cluster, tips pointing down
    //float pyramidOffset = 1.2f;
    //float pyramidHeight = 4.0f;
    //float pyramidRadius = 2.0f;
    float pyramidY = 5.5f;  // Elevated position

    //float pyramidPositions[4][2] = {
    //    {-pyramidOffset, -pyramidOffset},
    //    { pyramidOffset, -pyramidOffset},
    //    {-pyramidOffset,  pyramidOffset},
    //    { pyramidOffset,  pyramidOffset}
    //};

    //for (int i = 0; i < 4; i++)
    //{
    //    // Main inverted pyramid (cone flipped)
    //    m_parts.push_back(MeshPart::CreateCone(ctx,
    //        pyramidHeight, pyramidRadius,
    //        Vector3(pyramidPositions[i][0], pyramidY, pyramidPositions[i][1]),
    //        Vector3(XM_PI, 0, 0),  // Flipped upside-down!
    //        concrete,
    //        4));

    //    // Glow ring at tip of each pyramid
    //    m_parts.push_back(MeshPart::CreateTorus(ctx,
    //        0.6f, 0.05f,
    //        Vector3(pyramidPositions[i][0], pyramidY - pyramidHeight / 2 - 0.3f, pyramidPositions[i][1]),
    //        Vector3(XM_PIDIV2, 0, 0),
    //        glowColor));
    //}

    // === CENTRAL ENERGY CORE ===
    // Sphere in the middle where pyramids meet
    m_parts.push_back(MeshPart::CreateSphere(ctx,
        3.5f,
        Vector3(0, pyramidY - 1.0f, 0),
        coreColor));

    // Core containment ring
    m_parts.push_back(MeshPart::CreateTorus(ctx,
        2.0f, 0.1f,
        Vector3(0, pyramidY - 1.0f, 0),
        Vector3(XM_PIDIV2, 0, 0),
        glowColor));

    // Vertical core ring
    m_parts.push_back(MeshPart::CreateTorus(ctx,
        1.8f, 0.08f,
        Vector3(0, pyramidY - 1.0f, 0),
        Vector3(0, 0, 0),
        glowColor));

    // === TOP SPIRE ===
    m_parts.push_back(MeshPart::CreateCone(ctx,
        2.5f, 0.4f,
        Vector3(0, pyramidY + 2.5f, 0),
        Vector3::Zero,  // Pointing up
        concreteDark));

    // Spire glow tip
    m_parts.push_back(MeshPart::CreateSphere(ctx,
        0.3f,
        Vector3(0, pyramidY + 3.8f, 0),
        glowColor));

    // === LOWER STRUCTURE ===
    // Cross beams connecting columns
    //m_parts.push_back(MeshPart::CreateBox(ctx,
    //    XMFLOAT3(4.5f, 0.2f, 0.2f),
    //    Vector3(0, 3.5f, 0),
    //    Vector3::Zero,
    //    concrete));

    //m_parts.push_back(MeshPart::CreateBox(ctx,
    //    XMFLOAT3(0.2f, 0.2f, 4.5f),
    //    Vector3(0, 3.5f, 0),
    //    Vector3::Zero,
    //    concrete));

    // Update bounding sphere to encompass tower
    m_boundingSphere.Radius = 6.0f;
}