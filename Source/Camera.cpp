#include "pch.h"
#include "Camera.h"
#include "InputManager.h"

Camera::Camera()
	: m_position(0.0f, 0.0f, 5.0f)
	, m_forward(0.0f, 0.0f, -1.0f)
	, m_right(1.0f, 0.0f, 0.0f)
	, m_up(0.0f, 1.0f, 0.0f)
	, m_pitch(0.0f)
	, m_yaw(0.0f)
	, m_roll(0.0f)
	, m_fov(XMConvertToRadians(45.0f))
	, m_aspectRatio(16.0f / 9.0f)
	, m_nearPlane(0.1f)
	, m_farPlane(1000.0f)
	, m_normalFOV(XMConvertToRadians(45.0f))
	, m_zoomFOV(XMConvertToRadians(25.0f))
	, m_currentFOV(XMConvertToRadians(45.0f))
{
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void Camera::SetLookAt(const Vector3& target)
{
	m_forward = target - m_position;
	m_forward.Normalize();

	Vector3 worldUp(0.0f, 1.0f, 0.0f);
	m_right = m_forward.Cross(worldUp);
	m_right.Normalize();

	m_up = m_right.Cross(m_forward);
	m_up.Normalize();

	m_yaw = XMConvertToDegrees(atan2f(m_forward.x, m_forward.z));
	m_pitch = XMConvertToDegrees(asinf(-m_forward.y));
}

void Camera::SetRotation(float pitch, float yaw, float roll)
{
	m_pitch = pitch;
	m_yaw = yaw;
	m_roll = roll;
	UpdateDirectionVectors();
}

void Camera::MoveForward(float distance)
{
	m_position += m_forward * distance;
}

void Camera::MoveRight(float distance)
{
	m_position += m_right * distance;
}

void Camera::MoveUp(float distance)
{
	m_position += m_up * distance;
}

void Camera::AddPitch(float degrees)
{
	m_pitch += degrees;

	// Clamp pitch to avoid gimbal lock
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;

	UpdateDirectionVectors();
}

void Camera::AddYaw(float degrees)
{
	m_yaw += degrees;

	// Wrap yaw around
	if (m_yaw > 360.0f)
		m_yaw -= 360.0f;
	if (m_yaw < 0.0f)
		m_yaw += 360.0f;

	UpdateDirectionVectors();
}

void Camera::UpdateDirectionVectors()
{
	// Convert degrees to radians
	float pitchRad = XMConvertToRadians(m_pitch);
	float yawRad = XMConvertToRadians(m_yaw);
	float rollRad = XMConvertToRadians(m_roll);

	// Calculate forward vector
	m_forward.x = sinf(yawRad) * cosf(pitchRad);
	m_forward.y = -sinf(pitchRad);
	m_forward.z = -cosf(yawRad) * cosf(pitchRad);
	m_forward.Normalize();

	// Calculate right vector (assuming world up is Y)
	Vector3 worldUp(0.0f, 1.0f, 0.0f);
	m_right = m_forward.Cross(worldUp); // The cross product order matters! opposite order for left-handed
	m_right.Normalize();

	// Calculate up vector
	m_up = m_right.Cross(m_forward);
	m_up.Normalize();

	// Apply roll if needed
	if (fabs(m_roll) > 0.001f)
	{
		Matrix rollMatrix = Matrix::CreateFromAxisAngle(m_forward, rollRad);
		m_right = Vector3::Transform(m_right, rollMatrix);
		m_up = Vector3::Transform(m_up, rollMatrix);
	}
}

void Camera::UpdateViewMatrix()
{
	// Build camera's world matrix
	Matrix worldMatrix = Matrix::Identity;

	// Row 0: Camera's Right vector (local X axis)
	worldMatrix._11 = m_right.x;
	worldMatrix._12 = m_right.y;
	worldMatrix._13 = m_right.z;
	worldMatrix._14 = 0.0f;

	// Row 1: Camera's Up vector (local Y axis)
	worldMatrix._21 = m_up.x;
	worldMatrix._22 = m_up.y;
	worldMatrix._23 = m_up.z;
	worldMatrix._24 = 0.0f;

	// Row 2: Camera's Forward vector (local Z axis)
	worldMatrix._31 = -m_forward.x;
	worldMatrix._32 = -m_forward.y;
	worldMatrix._33 = -m_forward.z;
	worldMatrix._34 = 0.0f;

	// Row 3: Camera's Position (translation)
	Vector3 shakeOffset = GetShakeOffset();
	worldMatrix._41 = m_position.x + shakeOffset.x;
	worldMatrix._42 = m_position.y + shakeOffset.y;
	worldMatrix._43 = m_position.z + shakeOffset.z;
	worldMatrix._44 = 1.0f;

	// View matrix = inverse of world matrix
	m_viewMatrix = worldMatrix.Invert();
}

void Camera::SetProjectionParameters(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	m_normalFOV = XMConvertToRadians(fov);
	m_currentFOV = m_normalFOV;
	m_fov = m_normalFOV;
	m_aspectRatio = aspectRatio;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	UpdateProjectionMatrix();
}

void Camera::UpdateProjectionMatrix()
{
	m_projectionMatrix = Matrix::CreatePerspectiveFieldOfView(
		m_fov,
		m_aspectRatio, 
		m_nearPlane, 
		m_farPlane
	);
}

void Camera::Update(float deltaTime, InputManager* input)
{
	switch (m_mode) {
	case CameraMode::Free:
		UpdateFreeMode(deltaTime, input);
		break;
	case CameraMode::Follow:
		UpdateFollowMode(deltaTime, input);
		break;
	}
	UpdateShake(deltaTime);
	UpdateZoom(deltaTime);
	UpdateViewMatrix();
}

void Camera::SetMode(CameraMode mode)
{
	m_mode = mode;
	if (mode == CameraMode::Follow && m_followTargetPos)
	{
		m_smoothedPosition = *m_followTargetPos;
	}
}

const char* Camera::GetModeName() const
{
	switch (m_mode) {
	case CameraMode::Free:   return "Free Camera";
	case CameraMode::Follow: return "Follow Camera";
	default:                 return "Unknown";
	}
}

void Camera::SetFollowTarget(Vector3* position, float* rotation)
{
	m_followTargetPos = position;
	m_followTargetRot = rotation;

	if (m_followTargetPos)
	{
		m_smoothedPosition = *m_followTargetPos;
	}
}

void Camera::UpdateFreeMode(float deltaTime, InputManager* input)
{
	float speed = m_moveSpeed * deltaTime;

	// Sprint
	if (input->isKeyDown(Keyboard::Keys::LeftShift))
		speed *= 2.0f;

	// WASD movement
	if (input->isKeyDown(Keyboard::Keys::W))
		MoveForward(speed);
	if (input->isKeyDown(Keyboard::Keys::S))
		MoveForward(-speed);
	if (input->isKeyDown(Keyboard::Keys::A))
		MoveRight(-speed);
	if (input->isKeyDown(Keyboard::Keys::D))
		MoveRight(speed);

	// Vertical movement
	if (input->isKeyDown(Keyboard::Keys::Space))
		MoveUp(speed);
	if (input->isKeyDown(Keyboard::Keys::LeftControl))
		MoveUp(-speed);

	// Mouse look
	Vector2 look = input->getLookInput();
	AddYaw(look.x * m_mouseSensitivity);
	AddPitch(look.y * m_mouseSensitivity);
}

void Camera::UpdateFollowMode(float deltaTime, InputManager* input)
{
	if (!m_followTargetPos)
		return;

	// FPS-style mouse look
	Vector2 look = input->getLookInput();
	AddYaw(look.x * m_mouseSensitivity);
	AddPitch(look.y * m_mouseSensitivity);

	// Camera position: orbit around player
	Vector3 targetPos = *m_followTargetPos;

	float yawRad = XMConvertToRadians(m_yaw);
	float pitchRad = XMConvertToRadians(m_pitch);

	// Behind player offset (existing)
	Vector3 offset;
	offset.x = -sinf(yawRad) * cosf(pitchRad) * m_followDistance;
	offset.y = m_followHeight + sinf(pitchRad) * m_followDistance;
	offset.z = cosf(yawRad) * cosf(pitchRad) * m_followDistance;

	// ============================================
	// Right side offset
	// ============================================
	float shoulderOffset = 1.5f;  // Adjust this: higher = more to the right

	// Get camera's right direction (perpendicular to forward)
	Vector3 rightDir;
	rightDir.x = cosf(yawRad);
	rightDir.z = sinf(yawRad);
	rightDir.y = 0.0f;

	offset += rightDir * shoulderOffset;

	// Direct position - no smoothing
	SetPosition(targetPos + offset);
}

void Camera::TriggerShake(float intensity, float duration)
{
	// allow new shakes to add to existing ones
	if (m_shakeTimer < m_shakeDuration)
	{
		// add intensities together, but cap at reasonable maximum
		m_shakeIntensity = std::min(m_shakeIntensity + intensity * 0.5f, 30.0f);
		m_shakeDuration = std::max(m_shakeDuration, duration);
	}
	else
	{
		// no shake active, start fresh
		m_shakeIntensity = intensity;
		m_shakeDuration = duration;
		m_shakeTimer = 0.0f;
	}
}

void Camera::UpdateShake(float deltaTime)
{
	if (m_shakeTimer < m_shakeDuration)
	{
		m_shakeTimer += deltaTime;

		// exponential falloff
		float progress = m_shakeTimer / m_shakeDuration;
		float falloff = 1.0f - progress;
		falloff = falloff * falloff;

		m_currentShakeIntensity = m_shakeIntensity * falloff;
	}
	else
	{
		m_currentShakeIntensity = 0.0f;
		m_shakeIntensity = 0.0f;
	}
}

Vector3 Camera::GetShakeOffset() const
{
	if (m_currentShakeIntensity <= 0.01f)
		return Vector3::Zero;

	// Random offset based on intensity
	float x = ((rand() % 200 - 100) / 100.0f) * m_currentShakeIntensity * 0.1f;
	float y = ((rand() % 200 - 100) / 100.0f) * m_currentShakeIntensity * 0.1f;
	float z = ((rand() % 200 - 100) / 100.0f) * m_currentShakeIntensity * 0.05f;

	return Vector3(x, y, z);
}

void Camera::SetZoom(bool zooming)
{
	m_isZooming = zooming;
}

void Camera::UpdateZoom(float deltaTime)
{
	// target fov based on zoom state
	float targetFOV = m_isZooming ? m_zoomFOV : m_normalFOV;

	// smoothly interpolate
	if (m_currentFOV != targetFOV)
	{
		float diff = targetFOV - m_currentFOV;
		float step = m_zoomSpeed * deltaTime;

		if (abs(diff) < step)
		{
			m_currentFOV = targetFOV;
		}
		else
		{
			m_currentFOV += (diff > 0 ? step : -step);
		}

		// update projection with new FOV
		m_fov = m_currentFOV;
		UpdateProjectionMatrix();
	}
}
