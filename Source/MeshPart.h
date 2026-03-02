#pragma once
#include "DeviceResources.h"
#include "GeometricPrimitive.h"
#include <SimpleMath.h>
#include <vector>

using namespace DirectX::SimpleMath;

// A single primitive with local transform - building block for composite models
struct MeshPart
{
    std::unique_ptr<DirectX::GeometricPrimitive> mesh;
    Vector3 localPosition;
    Vector3 localRotation;  // Euler angles (radians)
    Color color;

    // Factory functions - create MeshPart from primitives
    static MeshPart CreateBox(
        ID3D11DeviceContext* ctx,
        const XMFLOAT3& size,
        const Vector3& pos,
        const Vector3& rot,
        const Color& color);

    static MeshPart CreateCone(
        ID3D11DeviceContext* ctx,
        float height,
        float radius,
        const Vector3& pos,
        const Vector3& rot,
        const Color& color,
        size_t tessellation = 32);

    static MeshPart CreateCylinder(
        ID3D11DeviceContext* ctx,
        float height,
        float radius,
        const Vector3& pos,
        const Vector3& rot,
        const Color& color);

    static MeshPart CreateTorus(   //　トーラス
        ID3D11DeviceContext* ctx,  //　メッシュ
        float diameter,            //　直径（リングの大きさ）
        float thickness,           //　厚さ（チューブの太さ）
        const Vector3& pos,        //　ローカル　位置
        const Vector3& rot,        //　ローカル　回転
        const Color& color);       //　色

    static MeshPart CreateSphere(
        ID3D11DeviceContext* ctx,
        float diameter,
        const Vector3& pos,
        const Color& color);

    static MeshPart CreateIcosahedron(
        ID3D11DeviceContext* ctx,
        float size,
        const Vector3& pos,
        const Color& color);

    static MeshPart CreateDodecahedron(
        ID3D11DeviceContext* ctx,
        float size,
        const Vector3& pos,
        const Color& color);

    static MeshPart CreateOctahedron(
        ID3D11DeviceContext* ctx,
        float size,
        const Vector3& pos,
        const Color& color);

    static MeshPart CreateGeoSphere(
        ID3D11DeviceContext* ctx,
        float diameter,
        const Vector3& pos,
        const Color& color,
        size_t tessellation = 3);
};
