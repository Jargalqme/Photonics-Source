#include "pch.h"
#include "MeshPart.h"

using namespace DirectX;

MeshPart MeshPart::CreateBox(
    ID3D11DeviceContext* ctx,
    const XMFLOAT3& size,
    const Vector3& pos,
    const Vector3& rot,
    const Color& color)
{
    MeshPart part;
    part.mesh = GeometricPrimitive::CreateBox(ctx, size, false /*rhcoords*/);
    part.localPosition = pos;
    part.localRotation = rot;
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateCone(
    ID3D11DeviceContext* ctx,
    float height,
    float radius,
    const Vector3& pos,
    const Vector3& rot,
    const Color& color,
    size_t tessellation)
{
    MeshPart part;
    part.mesh = GeometricPrimitive::CreateCone(ctx, height, radius, tessellation);
    part.localPosition = pos;
    part.localRotation = rot;
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateCylinder(
    ID3D11DeviceContext* ctx,
    float height,
    float radius,
    const Vector3& pos,
    const Vector3& rot,
    const Color& color)
{
    MeshPart part;
    part.mesh = GeometricPrimitive::CreateCylinder(ctx, height, radius);
    part.localPosition = pos;
    part.localRotation = rot;
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateTorus(
    ID3D11DeviceContext* ctx,
    float diameter,
    float thickness,
    const Vector3& pos,
    const Vector3& rot,
    const Color& color)
{
    MeshPart part;
    part.mesh = GeometricPrimitive::CreateTorus(ctx, diameter, thickness);
    part.localPosition = pos;
    part.localRotation = rot;
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateSphere(
    ID3D11DeviceContext* ctx,
    float diameter,
    const Vector3& pos,
    const Color& color)
{
    MeshPart part;
    part.mesh = GeometricPrimitive::CreateSphere(ctx, diameter);
    part.localPosition = pos;
    part.localRotation = Vector3::Zero;  // Spheres don't need rotation
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateIcosahedron(
    ID3D11DeviceContext* ctx,
    float size,
    const Vector3& pos,
    const Color& color)
{
    MeshPart part;
    part.mesh = GeometricPrimitive::CreateIcosahedron(ctx, size);
    part.localPosition = pos;
    part.localRotation = Vector3::Zero;
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateDodecahedron(
    ID3D11DeviceContext* ctx,
    float size,
    const Vector3& pos,
    const Color& color)
{
    MeshPart part;
    part.mesh = GeometricPrimitive::CreateDodecahedron(ctx, size);
    part.localPosition = pos;
    part.localRotation = Vector3::Zero;
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateOctahedron(
    ID3D11DeviceContext* ctx,
    float size,
    const Vector3& pos,
    const Color& color)
{
    MeshPart part;
    part.mesh = DirectX::GeometricPrimitive::CreateOctahedron(ctx, size);
    part.localPosition = pos;
    part.localRotation = Vector3::Zero;
    part.color = color;
    return part;
}

MeshPart MeshPart::CreateGeoSphere(
    ID3D11DeviceContext* ctx,
    float diameter,
    const Vector3& pos,
    const Color& color,
    size_t tessellation)
{
    MeshPart part;
    part.mesh = DirectX::GeometricPrimitive::CreateGeoSphere(ctx, diameter, tessellation);
    part.localPosition = pos;
    part.localRotation = Vector3::Zero;
    part.color = color;
    return part;
}