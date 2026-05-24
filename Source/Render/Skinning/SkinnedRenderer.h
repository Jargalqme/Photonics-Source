#pragma once

#include "SkinnedModelData.h"

#include <SimpleMath.h>
#include <wrl/client.h>

class SkinnedModel;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;

// Self-contained renderer for skinned character meshes.
class SkinnedRenderer
{
public:
    bool initialize(ID3D11Device* device);
    void finalize();

    void draw(
        ID3D11DeviceContext* context,
        const SkinnedModel& model,
        const DirectX::SimpleMath::Matrix* palette,
        uint32_t paletteCount,
        const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector4& tintColor =
            DirectX::SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f),
        const DirectX::SimpleMath::Vector4& lightDirectionAndAmbient =
            DirectX::SimpleMath::Vector4(-0.35f, -0.85f, 0.35f, 0.28f));

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_ps;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_transformCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_paletteCB;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>      m_sampler;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rasterizer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthState;
};
