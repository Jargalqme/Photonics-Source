#include "pch.h"
#include "Gameplay/PlayerSystem.h"

#include "Gameplay/Player.h"

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using namespace DirectX;

void PlayerSystem::update(
    Player& player,
    const PlayerInputState& input,
    std::vector<ShotIntent>& outIntents,
    float deltaTime)
{
    Vector2 weaponLookDeltaDeg = Vector2::Zero;
    if (input.cursorHidden)
    {
        Vector2 look = input.lookDelta;
        float sens = player.getMouseSensitivity();
        weaponLookDeltaDeg = player.applyLookDelta(look.x * sens, look.y * sens);
    }

    player.setAiming(input.cursorHidden && input.aimHeld);

    if (input.cursorHidden && input.fireHeld)
    {
        player.startFire();
    }
    else
    {
        player.stopFire();
    }

    if (input.reloadPressed)
    {
        player.reload();
    }

    if (input.cursorHidden)
    {
        Vector3 lookForward = player.getLookForward();
        lookForward.y = 0.0f;
        lookForward.Normalize();

        Vector3 lookRight = player.getLookRight();
        lookRight.y = 0.0f;
        lookRight.Normalize();

        Vector3 moveDir = lookForward * input.move.y + lookRight * input.move.x;

        const float aimYaw = XMConvertToRadians(player.getLookYaw());
        player.applyMovement(moveDir, aimYaw, deltaTime);

        if (input.jumpPressed)
        {
            player.jump();
        }
    }

    player.tick(deltaTime, weaponLookDeltaDeg);

    const Vector3 hitScanDirection = player.getLookForward();
    const Vector3 hitScanOrigin = player.getEyePosition();
    const Vector3 tracerStart = player.getMuzzlePosition();
    player.updateWeapon(deltaTime, hitScanOrigin, hitScanDirection, tracerStart, outIntents);
}
