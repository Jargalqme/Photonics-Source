#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

struct Transform
{
    // TRANSFORM DATA
    Vector3 scale    = { 1.0f, 1.0f, 1.0f };    // Size multiplier
    Vector3 rotation = { 0.0f, 0.0f, 0.0f };    // Euler angles in RADIANS (pitch, yaw, roll)
    Vector3 position = { 0.0f, 0.0f, 0.0f };    // World pos (x, y, z)

    // GET TRANSFORMATION MATRIX
    Matrix getMatrix() const
    {
        Matrix S = Matrix::CreateScale(scale);

        Matrix R = Matrix::CreateFromYawPitchRoll(
            rotation.y,     // yaw    (Y-axis) - turn left/right
            rotation.x,     // pitch  (X-axis) - look up/down
            rotation.z      // roll   (Z-axis) - tilt sideways
        );

        Matrix T = Matrix::CreateTranslation(position);

        return S * R * T;
    }

    void reset()
    {
        scale    = Vector3::One;
        rotation = Vector3::Zero;
        position = Vector3::Zero;
    }
#if 0
    Matrix getMatrix() const
    {
        return Matrix::CreateScale(scale) *
               Matrix::CreateFromYawPitchRoll(rotation.y, rotation.x, rotation.z) *
               Matrix::CreateTranslation(position);
    }
#endif
};