#pragma once

#include <SimpleMath.h>

struct PlayerInputState
{
    bool cursorHidden = false;
    DirectX::SimpleMath::Vector2 lookDelta = DirectX::SimpleMath::Vector2::Zero;
    DirectX::SimpleMath::Vector2 move = DirectX::SimpleMath::Vector2::Zero;
    bool aimHeld = false;
    bool fireHeld = false;
    bool reloadPressed = false;
    bool jumpPressed = false;
};

