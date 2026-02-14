#pragma once
#include "DeviceResources.h"

class ColorMaskEffect
{
public:
	ColorMaskEffect(DX::DeviceResources* deviceResources);
	~ColorMaskEffect() = default;

	void CreateDeviceDependentResources();      // Load shaders
	void CreateWindowSizeDependentResources();
	void OnDeviceLost();						// Release GPU resources

	// Apply color mask to input texture, output to render target
	void Process(ID3D11ShaderResourceView* inputSRV,
		ID3D11RenderTargetView* outputRTV);

	// Set mask directly
	void SetColorMask(float r, float g, float b);

	// Get current mask (for ImGui)
	float* GetColorMaskPtr() { return &m_colorMask.x; }

	// Convenience for core death
	void DisableRed()   { m_colorMask.x = 1.0f; }
	void DisableGreen() { m_colorMask.y = 1.0f; }
	void DisableBlue()  { m_colorMask.z = 1.0f; }
	void ResetMask()    { m_colorMask = DirectX::SimpleMath::Vector3(1, 1, 1); }

private:
	DX::DeviceResources* m_deviceResources;

	// Color mask value (1,1,1 = full, 0,0,0 = black)
	DirectX::SimpleMath::Vector3 m_colorMask;

	// Constant buffer structure
	struct ColorMaskCB
	{
		DirectX::SimpleMath::Vector3 colorMask;
		float padding;
	};

	// GPU resources
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
};