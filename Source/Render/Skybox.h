#pragma once
#include "DeviceResources.h"

class Skybox
{
public:
    Skybox(DX::DeviceResources* deviceResources);

    void initialize();

    void render(const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj);

    void finalize();

private:
    struct ConstantBuffer
    {
        DirectX::SimpleMath::Matrix inverseViewProjection;
    };

    DX::DeviceResources* m_deviceResources;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>       m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>        m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer>             m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeSRV;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>       m_sampler;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState>  m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>    m_rasterizerState;
};
