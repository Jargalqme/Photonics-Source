#pragma once

class Player;
class Camera;
class InputManager;

class PlayerController {
public:
	PlayerController(Player* player, Camera* camera);
	void update(float deltaTime, InputManager* input);

private:
	Player* m_player;
	Camera* m_camera;
};
