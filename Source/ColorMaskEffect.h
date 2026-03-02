#pragma once
#include "DeviceResources.h"

class ColorMaskEffect
{
public:
	ColorMaskEffect(DX::DeviceResources* deviceResources);
	~ColorMaskEffect() = default;

	void createDeviceDependentResources();      // Load shaders
	void createWindowSizeDependentResources();
	void onDeviceLost();						// Release GPU resources

	// Apply color mask to input texture, output to render target
	void process(ID3D11ShaderResourceView* inputSRV,
		ID3D11RenderTargetView* outputRTV);

	// Set mask directly
	void setColorMask(float r, float g, float b);

	// Get current mask (for ImGui)
	float* getColorMaskPtr() { return &m_colorMask.x; }

	// Convenience for core death
	void disableRed()   { m_colorMask.x = 0.5f; }
	void disableGreen() { m_colorMask.y = 0.5f; }
	void disableBlue()  { m_colorMask.z = 0.5f; }
	void resetMask()    { m_colorMask = DirectX::SimpleMath::Vector3(1, 1, 1); }

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