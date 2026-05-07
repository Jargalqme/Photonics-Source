#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

struct Transform
{
    Vector3 scale    = { 1.0f, 1.0f, 1.0f };
    Vector3 rotation = { 0.0f, 0.0f, 0.0f };  // オイラー角（ラジアン: pitch, yaw, roll）
    Vector3 position = { 0.0f, 0.0f, 0.0f };

    Matrix getMatrix() const
    {
        Matrix S = Matrix::CreateScale(scale);

        Matrix R = Matrix::CreateFromYawPitchRoll(
            rotation.y,  // yaw   (Y軸) — 左右回転
            rotation.x,  // pitch (X軸) — 上下回転
            rotation.z   // roll  (Z軸) — 傾き
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
};
