#pragma once

#include <string>

namespace RenderUtil
{
	// === 定数バッファ作成（DYNAMIC + CPU_WRITE） ===
	// 呼び出し例：m_cb = RenderUtil::createConstantBuffer<CBData>(device);
	template <typename T>
	Microsoft::WRL::ComPtr<ID3D11Buffer> createConstantBuffer(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth      = sizeof(T);
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, buffer.ReleaseAndGetAddressOf()));
		return buffer;
	}

	// === シェーダ読み込み（.cso 前提、コンパイル済みバイトコード） ===
	// outBlob は入力レイアウト作成用にバイトコードが必要な場合のみ渡す
	Microsoft::WRL::ComPtr<ID3D11VertexShader> loadVS(
		ID3D11Device* device,
		const std::wstring& filename,
		Microsoft::WRL::ComPtr<ID3DBlob>* outBlob = nullptr);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> loadPS(
		ID3D11Device* device,
		const std::wstring& filename);
}
