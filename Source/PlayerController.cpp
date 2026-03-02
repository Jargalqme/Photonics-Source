#include "pch.h"
#include "PlayerController.h"
#include "Player.h"
#include "Camera.h"
#include "InputManager.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

PlayerController::PlayerController(Player* player, Camera* camera)
	: m_player(player)
	, m_camera(camera)
{
}

void PlayerController::update(float deltaTime, InputManager* input)
{
	if (!m_player || m_camera->getMode() != CameraMode::Follow)
		return;

	// Don't process movement when cursor is visible (UI mode)
	if (input->isCursorVisible())
		return;

	// Get camera directions flattened to XZ plane
	Vector3 cameraForward = m_camera->getForward();
	cameraForward.y = 0.0f;
	cameraForward.Normalize();

	Vector3 cameraRight = m_camera->getRight();
	cameraRight.y = 0.0f;
	cameraRight.Normalize();

	// Build movement direction from WASD
	Vector3 moveDirection = Vector3::Zero;

	if (input->isKeyDown(Keyboard::Keys::W))
		moveDirection += cameraForward;
	if (input->isKeyDown(Keyboard::Keys::S))
		moveDirection -= cameraForward;
	if (input->isKeyDown(Keyboard::Keys::A))
		moveDirection -= cameraRight;
	if (input->isKeyDown(Keyboard::Keys::D))
		moveDirection += cameraRight;

	// Gamepad support
	Vector2 stick = input->getGamePadLeftStick();
	if (stick.LengthSquared() > 0.01f)
	{
		moveDirection += cameraForward * stick.y;
		moveDirection += cameraRight * stick.x;
	}

	// Movement and Facing
	bool isMoving = moveDirection.LengthSquared() > 0.001f;

	float aimYaw = XMConvertToRadians(m_camera->getYaw());

	if (isMoving)
	{
		// Face movement direction, not camera
		Vector3 dir = moveDirection;
		dir.Normalize();
		float moveYaw = atan2f(dir.x, dir.z);
		m_player->moveInDirection(moveDirection, moveYaw, deltaTime);
	}
	else
	{
		// Idle ? keep last rotation, don't snap to camera
		float currentYaw = m_player->getTransformPtr()->rotation.y;
		m_player->updateIdle(currentYaw, deltaTime);
	}

	// Jump
	if (input->isKeyPressed(Keyboard::Keys::Space))
	{
		m_player->jump();
	}
}