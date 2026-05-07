#pragma once
#include "DeviceResources.h"

class ColorMaskEffect
{
public:
    ColorMaskEffect(DX::DeviceResources* deviceResources);
    ~ColorMaskEffect() = default;

    void createDeviceDependentResources();

    void createWindowSizeDependentResources();

    void finalize();

    void process(ID3D11ShaderResourceView* inputSRV,
        ID3D11RenderTargetView* outputRTV);

    void setColorMask(float r, float g, float b);
    float* getColorMaskPtr() { return &m_colorMask.x; }

    void disableRed()   { m_colorMask.x = 0.5f; }
    void disableGreen() { m_colorMask.y = 0.5f; }
    void disableBlue()  { m_colorMask.z = 0.5f; }
    void resetMask()    { m_colorMask = DirectX::SimpleMath::Vector3(1, 1, 1); }

private:
    DX::DeviceResources* m_deviceResources;

    // カラーマスク値（1,1,1 = フルカラー、0,0,0 = 黒）
    DirectX::SimpleMath::Vector3 m_colorMask;

    struct ColorMaskCB
    {
        DirectX::SimpleMath::Vector3 colorMask;
        float padding;
    };

    // GPUリソース
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
};
