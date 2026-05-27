//---------------------------------------------------------------------------
//! @file   Camera.cpp
//! @brief  カメラ (ビュー行列・投影行列のみ)
//---------------------------------------------------------------------------
#include "pch.h"
#include "Camera.h"

//---------------------------------------------------------------------------
//! コンストラクタ
//---------------------------------------------------------------------------
Camera::Camera()
{
	// メンバ初期値から行列を構築しておく
	update();
}

//---------------------------------------------------------------------------
//! 回転を設定 (単位:度)
//---------------------------------------------------------------------------
void Camera::setRotation(float pitch, float yaw, float roll) noexcept
{
	m_pitch = pitch;
	m_yaw   = yaw;
	m_roll  = roll;
}

//---------------------------------------------------------------------------
//! 投影パラメータを設定 (FOV単位:度)
//---------------------------------------------------------------------------
void Camera::setProjection(float fovDegrees, float aspectRatio, float nearZ, float farZ) noexcept
{
	m_fovy        = XMConvertToRadians(fovDegrees);
	m_aspectRatio = aspectRatio;
	m_nearZ       = nearZ;
	m_farZ        = farZ;
}

//---------------------------------------------------------------------------
//! 更新 (現在の状態からビュー行列・投影行列を再構築)
//---------------------------------------------------------------------------
void Camera::update()
{
	updateDirectionVectors();
	updateViewMatrix();         // virtual ─ 継承先で上書きされる可能性あり
	updateProjectionMatrix();
}

//---------------------------------------------------------------------------
//! 方向ベクトルをピッチ・ヨー・ロールから再計算
//---------------------------------------------------------------------------
void Camera::updateDirectionVectors() noexcept
{
	const float pitchRad = XMConvertToRadians(m_pitch);
	const float yawRad   = XMConvertToRadians(m_yaw);
	const float rollRad  = XMConvertToRadians(m_roll);

	// ヨー・ピッチから前方ベクトルを生成
	m_forward.x =  sinf(yawRad) * cosf(pitchRad);
	m_forward.y = -sinf(pitchRad);
	m_forward.z =  cosf(yawRad) * cosf(pitchRad);
	m_forward.Normalize();

	// 右ベクトルと上ベクトルを外積で導出
	const Vector3 worldUp{ 0.0f, 1.0f, 0.0f };
	m_right = worldUp.Cross(m_forward);
	m_right.Normalize();

	m_up = m_forward.Cross(m_right);
	m_up.Normalize();

	// ロールがある場合のみ、前方ベクトルを軸に右と上を回転
	if(fabsf(m_roll) > 0.001f) {
		const Matrix rollMatrix = Matrix::CreateFromAxisAngle(m_forward, rollRad);
		m_right = Vector3::Transform(m_right, rollMatrix);
		m_up    = Vector3::Transform(m_up,    rollMatrix);
	}
}

//---------------------------------------------------------------------------
//! ビュー行列を再構築
//---------------------------------------------------------------------------
void Camera::updateViewMatrix() noexcept
{
	const Vector3 target = m_position + m_forward;
	m_matView = XMMatrixLookAtLH(m_position, target, m_up);
}

//---------------------------------------------------------------------------
//! 投影行列を再構築
//---------------------------------------------------------------------------
void Camera::updateProjectionMatrix() noexcept
{
	m_matProj = XMMatrixPerspectiveFovLH(m_fovy, m_aspectRatio, m_nearZ, m_farZ);
}
