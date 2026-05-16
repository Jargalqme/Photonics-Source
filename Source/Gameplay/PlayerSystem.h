#pragma once

#include "Gameplay/PlayerInputState.h"

#include <vector>

class Player;
struct ShotIntent;

class PlayerSystem
{
public:
	PlayerSystem() = default;
	void update(Player& player, const PlayerInputState& input, std::vector<ShotIntent>& outIntents, float deltaTime);
};
