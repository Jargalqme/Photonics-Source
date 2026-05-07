#include "pch.h"
#include "Billboard.h"
#include "WICTextureLoader.h"
#include "DeviceResources.h"
#include "Services/SceneContext.h"
#include "Render/ShaderCache.h"
#include "Render/RenderUtil.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

Billboard::Billboard(SceneContext& context)
	: m_context(&context)
{
}

// === 初期化・終了 ===

void Billboard::initialize(const std::wstring& texturePath)
{
	auto* device = m_context->device->GetD3DDevice();

	m_vertexShader = m_context->shaders->getVS(L"BillboardVS.cso");
	m_pixelShader  = m_context->shaders->getPS(L"BillboardPS.cso");

	m_constantBuffer = RenderUtil::createConstantBuffer<CBData>(device);

	DX::ThrowIfFailed(CreateWICTextureFromFile(
		device, texturePath.c_str(),
		nullptr, m_textureSRV.ReleaseAndGetAddressOf()));
}

void Billboard::finalize()
{
	m_vertexShader = nullptr;
	m_pixelShader  = nullptr;
	m_constantBuffer.Reset();
	m_textureSRV.Reset();
}

// === 描画 ===

void Billboard::render(
	const Matrix& view,
	const Matrix& projection,
	const Vector3& position,
	float size) const
{
	auto* context      = m_context->device->GetD3DDeviceContext();
	auto* commonStates = m_context->commonStates;

	// ビュー行列からカメラの右・上ベクトルを抽出
	Vector3 cameraRight(view._11, view._21, view._31);
	Vector3 cameraUp   (view._12, view._22, view._32);

	// 定数バッファ更新
	D3D11_MAPPED_SUBRESOURCE mapped;
	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	CBData* cb = reinterpret_cast<CBData*>(mapped.pData);
	cb->viewProjection = (view * projection).Transpose();
	cb->worldPosition  = position;
	cb->billboardSize  = size;
	cb->cameraRight    = cameraRight;
	cb->cameraUp       = cameraUp;
	context->Unmap(m_constantBuffer.Get(), 0);

	// パイプライン設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(nullptr);

	context->VSSetShader(m_vertexShader, nullptr, 0);
	ID3D11Buffer* constantBuffer = m_constantBuffer.Get();
	context->VSSetConstantBuffers(0, 1, &constantBuffer);

	context->PSSetShader(m_pixelShader, nullptr, 0);
	ID3D11ShaderResourceView* textureSRV = m_textureSRV.Get();
	context->PSSetShaderResources(0, 1, &textureSRV);

	auto* sampler = commonStates->LinearClamp();
	context->PSSetSamplers(0, 1, &sampler);

	float blendFactor[4] = { 0, 0, 0, 0 };
	context->OMSetBlendState(commonStates->Additive(), blendFactor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(commonStates->DepthRead(), 0);
	context->RSSetState(commonStates->CullNone());

	// クアッド描画（4頂点、トライアングルストリップ）
	context->Draw(4, 0);

	// デフォルトステートに復元
	context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(nullptr, 0);
	context->RSSetState(nullptr);
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->PSSetShaderResources(0, 1, &nullSRV);
}
